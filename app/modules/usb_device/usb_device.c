#include "include.h"
#include "usb_device.h"

#if FUNC_USBDEV_EN || ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN

uint8_t usb_det_ver_sel     = USB_DET_VER_SEL;

uint8_t uac_ver_sel         = UAC_VER_SEL;
uint8_t cfg_desc_cfg_type   = CFG_DESC_CFG_TYPE;
uint8_t cfg_usb_maxpower    = CFG_USB_MAXPOWER;
uint8_t cfg_get_conf_len    = CFG_GET_CONF_LEN;
uint8_t usb_driver_max_en   = USB_DRIVER_MAX_EN;
uint8_t udm_vol_default_sel = UDM_VOL_DEFAULT_SEL;
uint8_t uda_balance_vol_en  = UDA_BALANCE_VOL_EN;

uint8_t usb_spk_spl_en      = USB_SPK_SPL_EN;
uint8_t usb_spk_bits_en     = USB_SPK_BITS_EN;
uint8_t usb_spk_sync_mode_en= USB_SPK_SYNC_MODE_EN;

uint8_t usb_mic_dch_en      = USB_MIC_DCH_EN;
uint8_t usb_mic_spl_en      = USB_MIC_SPL_EN;

#if UDE_MIC_EN
uint8_t usb_mic_bits_en     = USB_MIC_BITS_EN;
#endif


f_ude_t f_ude;


u8 ude_get_sys_volume(void)
{
    return f_ude.db_level;
}

AT(.usbdev.com)
void ude_set_sys_volume(u8 level)
{
    if (f_ude.db_level != level) {
        f_ude.db_level = level;
        msg_enqueue(EVT_UDE_SET_VOL);
    }
}

//系统音量mute与usb mute同步
AT(.usbdev.com)
void ude_set_sys_mute(u8 sys_mute)
{
    if (sys_cb.mute != sys_mute) {
        msg_enqueue(EVT_UDE_SET_MUTE);
    }
}

//usb左右声道均衡
AT(.usbdev.com)
void uda_set_balance_vol(u8 db_level_l, u8 db_level_r)
{
#if UDA_BALANCE_VOL_EN
    if (f_ude.db_level_l != db_level_l || f_ude.db_level_r != db_level_r) {
        f_ude.db_level_l = db_level_l;
        f_ude.db_level_r = db_level_r;
        msg_enqueue(EVT_UDE_SET_VOL_L_R);
    }
#endif
}

#if UDE_STORAGE_EN
AT(.text.func.usbdev)
void ude_sdcard_switch(u8 dev)
{
//    printf("%s, %d, %d\n", __func__, f_ude.cur_dev, dev);
    if (f_ude.cur_dev != dev) {
        ude_sd_remove();
        f_ude.cur_dev = dev;
        fsdisk_callback_init(f_ude.cur_dev);
        f_ude.dev_change = 1;
    } else {
        sd0_init();
    }
}

AT(.text.func.usbdev)
void ude_sdcard_change_process(void)
{
    static u8 cnt_s = 0;
    if (f_ude.dev_change) {
        cnt_s++;
        if (cnt_s >= 5) {
            f_ude.dev_change = 0;
            sd0_init();
        }
    } else {
        cnt_s = 0;
    }
}
#endif // UDE_STORAGE_EN

#endif // FUNC_USBDEV_EN

