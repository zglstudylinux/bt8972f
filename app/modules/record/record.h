#ifndef __RECORD_H
#define __RECORD_H

typedef struct {
    u8 kick_flag;
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} mic_rec_t;

void mic_rec_audio_input(u8 *ptr, u32 samples, u32 params);
void mic_rec_output_callback_set(audio_callback_t callback);
void mic_rec_init(u8 sample_rate, u16 samples, u8 channel);
void bt_rec_audio_input(u8 *ptr, u32 samples);

#endif //__RECORD_H
