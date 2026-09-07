#ifndef __AGC_H
#define __AGC_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} agc_mic_cfg_t;

///库接口外的模块接口声明
void agc_mic_init(u8 sample_rate, u16 samples, u8 channel);
void agc_mic_audio_input(u8 *ptr, u32 samples, u32 params);
void agc_mic_output_callback_set(audio_callback_t callback);
void agc_mic_param_set(s16 agc_nt);
uint8_t agc_mic_mute_get(void);
void agc_mic_mute_set(uint8_t mute);
void agc_mic_exit(void);
void agc_mic_proc_cb(void);

#endif
