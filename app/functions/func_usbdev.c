#include "include.h"
#include "func.h"
#include "func_usbdev.h"

#if FUNC_USBDEV_EN

void func_usbdev_insert(u8 func_sta)
{
    func_cb.sta = FUNC_USBDEV;
}

void func_usbdev_remove(u8 func_sta)
{
    if(func_cb.curr == FUNC_USBDEV) {
        func_cb.sta = func_cb.last;
    }
}

AT(.text.func.usbdev)
void func_usbdev_process(void)
{
    func_process();
    usb_device_process();
#if UDE_STORAGE_EN
    f_ude.rw_sta = sd0_get_rw_sta();
#endif
}

static void func_usbdev_enter(void)
{
    memset(&f_ude, 0, sizeof(f_ude));
    f_ude.db_level = bsp_volume_vol2lev(sys_cb.vol);
    f_ude.vol = sys_cb.vol;         //restore system volume
    if (!dev_is_online(DEV_USBPC)) {
        func_cb.sta = FUNC_NULL;
        return;
    }

    f_ude.cur_dev = DEV_SDCARD;
    f_ude.dev_change = 0;
    func_usbdev_enter_display();
    led_idle();

    ude_info_init();

#if WARNING_FUNC_USBDEV
    bsp_res_play(RES_IDX_PC_MODE);
#endif // WARNING_FUNC_USBDEV

#if UDE_SPEAKER_EN
    adpll_spr_set(DAC_OUT_96K);
    dac_spr_set(DAC_OUT_96K);     	//samplerate 96K
    dac_phase_set(0);
    dac_fade_in();
    aubuf0_dma_init();
#endif // UDE_SPEAKER_EN

#if UDE_STORAGE_EN
    if (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_SDCARD1)) {
        if (dev_is_online(DEV_SDCARD)) {
            f_ude.cur_dev = DEV_SDCARD;
        } else if (dev_is_online(DEV_SDCARD1)) {
            f_ude.cur_dev = DEV_SDCARD1;
        }
        fsdisk_callback_init(f_ude.cur_dev);
        fs_mount();
    }
#endif // UDE_STORAGE_EN
    usb_device_enter(UDE_ENUM_TYPE);
    sys_cb.ude_flag = true;
}

static void func_usbdev_exit(void)
{
    sys_cb.ude_flag = false;
    usb_device_exit();
    func_usbdev_exit_display();
#if UDE_SPEAKER_EN
    adpll_spr_set(DAC_OUT_SPR);
    sys_cb.vol = f_ude.vol;         //recover system volume
    bsp_change_volume(sys_cb.vol);
    dac_fade_out();
    dac_phase_set(0);
#endif // UDE_SPEAKER_EN
#if UDE_STORAGE_EN
    if (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_SDCARD1)) {
        sd0_stop(1);
    #if I2C_MUX_SD_EN
        sd0_go_idle_state();
    #endif
    }
#endif // UDE_STORAGE_EN
    func_cb.last = FUNC_USBDEV;
}

AT(.text.func.usbdev)
void func_usbdev(void)
{
    printf("%s\n", __func__);

    func_usbdev_enter();

    while (func_cb.sta == FUNC_USBDEV) {
        func_usbdev_process();
        func_usbdev_message(msg_dequeue());
        func_usbdev_display();
    }

    func_usbdev_exit();
}
#endif // FUNC_USBDEV_EN
