#ifndef __FUNC_ADAPTER_H
#define __FUNC_ADAPTER_H

void func_adapter(void);
void func_adapter_init(void);
void func_adapter_message(u16 msg);

void func_adapter_set_create_con(void);

void func_adapter_set_nr_sta(void);
void func_adapter_cmd_rx_cb(u8 idx, u8 opcode, u8 *buf);

void wireless_second_emit_clr(void);

#endif
