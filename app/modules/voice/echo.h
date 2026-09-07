#ifndef __ECHO_API_H
#define __ECHO_API_H

typedef struct {
    u8 mute;
    u8 sample_rate;
    u16 samples;
    u32 params;
    u8 bits_mode;                       //ADC BITS选择；0: 24bits, 1: 16bits, 0xff: 跟随DAC的BIT MODE
    u16 max_delay_len;
    u16 delay_len_step;
    u16 delay_level;
    u16 attenuation_level;
    audio_callback_t callback;
} echo_cfg_t;

///库接口外的模块接口声明
void echo_audio_init(u8 sample_rate, u16 samples, u8 channel);
void echo_audio_input(u8 *ptr, u32 samples, u32 params);
void echo_audio_output_callback_set(audio_callback_t callback);
void echo_audio_exit(void);
void echo_audio_process(s16 *ptr, u32 samples);

void echo_audio_mute_set(uint8_t mute);
uint8_t echo_audio_mute_get(void);
void echo_audio_param_set(u16 attenuation ,s32 delay_length ,u32 cutoffFreq_set ,u32 lp_filter_en , u16 dry_set ,u16 wet_set);

void echo_delay_level_change(void);
void echo_delay_level_up(void);
void echo_delay_level_down(void);
void echo_delay_level_set(u8 delay_len_level);
u8 echo_delay_level_get(void);
bool echo_delay_level_is_max_min(void);
void echo_attenuation_set(u16 attenuation);
void echo_attenuation_level_set(u8 attenuation_level);
void echo_attenuation_level_change(void);
#endif
