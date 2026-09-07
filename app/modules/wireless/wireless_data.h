#ifndef _WIRELESS_DATA_H
#define _WIRELESS_DATA_H

//外部函数声明
void wireless_data_buf_init(void);
bool wireless_send_data(u8 index, u8 *data, u8 len);
void wireless_send_data_until_success(u8 index, u8 *data, u8 len);
void wireless_rx_user_data(u8 index, u8 *ptr, u8 len);
bool wireless_con_user_data_tx_req(uint8_t index);
#endif
