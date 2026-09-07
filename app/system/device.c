#include "include.h"


AT(.com_text.detect)
u8 get_usbtf_muxio(void)
{
#if SD_USB_MUX_IO_EN
	return 1;
#else
	return 0;
#endif
}

//AT(.com_text.const)
//const char usb_detect_str[] = "USB STA:%d\r\n";
//AT(.com_text.const)
//const char usb_insert_str[] = "udisk insert\n";
//AT(.com_text.const)
//const char usb_remove_str[] = "udisk remove\n";

#if USB_SUPPORT_EN
AT(.com_text.detect)
void usb_detect(void)
{
    if (!is_usb_support()) {
        return;
    }

    if(!sys_cb.bt_is_inited){   //等模式初始化完，避免有变量被覆盖的情况
        return;
    }

    u8 usb_sta;
#if USB_DET_VER_SEL
    usb_sta = usb_connect();
#else
#if FUNC_USBDEV_EN || ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN
    usb_sta = usbchk_connect(USBCHK_OTG);
#else
    usb_sta = usbchk_connect(USBCHK_ONLY_HOST);
#endif
#endif

#if FUNC_USBDEV_EN || ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN
    if (usb_sta == USB_PC_CONNECTED) {
        if (dev_online_filter(DEV_USBPC)) {
//            printf("pc insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_USBPC)) {
            if (sys_cb.ude_flag) {
                pc_remove();
            }
//            printf("pc remove\n");
        }
    }
#endif
}
#endif // USB_SUPPORT_EN

#if AUX_DETECT_EN
AT(.com_text.detect)
void aux_detect(void)
{
    if (AUX_DETECT_IS_BUSY()) {
        return;
    }
    if (AUX_IS_ONLINE()) {
        if (dev_online_filter(DEV_AUX)) {
//            printf("aux insert\n");
        }
    } else {
        if (dev_offline_filter(DEV_AUX)) {
//            printf("aux remove\n");
        }
    }
}
#endif // AUX_DETECT_EN

