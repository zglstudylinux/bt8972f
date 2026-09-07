#include "include.h"
#include "usb_audio.h"
#include "wireless.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


//-----------------------------------------------------------------------------------
#if ADAPTER_USB_MIC_RX_EN

usb_mic_in_cfg_t usb_mic_in_cfg;
mic_pcm_t src_pcm_obuf[WIRELESS_MIC_SAMPLES_SELECT + 52] AT(.buf.src); //52 = 146*2 - 240
mic_pcm_t src_pcm_obuf_extra[WIRELESS_MIC_SAMPLES_SELECT + 52] AT(.buf.src); //52 = 146*2 - 240
static uint8_t stereo_mic0_incache[WIRELESS_MIC_SAMPLES_SELECT*4*2] AT(.buf.src);
static uint8_t stereo_mic1_incache[WIRELESS_MIC_SAMPLES_SELECT*4*2] AT(.buf.src);
static uint8_t stereo_mic0_outcache[WIRELESS_MIC_SAMPLES_SELECT*4] AT(.buf.src);
static uint8_t stereo_mic1_outcache[WIRELESS_MIC_SAMPLES_SELECT*4] AT(.buf.src);
uint8_t stereo_outcache[WIRELESS_MIC_SAMPLES_SELECT*4*2] AT(.buf.src);

static const rbuf_tbl_t stereo_mic0_buf_tbl[1] = {
    {
        .buf = stereo_mic0_incache,
        .size = WIRELESS_MIC_SAMPLES_SELECT*4*2,
    },
};
static const rbuf_tbl_t stereo_mic1_buf_tbl[1] = {
    {
        .buf = stereo_mic1_incache,
        .size = WIRELESS_MIC_SAMPLES_SELECT*4*2,
    },
};

AT(usbdev.com.rodata)
static int usb_mic_speed_tbl[][2] = {
//  samples,    phase
    {-96,       -65536/2},      //调慢，样点多1/240
    {-48,       -65536/4},      //调慢，样点多1/480
    {-24,       -65536/8},      //调慢，样点多1/960
    {24,        0},             //不调速
    {48,        65536/8},       //调快，样点少1/960
    {96,        65536/4},       //调快，样点少1/480
    {0xffff,    65536/2},       //调快，样点少1/240
};

AT(.usbdev.com.adj)
void usb_mic_in_src_adj(void)
{
    if (usb_mic_in_cfg.mic_start) {
        uint target = (WIRELESS_MIC_SAMPLES_SELECT + 2*48) * ((usb_mic_in_audio_pcm_mode_get()&PCM_STEREO)? 2 : 1);
        uint curr = uda_mic_in_get_samples();
//        u8 speed = 0;

        for(uint i=0; i<sizeof(usb_mic_speed_tbl)/(2*sizeof(int)); i++) {
            if(curr <= target + usb_mic_speed_tbl[i][0]) {
                src_phase_comp_set(0, usb_mic_speed_tbl[i][1]);
                src_phase_comp_set(1, usb_mic_speed_tbl[i][1]);

//                speed = i-3;
                break;
            }
        }

//        my_spi_putc(0x55);
//        my_spi_putc(curr>>8);
//        my_spi_putc(curr>>0);
//        if(usb_mic_in_cfg.speed != speed) {
//            GPIOESET = BIT(5);
//            my_spi_putc(0x55);
//            my_spi_putc(speed);
//            my_spi_putc(usb_mic_in_cfg.speed);
//            GPIOECLR = BIT(5);
//            usb_mic_in_cfg.speed = speed;
//        }
    }
}

AT(.com_text.usb_mic_in)
void usb_mic_in_audio_input(u8 *ptr, u32 samples, u32 params)
{
    u32 resamples = samples;
    u32 pcm_mode = params & 0xffff;
    void *obuf = src_pcm_obuf; //ptr
    s32 *dst = (s32 *)&stereo_outcache[0];

    if (usb_mic_in_cfg.pcm_mode & PCM_STEREO) { ///stereo usb mic
        if (usb_mic_in_cfg.mic_start && !usb_mic_in_cfg.mute) {
            //get buf
            if (!ring_buf_get(&(usb_mic_in_cfg.ring_pcm0_buf), stereo_mic0_outcache, samples*sizeof(mic_pcm_t))) {
                memset(stereo_mic0_outcache, 0x00, samples*sizeof(mic_pcm_t));
            }
            if (!ring_buf_get(&(usb_mic_in_cfg.ring_pcm1_buf), stereo_mic1_outcache, samples*sizeof(mic_pcm_t))) {
                memset(stereo_mic1_outcache, 0x00, samples*sizeof(mic_pcm_t));
            }
            //两路麦分别进src
            resamples = src_frame_resample_24bit(0, (int *)stereo_mic0_outcache, (int *)src_pcm_obuf, samples);
                        src_frame_resample_24bit(1, (int *)stereo_mic1_outcache, (int *)src_pcm_obuf_extra, samples);
            //转stereo格式
            for (uint i=0; i<resamples; i++) {
                dst[2*i]   = src_pcm_obuf[i];
                dst[2*i+1] = src_pcm_obuf_extra[i];
            }
            //STEREO EQ DRC
#if ADAPTER_USB_MIC_EQ_DRC_EN
            usb_mic_stereo_pacc_process((u8 *)stereo_outcache, (u8 *)stereo_outcache, resamples);
#endif
                //调试用
//            aubuf0_dma_w4_done();
//            aubuf0_dma_kick(stereo_outcache, resamples, 2, true);
            uda_mic_in_process(stereo_outcache, resamples, usb_mic_in_cfg.pcm_mode);
        }
    } else { ///mono usb mic
        if (usb_mic_in_cfg.mic_start && !usb_mic_in_cfg.mute) {
#if WIRELESS_MIC_24B_PCM_EN
            samples = src_frame_resample_24bit(0, (int *)ptr, obuf, samples);
#else
            samples = src_frame_resample(0, (short *)ptr, obuf, samples);
#endif
            uda_mic_in_process(obuf, samples, pcm_mode);
        }
    }
    if (usb_mic_in_cfg.callback) {
        usb_mic_in_cfg.callback(obuf, samples, params);
    }
}

AT(.com_text.usb_mic_in)
void usb_mic_in_audio_input_stereo(u8 *ptr0, u8 *ptr1, u32 samples)
{
    if (usb_mic_in_cfg.pcm_mode & PCM_STEREO) {
        if (usb_mic_in_cfg.mic_start && !usb_mic_in_cfg.mute) {
        //put pcm0 ringbuf
        ring_buf_put(&(usb_mic_in_cfg.ring_pcm0_buf), ptr0, samples*sizeof(mic_pcm_t));
        //put pcm1 ringbuf
        ring_buf_put(&(usb_mic_in_cfg.ring_pcm1_buf), ptr1, samples*sizeof(mic_pcm_t));
        }
    }
}

AT(.text.usb_mic_in)
void usb_mic_in_audio_output_callback_set(audio_callback_t callback)
{
    usb_mic_in_cfg.callback = callback;
}

AT(.text.usb_mic_in)
void usb_mic_in_audio_pcm_mode_set(u32 pcm_mode)
{
#if ADAPTER_USB_MIC_EQ_DRC_EN
    usb_mic_eq_drc_reinit(pcm_mode);
#endif
    usb_mic_in_cfg.pcm_mode = pcm_mode;
}

AT(.text.usb_mic_in)
u32 usb_mic_in_audio_pcm_mode_get(void)
{
    return usb_mic_in_cfg.pcm_mode;
}

AT(.text.usb_mic_in)
void usb_mic_in_audio_mute_set(u8 mute)
{
    usb_mic_in_cfg.mute = mute;
}

AT(.text.usb_mic_in)
void usb_mic_in_init(u8 sample_rate, u16 samples, u8 channel)
{
    ///还需要lock一下src的代码，先不处理
    memset(&usb_mic_in_cfg, 0, sizeof(usb_mic_in_cfg));

    memset(&stereo_mic0_incache, 0, WIRELESS_MIC_SAMPLES_SELECT*4*2);
    memset(&stereo_mic1_incache, 0, WIRELESS_MIC_SAMPLES_SELECT*4*2);
    ring_buf_init(&(usb_mic_in_cfg.ring_pcm0_buf), stereo_mic0_buf_tbl, 1, 0);
    ring_buf_init(&(usb_mic_in_cfg.ring_pcm1_buf), stereo_mic1_buf_tbl, 1, 0);

    usb_mic_in_audio_pcm_mode_set(PCM_STEREO | PCM_24BIT);
//    usb_mic_in_audio_pcm_mode_set(PCM_24BIT);
}


#if UDE_MIC_EN
AT(.text.func.usbdev)
void ude_mic_start(uint mic_bits)
{
    src_init(0, 48000,48000);
    src_init(1, 48000,48000);
    usb_mic_in_cfg.mic_start = true;
}

AT(.text.func.usbdev)
void ude_mic_stop(void)
{
    usb_mic_in_cfg.mic_start = false;
}
#endif

#if UDE_SPEAKER_EN
void ude_spk_out_callback(void *buf, uint len, u32 is_24bit)
{
}
#endif

#endif // ADAPTER_USB_MIC_RX_EN

//-----------------------------------------------------------------------------------
#if FUNC_USBDEV_EN
#if UDE_MIC_EN
AT(.text.func.usbdev)
void ude_mic_start(uint mic_bits)
{
    audio_path_init(AUDIO_PATH_USBMIC);
    audio_path_start(AUDIO_PATH_USBMIC);
}

AT(.text.func.usbdev)
void ude_mic_stop(void)
{
    audio_path_exit(AUDIO_PATH_USBMIC);
}
#endif

#if UDE_SPEAKER_EN
void uda_put_samples(void *buf, uint len, u32 is_24bit);
void ude_spk_out_callback(void *buf, uint len, u32 is_24bit)
{
    uda_put_samples(buf, len, is_24bit);
}
#endif
#endif // FUNC_USBDEV_EN
