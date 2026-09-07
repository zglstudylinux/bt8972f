/*
 *  api_btstack.h
 *
 *  Created by zoro on 2017-8-24.
 *
 *  Note:the file is copied from the btstack\btconfig path.
 */
#ifndef _API_BTSTACK_H
#define _API_BTSTACK_H


//----------------------------------------------------------------------------
//工作模式
#define MODE_NORMAL                     0       //正常连接模式，关闭DUT测试
#define MODE_NORMAL_DUT                 1       //正常连接模式，使能DUT测试
#define MODE_FCC_TEST                   2       //FCC测试模式，通过串口通信
#define MODE_BQB_RF_BREDR               3       //BR/EDR DUT测试模式，通过蓝牙通信
#define MODE_BQB_RF_BLE                 4       //BLE DUT测试模式，通过串口通信
#define MODE_IODM_TEST                  5       //IODM测试模式，通过串口通信

//FCC测试类型
#define FCC_TYPE_MODE_UNDEFINE          0
#define FCC_TYPE_MODE_BR                1
#define FCC_TYPE_MODE_EDR               2
#define FCC_TYPE_MODE_BLE1M             4
#define FCC_TYPE_MODE_BLE2M             8

extern uint8_t cfg_bt_work_mode;
extern bool cfg_bt_dual_mode;
extern uint8_t cfg_bt_rf_def_txpwr;
extern uint8_t cfg_bt_page_txpwr;
extern uint8_t cfg_bt_inq_txpwr;
extern uint8_t cfg_ble_page_txpwr;
extern uint8_t cfg_ble_page_rssi_thr;
extern const uint16_t link_info_page_size;
extern bool cfg_bt_2acl_adjust_ble_interval;
extern uint8_t cfg_wireless_tx_interval;
extern uint8_t cfg_wireless_con_interval;
extern uint8_t cfg_wireless_tx_retry;
extern uint8_t cfg_discon_auto_pwroff;
extern uint8_t cfg_bb_rf_freq_bands;
extern uint8_t cfg_wireless_codec[2];

//蓝牙消息
enum bt_msg_t {
    BT_MSG_CTRL                 = 0,            //蓝牙控制消息
    BT_MSG_BLE,                                 //BLE消息
    BT_MSG_TWS,                                 //TWS消息
    BT_MSG_ADV0,
    BT_MSG_A2DP,                                //A2DP消息
    BT_MSG_HFP,                                 //HFP消息
    BT_MSG_HSP,                                 //HSP消息
    BT_MSG_HID,                                 //HID消息，KEYPAD按键/CONSUMER按键/触摸屏
    BT_MSG_PBAP,                                //PBAP消息
    BT_MSG_MAP,                                 //MAP消息
    BT_MSG_WIRELESS,
    BT_MSG_MAX,
    BT_MSG_RES                  = 0xf0,         //0xf0~0xff保留给传参较多的api
};

void bt_send_msg_do(uint msg);
#define BT_MSG_INVL_PARAM                       0xffff
#define bt_send_msg(ogf, ocf)                   bt_send_msg_do((ogf<<24) | (ocf))
#define bt_ctrl_msg(msg, param)                 bt_send_msg(BT_MSG_CTRL, (msg<<16) | (u16)param)
#define bt_ctrl0_msg(msg)                       bt_send_msg(BT_MSG_CTRL, (BT_CTRL0<<16) | msg)
#define bt_tws_msg(msg, param)                  bt_send_msg(BT_MSG_TWS, (msg<<16) | (u16)param)
#define bt_ble_msg(msg, param)                  bt_send_msg(BT_MSG_BLE, (msg<<16) | (u16)param)
#define bt_a2dp_msg(msg, param)                 bt_send_msg(BT_MSG_A2DP, (msg<<16) | (u16)param)
#define bt_hfp_msg(msg, param)                  bt_send_msg(BT_MSG_HFP, (msg<<16) | (u16)param)
#define bt_hid_msg(msg, param)                  bt_send_msg(BT_MSG_HID, (msg<<16) | (u16)param)
#define bt_pbap_msg(msg, param)                 bt_send_msg(BT_MSG_PBAP, (msg<<16) | (u16)param)

//control
void bb_run_loop(void);
void bt_fcc_init(void);
void bt_init(void);                             //初始化蓝牙变量
int bt_setup(void);                             //打开蓝牙模块
void bt_off(void);                              //关闭蓝牙模块
void bt_wakeup(void);                           //唤醒蓝牙模块
void bt_start_work(uint8_t opcode, uint8_t scan_en); //蓝牙开始工作，opcode: 0=回连, 1=不回连
void bt_send_msg_do(uint msg);                  //蓝牙控制消息，参数详见bt_msg_t
void bt_thread_check_trigger(void);

#define bt_is_iodm_test()               (cfg_bt_work_mode == MODE_IODM_TEST)
#define bt_is_fcc_test()                (cfg_bt_work_mode == MODE_FCC_TEST)
#define bt_is_cbt_test()                (cfg_bt_work_mode == MODE_NORMAL_DUT)
#define bt_is_bqb_rf_bredr()            (cfg_bt_work_mode == MODE_BQB_RF_BREDR)
#define bt_is_bqb_rf_ble()              (cfg_bt_work_mode == MODE_BQB_RF_BLE)
#define bt_is_dut_en()                  (bt_is_cbt_test() || bt_is_bqb_rf_bredr())

//rf
enum {
    TEST_TYPE_TONE          = 0,
    TEST_TYPE_BR,
    TEST_TYPE_EDR,
    TEST_TYPE_RSVD,
    TEST_TYPE_LE_1M,
    TEST_TYPE_LE_2M,
    TEST_TYPE_LE_CODED_S8,
    TEST_TYPE_LE_CODED_S2,
};
void rf_test_ctrl_msg(void);                    //发消息给蓝牙线程后通过rf_test_get_ctrl_cmd_cb获取命令
uint8_t *rf_test_get_ctrl_cmd_cb(void);         //蓝牙线程获取test_ctrl命令的回调函数
uint8_t rf_test_get_txpwr_dec_cb(uint8_t test_type);    //蓝牙线程获取测试功率衰减的回调函数


//----------------------------------------------------------------------------
//pkt
typedef void (*kick_func_t)(void);
struct ring_buf_tag;

struct txbuf_tag {
    uint8_t *ptr;
    uint16_t len;
    uint16_t handle;
} __attribute__ ((packed)) ;

struct txpkt_tag
{
    struct txbuf_tag txbuf;
    struct ring_buf_tag *pool;
    kick_func_t send_kick;
    uint16_t mtu;
};

void txpkt_init(struct txpkt_tag *txpkt, uint8_t *txbuf, void *pool, uint16_t mtu, kick_func_t kick_func);

//----------------------------------------------------------------------------
//bb_utils
uint8_t bb_crc8(const uint8_t *data, size_t len);

//----------------------------------------------------------------------------
//GAP
typedef enum {
	GAP_CONNECTION_INVALID,
	GAP_CONNECTION_ACL,
	GAP_CONNECTION_SCO,
	GAP_CONNECTION_LE
} gap_connection_type_t;

gap_connection_type_t gap_get_connection_type(uint16_t connection_handle);

//----------------------------------------------------------------------------
//GATT 服务相关
/**
   GATTS ERR return
*/
#define GATTS_SUCCESS                                       0x00
#define GATTS_SRVC_TYPE_ERR                                 0x01
#define GATTS_SRVC_RULES_ERR                                0x02
#define GATTS_SRVC_PROPS_ERR                                0x03
#define GATTS_SRVC_ATT_FULL_ERR                             0x04
#define GATTS_SRVC_PROFILE_FULL_ERR                         0x05


/**
    att property flag
*/
#define ATT_BROADCAST                                           0x01
#define ATT_READ                                                0x02
#define ATT_WRITE_WITHOUT_RESPONSE                              0x04
#define ATT_WRITE                                               0x08
#define ATT_NOTIFY                                              0x10
#define ATT_INDICATE                                            0x20
#define ATT_AUTHENTICATED_SIGNED_WRITE                          0x40
#define ATT_EXTENDED_PROPERTIES                                 0x80
#define ATT_DYNAMIC                                             0x100
#define ATT_UUID128                                             0x200
#define ATT_AUTHENTICATION_REQUIRED                             0x400
#define ATT_AUTHORIZATION_REQUIRED                              0x800


/**
    define group GATT Server Service Types
*/
typedef enum {
    GATTS_SRVC_TYPE_PRIMARY  =   0x00,                //Primary Service
    GATTS_SRVC_TYPE_SEVONDARY=   0x01,                //Secondary Service
    GATTS_SRVC_TYPE_INCLUDE  =   0x02,                //include Type
}gatts_service_type;

/**
    define group GATT Server UUID Types
*/
typedef enum {
    GATTS_UUID_TYPE_16BIT  =   0x00,                //UUID 16BIT
    GATTS_UUID_TYPE_128BIT =   0x01,                //UUID 128BIT
}gatts_uuid_type;

/**
    define GATT handler callback
    con_handle: connect handle
    handle    : attribute handle
    flag      : call back flag
*/
#define ATT_CB_FALG_OFFSET         (0xffff)
#define ATT_CB_FALG_DIR_WRITE      (0x10000)
typedef int (*gatt_callback_func)(uint16_t con_handle, uint16_t handle, uint32_t flag, uint8_t *ptr, uint16_t len);

/**
    define GATT service base
*/
typedef struct gatts_service_base{
    uint16_t handle;
}gatts_service_base_st;


/**
    define GATT service uuid
*/
typedef struct gatts_uuid_base{
    uint16_t props;
    uint8_t type;
    const uint8_t *uuid;
}gatts_uuid_base_st;
/**
    define GATT service call back info
*/
typedef struct {
    uint16_t  client_config; // att config  1:notify enable  2:indicate enable
    uint16_t value_len;
    gatt_callback_func att_callback_func;
    uint8_t *value;         //read return data when att_callback_func is NULL
} gatt_characteristic_cb_info_t;

#define GATT_CLIENT_CONFIG_NOTIFY       1
#define GATT_CLIENT_CONFIG_INDICATE     2

/**
    define group GATT Server profile struct
*/
typedef struct gatts_profile_list{
    void *item;
    uint16_t  profile_start_handle;
    uint16_t  profile_end_handle;
    const uint8_t   *profile_date;
    uint16_t profile_date_size;
}gatts_profile_list_st;

//init gatt
//profile_table: profile cache buf
//cb_info_table_p: call back info cache
void gatts_init(uint8_t *profile_table, uint16_t profile_table_size,
                    gatt_characteristic_cb_info_t **cb_info_table_p,
                    uint16_t gatt_max_att);
int gatts_service_add(gatts_service_type service_type, const uint8_t *service_uuid, gatts_uuid_type uuid_type, uint16_t *service_handle);
int gatts_characteristic_add( const uint8_t *att_uuid, gatts_uuid_type uuid_type, uint16_t props,
                                          uint16_t *att_handle, gatt_characteristic_cb_info_t *cb_info);
bool gatt_init_att_info(uint16_t att_handle, gatt_characteristic_cb_info_t *att_cb_info);
int gatts_inlcude_service_add(gatts_uuid_type uuid_type, const uint8_t *service_uuid, uint16_t start_handle, uint16_t end_handle, uint16_t *att_handle);
int gatts_add_profile_date(gatts_profile_list_st *gatts_profile);
int gatts_attribute_add(const uint8_t *att_data, uint16_t *att_handle);
bool gatts_profile_mg_alloc_att_num_check(uint8_t att_num);


#endif //_API_BTSTACK_H
