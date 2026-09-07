#ifndef __DG_ADC_H
#define __DG_ADC_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} dg_adc_mic_cfg_t;

void dg_adc_mic_mute_set(uint8_t mute);
void dg_adc_mic_audio_input(u8 *ptr, u32 samples, u32 params);
void dg_adc_mic_output_callback_set(audio_callback_t callback);
void dg_adc_mic_init(u8 sample_rate, u16 samples, u8 channel);
uint8_t dg_adc_mic_mute_get(void);
void dg_adc_mic_proc_cb(void);
void dg_adc_init(void);
void dg_adc_pro(mic_pcm_t* input, s32 *y);
#endif
