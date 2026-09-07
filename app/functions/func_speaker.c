#include "include.h"
#include "func.h"
#include "func_speaker.h"

#if FUNC_SPEAKER_EN

AT(.buf.spk.cb)
func_speaker_t f_spk;

AT(.text.bsp.speaker)
void func_speaker_stop(void)
{
    dac_fade_out();
    dac_fade_wait();                    //等待淡出完成
    audio_path_exit(AUDIO_PATH_SPEAKER);
}

AT(.text.bsp.speaker)
void func_speaker_start(void)
{
    dac_fade_wait();                    //等待淡出完成
    audio_path_init(AUDIO_PATH_SPEAKER);
    audio_path_start(AUDIO_PATH_SPEAKER);
    dac_fade_in();
}

AT(.text.bsp.speaker)
void func_speaker_pause_play(void)
{
    if (f_spk.pause) {
        led_music_play();
        func_speaker_start();
    } else {
        led_idle();
        func_speaker_stop();
    }
    f_spk.pause ^= 1;
}

AT(.text.bsp.speaker)
void func_speaker_setvol_callback(u8 dir)
{
    if (f_spk.pause) {
        func_speaker_pause_play();
    }
}

AT(.com_text.func.speaker)
void speaker_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode)
{
    bool is_24bit = pcm_mode & PCM_24BIT;
    uint nch = pcm_mode & PCM_CHMASK;

//    mic_pacc_proc(ptr, ptr, samples);

    aubuf0_dma_w4_done();
    aubuf0_dma_kick(ptr, samples, nch, is_24bit);
}

AT(.text.func.speaker)
void func_speaker_process(void)
{
    func_process();
}

static void func_speaker_enter(void)
{
#if WARNING_FUNC_SPEAKER
    bool tone_en = true;
#if BT_BACKSTAGE_EN
    tone_en = !func_cb.back_flag;
#endif
#endif

    memset(&f_spk, 0, sizeof(f_spk));
    func_cb.set_vol_callback = func_speaker_setvol_callback;

    aubuf0_dma_init();

//    //先初始化
//    uint pacc_en = 0;
//    mic_pacc_init(S32_Q23_SAT, S32_Q23_SAT);
//    //然后设置参数
//    if(pacc_eq_set_by_res(RES_BUF_EQ_BT_MIC_16K_EQ, RES_LEN_EQ_BT_MIC_16K_EQ)) {
//        pacc_en |= MIC_PACC_EQ_BIT;
//    }
//    if(pacc_drc_set_by_res(RES_BUF_EQ_BT_MIC_16K_DRC, RES_BUF_EQ_BT_MIC_16K_DRC)) {
//        pacc_en |= MIC_PACC_DRC_BIT;
//    }
//    //最后使能
//    mic_pacc_enable(pacc_en);

    func_speaker_enter_display();
    led_music_play();
#if WARNING_FUNC_SPEAKER
    if(tone_en) {
        bsp_res_play(RES_IDX_SPK_MODE);
    }
#endif

    func_speaker_start();
}

static void func_speaker_exit(void)
{
    func_speaker_exit_display();
    led_idle();
    func_speaker_stop();
//    mic_pacc_exit();

    aubuf0_dma_w4_done();
    aubuf0_dma_exit();
    func_cb.last = FUNC_SPEAKER;
}

AT(.text.func.speaker)
void func_speaker(void)
{
    printf("%s\n", __func__);

    func_speaker_enter();

    while (func_cb.sta == FUNC_SPEAKER) {
        func_speaker_process();
        func_speaker_message(msg_dequeue());
        func_speaker_display();
    }

    func_speaker_exit();
}

#endif // FUNC_SPEAKER_EN

