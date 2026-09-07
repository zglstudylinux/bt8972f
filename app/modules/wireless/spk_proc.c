#include "include.h"
#include "wireless.h"

#if WIRELESS_SPK_EN && !DEVICE_INTERPHONE_EN


#define SPK_ENC_BUFFER_SIZE             WIRELESS_SPK_FRAME_SIZE
#define SPK_DEC_BUFFER_SIZE            (WIRELESS_SPK_FRAME_SIZE)

#define SPK_DEC_OBUF_SIZE              (WIRELESS_SPK_SAMPLES_SELECT*WIRELESS_SPK_CHANNEL_SELECT*sizeof(mic_pcm_t))

void dac1_get_fifocnt(u32 tick_cnt);
void dac1_play_sync_fifocnt(u16 high_thr, u16 low_thr);
void wl_get_tick1_time(uint tick_cnt);
void wl_play_sync_tick1(bool from_dac, uint8_t interval);

typedef struct {
//    audio_callback_t callback;
    u8 frame[SPK_ENC_BUFFER_SIZE];
    u8 mute_en;
} spk_enc_t;

typedef struct {
    audio_callback_t callback;
    u8 obuf[SPK_DEC_OBUF_SIZE];
    u8 frame[SPK_DEC_BUFFER_SIZE];
    u8 last_bfi;
    u8 fifo_sta;
} spk_dec_t;

spk_enc_t spk_enc;
spk_dec_t spk_dec;

//------------------------------------------------------------------------------------------
#define spk_enc_adc_get_sync(n)         wl_get_tick1_time(n)                                                             //获取ADC采集偏差
#define spk_enc_adc_sync_proc()         wl_play_sync_tick1(false, WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_COMB_NB*2)       //ADC采集偏差处理

//获取DAC播放偏差
AT(.com_text.spk_dec)
static void spk_dec_dac_get_sync(uint tick_cnt)
{
#if DEVICE_DAC_OUTPUT_EN
    dac0_get_fifocnt(tick_cnt);
#endif
}

//DAC播放偏差处理
AT(.com_text.spk_dec)
static void spk_dec_dac_sync_proc(void)
{
#if DEVICE_DAC_OUTPUT_EN
    if(spk_dec.fifo_sta <= 200) {
        spk_dec.fifo_sta++;
        return;
    } else {
        uint high_thr, low_thr;
        uint samples = WIRELESS_SPK_DEC_MAX_US*48/1000 + 10;

#if WIRELESS_SPK_24B_PCM_EN
        high_thr = (samples + 6) * 2;
        low_thr = (samples - 6) * 2;
#else
        high_thr = samples + 6;
        low_thr = samples - 6;
#endif

        spk_dec.fifo_sta = 0;
        dac0_play_sync_fifocnt(high_thr, low_thr);
    }
#endif
}

//------------------------------------------------------------------------------------------
#if ADAPTER_USB_SPK_TX_EN
AT(.com_text.spk_enc)
void spk_enc_kick_cb(u8 idx)
{
}
#else
AT(.com_text.mic_enc)
void spk_enc_adc_dma_kick(uint tick_cnt)
{
    spk_enc_adc_get_sync(tick_cnt);

    if (wireless_cb.alg_en) {
//        GPIOESET = BIT(5);
        spk_dma_start();
//        GPIOECLR = BIT(5);
    }
}
#endif


AT(.com_text.spk_enc)
void spk_enc_audio_input(u8 *pcm, u32 samples, u32 params)
{
//    GPIOESET = BIT(4);
    if(wireless_cb.alg_en) {
        samples = WIRELESS_SPK_SAMPLES_SELECT;

        if(spk_enc.mute_en) {
            memset(pcm, 0x00, samples*sizeof(mic_pcm_t));
        }

#if  (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
        lc3s_enc((mic_pcm_t *)pcm, spk_enc.frame, samples);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
        lc3f_enc((mic_pcm_t *)pcm, spk_enc.frame, samples);
#endif
        wireless_a2d_put_tx_frame(0, spk_enc.frame, WIRELESS_SPK_FRAME_SIZE);
    }
//    GPIOECLR = BIT(4);

    spk_enc_adc_sync_proc();

//    //输出到下一级
//    if (spk_enc.callback) {
//        spk_enc.callback(ptr, samples, params);
//    }
}

AT(.com_text.spk_enc)
void spk_enc_audio_output_callback_set(audio_callback_t callback)
{
//    spk_enc.callback = callback;
}

AT(.text.spk_enc)
void spk_enc_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&spk_enc, 0x00, sizeof(spk_enc));

//    spk_enc.callback = NULL;
}

void spk_enc_reset(void)
{
}

//------------------------------------------------------------------------------------------
AT(.com_text.spk_dec)
void spk_dec_dac_fifo_get(void)
{
    spk_dec_dac_get_sync(0);
}

AT(.com_text.spk_dec)
void spk_dec_kick_cb(u8 idx)
{
    decoder_prio_trans_audio_input(NULL, WIRELESS_SPK_FRAME_SIZE, (idx<<16) | WIRELESS_MIC_PCM_MODE);
}

AT(.com_text.spk_dec)
static void spk_dec_pcm_out(void)
{
#if (WIRELESS_CON_LINK_NB == 1)
    //输出到下一级
    if (spk_dec.callback) {
        spk_dec.callback((void *)spk_dec.obuf, WIRELESS_SPK_SAMPLES_SELECT, WIRELESS_MIC_PCM_MODE);
    }
#else
#endif
}

AT(.com_text.spk_dec)
void spk_dec_audio_input(u8 *ptr, u32 samples, u32 params)
{
    mic_pcm_t *pcm = (mic_pcm_t *)spk_dec.obuf;
    samples = WIRELESS_MIC_SAMPLES_SELECT;

//    GPIOBSET = BIT(0);
    if(wireless_cb.alg_en) {
        u8 con_status = 1;//wireless_con_get_status();
        if(con_status) {
            bool bfi = wireless_a2d_get_rx_frame(spk_dec.frame, WIRELESS_SPK_FRAME_SIZE);

            if(!bfi) {
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
                lc3f_dec(spk_dec.frame, pcm, samples, bfi, 0);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
                lc3s_dec(spk_dec.frame, pcm, samples, bfi, 0);
#endif
            } else {
                memset(pcm, 0, SPK_DEC_OBUF_SIZE);
            }

#if WIRELESS_SPK_SINGLE_PLC_EN
#if WIRELESS_SPK_24B_PCM_EN
            plc_soft_process((s32 *)pcm, 240, bfi, 0);
#else
            plc_soft_v2_process((s16 *)pcm, bfi, 0, 0);
#endif
#endif

        } else {
            memset(pcm, 0x00, SPK_DEC_OBUF_SIZE);
        }

        spk_dec_pcm_out();

        //DAC调速，避免长时间后播放速度和发射端不匹配
        spk_dec_dac_sync_proc();
	}
//    GPIOBCLR = BIT(0);

//    //输出到下一级，放到spk_dec_dac_dma_kick处理了，这里不需要调用
//    if (spk_dec.callback) {
//        spk_dec.callback(obuf, WIRELESS_SPK_SAMPLES_SELECT, WIRELESS_SPK_CHANNEL_SELECT, 0);
//    }
}

void spk_dec_buf_clr(u8 idx)
{
    memset(spk_dec.obuf, 0, SPK_DEC_OBUF_SIZE);
}

AT(.com_text.spk_dec)
void spk_dec_audio_output_callback_set(audio_callback_t callback)
{
    spk_dec.callback = callback;
}

AT(.com_text.spk_dec.trans)
uint16_t spk_dec_us_trans_samples(uint16_t us, uint8_t spr_idx)
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

AT(.com_text.spk_dec)
void spk_dec_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&spk_dec, 0x00, sizeof(spk_dec));

    spk_dec.callback = NULL;
}

void spk_dec_reset(void)
{
    spk_dec.last_bfi = 0;
    memset(spk_dec.obuf, 0, SPK_DEC_OBUF_SIZE);
}
#endif
