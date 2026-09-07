#ifndef _API_USB_H
#define _API_USB_H

enum {
    USBCHK_ONLY_HOST,
    USBCHK_ONLY_DEVICE,
    USBCHK_OTG,
};

enum {
    USB_DISCONNECT,
    USB_UDISK_CONNECTED,
    USB_PC_CONNECTED,
};

//USB Device HID Comsumer usage id
#define UDE_HID_PLAYPAUSE       BIT(0)                  //0xcd
#define UDE_HID_VOLUP           BIT(1)                  //0xe9
#define UDE_HID_VOLDOWN         BIT(2)                  //0xea
#define UDE_HID_RESV            BIT(3)                  //0xcf
#define UDE_HID_HOME            BIT(4)                  //0x40
#define UDE_HID_NEXTFILE        BIT(5)                  //0xb5
#define UDE_HID_PREVFILE        BIT(6)                  //0xb6
#define UDE_HID_MUTE            BIT(7)                  //0xe2

void usb_init(void);
void usb_disable(void);
u8 usbchk_connect(u8 mode);                             //usb旧检测方式
u8 usb_connect(void);                                   //usb新检测方式
void udisk_remove(void);
void udisk_insert(void);
void udisk_invalid(void);

void udisk_suspend(void);
void udisk_resume(void);

void usb_device_enter(u8 enum_type);
void usb_device_exit(void);
void usb_device_process(void);
void ude_tmr_isr(void);
void pc_remove(void);
bool usb_device_hid_send(u16 hid_val, int auto_release);
u8 typec_computer_mobile_sta(void);

void uda_get_spk_mute(u8 *buf);                         //获取speaker mute
void uda_set_spk_mute(u8 val);                          //设置speaker mute
u16 uda_get_spk_volume(u8 ch);                          //获取speaker 0-L,1-R,2-LR音量值
u8 uda_get_spk_spr(void);                               //获取speaker当前采样率
u8 uad_get_spk_bits(void);                              //获取speaker当前bits长度

void uda_get_mic_mute(u8 *buf);                         //获取mic mute
void uda_set_mic_mute(u8 val);                          //设置mic mute
u8 uda_get_mic_spr(void);                               //获取mic当前采样率
u16 uda_get_mic_volume(u8 ch);                          //获取mic音量
u8 uad_get_mic_bits(void);                              //获取mic当前bits长度
void uda_mic_in_process(u8 *ptr, u32 samples, u32 pcm_mode); //推送pcm样点到mic_in的buffer
uint uda_mic_in_get_samples(void);                       //获取mic_in的buffer中pcm样点数量

#endif // _API_USB_H
