#ifndef _HUART_AUDIO_IN_MIX_H
#define _HUART_AUDIO_IN_MIX_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    audio_callback_t callback;
} huart_audio_in_cfg_t;

void huart_audio_in_input(u8 *ptr, u32 samples, u32 params);
void huart_audio_in_output_callback_set(audio_callback_t callback);
void huart_audio_in_init(u8 sample_rate, u16 samples, u8 channel);
void huart_audio_in_mute_set(uint8_t mute);

#endif //_WIRELESS_H
