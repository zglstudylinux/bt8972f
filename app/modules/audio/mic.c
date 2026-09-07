#include "include.h"

static sdadc_cfg_t mic_cfg = {CH_MIC4,  SPR_48000, 3,  1, 1,  ADC2DAC_EN,  120, NULL};
#if DEVICE_DOUBLE_MIC
static sdadc_cfg_t mic_stereo_cfg;
#endif
volatile bool sdadc_w4_kick;

AT(.text.bsp.wireless_mic)
void mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    mic_cfg.sample_rate = sample_rate;
    mic_cfg.samples = samples*2;
    mic_cfg.bits_mode = WIRELESS_MIC_24B_PCM_EN? 0 : 1;

    mic_cfg.anl_gain = ((xcfg_cb.mic_anl_gain) |
                    (xcfg_cb.mic_anl_gain<<6) |
                    (xcfg_cb.mic_anl_gain<<12)|
                    (xcfg_cb.mic_anl_gain<<18)|
                    (xcfg_cb.mic_anl_gain<<24));
    mic_cfg.dig_gain = ((xcfg_cb.bt_mic_dig_gain) |
                    (xcfg_cb.bt_mic_dig_gain<<6) |
                    (xcfg_cb.bt_mic_dig_gain<<12)|
                    (xcfg_cb.bt_mic_dig_gain<<18)|
                    (xcfg_cb.bt_mic_dig_gain<<24));


#if DEVICE_DOUBLE_MIC
    if(xcfg_cb.call_talk_mic != SECOND_MIC) {                                  //双麦，暂时固定MIC1，后续根据实际场景开放给客户在配置中选择
        mic_cfg.channel = (mic_mapping_tbl[xcfg_cb.call_talk_mic] | (mic_mapping_tbl[SECOND_MIC] << 8));
    }
#else
    mic_cfg.channel = mic_mapping_tbl[xcfg_cb.call_talk_mic];
#endif

//    mic_cfg.callback = (pcm_callback_t)sdadc_dummy;
}

AT(.text.bsp.wireless_mic)
void mic_audio_output_callback_set(audio_callback_t callback)
{
    mic_cfg.callback = (pcm_callback_t)callback;

    //改到蓝牙连接后再初始化，节省未连接功耗
//    sdadc_init(&mic_cfg);
//    sdadc_start(mic_cfg.channel);
}



AT(.text.bsp.wireless_mic)
void mic_start(void)
{
    MIC_LDO_EN();

    int ret = sdadc_init(&mic_cfg);
    if(ret < 0) {
        printf("sdadc_init fail: %d\n", ret);
    }
    sdadc_start(mic_cfg.channel);

    sdadc_w4_kick = true;
}

AT(.com_text.bsp.wireless_mic)
void mic_dma_start(void)
{
    if (sdadc_w4_kick) {
        sdadc_w4_kick = false;
        sdadc_dma_start(mic_cfg.channel);
    }
}

AT(.text.bsp.wireless_mic)
void mic_stop(void)
{
    sdadc_exit(mic_cfg.channel);

    MIC_LDO_DIS();
}

#if ADAPTER_LOCAL_MIC_MIX_EN
AT(.text.bsp.wireless_mic)
void local_mic_init(void)
{
    mic_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_CHANNEL_SELECT);
    mic_cfg.callback = mic_mix_process_cb;

    mic_start();
    mic_dma_start();
}

AT(.text.mic_mix)
void local_mic_exit(void)
{
    sdadc_exit(mic_cfg.channel);
}
#endif

#if DEVICE_DOUBLE_MIC
AT(.text.bsp.wireless_mic)
void mic_stereo_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&mic_stereo_cfg, 0, sizeof(sdadc_cfg_t));
}

AT(.com_text.wireless_mic)
void mic_stereo_audio_input(u8 *ptr, u32 samples, u32 params)
{
    u32 pcm_mode = params & 0xffff;
    uint nch = pcm_mode & PCM_CHMASK;

    nch = 1;
    pcm_mode = (pcm_mode & ~PCM_CHMASK) | nch;


    if (mic_stereo_cfg.callback) {
        mic_stereo_cfg.callback((u8 *)ptr, samples, pcm_mode);
    }
}


AT(.text.bsp.wireless_mic)
void mic_stereo_audio_output_callback_set(audio_callback_t callback)
{
    mic_stereo_cfg.callback = (pcm_callback_t)callback;
}
#endif

