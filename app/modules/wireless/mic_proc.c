#include "include.h"
#include "wireless.h"

#if WIRELESS_MIC_EN


#define MIC_ENC_BUFFER_SIZE         WIRELESS_MIC_FRAME_SIZE
#define MIC_DEC_BUFFER_SIZE         WIRELESS_MIC_FRAME_SIZE

#define MIC_DEC_OBUF_SIZE           (WIRELESS_MIC_SAMPLES_SELECT*WIRELESS_MIC_CHANNEL_SELECT*sizeof(mic_pcm_t))

void dac1_get_fifocnt(u32 tick_cnt);
void dac1_play_sync_fifocnt(u16 high_thr, u16 low_thr);
void wl_get_tick1_time(uint tick_cnt);
void wl_play_sync_tick1(bool from_dac, uint8_t interval);

typedef struct {
//    audio_callback_t callback;
    u8 frame[MIC_ENC_BUFFER_SIZE];
    u8 mute_en;
} mic_enc_t;

typedef struct {
    audio_callback_t callback;
    u8 obuf[MIC_DEC_OBUF_SIZE];
    struct {
        u8 buf[MIC_DEC_OBUF_SIZE];
        u16 buf_cnt;
    } pcm[WIRELESS_CON_LINK_NB];

    u8 frame[MIC_DEC_BUFFER_SIZE];
    u8 last_bfi[WIRELESS_CON_LINK_NB];
    u8 frag_samples[2];
    bool frag0_done_flag;
    u8 sync_idx;
    u8 fifo_sta;
} mic_dec_t;


static mic_enc_t mic_enc AT(.btmem.wireless.enc);
static mic_dec_t mic_dec AT(.btmem.wireless.dec);


//------------------------------------------------------------------------------------------
#define mic_enc_adc_get_sync(n)         wl_get_tick1_time(n)                                                             //获取ADC采集偏差
#define mic_enc_adc_sync_proc()         wl_play_sync_tick1(false, WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_COMB_NB*2)       //ADC采集偏差处理

//获取DAC播放偏差
AT(.text.adapter.mic_dec)
static void mic_dec_dac_get_sync(uint tick_cnt)
{
#if ADAPTER_DAC_OUTPUT_EN
    dac0_get_fifocnt(tick_cnt);
#endif
}

//DAC播放偏差处理
AT(.text.adapter.mic_dec)
static void mic_dec_dac_sync_proc(void)
{
#if ADAPTER_DAC_OUTPUT_EN
    if(mic_dec.fifo_sta <= 200) {
        mic_dec.fifo_sta++;
        return;
    } else {
        uint high_thr, low_thr;
        uint samples = WIRELESS_MIC_DEC_MAX_US*48/1000 + 10;

#if WIRELESS_MIC_24B_PCM_EN
        high_thr = (samples + 6) * 2;
        low_thr = (samples - 6) * 2;
#else
        high_thr = samples + 6;
        low_thr = samples - 6;
#endif

        mic_dec.fifo_sta = 0;
        dac0_play_sync_fifocnt(high_thr, low_thr);
    }
#endif
}

//------------------------------------------------------------------------------------------
AT(.com_text.mic_enc)
void mic_enc_adc_dma_kick(uint tick_cnt)
{
    mic_enc_adc_get_sync(tick_cnt);

    if (wireless_cb.alg_en) {
//        GPIOESET = BIT(5);
        mic_dma_start();
//        GPIOECLR = BIT(5);
    }
}

AT(.com_text.mic_enc)
void mic_mute_set(u8 mute)
{
    mic_enc.mute_en = mute;
}

AT(.com_text.mic_enc)
void mic_enc_audio_input(u8 *pcm, u32 samples, u32 params)
{
//    GPIOESET = BIT(4);
    if(wireless_cb.alg_en) {
        samples = WIRELESS_MIC_SAMPLES_SELECT;

//        //发射1k正弦波，测试用
//        extern const u8 test_sin_1k[];
//        u8 *ptr = pcm;
//        for(uint i=0; i<WIRELESS_MIC_SAMPLES_SELECT/48; i++) {
//            memcpy(ptr, test_sin_1k, 48*sizeof(mic_pcm_t));
//            ptr += 48*sizeof(mic_pcm_t);
//        }

        if(mic_enc.mute_en) {
            memset(pcm, 0x00, samples*sizeof(mic_pcm_t));
        }

#if DEVICE_MIC_FADE_IN_EN
        soft_gain_proc((void *)pcm, samples);
#endif

#if  (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
        lc3s_enc((mic_pcm_t *)pcm, mic_enc.frame, samples);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
        lc3f_enc((mic_pcm_t *)pcm, mic_enc.frame, samples);
#endif
        wireless_d2a_put_tx_frame(mic_enc.frame, WIRELESS_MIC_FRAME_SIZE);
    }
//    GPIOECLR = BIT(4);

    mic_enc_adc_sync_proc();

//    //输出到下一级
//    if (mic_enc.callback) {
//        mic_enc.callback(ptr, WIRELESS_MIC_SAMPLES_SELECT, params);
//    }
}

AT(.text.mic_enc)
void mic_enc_audio_output_callback_set(audio_callback_t callback)
{
//    mic_enc.callback = callback;
}

AT(.text.mic_enc)
void mic_enc_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&mic_enc, 0x00, sizeof(mic_enc));

//    mic_enc.callback = NULL;
}

void mic_enc_reset(void)
{
}

//------------------------------------------------------------------------------------------
AT(.com_text.mic_enc)
void mic_dec_dac_fifo_get(u8 idx)
{
    mic_dec.sync_idx = idx;
    mic_dec_dac_get_sync(0);
}

AT(.com_text.mic_enc)
void mic_dec_kick_cb(u8 idx)
{
    decoder_prio_trans_audio_input(NULL, WIRELESS_MIC_FRAME_SIZE, (idx<<16) | WIRELESS_MIC_PCM_MODE);
}

#if WIRELESS_CON_COMB_BUF_EN
void mic_dec_pcm_out(u8 idx)
{
    mic_pcm_t *obuf = (mic_pcm_t *)mic_dec.obuf;
    u8 samples = WIRELESS_MIC_SAMPLES_SELECT/2;

    mic_dec.pcm[idx].buf_cnt = 0;                   //复位pcm[idx].buf_cnt，确保先copy前半段pcm[idx].buf

    mic_pcm_t *pcm0 = (mic_pcm_t *)(mic_dec.pcm[0].buf + mic_dec.pcm[0].buf_cnt);
    mic_pcm_t *pcm1 = (mic_pcm_t *)(mic_dec.pcm[1].buf + mic_dec.pcm[1].buf_cnt);

#if ADAPTER_MIX_DRC_EN
     mix_drc_audio_input(pcm0, pcm1, obuf, samples);
#else
    for(uint i=0; i<samples; i++) {
        s32 tmp = pcm0[i] + pcm1[i];
        if(tmp > PCM_MAX_V) {
            tmp = PCM_MAX_V;
        } else if(tmp < PCM_MIN_V) {
            tmp = PCM_MIN_V;
        }
        obuf[i] = tmp;
    }
#endif

    mic_dec.pcm[0].buf_cnt += MIC_DEC_OBUF_SIZE/2;
    if(mic_dec.pcm[0].buf_cnt >= MIC_DEC_OBUF_SIZE) {
        mic_dec.pcm[0].buf_cnt = 0;
    }
    mic_dec.pcm[1].buf_cnt += MIC_DEC_OBUF_SIZE/2;
    if(mic_dec.pcm[1].buf_cnt >= MIC_DEC_OBUF_SIZE) {
        mic_dec.pcm[1].buf_cnt = 0;
    }

    //输出到下一级
    if (mic_dec.callback) {
        mic_dec.callback((void *)obuf, samples, WIRELESS_MIC_PCM_MODE);
    }
}
#else // WIRELESS_CON_COMB_BUF_EN

#if (WIRELESS_CON_LINK_NB > 1)
AT(.text.adapter.mic_dec)
static void mic_dec_pcm_out_samples(mic_pcm_t *pcm0, mic_pcm_t *pcm1, uint8_t samples, uint8_t adj_dac_flag)
{
    mic_pcm_t *obuf = (void *)mic_dec.obuf;

#if ADAPTER_USB_MIC_RX_EN
    usb_mic_in_audio_input_stereo((u8 *)pcm0, (u8 *)pcm1, samples);
#endif

#if ADAPTER_MIX_DRC_EN
     mix_drc_audio_input(pcm0, pcm1, obuf, samples);
#else
    for(uint i=0; i<samples; i++) {
        s32 tmp = pcm0[i] + pcm1[i];
        if(tmp > PCM_MAX_V) {
            tmp = PCM_MAX_V;
        } else if(tmp < PCM_MIN_V) {
            tmp = PCM_MIN_V;
        }
        obuf[i] = tmp;
    }
#endif

    //输出到下一级
    if (mic_dec.callback) {
        mic_dec.callback((void *)obuf, samples, WIRELESS_MIC_PCM_MODE);
    }
}

AT(.text.adapter.mic_dec)
static void mic_dec_pcm_out_frag0(u8 dec_flag)
{
    uint8_t frag0_samples = mic_dec.frag_samples[0];

    mic_pcm_t *pcm0 = (mic_pcm_t *)(&mic_dec.pcm[0].buf[0]);
    mic_pcm_t *pcm1 = (mic_pcm_t *)(&mic_dec.pcm[1].buf[0]) + (WIRELESS_MIC_SAMPLES_SELECT-frag0_samples);

    mic_dec_pcm_out_samples(pcm0, pcm1, frag0_samples, dec_flag);
}

//format pcm / upsample / mix
AT(.text.adapter.mic_dec)
static void mic_dec_pcm_out_frag1(u8 dec_flag)
{
    uint8_t frag0_samples = mic_dec.frag_samples[0];
    uint8_t frag1_samples = mic_dec.frag_samples[1];

    mic_pcm_t *pcm0 = (mic_pcm_t *)(&mic_dec.pcm[0].buf[0]) + frag0_samples;
    mic_pcm_t *pcm1 = (mic_pcm_t *)(&mic_dec.pcm[1].buf[0]);

    mic_dec_pcm_out_samples(pcm0, pcm1, frag1_samples, dec_flag);
}
#endif

AT(.text.adapter.mic_dec)
static void mic_dec_pcm_out(u8 idx)
{
#if (WIRELESS_CON_LINK_NB == 1)
    //输出到下一级
    if (mic_dec.callback) {
        mic_dec.callback((void *)(&mic_dec.pcm[idx].buf[0]), WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_PCM_MODE);
    }
#else
    u8 con_status;
    u8 dec_flag = 0;

    if(idx == 0) {
        if (mic_dec.frag0_done_flag == 0) {
            //推通道1前半帧+通道2后半帧
            mic_dec_pcm_out_frag0(dec_flag);
            mic_dec.frag0_done_flag = 1;
        }

        //若通道2未连接，把frag1也转换了
        con_status = wireless_con_get_status();
        if((con_status & ~BIT(idx)) == 0) {
            mic_dec_pcm_out_frag1(0);
            mic_dec.frag0_done_flag = 0;
        }
    } else {
        //通道2解码完，再转换frag1，输出到OBUF后半段
        if(mic_dec.frag0_done_flag) {
            mic_dec_pcm_out_frag1(dec_flag);
            mic_dec.frag0_done_flag = 0;
        }

        //若通道1未连接，把frag0也转换了
        con_status = wireless_con_get_status();
        if((con_status & ~BIT(idx)) == 0) {
            mic_dec_pcm_out_frag0(0);
            mic_dec.frag0_done_flag = 1;
        }
    }
#endif
}
#endif // WIRELESS_CON_COMB_BUF_EN

AT(.text.adapter.mic_dec)
void mic_dec_audio_input(u8 *ptr, u32 samples, u32 params)
{
    u8 idx = (params >> 16);
//    u32 pcm_mode = params & 0xffff;

    mic_pcm_t *pcm = (mic_pcm_t *)&mic_dec.pcm[idx].buf;
    samples = WIRELESS_MIC_SAMPLES_SELECT;

//    GPIOESET = BIT(4);
    if(wireless_cb.alg_en) {
        u8 con_status = wireless_con_get_status();
        if(con_status & BIT(idx)) {
            bool bfi = wireless_d2a_get_rx_frame(idx, mic_dec.frame, WIRELESS_MIC_FRAME_SIZE);

            if(!bfi) {
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
                lc3f_dec(mic_dec.frame, pcm, samples, bfi, idx);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
                lc3s_dec(mic_dec.frame, pcm, samples, bfi, idx);
#endif
            } else {
                memset(pcm, 0, MIC_DEC_OBUF_SIZE);
            }

#if WIRELESS_MIC_SINGLE_PLC_EN
#if WIRELESS_MIC_24B_PCM_EN
            plc_soft_process((s32 *)pcm, 240, bfi, idx);
#else
            plc_soft_v2_process((s16 *)pcm, bfi, idx, 0);
#endif
#endif
#if WIRELESS_CON_PWR_CTR
            ws_pwr_ctr_rx_process(idx, bfi);
#endif
        } else {
            //断连时跑到mic_dec_reset没那么及时，这里清buf避免出现杂波
            memset(pcm, 0, MIC_DEC_OBUF_SIZE);
        }

        mic_dec_pcm_out(idx);

        if(idx == mic_dec.sync_idx) {
#if ADAPTER_USB_MIC_RX_EN
            //usb调速，避免长时间后卡顿
            usb_mic_in_src_adj();
#endif
            //DAC调速，避免长时间后播放速度和发射端不匹配
            mic_dec_dac_sync_proc();
        }
    } else {
//        memset(pcm, 0x00, samples*sizeof(mic_pcm_t));
    }
//    GPIOECLR = BIT(4);
}

AT(.text.adapter.mic_dec)
void mic_dec_buf_clr(u8 idx)
{
    memset(mic_dec.pcm[idx].buf, 0x00, MIC_DEC_OBUF_SIZE);
}

AT(.text.mic_dec)
void mic_dec_audio_output_callback_set(audio_callback_t callback)
{
    mic_dec.callback = callback;
}

static uint16_t mic_dec_us_trans_samples(uint16_t us, uint8_t spr_idx)
{
    uint16_t samples_duration = 0;

    switch(spr_idx) {
        ///对应48k采样率的情况，一个样点对应20.8us
        case SAMPLE_RATE_48K:
            samples_duration = 21;
            break;

        ///对应16k采样率的情况，一个样点对应62.5us
        case SAMPLE_RATE_16K:
            samples_duration = 62;
            break;

        default:
            samples_duration = 21;
            break;
    }

    return (us/samples_duration);
}

AT(.text.mic_dec)
void mic_dec_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&mic_dec, 0x00, sizeof(mic_dec));

#if WIRELESS_CON_LINK_NB > 1 && !WIRELESS_CON_COMB_BUF_EN
    mic_dec.frag_samples[0] = mic_dec_us_trans_samples(wl_single_link_duration_get(cfg_le_conn_vers()), WIRELESS_MIC_SAMPLE_RATE_SELECT)%WIRELESS_MIC_SAMPLES_SELECT;
    mic_dec.frag_samples[1] = WIRELESS_MIC_SAMPLES_SELECT - mic_dec.frag_samples[0];

    printf("frag_samples=%d, %d\n", mic_dec.frag_samples[0], mic_dec.frag_samples[1]);
#endif

//    mic_dec.callback = NULL;
}

void mic_dec_reset(u8 idx)
{
    mic_dec.last_bfi[idx] = 0;
    memset(mic_dec.pcm[idx].buf, 0, MIC_DEC_OBUF_SIZE);
}


#endif
