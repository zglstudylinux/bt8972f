#ifndef _USB_AUDIO_H
#define _USB_AUDIO_H


typedef struct {
    u8  mic_start;
    u8  speed;
    u8  input_cnt;
    u16 input_min;
    u16 input_min1;

    u8 mute;
    u8 sample_rate;
    u16 samples;
    u32 pcm_mode;
#if 1
    ring_buf_t ring_pcm0_buf;
    ring_buf_t ring_pcm1_buf;
#endif
    audio_callback_t callback;
} usb_mic_in_cfg_t;

void usb_mic_in_audio_input(u8 *ptr, u32 samples, u32 params);
void usb_mic_in_audio_output_callback_set(audio_callback_t callback);
void usb_mic_in_audio_mute_set(u8 mute);
void usb_mic_in_init(u8 sample_rate, u16 samples, u8 channel);
void usb_mic_in_src_adj(void);
void usb_mic_in_audio_input_stereo(u8 *ptr0, u8 *ptr1, u32 samples);
u32 usb_mic_in_audio_pcm_mode_get(void);
#endif
