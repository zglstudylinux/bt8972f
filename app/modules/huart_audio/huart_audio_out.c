#include "include.h"
#include "huart_audio_out.h"
/*
 * 文件名称: huart_audio_out.c
 * 功能描述: 本文件为HUART DMA 输出数字信号 处理模块
    开启本功能后，在线eq调试会失效哦~
 ****************************************************************************************
    code：
    buf:
    time :
 */
#if ADAPTER_HUART_OUTPUT_EN
#define FRAME_LEN                       WIRELESS_MIC_SAMPLES_SELECT
#define HUART_AUDIO_BAUD                4000000                         //haurt波特率
#define HUART_DATA_LEN                  FRAME_LEN*sizeof(mic_pcm_t)
#define HUART_OUT_CACHE_SIZE            HUART_DATA_LEN * 2

#define HUART_TX_PORT_DEF               HUART_TR_PB4                    //huart发射引脚
#define HUART_RX_PORT_DEF               HUART_TR_PB4                    //huart接收引脚


static huart_audio_out_cfg_t huart_audio_out_cfg AT(.buf.huart);
static u8 huart_buffer_tx[HUART_DATA_LEN * 2] AT(.buf.huart);
static u8 huart_buffer_tx_cache[HUART_DATA_LEN] AT(.buf.huart);
static ring_buf_t huart_ring_buf AT(.buf.huart);

static const rbuf_tbl_t huart_buf_tbl[1] = {
    {
        .buf = (u8 *)&huart_buffer_tx,
        .size = HUART_OUT_CACHE_SIZE,
    },
};

AT(.com_text.huart.input)
void huart_audio_out_input(u8 *ptr, u32 samples, u32 params)
{
    if(!huart_audio_out_cfg.mute){
        if ((samples * sizeof(mic_pcm_t)) != HUART_DATA_LEN) {
            if (ring_buf_get(&huart_ring_buf, huart_buffer_tx_cache, HUART_DATA_LEN)) {
                huart_wait_txdone();
                huart_tx(huart_buffer_tx_cache, HUART_DATA_LEN);
            }
            ring_buf_put(&huart_ring_buf, ptr, samples * sizeof(mic_pcm_t));
        } else {
            huart_wait_txdone();
            huart_tx(ptr, HUART_DATA_LEN);
        }

        if (huart_audio_out_cfg.callback) {
            huart_audio_out_cfg.callback(ptr, samples, params);
        }
    }
}

AT(.text.huart.callback)
void huart_audio_out_output_callback_set(audio_callback_t callback)
{
    huart_audio_out_cfg.callback = callback;
}

AT(.text.huart.mute)
void huart_audio_out_mute_set(uint8_t mute)
{
    huart_audio_out_cfg.mute = mute;
}

AT(.text.huart.init)
void huart_audio_out_init(u8 sample_rate, u16 samples, u8 channel)
{
    huart_t huart0;
    memset(&huart_audio_out_cfg, 0, sizeof(huart_audio_out_cfg));
    memset(&huart0, 0x00, sizeof(huart0));

    ring_buf_init(&huart_ring_buf, &huart_buf_tbl[0], 1, 0);

    huart0.txisr_en = 1;
    //如果在setting使能了HUART音频功能,则在setting配置文件决定是否进行HUART初始化
    if (xcfg_cb.huart_audio_out_en) {
        //HUART串口选择: PA7: 0, PB2: 1, PB3: 2, PE7: 3, PF0: 4, PA6: 5, PB1: 6, PB4: 7 PE6:8 PF1:9 VUSB:10
        if(xcfg_cb.huart_out_sel <= HUART_TR_VUSB) {
            huart0.rx_port = xcfg_cb.huart_in_sel;
            huart0.tx_port = xcfg_cb.huart_in_sel;
            huart_init(&huart0, HUART_AUDIO_BAUD);
        } else {
            //NONE
            printf("please select huart audio output pin....!!!!\n");
            huart_audio_out_mute_set(1);
        }
    } else {
        huart0.rx_port = HUART_RX_PORT_DEF;
        huart0.tx_port = HUART_TX_PORT_DEF;
        huart_init(&huart0, HUART_AUDIO_BAUD);
    }
    printf("%s enable = %d,io = %d,out_len = %d\n",__func__,xcfg_cb.huart_audio_out_en,xcfg_cb.huart_out_sel,HUART_DATA_LEN);
}

AT(.text.huart.exit)
void huart_audio_out_exit(void)
{

}
#endif

