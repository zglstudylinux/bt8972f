#ifndef  __MIC_H_
#define  __MIC_H_


void mic_init(u8 sample_rate, u16 samples, u8 channel);
void mic_audio_output_callback_set(audio_callback_t callback);
void mic_start(void);
void mic_dma_start(void);
void mic_stop(void);
void wireless_mic_reset(void);

void local_mic_init(void);
void local_mic_exit(void);

void mic_stereo_init(u8 sample_rate, u16 samples, u8 channel);
void mic_stereo_audio_output_callback_set(audio_callback_t callback);
void mic_stereo_audio_input(u8 *ptr, u32 samples, u32 params);

#endif
