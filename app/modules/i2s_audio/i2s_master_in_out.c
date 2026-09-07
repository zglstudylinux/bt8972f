#include "include.h"
#include "i2s_audio.h"

#if ADAPTER_I2S_IN_OUT_EN
typedef struct {
    audio_callback_t callback;
    i2s_cfg_t i2s_cfg;
} i2s_in_and_out_ctl_t;

typedef struct {
    uint8_t i2s_rx_cache[I2S_INCACHE_SIZE];

    ring_buf_t i2s_rx_ring_buf;
} i2s_audio_buf_t;
//iis tx&rxbuf cbuf
static i2s_audio_buf_t i2s_audio_buf AT(.i2s_buf.dma);
//iis控制
static i2s_in_and_out_ctl_t i2s_in_and_out_ctl;


const rbuf_tbl_t i2s_rx_buf_tbl[1] = {
    {
        .buf = i2s_audio_buf.i2s_rx_cache,
        .size = I2S_INCACHE_SIZE,
    },
};
//初始化i2s_audio_buf，清0，绑定cbuf
void i2s_audio_buf_init(){
    memset(i2s_audio_buf.i2s_rx_cache, 0, sizeof(i2s_audio_buf.i2s_rx_cache));
    ring_buf_init(&(i2s_audio_buf.i2s_rx_ring_buf), i2s_rx_buf_tbl, 1, 0);
}

AT(.com_text.i2s_in_out_proc)
void i2s_tx_process(void *buf, u32 samples, bool iis_32bit)
{
}

//I2S接收中断回调函数,缓存和处理接收数据
AT(.com_text.i2s_in_out_proc)
void i2s_rx_process(void *buf, u32 samples, bool i2s_32bit)
{
    u32 *ptr32 = (u32*)buf;
    u16 *ptr16 = (u16*)buf;

    if (samples > I2S_DMA_SAMPLES) {
        samples = I2S_DMA_SAMPLES;//回调函数传来的参数IISDMACNT太大，不是120，与初始化设置的IISDMACNT=240不符合，这里赋值为I2S_DMA_SAMPLES
    }

    for(int j = 0; j < samples; j++) {                //Stereo --> Mono
#if WIRELESS_MIC_24B_PCM_EN
        ptr32[j] = ptr32[2*j];
#else
        ptr16[j] = ptr16[2*j];
#endif // WIRELESS_MIC_24B_PCM_EN
    }

    ring_buf_t *ring_buf = &(i2s_audio_buf.i2s_rx_ring_buf);
    u8 ch_mode = 1;
    ring_buf_put(ring_buf, (u8 *)buf, samples * 4 * ch_mode);
}

#if I2S_BIT_MODE == I2S_32BIT
AT(.com_text.i2s_mic_out)
void i2s_fram_16_to_32(u8 *obuf, u8 *ibuf, u32 samples, int ch_mode)
{
    s32 *ptr = (s32 *)obuf;
    s16 *ptr16 = (s16 *)ibuf;
    s32 *ptr32 = (s32 *)ibuf;
    if(ch_mode == 1) {                                      //TX Mono
        for (int i = 0; i< samples; i++) {                  //16->32位扩展
#if WIRELESS_MIC_24B_PCM_EN
            ptr[2 * i] = (s32)(ptr32[i]);
            ptr[2 * i + 1] = (s32)(ptr32[i]);
#else
            ptr[2 * i] = (s32)(ptr16[i] << 16);
            ptr[2 * i + 1] = (s32)(ptr16[i] << 16);
#endif // WIRELESS_MIC_24B_PCM_EN
        }
    } else {                                                //TX Stero
        for (int i = 0; i< samples; i++) {
#if WIRELESS_MIC_24B_PCM_EN
            ptr[2 * i] = (s32)(ptr32[2 * i]);
            ptr[2 * i + 1] = (s32)(ptr32[2 * i + 1]);
#else
            ptr[2 * i] = (s32)(ptr16[2 * i] << 16);      //16->32位扩展
            ptr[2 * i + 1] = (s32)(ptr16[2 * i + 1] << 16);
#endif // WIRELESS_MIC_24B_PCM_EN
        }
    }
}
#endif

AT(.com_text.i2s_mic_out)WEAK
void i2s_audio_in_out_input(u8 *ptr, u32 samples, u32 params)
{
    u32 pcm_mode = params & 0xffff;
    uint ch_mode = pcm_mode & PCM_CHMASK;
    u8 *obuf = (u8 *)i2s_out_get_obuf(0);
    //一拖二时,MIC数据是分片解码下来的,这里要存够完整的一帧再放到I2S_DMA_BUF里面去传输
    if (samples == 94) {
        i2s_fram_16_to_32(obuf, ptr, samples, ch_mode);
    } else if (samples == 146) {
#if WIRELESS_MIC_24B_PCM_EN
        //24b是4字节存一个样点,iis_dma_buf存双声道
        i2s_fram_16_to_32(obuf + (94 * 4 * 2), ptr, samples, ch_mode);
#else
        //16b是2字节存一个样点,iis_dma_buf存双声道
        i2s_fram_16_to_32(obuf + (94 * 2 * 2), ptr, samples, ch_mode);
#endif // WIRELESS_MIC_24B_PCM_EN
    }

    ring_buf_t *ring_buf = &(i2s_audio_buf.i2s_rx_ring_buf);
    ring_buf = &(i2s_audio_buf.i2s_rx_ring_buf);
#if WIRELESS_MIC_24B_PCM_EN
    ring_buf_get(ring_buf,(u8 *)ptr, samples * 4 * ch_mode);
#else
    ring_buf_get(ring_buf,(u8 *)ptr, samples * 2 * ch_mode);
#endif

    if(i2s_in_and_out_ctl.callback != NULL) {
        i2s_in_and_out_ctl.callback((u8 *)ptr, samples, params);//把IIS收到的数据发给下一层
    }
}

AT(.text.i2s_mic_out)WEAK
void i2s_audio_in_out_output_callback_set(audio_callback_t callback)
{
    i2s_in_and_out_ctl.callback = callback;
}

AT(.text.i2s_mic_out)WEAK
void i2s_audio_in_out_init(u8 sample_rate, u16 samples, u8 channel)
{
    i2s_cfg_t *p_cfg = &i2s_in_and_out_ctl.i2s_cfg;
    memset(&i2s_in_and_out_ctl, 0, sizeof(i2s_in_and_out_ctl));
    i2s_audio_buf_init();

    p_cfg->mode        = I2S_MASTER_DMATX_DMARX;//I2S_MASTER_DMATX_DMARX;I2S_MASTER_DMARX; I2S_MASTER_DMATX; I2S_SLAVE_DMATX_DMARX
    p_cfg->iomap       = I2S_MAPPING_SEL;
    p_cfg->bit_mode    = I2S_BIT_MODE;
    p_cfg->data_mode   = I2S_DATA_MODE;
    p_cfg->mclk_sel    = I2S_MCLK_SEL;
    p_cfg->mclk_out_en = I2S_MCLK_EN;
    if (I2S_DMA_EN) {
        p_cfg->dma_cfg.samples = I2S_DMA_SAMPLES;
        p_cfg->dma_cfg.isr_rx_callback = i2s_rx_process;
        p_cfg->dma_cfg.isr_tx_callback = i2s_tx_process;
    }

    i2s_init(p_cfg);            //I2S初始化
}
#endif

