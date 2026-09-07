#include "include.h"
#include "func.h"
#include "func_aux.h"

#if FUNC_AUX_EN
AT(.buf.aux.cb)
func_aux_t f_aux;

///AUX立体声直通不用能DAC动态降噪，可以使用下面降噪函数
void aux_dnr_init(u16 v_cnt, u16 v_pow, u16 s_cnt, u16 s_pow);
u8 aux_dnr_process(u8 *ptr, u32 len);       //返回值 1：voice(需要淡入)， 返回2: silence (需要淡出)

AT(.com_text.func.aux)
void aux_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode)
{
    bool is_24bit = pcm_mode & PCM_24BIT;
    uint nch = pcm_mode & PCM_CHMASK;

    if (f_aux.skip_frame_cnt) {
        f_aux.skip_frame_cnt --;
        return;
    }
#if AUX_SNR_EN
    aux_dnr_process(ptr, samples);
#endif // AUX_SNR_EN

    aubuf0_dma_w4_done();
    aubuf0_dma_kick(ptr, samples, nch, is_24bit);
}

#if AUX_DETECT_EN
void func_aux_insert(u8 func_sta)
{
#if AUX_MODE_2_PWROFF_EN
    sys_cb.pwroff.aux_insert_ind = 1;
#else
    func_cb.sta = FUNC_AUX;
#endif
}

void func_aux_remove(u8 func_sta)
{
    if(func_cb.last != FUNC_AUX) {
        func_cb.sta = func_cb.last;
    } else {
        func_cb.sta = FUNC_NULL;
    }
}
#endif

AT(.text.func.aux)
void func_aux_start(void)
{
    f_aux.skip_frame_cnt = 180;            //44.1\48khz->180 frames 500ms;其他采样率要调整skip帧数
    bsp_aux_start();
}

AT(.text.func.aux)
void func_aux_stop(void)
{
    bsp_aux_stop();
}

AT(.text.func.aux)
void func_aux_pause_play(void)
{
    if (f_aux.pause) {
        led_aux_play();
        func_aux_start();
    } else {
        led_idle();
        func_aux_stop();
    }
    f_aux.pause ^= 1;
}

AT(.text.func.aux)
void func_aux_setvol_callback(u8 dir)
{
    if (f_aux.pause) {
        func_aux_pause_play();
    }

    if (sys_cb.vol == 0) {
        bsp_aux_mute(0x03);
    } else if ((sys_cb.vol == 1) && (dir)) {
        bsp_aux_unmute(0x03);
    }
}

AT(.text.func.aux)
void func_aux_process(void)
{
    func_process();
}

static void func_aux_enter(void)
{
#if WARNING_FUNC_AUX
    bool tone_en = true;
#if BT_BACKSTAGE_EN
    tone_en = !func_cb.back_flag;
#endif
#endif

    if (!is_aux_enter_enable()) {
        func_cb.sta = FUNC_NULL;
        return;
    }

    memset(&f_aux, 0, sizeof(func_aux_t));
    func_cb.set_vol_callback = func_aux_setvol_callback;
//    msg_queue_clear();

    aubuf0_dma_init();

#if AUX_SNR_EN
    aux_dnr_init(2, 0x200, 60, 0x180);
#endif // AUX_SNR_EN

    led_aux_play();
    func_aux_enter_display();
#if WARNING_FUNC_AUX
    if(tone_en) {
        bsp_res_play(RES_IDX_AUX_MODE);
    }
#endif

    func_aux_start();

    led_aux_play();
}

static void func_aux_exit(void)
{
    func_aux_exit_display();
    func_aux_stop();

    aubuf0_dma_w4_done();
    aubuf0_dma_exit();
    func_cb.last = FUNC_AUX;
}

AT(.text.func.aux)
void func_aux(void)
{
    printf("%s\n", __func__);

    func_aux_enter();

    while (func_cb.sta == FUNC_AUX) {
        func_aux_process();
        func_aux_message(msg_dequeue());
        func_aux_display();
    }

    func_aux_exit();
}
#endif  //FUNC_AUX_EN
