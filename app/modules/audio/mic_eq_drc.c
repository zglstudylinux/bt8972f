/*
 * 文件名称: mic_eq_drc.c
 * 功能描述: EQ_DRC主要用来处理音效,主频160M，120个点,EQ和DRC算法时间分别为250us和180us左右;
 *           MIX_DRC主要用来一拖二音频混合,主频160M，120个点,算法时间为180us左右;
 ****************************************************************************************
 */
#include "include.h"
#include "mic_eq_drc.h"


#if WIRELESS_MIC_EQ_DRC_EN

static mic_eq_drc_cfg_t wireless_mic_eq_drc_cfg;

#if !WIRELESS_MIC_24B_PCM_EN
static u8 pacc_obuf[WIRELESS_MIC_SAMPLES_SELECT*4];
#endif
//------------------------------------------------------------------------------------------
//EQ_DRC处理

AT(.com_text.mic_eq_drc.input)
void mic_eq_drc_audio_input(u8 *ptr, u32 samples, u32 params)
{
    //硬件EQ_DRC
    if(wireless_mic_eq_drc_cfg.eq_drc_en && samples != 0) {
#if WIRELESS_MIC_24B_PCM_EN
        loc_mic_pacc_process(ptr, ptr, samples);
#else
        loc_mic_pacc_process(pacc_obuf, ptr, samples);
        ptr = (u8 *)pacc_obuf;
#endif
    }

    //输出到下一级
    if (wireless_mic_eq_drc_cfg.callback) {
        wireless_mic_eq_drc_cfg.callback(ptr, samples, params);
    }
}

AT(.text.mic_eq_drc.set)
void mic_eq_drc_audio_output_callback_set(audio_callback_t callback)
{
    wireless_mic_eq_drc_cfg.callback = callback;
}

AT(.text.mic_eq_drc.init)
void mic_eq_drc_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&wireless_mic_eq_drc_cfg, 0, sizeof(wireless_mic_eq_drc_cfg));

#if WIRELESS_MIC_EQ_DRC_EN
    //先初始化PACC链路
    loc_mic_pacc_init();

    //然后设置参数
    loc_mic_pacc_set_param();

    //最后使能PACC
    if(loc_mic_pacc_enable()) {
        wireless_mic_eq_drc_cfg.eq_drc_en = true;
    }
#endif
}

AT(.text.mic_eq_drc.exit)
void mic_eq_drc_exit(u8 sample_rate, u16 samples)
{
#if WIRELESS_MIC_EQ_DRC_EN
    loc_mic_pacc_exit();
#endif
}
#endif

#if ADAPTER_MIX_DRC_EN
//------------------------------------------------------------------------------------------
//MIX_DRC处理
//若output = NULL, MIX_DRC处理输出到pcm0
AT(.com_text.mix_drc.input)
void mix_drc_audio_input(mic_pcm_t *pcm0, mic_pcm_t *pcm1, mic_pcm_t *output, u16 samples)
{
    mix_pacc_process(output, pcm0, pcm1, samples);
}

AT(.text.mix_drc.init)
void mix_drc_init(void)
{
    //先初始化PACC链路
    mix_pacc_init();

    //然后设置参数
    mix_pacc_set_param();

    //最后使能PACC
    mix_pacc_enable();
}

AT(.text.mix_drc.exit)
void mix_drc_exit(void)
{
    mix_pacc_exit();
}
#endif

#if USB_MIC_EQ_DRC_EN
//------------------------------------------------------------------------------------------
//USB MIC EQ_DRC处理
static mic_eq_drc_cfg_t usb_mic_eq_drc_cfg;

AT(.com_text.usb_mic_eq_drc.input)
void usb_mic_eq_drc_audio_input(u8 *ptr, u32 samples, u32 params)
{

    //考虑到pacc中间处理过程已经开辟了temp 以及运行时间这里应该不再开辟一个temp 去规避dac out
    //硬件EQ_DRC
    if((!usb_mic_eq_drc_cfg.mute)&&(usb_mic_eq_drc_cfg.eq_drc_en) && (samples != 0)) {
        if(usb_mic_in_audio_pcm_mode_get() & PCM_STEREO) {

        } else {
            usb_mic_pacc_process(ptr, ptr, samples);
        }
    }

    //输出到下一级
    if (usb_mic_eq_drc_cfg.callback) {
        usb_mic_eq_drc_cfg.callback(ptr, samples, params);
    }
}

AT(.text.usb_mic_eq_drc.set)
void usb_mic_eq_drc_audio_output_callback_set(audio_callback_t callback)
{
    usb_mic_eq_drc_cfg.callback = callback;
}

void usb_mic_eq_drc_mute_set(uint8_t mute)
{
    usb_mic_eq_drc_cfg.mute = mute;
}

AT(.text.usb_mic_eq_drc.init)
void usb_mic_eq_drc_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&usb_mic_eq_drc_cfg, 0, sizeof(usb_mic_eq_drc_cfg));

    //先初始化PACC链路
    usb_mic_pacc_init();

    //然后设置参数
    usb_mic_pacc_set_param();

    //最后使能PACC
    if (usb_mic_pacc_enable()) {
        usb_mic_eq_drc_cfg.eq_drc_en = true;
    }

    usb_mic_eq_drc_mute_set(1);
}

AT(.text.usb_mic_eq_drc.init)
void usb_mic_eq_drc_reinit(u32 pcm_mode)
{
    usb_mic_eq_drc_mute_set(1);
    bool is_stereo = (bool)(pcm_mode & PCM_STEREO);
    if (is_stereo) {

        //先初始化PACC链路
        usb_mic_stereo_pacc_init();

        //然后设置参数
        usb_mic_stereo_pacc_set_param();

        //最后使能PACC
        if (usb_mic_stereo_pacc_enable()) {
//            usb_mic_eq_drc_cfg.eq_drc_en = true;
        }
    } else {

        //先初始化PACC链路
        usb_mic_pacc_init();

        //然后设置参数
        usb_mic_pacc_set_param();

        //最后使能PACC
        if (usb_mic_pacc_enable()) {
            usb_mic_eq_drc_cfg.eq_drc_en = true;
        }
    }
    usb_mic_eq_drc_mute_set(0);
}

AT(.text.usb_mic_eq_drc.exit)
void usb_mic_eq_drc_exit(u8 sample_rate, u16 samples)
{
    usb_mic_pacc_exit();
}
#endif
