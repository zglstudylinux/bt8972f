#ifndef _ENCODER_PRIO_TRANS_H
#define _ENCODER_PRIO_TRANS_H

#define ENCODER_BUF_EN      0  // «∑Ò∆Ù”√ª∫¥Ê

typedef struct {
    u8  sample_rate;
    u16 samples;
    u32 params;
#if ENCODER_BUF_EN
    ring_buf_t ring_buf;
#endif
    u8  *ptr;
    audio_callback_t callback;
    audio_callback_t handle;
} encoder_prio_trans_cfg_t;


void encoder_prio_trans_init(u8 sample_rate, u16 samples, u8 channel);
void encoder_prio_trans_audio_output_callback_set(audio_callback_t callback);
void encoder_prio_trans_audio_input(u8 *ptr, u32 samples, u32 params);
void encoder_prio_trans_kick(uint samples, u32 params, audio_callback_t handle);

#endif
