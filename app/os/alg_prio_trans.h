#ifndef __ALG_PRIO_TRANS_H
#define __ALG_PRIO_TRANS_H


typedef struct {
    u8  sample_rate;
    u16 samples;
    u32 params;
    ring_buf_t ring_buf;
    u8  *ptr;
    audio_callback_t callback;
    audio_callback_t handle;
} alg_prio_trans_cfg_t;

void alg_prio_trans_init(u8 sample_rate, u16 samples, u8 channel);
void alg_prio_trans_audio_input(u8 *ptr, u32 samples, u32 params);
void alg_prio_trans_audio_output_callback_set(audio_callback_t callback);
void alg_prio_trans_audio_mute_set(uint8_t mute);
void alg_prio_trans_process(void);
void alg_prio_trans_kick(uint samples, u32 params, audio_callback_t handle);

#endif
