#ifndef _FUNC_USBDEV_H
#define _FUNC_USBDEV_H


void func_usbdev_message(u16 msg);
void ude_sdcard_switch(u8 dev);
void ude_sdcard_change_process(void);

void func_usbdev_insert(u8 func_sta);
void func_usbdev_remove(u8 func_sta);

#if (GUI_SELECT != GUI_NO)
void func_usbdev_display(void);
void func_usbdev_enter_display(void);
void func_usbdev_exit_display(void);
#else
#define func_usbdev_display()
#define func_usbdev_enter_display()
#define func_usbdev_exit_display()
#endif

#endif // _FUNC_USBDEV_H
