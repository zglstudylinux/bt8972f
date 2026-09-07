/*
 *  api_btstack.h
 *
 *  Created by zoro on 2024-12-13.
 *
 *  Note:the file is copied from the btstack\btconfig path.
 */
#ifndef _API_BLE_H
#define _API_BLE_H


//LE Addr Type, public or random(static or non resolvable or resolvable)
enum {
    GAP_RANDOM_ADDRESS_TYPE_OFF = 0,    //Public
    GAP_RANDOM_ADDRESS_TYPE_STATIC,
    GAP_RANDOM_ADDRESS_NON_RESOLVABLE,
    GAP_RANDOM_ADDRESS_RESOLVABLE,
};

//LE状态
enum {
    LE_STA_STANDBY,
    LE_STA_ADVERTISING,                         //正在广播
    LE_STA_CONNECTION,                          //已连接
};


/*****************************************************************************
 * BLE连接相关（通道1，与BLE无连接广播相互独立）
 *****************************************************************************/
//BLE
void ble_adv_dis(void);
void ble_adv_en(void);
void ble_set_adv(u8 chanel, u8 type);
void ble_set_adv_interval(u16 interval);
bool ble_set_adv_data(const u8 *adv_buf, u32 size);
bool ble_set_scan_rsp_data(const u8 *scan_rsp_buf, u32 size);
void ble_update_conn_param(u16 interval, u16 latency, u16 timeout);
u8 ble_get_status(void);
void ble_disconnect(void);
bool ble_is_connect(void);

void ble_init_att_for_handle(u16 handle, uint8_t *buf, uint16_t len);
void ble_send_kick(void);
u16 ble_get_gatt_mtu(void);
int ble_tx_notify(u16 handle, u8* buf, u8 len);

uint8_t ble_set_delta_gain(void);

extern struct txpkt_tag notify_tx;

/*****************************************************************************
 * BLE无连接广播相关（通道0，与BLE连接相互独立）
 *****************************************************************************/
#define BLE_ADV0_EN_BIT             0x01
#define BLE_ADV0_MS_VAR_BIT         0x02
#define BLE_ADV0_ADDR_PUBIC_BIT          0x04

extern const uint8_t cfg_ble_adv0_en;

void ble_adv0_set_intv(u16 intv);                       //设置广播间隔，单位625us
void ble_adv0_set_ctrl(uint opcode);                    //0=关闭广播, 1=打开广播, 2=更新广播数据（打开时直接广播，关闭时仅更新buffer）
uint8_t ble_adv0_get_adv_en(void);

void ble_adv0_update_adv(void);
void ble_adv0_idx_update(void);

/*****************************************************************************
 * BLE私有连接相关（与BLE连接相互独立）
 *****************************************************************************/
 enum ble_priv_con_opcode_t {
    BLE_PRIV_ADV_DISABLE,
    BLE_PRIV_ADV_ENABLE,
    BLE_PRIV_ADV_RESUME,
    BLE_PRIV_ADV_SUSPEND,
    BLE_PRIV_DISCON,
    BLE_PRIV_UPDATE_ADV_DATA,
    BLE_PRIV_UPDATE_SCAN_RSP_DATA,
};
uint8_t ble_priv_adv_get_adv_en(void);
int ble_priv_tx_notify(u16 att_handle, u8* buf, u8 len);
void ble_priv_con_ctrl(u16 opcode);
bool ble_priv_is_connect(void);


#endif //_API_BLE_H
