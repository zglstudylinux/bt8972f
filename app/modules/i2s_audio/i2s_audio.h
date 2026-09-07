#ifndef _I2S_AUDIO_H
#define _I2S_AUDIO_H

#include "../../bsp/bsp_i2s.h"

#define I2S_INCACHE_SIZE            1024//一次放入数据980
#define I2S_OUTCACHE_SIZE           1024//取出数据584,376

#if I2S_AUDIO_IN_EN

//I2S INPUT
void i2s_audio_in_input(u8 *ptr, u32 samples, u32 params);
void i2s_audio_input_callback_set(audio_callback_t callback);
void i2s_audio_in_init(u8 sample_rate, u16 samples, u8 channel);
#else

#define  iis_mic_in_audio_input()

#endif


//I2S OUTPUT
void i2s_audio_out_input(u8 *ptr, u32 samples, u32 params);
void i2s_audio_output_callback_set(audio_callback_t callback);
void i2s_audio_out_init(u8 sample_rate, u16 samples, u8 channel);

void i2s_audio_in_out_input(u8 *ptr, u32 samples, u32 params);
void i2s_audio_in_out_output_callback_set(audio_callback_t callback);
void i2s_audio_in_out_init(u8 sample_rate, u16 samples, u8 channel);
void i2s_audio_in_out_exit(void);
#endif
