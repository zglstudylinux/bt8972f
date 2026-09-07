#include "include.h"
#include "record.h"
#include "sfunc_record.h"

#if MUSIC_SDCARD_EN

static mic_rec_t mic_rec;
u8 buf_rec[960*3] AT(.buf.record);

AT(.com_text.mic_rec)
void mic_rec_audio_input(u8 *ptr, u32 samples, u32 params)
{
    u8 *temp_ptr = ptr;
    if(sfunc_is_recording()){
        for(u8 u = 0; u < samples; u++){

            buf_rec[4*u] = 0;
            buf_rec[4*u+1] = temp_ptr[4*u];
            buf_rec[4*u+2] = temp_ptr[4*u+1];
            buf_rec[4*u+3] = temp_ptr[4*u+2];

        }
//#if (REC_TYPE_SEL == REC_WAV)
        puts_rec_encbuf(buf_rec, samples*4);                     //wav文件，ADC数据直接给到enc_buf
//#else
//        puts_rec_obuf(buf_rec, samples*3);
//#endif
    }
    if (mic_rec.callback) {
        mic_rec.callback(ptr, samples, params);
    }
}

AT(.com_text.mic_rec)
void bt_rec_audio_input(u8 *ptr, u32 samples)
{
    u8 *temp_ptr = ptr;
    if(sfunc_is_recording()){
        for(u8 u = 0; u < samples; u++){
            buf_rec[2*u] = temp_ptr[2*u];
            buf_rec[2*u+1] = temp_ptr[2*u+1];
        }

        puts_rec_encbuf(buf_rec, samples*2);                     //wav文件，ADC数据直接给到enc_buf
    }
}


AT(.text.mic_rec)
void mic_rec_output_callback_set(audio_callback_t callback)
{
    mic_rec.callback = callback;
}

AT(.text.mic_rec)
void mic_rec_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(buf_rec, 0 ,sizeof(buf_rec));
}

#endif
