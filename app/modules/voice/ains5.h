#ifndef __AINS5_H
#define __AINS5_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    volatile u8 kick_proc_done;
    u16 samples;
    audio_callback_t callback;
} ains5_mic_cfg_t;

///库接口外的模块接口声明
void ains5_mic_init(u8 sample_rate, u16 samples, u8 channel);
void ains5_mic_audio_input(u8 *ptr, u32 samples, u32 params);
void ains5_mic_output_callback_set(audio_callback_t callback);
void ains5_mic_param_set(s16 ains5_nt);
uint8_t ains5_mic_mute_get(void);
void ains5_mic_mute_set(uint8_t mute);
void ains5_mic_exit(void);
void ains5_mic_proc_cb(void);

#endif
