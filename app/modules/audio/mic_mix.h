#ifndef _MIC_MIX_H
#define _MIC_MIX_H


typedef struct {
    u8 kick_flag;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} mic_mix_t;

void mic_mix_audio_input(u8 *ptr, u32 samples, u32 params);
void mic_mix_output_callback_set(audio_callback_t callback);
void mic_mix_init(u8 sample_rate, u16 samples, u8 channel);

void mic_mix_process_cb(u8 *ptr, u32 samples, u32 params);
void local_mic_output(void);

void local_mic_pacc_init(void);
s16 *mic_mix_buf_read(u16 samples);

#endif
