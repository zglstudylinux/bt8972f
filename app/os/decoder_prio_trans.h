#ifndef _DECODER_PRIO_TRANS_H
#define _DECODER_PRIO_TRANS_H


typedef struct {
    u8 *ptr;
    u8  mute;
    u8  sample_rate;
    u16 samples;
    u32 params;
    audio_callback_t callback;
} decoder_prio_trans_cfg_t;



void decoder_prio_trans_init(u8 sample_rate, u16 samples, u8 channel);
void decoder_prio_trans_audio_output_callback_set(audio_callback_t callback);
void decoder_prio_trans_audio_input(u8 *ptr, u32 samples, u32 params);
void decoder_prio_trans_process(u8 idx);


#endif
