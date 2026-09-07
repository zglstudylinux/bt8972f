#ifndef  __SPK_H_
#define  __SPK_H_

//spk模拟通路
//void spk_init(u8 sample_rate, u16 samples, u8 channel);
//void spk_audio_output_callback_set(audio_callback_t callback);
//void spk_start(void);
//void spk_dma_start(void);
//void spk_stop(void);

#define spk_init                            mic_init        //for test
#define spk_audio_output_callback_set       mic_audio_output_callback_set   //for test
#define spk_start                           mic_start       //for test
#define spk_dma_start                       mic_dma_start   //for test
#define spk_stop                            mic_stop        //for test

#endif
