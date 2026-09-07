/*
 *  api_tws.h
 *
 *  Created by zoro on 2022-9-14.
 *
 *  Note:the file is copied from the btstack\btconfig path.
 */
#ifndef __API_TWS_H
#define __API_TWS_H


//TWS feature
#define TWS_FEAT_TSCO                   0x0001  //双耳通话
#define TWS_FEAT_TSCO_RING              0x0002  //同步铃声
#define TWS_FEAT_CODEC_MSBC             0x0004  //MSBC高清通话
#define TWS_FEAT_CODEC_AAC              0x0008  //AAC高清音频
#define TWS_FEAT_MS_SWITCH              0x0010  //主从切换
#define TWS_FEAT_NAME_MENU              0x0020  //蓝牙改名称菜单
#define TWS_FEAT_MS_BONDING             0x0040  //主从绑定
#define TWS_FEAT_PAIR_NOR_LINKINFO      0x0400  //TWS配对时判断是否存在手机信息，有信息的为主耳
#define TWS_FEAT_VERSION_MASK           0x0380  //版本号, bit7~9
#define TWS_FEAT_CODEC_HIRES            0x1000  //LHDC&LADC
#define TWS_FEAT_PUBLIC_ADDR            0x2000  //TWS连接后主耳使用公共地址连接手机
#define TWS_FEAT_SNATCH                 0x4000  //TWS一拖一和一拖二是否支持抢连

//TWS pair
#define TWS_PAIR_OP_MASK                0x0f
#define TWS_PAIR_OP_AUTO                0x00
#define TWS_PAIR_OP_API                 0x01
#define TWS_PAIR_MS_BONDING_MASK        0x20
#define TWS_PAIR_MS_BONDING             0x20
#define TWS_PAIR_MODE_MASK              0xc0
#define TWS_PAIR_MODE_BTID              0x40
#define TWS_PAIR_MODE_BTNAME            0x00

#define BT_TWS_SYNC_TO                  50      //BT同步等待，对齐到一个50ms，250ms超时


//param[0]=feat,index
enum
{
    FEAT_TWS_FLAG       = 0x80,
    FEAT_TWS_ROLE       = 0x40,
    FEAT_TWS_MUTE_FLAG  = 0x20,
    FEAT_TWS_FIRST_ROLE = 0x10,
    FEAT_INCOME_CON     = 0x08,
    FEAT_FIRST_CON      = 0x04,
    FEAT_INDEX_MASK     = 0x03,
};

//蓝牙通知
enum {
    BT_NOTICE_TWS_SEARCH_FAIL = 0x40,           //搜索TWS失败, param[0]=reason(0=timeout, 0x0B=conn_exists, 0x0C=cmd_disallow)
    BT_NOTICE_TWS_CONNECT_START,                //开始回连TWS, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_CONNECT_FAIL,                 //TWS回连失败, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_DISCONNECT,                   //TWS牙断开,   param[0]=feat,index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_LOSTCONNECT,                  //TWS连接丢失, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_CONNECTED,                    //TWS连接成功, param[0]=feat,index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_TWS_INIT_VOL,                     //TWS设置副耳初始音乐音量
    BT_NOTICE_TWS_HID_SHUTTER,                  //远端TWS拍照键
    BT_NOTICE_TWS_USER_KEY,                     //TWS自定义按键
    BT_NOTICE_TWS_SET_OPERATION,
    BT_NOTICE_TWS_STATUS_CHANGE,
    BT_NOTICE_TWS_ROLE_CHANGE,                  //主从角色变换
    BT_NOTICE_TWS_RES5,
    BT_NOTICE_TWS_RES6,
    BT_NOTICE_TWS_RES7,
    BT_NOTICE_TWS_WARNING,
    BT_NOTICE_TWS_SWITCH_FAIL,                  //主从切换失败
    BT_NOTICE_TWS_SWITCH_START,                 //主从切换开始
};

enum bt_msg_tws_t {
    TWS_SET_SCAN,
    TWS_SEARCH_SLAVE,
    TWS_USER_KEY,                               //用户自定义消息（主副互发）
    TWS_SYNC_INFO,                              //同步状态消息（主副同步）
    TWS_SET_STATUS,                             //tws_send_set_status
    TWS_RES_ADD,
    TWS_RES_DONE,
    TWS_ALARM_REQ,                              //同步ALARM请求（主发给副）
    TWS_DISCONNECT,
    TWS_CONNECT,
    TWS_SWITCH_ROLE,
    TWS_FOT_RESP,
};


//tws alarm
enum {
    ALARM_RES,
    ALARM_TONE,
    ALARM_VOL,
    ALARM_USER,
};

enum sync_info_enum {
    SYNC_INFO_SETTING,                  //VOL/LANG/EQ/BAT/...
    SYNC_INFO_LEDCNT,                   //同步 led cnt
    SYNC_INFO_EQPARAM,
    SYNC_INFO_FOT_DATA,
    SYNC_INFO_RING_STA,
    SYNC_INFO_CUSTOM_DATA,
};

//tws_status
enum
{
    TWS_STA_FLAG        = 0x80,
    TWS_STA_ROLE        = 0x40,
    TWS_STA_NOR_FLAG    = 0x01,
};


extern bool cfg_bt_tws_mode;
extern uint8_t cfg_bt_tws_pair_mode;
extern uint16_t cfg_bt_tws_feat;
extern uint8_t cfg_bt_tws_not_auto_connect;

//info
uint bt_tws_get_link_info(uint8_t *bd_addr);    //获取tws配对信息，返回值（0=未连接过TWS，0x80=作为master连过副耳，0x81=作为slave连接过主耳），被连地址放在bd_addr
void bt_tws_put_link_info_addr(uint8_t *bd_addr, uint8_t *link_key);
void bt_tws_put_link_info_feature(uint8_t *bd_addr, uint8_t feature);
void bt_tws_delete_link_info_with_tag(const char *tag, uint32_t ra_addr);      //带参数删除tws配对信息，参数会保存在flash，方便追溯
#define bt_tws_delete_link_info()               bt_tws_delete_link_info_with_tag("USER", (uint32_t)__builtin_return_address(0))     //删除tws配对信息

//tws api
uint8_t bt_tws_get_status(void);                //获取tws连接状态
uint8_t bt_tws_get_scan(void);                  //tws是否已打开可被搜索与连接
void bt_tws_set_scan(uint8_t scan_en);          //打开/关闭tws可被搜索与连接, bit0=可被发现, bit1=可被连接
void bt_tws_disconnect(void);                   //断开tws设备
void bt_tws_connect(void);                      //回连tws设备，连接成功返回BT_NOTICE_TWS_CONNECTED，连接失败返回BT_NOTICE_TWS_CONNECT_FAIL
void bt_tws_search_slave(uint timeout);         //搜索tws设备，搜索失败返回BT_NOTICE_TWS_SEARCH_FAIL，搜索成功并建立连接返回BT_NOTICE_TWS_CONNECTED
void bt_tws_unpair_device(void);                //删除tws配对信息并断开
uint8_t bt_tws_user_key(uint keycode);          //发送tws自定义按键, 16bit有效; 返回值：0=成功，1=失败，2=缓冲区满
void bt_tws_switch(void);                       //主从切换，例如通话时切主副MIC
bool bt_tws_need_switch(bool calling);          //查看是否需要切换
u8 bt_tws_get_force_role(void);
bool bt_tws_get_first_role(void);
bool bt_tws_get_link_rssi(int8_t *buffer, bool left_channel);
void bt_tws_report_dgb(void);

#define bt_is_tws_mode()                        cfg_bt_tws_mode
#define bt_tws_is_ms_switch()                   ((cfg_bt_tws_feat & TWS_FEAT_MS_SWITCH) != 0)
#define bt_tws_is_tsco()                        ((cfg_bt_tws_feat & TWS_FEAT_TSCO) != 0)
#define bt_tws_is_name_menu()                   ((cfg_bt_tws_feat & TWS_FEAT_NAME_MENU) != 0)
#define bt_tws_is_connected()                   (bool)(bt_tws_get_status() & TWS_STA_FLAG)
#define bt_tws_is_slave()                       (bool)(bt_tws_get_status() & TWS_STA_ROLE)

#define bt_tws_vol_change()
#define bt_tws_sync_setting()                   bt_tws_msg(TWS_SYNC_INFO, (SYNC_INFO_SETTING<<8))
#define bt_tws_sync_led()                       bt_tws_msg(TWS_SYNC_INFO, (SYNC_INFO_LEDCNT<<8))
#define bt_tws_sync_eq_param()                  bt_tws_msg(TWS_SYNC_INFO, (SYNC_INFO_EQPARAM<<8))
//#define bt_tws_sync_fot_data()                  bt_tws_msg(TWS_SYNC_INFO, (SYNC_INFO_FOT_DATA<<8))
#define bt_tws_sync_ring_sta()                  bt_tws_msg(TWS_SYNC_INFO, (SYNC_INFO_RING_STA<<8))
#define bt_tws_sync_custom_data()               bt_tws_msg(TWS_SYNC_INFO, (SYNC_INFO_CUSTOM_DATA<<8))

#define bt_tws_req_res(param)
#define bt_tws_res_add(param)                   bt_tws_msg(TWS_RES_ADD, param)
#define bt_tws_res_done(param)                  bt_tws_msg(TWS_RES_DONE, param)
#define bt_tws_req_alarm(param)                 bt_tws_msg(TWS_ALARM_REQ, param)
#define bt_tws_fot_resp()                       bt_tws_msg(TWS_FOT_RESP, 0)

#define bt_tws_req_alarm_res(res_idx)           bt_tws_req_alarm((ALARM_RES<<12)  | (0x0fff & (res_idx)))
#define bt_tws_req_alarm_tone(tone_idx)         bt_tws_req_alarm((ALARM_TONE<<12) | (0x0fff & (tone_idx)))
#define bt_tws_req_alarm_vol(vol, feat)         bt_tws_req_alarm((ALARM_VOL<<12)  | (0x00ff & (vol)) | ((0x000f & (feat))<<8))
#define bt_tws_req_alarm_user(value)            bt_tws_req_alarm((ALARM_USER<<12) | (0x0fff & (value)))


//--------------------------------------------------------------------------------------------
// TWS同步TICK用法
// a)tickn为本地时钟产生的1ms计数器，tick为tws同步后的1ms计数器
// b)tick是在tws连接上后同步得到的计数值，仅在tws连接状态有效
// c)tick用于tws之间发命令进行具有时间同步的操作
// d)作为发送方，用tws_time_get获取tickn和tickoff，计算出tick发送给对方，其中tickn用于本地执行
// e)作为接收方，收到tick后，用tws_time_get获取tickoff，计算出tickn，用于本地执行


//--------------------------------------------------------------------------------------------
#define TWS_TICK_PR                             (655360000/1000)    //TICK周期655360ms
#define TWS_TICK_MAX_DELAY                      (TWS_TICK_PR/2)         //TICK最大超时时间

#define TWS_TICK_ADD(tick_a, tick_b)            ((uint32_t)(((tick_b) + (tick_a)) % TWS_TICK_PR))               //tick_a + tick_b
#define TWS_TICK_SUB(tick_a, tick_b)            ((uint32_t)(((tick_a) + TWS_TICK_PR - (tick_b)) % TWS_TICK_PR)) //tick_a - tick_b
#define TWS_TICK_GEQ(tick_a, tick_b)            (TWS_TICK_SUB(tick_a, tick_b) < TWS_TICK_MAX_DELAY)             //tick_a >= tick_b

#define TWS_TICK_2_TICKN(tick, tickoff)         TWS_TICK_SUB(tick, tickoff)
#define TWS_TICKN_2_TICK(tickn, tickoff)        TWS_TICK_ADD(tickn, tickoff)

typedef struct {
    uint32_t tickn;             //本地tick
    uint32_t tickoff;           //偏移量，tickoff = TICK_SUB(sync_time, tickn)
} tws_time_t;

uint32_t tws_time_tickn_get(void);                          //获取本地tickn
bool tws_time_tickn_expire(uint32_t tickn);                 //查询tickn时刻是否到达

uint32_t tws_time_get(tws_time_t *time, uint32_t delay);    //获取tws时间，同时获取tickoff
                                                            //*time返回time->clkn = curr.clkn + delay, 返回值为time->clkn转换的tick
                                                            //注意：delay大于TICK_MAX_DELAY会影响tws_time_tickn_expire，尽量小于TICK_MAX_DELAY
uint32_t tws_time_tick2n(uint32_t tick);                    //将tws同步tick转换为本地tickn
uint32_t tws_time_n2tick(uint32_t tickn);                   //将tws本地tickn转换为同步tick


bool tws_alarm_add(uint32_t tickn, uint32_t params);
bool tws_alarm_is_full(void);

#endif // __API_TWS_H
