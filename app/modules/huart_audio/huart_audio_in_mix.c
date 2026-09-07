#include "include.h"
#include "huart_audio_in_mix.h"
/*
 * 文件名称: huart_audio_in_mix.c
 * 功能描述: 本文件为HUART DMA 输出数字信号 处理模块
    开启本功能后，在线eq调试会失效哦~
 ****************************************************************************************
    code：
    buf:
    time :
 */
#if ADAPTER_HUART_INPUT_EN
#define FRAME_LEN                       WIRELESS_MIC_SAMPLES_SELECT
#define HUART_DATA_LEN                  FRAME_LEN * sizeof(mic_pcm_t)
#define HUART_IN_CACHE_SIZE             HUART_DATA_LEN * 2

#define HUART_IN_INFO_PRINT             0
#define HUART_AUDIO_BAUD                4000000          //haurt波特率
#define HUART_TX_PORT_DEF               HUART_TR_PB4     //huart发射引脚
#define HUART_RX_PORT_DEF               HUART_TR_PB4     //huart接收引脚

static huart_audio_in_cfg_t huart_audio_in_cfg AT(.buf.huart);
static u8 huart_audio_in_buffer[HUART_IN_CACHE_SIZE] AT(.buf.huart);
static u8 huart_audio_in_cache[HUART_DATA_LEN] AT(.buf.huart);
static u8 huart_audio_process_cache[HUART_DATA_LEN] AT(.buf.huart);
static ring_buf_t huart_ring_buf AT(.buf.huart);

#if HUART_IN_INFO_PRINT
AT(.com_text.haurt_info)
const char huart_in_info[] = "HUART_IN samples = %d, isrcnt = %d (SR_%d)\n";
AT(.com_text.huart_info_1)
const char huart_in_info1[] = "HUART_IN size = %d\n";
#endif

static const rbuf_tbl_t huart_buf_tbl[1] = {
    {
        .buf = (u8 *)&huart_audio_in_buffer,
        .size = HUART_IN_CACHE_SIZE,
    },
};


AT(.com_text.huart_in_proc.input)
void huart_audio_in_irq_cb(u8 *ptr, u32 samples, int ch_mode)
{
#if HUART_IN_INFO_PRINT
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if (tick_check_expire(ticks,1000)) {
        my_printf(huart_in_info, samples, isr_cnt, samples*isr_cnt);
        isr_cnt = 0;
        ticks = tick_get();
    }
#endif

    if (huart_audio_in_cfg.mute) {
        memset(ptr, 0, HUART_DATA_LEN);
        return;
    }

    if (wireless_cb.connected_sta) {
        ring_buf_put(&huart_ring_buf, ptr, samples * sizeof(mic_pcm_t));
    } else {
        if (huart_audio_in_cfg.callback) {
            huart_audio_in_cfg.callback(ptr, samples, WIRELESS_MIC_PCM_MODE);
        }
    }
}

AT(.com_huart.text)
void huart_rx_done_cb(void)
{
    huart_audio_in_irq_cb(huart_audio_in_cache, FRAME_LEN, 1);
}

AT(.text.huart.input)
void huart_audio_in_input(u8 *ptr, u32 samples, u32 params)
{
    uint frame_size = samples * sizeof(mic_pcm_t);

    if (ring_buf_get(&huart_ring_buf, huart_audio_process_cache, frame_size)) {
        mic_pcm_t *pcm_huart = (mic_pcm_t *)huart_audio_process_cache;
        mic_pcm_t *pcm_mic = (mic_pcm_t *)ptr;
        s32 sample = 0;
        for (u8 i = 0; i < samples; i++) {
            sample = pcm_mic[i] + pcm_huart[i];
            if(sample > PCM_MAX_V) {
                pcm_mic[i] = PCM_MAX_V;
            } else if(sample < PCM_MIN_V){
                pcm_mic[i] = PCM_MIN_V;
            } else {
                pcm_mic[i] = sample;
            }
        }
    }

    if (huart_audio_in_cfg.callback) {
        huart_audio_in_cfg.callback((u8 *)ptr, samples, params);
    }
}

AT(.text.huart.callback)
void huart_audio_in_output_callback_set(audio_callback_t callback)
{
    huart_audio_in_cfg.callback = callback;
}

AT(.text.huart.mute)
void huart_audio_in_mute_set(uint8_t mute)
{
    huart_audio_in_cfg.mute = mute;
}

AT(.text.huart.init)
void huart_audio_in_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&huart_audio_in_cfg,0,sizeof(huart_audio_in_cfg));
    ring_buf_init(&huart_ring_buf, &huart_buf_tbl[0], 1, 0);
    huart_t huart0;
    memset(&huart0, 0x00, sizeof(huart0));
    huart0.rxbuf      = huart_audio_in_cache;
    huart0.rxbuf_size = HUART_DATA_LEN;
    huart0.rxisr_en = 1;
    huart0.txisr_en = 1;
    //如果在setting使能了HUART音频功能,则在setting配置文件决定是否进行HUART初始化
    if (xcfg_cb.huart_audio_in_en) {
        //HUART串口选择: PA7: 0, PB2: 1, PB3: 2, PE7: 3, PF0: 4, PA6: 5, PB1: 6, PB4: 7 PE6:8 PF1:9 VUSB:10
        if(xcfg_cb.huart_in_sel <= HUART_TR_VUSB) {
            huart0.rx_port = xcfg_cb.huart_in_sel;
            huart0.tx_port = xcfg_cb.huart_in_sel;
            huart_init(&huart0, HUART_AUDIO_BAUD);
        } else {
            //NONE
            printf("please select huart audio intput pin....!!!!\n");
            huart_audio_in_mute_set(1);
        }
    } else {
        huart0.rx_port = HUART_RX_PORT_DEF;
        huart0.tx_port = HUART_TX_PORT_DEF;
        huart_init(&huart0, HUART_AUDIO_BAUD);
    }
    printf("%s enable:%d,io = %d\n",__func__,xcfg_cb.huart_audio_in_en,xcfg_cb.huart_in_sel);
}
#endif

