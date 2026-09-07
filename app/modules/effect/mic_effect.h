#ifndef __MIC_EFFECT_H
#define __MIC_EFFECT_H

enum {
    LOC_MIC_PACC_EQ_CS          = 0,
    LOC_MIC_PACC_DRC_CS,

    LOC_MIC_PACC_MAX_CS,
};

enum {
    MIX_MIC_PACC_DRC_CS             = 0,
    MIX_MIC_PACC_EQ_CS,

#if ADAPTER_FREQ_SHIFT_EN
    MIX_MIC_PACC_FSH_CS,
#endif

    MIX_MIC_PACC_MAX_CS,
};

enum {
    SCO_PACC_EQ_CS             = 0,
    SCO_PACC_DRC_CS,

    SCO_PACC_MAX_CS,
};

enum {
    USB_PACC_EQ_CS             = 0,
    USB_PACC_DRC_CS,

    USB_PACC_MAX_CS,
};

enum {
    USB_MIC0_STEREO_PACC_EQ_CS             = 0,
    USB_MIC1_STEREO_PACC_EQ_CS,

    USB_MIC0_STEREO_PACC_DRC_CS,
    USB_MIC1_STEREO_PACC_DRC_CS,
    USB_MIC_STEREO_PACC_MAX_CS,
};

void loc_mic_pacc_init(void);
void loc_mic_pacc_set_param(void);
void loc_mic_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param);
void loc_mic_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param);
bool loc_mic_pacc_enable(void);
void loc_mic_pacc_process(u8 *obuf, u8 *ibuf, u32 samples);
void loc_mic_pacc_exit(void);

void mix_pacc_init(void);
void mix_pacc_set_param(void);
void mix_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param);
void mix_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param);
void mix_pacc_enable(void);
void mix_pacc_process(mic_pcm_t *obuf, mic_pcm_t *ibuf0, mic_pcm_t *ibuf1, u32 samples);
void mix_pacc_exit(void);

void sco_pacc_init(void);
void sco_pacc_set_param(bool is_msbc);
void sco_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param);
void sco_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param);
void sco_pacc_enable(void);
void sco_pacc_process(u8 *obuf, u8 *ibuf, u32 samples);
void sco_pacc_exit(void);
bool mic_sco_pacc_init(bool is_msbc);
#define mic_sco_pacc_exit()     sco_pacc_exit()

void usb_mic_pacc_init(void);
void usb_mic_pacc_set_param(void);
void usb_mic_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param);
void usb_mic_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param);
bool usb_mic_pacc_enable(void);
void usb_mic_pacc_process(u8 *obuf, u8 *ibuf, u32 samples);
void usb_mic_pacc_exit(void);

void usb_mic_stereo_pacc_init(void);
void usb_mic_stereo_pacc_set_param(void);
void usb_mic0_stereo_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param);
void usb_mic0_stereo_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param);
void usb_mic1_stereo_pacc_set_eq_by_param(u8 band_cnt, const u32 *eq_param);
void usb_mic1_stereo_pacc_set_drc_by_param(u8 band_cnt, const u32 *drc_param);
bool usb_mic_stereo_pacc_enable(void);
void usb_mic_stereo_pacc_process(u8 *obuf, u8 *ibuf, u32 samples);
void usb_mic_stereo_pacc_exit(void);
#endif
