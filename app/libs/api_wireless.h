/*
 *  api_btstack.h
 *
 *  Created by zoro on 2017-8-24.
 *
 *  Note:the file is copied from the btstack\btconfig path.
 */
#ifndef _API_WIRELESS_H
#define _API_WIRELESS_H

/*****************************************************************************
 * WIRRELESS连接相关
 *****************************************************************************/
//feature
#define FEAT_LINK_NB            0xf
#define FEAT_D2A                BIT(7)      //Device to ADAPTER
#define FEAT_A2D                BIT(6)      //ADAPTER to Device
#define FEAT_BONDING            BIT(5)      //组队绑定，需要清除配对才能重新组队
#define FEAT_VERS               (0x7<<8)    //bit[10:8]
#define FEAT_RATE               (0x7<<11)   //bit[13:11]，传输速率
#define FEAT_HOP_V1             BIT(14)     //bit[14]，新版本跳频计算
#define FEAT_ADAPTER_SAVE       BIT(15)     //bit[15]，发射端算法音频参数由接收端发送同步

//ext_feature
#define EXT_FEAT_ADV_EXT        BIT(0)      //广播数据包拓展，多8字节可用于广播包发送及scan回调
#define EXT_FEAT_VERS           (0x7<<1)    //bit[3:1]，扩展feat，跟FEAT_VERS合并成64个组合
#define EXT_FEAT_2TNR_NB        (0x7<<4)    //bit[6:4]，2TNR时的最大可连接NB数

#define WL_ADV_INTERVAL         0x0018
#define WL_ADV_SLEEP_INTERVAL   (0x0018*8)

//con_type, byte0
enum {
    CON_VERS1   = 0,
    CON_VERS2,
    CON_VERS3,
    CON_VERS4,
    CON_VERS5,
    CON_VERS6,
    CON_VERS7,
    CON_VERS8,
    CON_VERS9,
    CON_VERS10,
    CON_VERS11,
    CON_VERS12,
    //TODO: add more here

    //max = 63
};

//con_type, byte1
enum {
    //type, bit[3:0]
    CON_FLAG_T2R    = 0,     //T2R AUDIO
    CON_FLAG_RSVD0  = 1,
    CON_FLAG_RSVD1  = 2,
    CON_FLAG_NONE   = 3,
    CON_FLAG_RELAY  = 4,

    //flag, bit[7:4]
    CON_FLAG_SLAVE  = BIT(6),
//    CON_FLAG_TWS    = BIT(7),
    CON_FLAG_D2D_CON= BIT(7),
};

enum {
    WL_STATUS_OFF               = 0,
    WL_STATUS_IDLE,
    WL_STATUS_DISCONNECT,
    WL_STATUS_CONNECTED,
};

enum bt_msg_wireless_t {
    WIRELESS_ADV_SET_ENABLE,
    WIRELESS_ADV_SET_PARAM,
    WIRELESS_ADV_SET_DATA,
    WIRELESS_SET_SCAN_RSP_DATA,
    WIRELESS_SCAN_SET_ENABLE,
    WIRELESS_SCAN_SET_PARAM,
    WIRELESS_CONNECT_REQ,
    WIRELESS_DISCONNECT_REQ,
    WIRELESS_CON_PARAM_UPDATE,
    WIRELESS_CON_SEND_CMD,
    WIRELESS_SETUP,
    WIRELESS_OFF,

    WIRELESS_PER_ADV_SET_ENABLE,
    WIRELESS_PER_SCAN_SET_ENABLE,
    WIRELESS_PER_SYNC_SET_ENABLE,
    WIRELESS_PER_CH_MAP_UPDATE,

    WIRELESS_WL_SET_CON_FLAG,
};

typedef void (*audio_callback_t)(u8 *ptr, u32 samples, u32 params);

extern uint8_t cfg_wireless_role;
extern uint16_t cfg_wireless_feat;
extern uint16_t cfg_wireless_ext_feat;
extern uint16_t cfg_wireless_d2a_dec_us;

#define wireless_role_is_adapter()              cfg_wireless_role
#define wireless_mic_is_bonding()               (bool)(cfg_wireless_feat & FEAT_BONDING)
#define cfg_le_conn_vers()                      (((cfg_wireless_feat & FEAT_VERS)>>8) + (((cfg_wireless_ext_feat & EXT_FEAT_VERS)>>1)<<3))

#define bt_wireless_msg(msg, param)             bt_send_msg(BT_MSG_WIRELESS, (msg<<16) | (u16)param)
#define wireless_adv_set_enable(pscan, iscan)   bt_wireless_msg(WIRELESS_ADV_SET_ENABLE, (pscan<<1)|(iscan))     //广播使能，pscan=可被连接，iscan=可被发现
#define wireless_adv_set_interval(interval)     bt_wireless_msg(WIRELESS_ADV_SET_PARAM,interval)                 //设置ADV 广播间隔
#define wireless_scan_set_enable(en)            bt_wireless_msg(WIRELESS_SCAN_SET_ENABLE, en)                    //扫描使能
#define wireless_scan_set_param(intv,window)    bt_wireless_msg(WIRELESS_SCAN_SET_PARAM, (intv<<8)|window)       //设置ble_scan参数
#define wireless_connect_req(ms)                bt_wireless_msg(WIRELESS_CONNECT_REQ, ms)                        //发起连接，ms=连接超时（单位毫秒，0xffff时一直连接不超时）
#define wireless_disconnect_req(idx)            bt_wireless_msg(WIRELESS_DISCONNECT_REQ, idx)                    //发起断开，idx=连接通道
#define wireless_con_upd_ind(idx, interval)     bt_wireless_msg(WIRELESS_CON_PARAM_UPDATE, (idx&0x1)|(interval<<1))//更新连接参数，idx=连接通道
#define wireless_setup()                        bt_wireless_msg(WIRELESS_SETUP, 0)                               //无线麦状态设为setup
#define wireless_off()                          bt_wireless_msg(WIRELESS_OFF, 0)                                 //无线麦状态设为off

#define wireless_set_con_flag(adv_flag,scan_flag)    bt_wireless_msg(WIRELESS_WL_SET_CON_FLAG, (adv_flag<<8)|scan_flag)       //设置adv/scan类型，是用于t2r链路还是普通连接链路

void wireless_device_fast_exit(void);
bool wireless_pscan_adv_is_close(void);
void wireless_create_con_for_addr(uint8_t *addr, uint16_t timeout);
void wireless_con_channel_assess(uint8_t chidx, bool rx_ok);
bool wireless_con_user_cmd_tx_req(uint8_t index);        //请求发送用户私有命令，请求后库通过ble_con_user_cmd_get_tx_cb获取，返回值1代表成功，0代表失败
#define bt_get_link_info_nb()                   0
uint8_t wireless_con_get_status(void);
bool wireless_con_tws_is_slave(void);
void wireless_con_tws_int_set(uint8_t tws_int);
uint8_t wl_pscan_get_addr_for_index(uint8_t *con_addr, uint8_t index);
void wl_pscan_set_adv_ext_data(uint8_t *adv_ext_data, uint8_t adv_ext_len);
void load_code_wl_interphone(void);

//V3
uint16_t wl_single_link_duration_get(uint8_t con_type);
void wl_skip_first_set(uint8_t link, uint8_t index, uint8_t enable);
void wl_audio_pwr_ctr_set(uint8_t con_type, uint8_t idx, uint8_t pwr_level);
void wl_con_audio_stop(uint8_t index);
void wl_con_change_high_prio(uint8_t index);

#endif //_API_WIRELESS_H
