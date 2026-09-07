#include "include.h"
#include "i2s_audio.h"
#define FUNC_TEST_PRINT 1

#if I2S_AUDIO_OUT_EN
typedef struct {
    audio_callback_t callback;
    i2s_cfg_t i2s_cfg;
} i2s_out_ctl_t;

static i2s_out_ctl_t i2s_out_ctl;

AT(.com_text.i2s_mic_out)
void i2s_fram_16_to_32(u8 *obuf, u8 *ibuf, u32 samples, int ch_mode)
{
    s32 *ptr = (s32 *)obuf;
    s16 *ptr16 = (s16 *)ibuf;
    s32 *ptr32 = (s32 *)ibuf;
    if(ch_mode == 1) {                                             //TX Mono
        for (int i = 0; i< samples; i++) {                    //16->32位扩展
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

AT(.com_text.i2s_audio_out)
void i2s_audio_out_input(u8 *ptr, u32 samples, u32 params)
{

    u8 *obuf = (u8 *)i2s_out_get_obuf(0);
    u32 pcm_mode = params & 0xffff;
    uint ch_mode = pcm_mode & PCM_CHMASK;
    //一拖二时,MIC数据是分片解码下来的,这里要分两次才能存够完整的一帧放到I2S_DMA_BUF里面去传输
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

    if (i2s_out_ctl.callback != NULL) {
        i2s_out_ctl.callback(ptr, samples, params);
    }
}

AT(.com_text.i2s_audio_out)
void i2s_audio_output_callback_set(audio_callback_t callback)
{
    i2s_out_ctl.callback = callback;
}

AT(.text.i2s_audio_out)
void i2s_audio_out_init(u8 sample_rate, u16 samples, u8 channel)
{
    i2s_cfg_t *p_cfg = &i2s_out_ctl.i2s_cfg;
    memset(&i2s_out_ctl, 0, sizeof(i2s_out_ctl));

#if I2S_MASTER_EN
    p_cfg->mode        = I2S_MASTER_DMATX;
#else
    p_cfg->mode        = I2S_SLAVE_DMATX;
#endif // I2S_MASTER_EN
    p_cfg->iomap       = I2S_MAPPING_SEL;
    p_cfg->bit_mode    = I2S_BIT_MODE;
    p_cfg->data_mode   = I2S_DATA_MODE;
    p_cfg->mclk_sel    = I2S_MCLK_SEL;
    p_cfg->mclk_out_en = I2S_MCLK_EN;
    if (I2S_DMA_EN) {
        p_cfg->dma_cfg.samples = I2S_DMA_SAMPLES;
        p_cfg->dma_cfg.isr_tx_callback = NULL;
        p_cfg->dma_cfg.isr_rx_callback = NULL;
    }
    i2s_init(p_cfg);            //I2S初始化
}
#endif
