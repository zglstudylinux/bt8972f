#ifndef __FUNC_DEVICE_H
#define __FUNC_DEVICE_H

void func_device(void);
void func_device_init(void);
void func_device_set_create_con(void);
void func_device_message(u16 msg);

void func_device_cmd_rx_cb(u8 opcode, u8 *buf);

void func_device_interphone_set_con_cmp(void);

#endif
