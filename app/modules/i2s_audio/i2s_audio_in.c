#include "include.h"
#include "i2s_audio.h"

#if I2S_AUDIO_IN_EN
typedef struct {
    audio_callback_t callback;
    i2s_cfg_t i2s_cfg;
} i2s_in_ctl_t;

typedef struct {
    uint8_t i2s_rx_cache[I2S_INCACHE_SIZE];

    ring_buf_t i2s_rx_ring_buf;
} i2s_audio_buf_t;

static i2s_audio_buf_t i2s_audio_buf AT(.i2s_buf);
static i2s_in_ctl_t i2s_in_ctl;

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
//I2S接收中断回调函数,缓存和处理接收数据
AT(.com_text.i2s_audio_in)
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

AT(.com_text.i2s_audio_in)
void i2s_audio_in_input(u8 *ptr, u32 samples, u32 params)
{
    u32 pcm_mode = params & 0xffff;
    uint ch_mode = pcm_mode & PCM_CHMASK;

    ring_buf_t *ring_buf = &(i2s_audio_buf.i2s_rx_ring_buf);
    ring_buf = &(i2s_audio_buf.i2s_rx_ring_buf);
#if WIRELESS_MIC_24B_PCM_EN
    ring_buf_get(ring_buf,(u8 *)ptr, samples * 4 * ch_mode);
#else
    ring_buf_get(ring_buf,(u8 *)ptr, samples * 2 * ch_mode);
#endif

    if(i2s_in_ctl.callback != NULL) {
        i2s_in_ctl.callback((u8 *)ptr, samples, params);//把IIS收到的数据发给下一层
    }
}

AT(.com_text.i2s_audio_in)
void i2s_audio_input_callback_set(audio_callback_t callback)
{
    i2s_in_ctl.callback = callback;
}

AT(.text.i2s_audio_in)
void i2s_audio_in_init(u8 sample_rate, u16 samples, u8 channel)
{
    i2s_cfg_t *p_cfg = &i2s_in_ctl.i2s_cfg;
    memset(&i2s_in_ctl, 0, sizeof(i2s_in_ctl));
    i2s_audio_buf_init();

#if I2S_MASTER_EN
    p_cfg->mode        = I2S_MASTER_DMARX;
#else
    p_cfg->mode        = I2S_SLAVE_DMARX;
#endif // I2S_MASTER_EN
    p_cfg->iomap       = I2S_MAPPING_SEL;
    p_cfg->bit_mode    = I2S_BIT_MODE;
    p_cfg->data_mode   = I2S_DATA_MODE;
    p_cfg->mclk_sel    = I2S_MCLK_SEL;
    p_cfg->mclk_out_en = I2S_MCLK_EN;
    if (I2S_DMA_EN) {
        p_cfg->dma_cfg.samples = I2S_DMA_SAMPLES;
        p_cfg->dma_cfg.isr_rx_callback = i2s_rx_process;
        p_cfg->dma_cfg.isr_tx_callback = NULL;
    }
    i2s_init(p_cfg);                       //可以改到callback set之后再初始化
}
#endif // I2S_AUDIO_IN_EN
