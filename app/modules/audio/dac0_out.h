#ifndef _DAC0_OUT_H
#define _DAC0_OUT_H


typedef struct {
    u8  mute;
    u8  sample_rate;
    u16 samples;
    u8  low_thr;
    u8  high_thr;
    audio_callback_t callback;
} dac0_out_cfg_t;

typedef struct {
    u8  sample_rate;
    u8  low_thr;
    u8  high_thr;
} dac0_out_param_cfg_t;

void dac0_out_audio_input(u8 *ptr, u32 samples, u32 params);
void dac0_out_audio_output_callback_set(audio_callback_t callback);
void dac0_out_init(u8 sample_rate, u16 samples, u8 channel);
void dac0_out_audio_mute_set(uint8_t mute);
void dac0_get_fifocnt(u32 tick_cnt);
void dac0_play_sync_fifocnt(u16 high_thr, u16 low_thr);


#endif
