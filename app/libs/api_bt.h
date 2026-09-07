/*
 *  api_btstack.h
 *
 *  Created by zoro on 2024-12-13.
 *
 *  Note:the file is copied from the btstack\btconfig path.
 */
#ifndef _API_BT_H
#define _API_BT_H


//----------------------------------------------------------------------------
//蓝牙功能
#define PROF_A2DP                       0x0007  //蓝牙音乐功能
#define PROF_HID                        0x0018  //蓝牙键盘功能
#define PROF_HFP                        0x0060  //蓝牙通话功能
#define PROF_SPP                        0x0080  //蓝牙串口功能
#define PROF_PBAP                       0x0100  //蓝牙电话本功能
#define PROF_HSP                        0x0200
#define PROF_MAP                        0x0400  //蓝牙短信功能
#define PROF_GATT                       0x0800  //GATT over BREDR功能

//蓝牙特性
#define HFP_BAT_REPORT                  0x01    //是否支持手机电量显示
#define HFP_3WAY_CALL                   0x02    //是否支持三通电话
#define HFP_INBAND_RING_TONE            0x04    //是否支持手机来电铃声
#define HFP_CALL_PRIVATE                0x08    //是否打开强制私密接听
#define HFP_SIRI_CMD                    0x10    //是否打开siri控制命令
#define HFP_EC_AND_NR                   0x20    //是否打开手机端回音和降噪
#define HFP_RING_NUMBER_EN              0x40    //是否支持来电报号
#define A2DP_AVRCP_VOL_CTRL             0x01    //是否支持手机音量控制同步
#define A2DP_AVRCP_PLAY_STATUS_IOS      0x02    //是否支持IOS手机播放状态同步，可加快播放暂停响应速度，蓝牙后台建议打开；注意：打开后微信小视频会无声
#define A2DP_AVRCP_PLAY_STATUS          0x04    //是否支持手机播放状态同步，可加快播放暂停响应速度，蓝牙后台建议打开；
#define A2DP_RESTORE_PLAYING            0x08    //是否支持掉线回连后恢复播放
#define A2DP_AVDTP_DELAY_REPORT         0x10    //是否支持AVDTP delay report功能
#define A2DP_AVDTP_DYN_LATENCY          0x20    //是否支持动态延迟控制功能
#define A2DP_AVDTP_EXCEPT_REST_PLAY     0x40    //是否支持异常复位后恢复连接和播放状态功能
//a2dp特性扩展1
#define A2DP_AVRCP_RECORD_DEVICE_VOL    0x01    //分别记录不同连接设备的音量
#define A2DP_RESET_DEVICE_VOL           0x02    //不支持音量同步手机，连接恢复音量
#define A2DP_PLAY_SEQN_OPT              0x04    //是否支持不连续seqn pkt播放
//hfp特性扩展
#define HFP_RECORD_DEVICE_VOL           0x01    //分别记录不同连接设备的音量

//SPP rfcomm通道号
#define SPP_RFCOMM_SERVER_CHANNEL0      1
#define SPP_RFCOMM_SERVER_CHANNEL1      5
#define SPP_RFCOMM_SERVER_CHANNEL2      6

//蓝牙编解码
#define CODEC_SBC                       0x01
#define CODEC_AAC                       0x02
#define CODEC_MSBC                      0x04
#define CODEC_PLC                       0x08
#define CODEC_LHDCV5                    0x10
#define CODEC_LHDCV4                    0x20
#define CODEC_LDAC                      0x40

//一拖二特性
#define BT_2ACL_SCAN_ALWAYS             0x0001 	//连接第一个设备继续打开可被发现
#define BT_2ACL_RECONN_ODEV             0x0002  //只回连最后一次连接的手机
#define BT_2ACL_SNATCH                  0x0004  //是否支持一拖一抢连和一拖二抢连

#define SCO_STATUS_IDLE             0x00
#define SCO_STATUS_INIT             0x01
#define SCO_STATUS_W4_INIT          0x80
#define SCO_STATUS_W4_EXIT          0x81


//----------------------------------------------------------------------------
//蓝牙状态
enum {
    BT_STA_OFF,                                 //蓝牙模块已关闭
    BT_STA_INITING,                             //初始化中
    BT_STA_IDLE,                                //蓝牙模块打开，未连接

    BT_STA_SCANNING,                            //扫描中
    BT_STA_DISCONNECTING,                       //断开中
    BT_STA_CONNECTING,                          //连接中

    BT_STA_CONNECTED,                           //已连接
    BT_STA_PLAYING,                             //播放
    BT_STA_INCOMING,                            //来电响铃
    BT_STA_OUTGOING,                            //正在呼出
    BT_STA_INCALL,                              //通话中
    BT_STA_OTA,                                 //OTA升级中
};

//通话状态
enum {
    BT_CALL_IDLE,                               //
    BT_CALL_INCOMING,                           //来电响铃
    BT_CALL_OUTGOING,                           //正在呼出
    BT_CALL_ACTIVE,                             //通话中
    BT_CALL_3WAY_CALL,                          //三通电话或两部手机通话
};

//蓝牙通知
enum {
    BT_NOTICE_INIT_FINISH,                      //蓝牙初始化完成
    BT_NOTICE_CONNECT_START,                    //开始回连手机, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_CONNECT_FAIL,                     //回连手机失败, param[0]=reason, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_DISCONNECT,                       //蓝牙断开,     param[0]=feat,index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_LOSTCONNECT,                      //蓝牙连接丢失, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_CONNECTED,                        //蓝牙连接成功, param[0]=feat,index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_SCO_SETUP,
    BT_NOTICE_SCO_FAIL,
    BT_NOTICE_SCO_KILL,
    BT_NOTICE_INCOMING,                         //来电
    BT_NOTICE_RING,                             //来电响铃
    BT_NOTICE_OUTGOING,                         //去电
    BT_NOTICE_CALL_NUMBER,                      //来电/去电号码
    BT_NOTICE_CALL,                             //建立通话
    BT_NOTICE_SET_SPK_GAIN,                     //设置通话音量
    BT_NOTICE_CALL_CHANGE_DEV,                  //1拖2时改变了通话设备
    BT_NOTICE_MUSIC_PLAY,                       //蓝牙音乐开始播放
    BT_NOTICE_MUSIC_STOP,                       //蓝牙音乐停止播放
    BT_NOTICE_MUSIC_CHANGE_VOL,                 //手机端改变蓝牙音乐音量, param[0]=down/up, param[1]=index, param[7:2]=bd_addr
    BT_NOTICE_MUSIC_SET_VOL,                    //手机端设置蓝牙音乐音量, param[0]=a2dp_vol, param[1]=index, param[7:2]=bd_addr
    BT_NOTICE_MUSIC_CHANGE_DEV,                 //1拖2时改变了播放设备, 例如从A手机切换到B手机, param[0]=a2dp_vol, param[1]=index, param[7:2]=bd_addr
    BT_NOTICE_HID_CONN_EVT,                     //HID服务连接事件
    BT_NOTICE_RECON_FINISH,                     //回连完成, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_ABORT_STATUS,                     //中止状态, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_NORCONNECT_FAIL,                  //手机发起连接到一半失败, param[0]=status, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_LOW_LATENCY_STA,                  //低延时状态切换

    BT_NOTICE_PBAP_CONNECTED,
    BT_NOTICE_PBAP_GET_PHONEBOOK_SIZE_COMPLETE, //获取电话薄数量完成
    BT_NOTICE_PBAP_PULL_PHONEBOOK_COMPLETE,     //获取电话薄完成
    BT_NOTICE_PBAP_DISCONNECT,

    BT_NOTICE_SNATCH_CON,                       //抢连

    BT_NOTICE_WIRELESS_CONNECTED = 0xc0,        //无线mic连接成功, param[0]=index, param[1]=reason, param[7:2]=bd_addr, param[8]=con_role
    BT_NOTICE_WIRELESS_DISCONNECT,              //无线mic断开连接, param[0]=index, param[1]=reason, param[7:2]=bd_addr
    BT_NOTICE_WIRELESS_CONNECT_FAIL,            //无线mic连接失败, param[0]=index, param[1]=reason, param[7:2]=bd_addr
};

//控制消息
enum bt_msg_ctrl_t {
    BT_CTRL0,                                    //无传参的消息,需要封装API
    BT_CTRL_START_WORK,                         //蓝牙开始工作
    BT_CTRL_SET_SCAN,                           //设置可被发现/可被连接
    BT_CTRL_CONNECT,                            //连接蓝牙
    BT_CTRL_DISCONNECT,                         //断开蓝牙
    BT_CTRL_ABORT_RECONNECT,                    //中止回连
    BT_CTRL_SET_SCAN_INTV,                      //设置PAGE SCAN PARAM
    BT_CTRL_ABORT_PAGE_PSCAN,                   //中止PAGE和PAGESCAN
    BT_CTRL_ADD_VOL_SET_QUEUE,                  //添加音量到设置队列中
    BT_CTRL_SET_LINK_MAX,                       //设置最大连接数量（一拖二开关）

    BT_CTRL_RELEASE,
    BT_CTRL_CONNECT_ADDRESS,
    BT_CTRL_DISCONNECT_ADDRESS,

    BT_CTRL_SPP_SEND,
    BT_CTRL_SPP_DISCONNECT,
    BT_CTRL_LATT_SEND,
    BT_CTRL_AAP_EAR_STA_CHANGE,
    BT_CTRL_AAP_NR_STA_CHANGE,
    BT_CTRL_AAP_USER_DATA,
    BT_CTRL_VA_ATT_MSG,

    BT_CTRL_MAX,
    BT_CTRL_NO                       = 0xffffff,
};

enum bt_msg_a2dp_t {
    BT_A2DP_CTRL,
    BT_A2DP_VOL_CHANGE,                          //音量调整，之后通过回调函数设置音量
    BT_A2DP_VOL_ADJ,                             //加减音量，之后通过回调函数调节音量
    BT_A2DP_PLAY_PAUSE,                          //切换播放、暂停
    BT_A2DP_2ACL_PALY_SWITCH,                    //一拖二播放切换
    BT_A2DP_LOW_LATENCY_SET,
    BT_A2DP_ID3_GET_ELE_ATT,                     //ID3获取歌曲信息（需打开ID3功能）
};

enum bt_msg_hfp_t {
    BT_HFP_CALL_REDIAL,                         //回拨电话（最后一次通话）
    BT_HFP_CALL_REDIAL_NUMBER,                  //回拨电话（从hfp_get_outgoing_number获取号码）
    BT_HFP_CALL_ANSWER_INCOM,                   //接听来电（三通时挂起当前通话）
    BT_HFP_CALL_ANSWER_INCOM_REJ_OTHER,         //接听来电（三通时挂断当前通话,一拖二时挂断当前链路通话）
    BT_HFP_CALL_ANSWER_INCOM_HOLD_OTHER,        //接听来电（三通时挂起当前通话,一拖二时挂起当前链路通话）
    BT_HFP_CALL_TERMINATE,                      //挂断通话或来电
    BT_HFP_CALL_SWAP,                           //切换三通电话
    BT_HFP_CALL_PRIVATE_SWITCH,                 //切换私密通话
    BT_HFP_CALL_VOL_CHANGE,                     //音量调整，之后通过回调函数设置音量
    BT_HFP_CALL_VOL_ADJ,                        //加减音量，之后通过回调函数调节音量
    BT_HFP_REPORT_BAT,                      //报告电池电量
    BT_HFP_MIC_GAIN,                        //设置通话麦克风音量
    BT_HFP_AT_CMD,                          //发送AT命令（从hfp_get_at_cmd获取命令）
    BT_HFP_SIRI_SW,                         //唤出/关闭siri
    BT_HFP_CUSTOM_AT_CMD,
    BT_HFP_RES3,
};

enum bt_msg_hid_t {
    HID_CONNECT,
    HID_DISCONNECT,
    HID_KEYBOARD,
    HID_CONSUMER,
    HID_TOUCH_SCREEN,
    HID_MOUSE,
    HID_SEND,
};

enum bt_msg_pbap_t {
    BT_PBAP_CTRL,
    BT_PBAP_SELECT_PHONEBOOK,
    BT_PBAP_GET_PHONEBOOK_SIZE,
    BT_PBAP_PULL_PHONEBOOK,
};


enum a2dp_ctrl_cmd_t {
    A2DP_VOLUME_UP           = 0x41,     //音量加
    A2DP_VOLUME_DOWN         = 0x42,     //音量减
    A2DP_MUTE                = 0x43,     //MUTE
    A2DP_PLAY                = 0x44,     //播放
    A2DP_STOP                = 0x45,     //停止
    A2DP_PAUSE               = 0x46,     //暂停
    A2DP_RECORD              = 0x47,
    A2DP_REWIND              = 0x48,     //快退
    A2DP_FAST_FORWARD        = 0x49,     //快进
    A2DP_EJECT               = 0x4a,
    A2DP_FORWARD             = 0x4b,     //下一曲
    A2DP_BACKWARD            = 0x4c,     //上一曲
    A2DP_REWIND_END          = 0xc8,     //结束快退
    A2DP_FAST_FORWARD_END    = 0xc9,     //结束快进
};


extern uint8_t cfg_bt_max_acl_link;
extern uint8_t cfg_bt_scan_ctrl_mode;
extern bool cfg_bt_simple_pair_mode;
extern uint16_t cfg_bt_support_profile;
extern uint16_t cfg_bt_support_codec;
extern uint8_t cfg_bt_hid_type;
extern const uint8_t cfg_bt_spp_rfcomm_server_ch0;
extern const uint8_t cfg_bt_spp_rfcomm_server_ch1;
extern const uint8_t cfg_bt_spp_rfcomm_server_ch2;
extern uint8_t cfg_bt_connect_times;
extern uint8_t cfg_bt_pwrup_connect_times;
extern uint16_t cfg_bt_sup_to_connect_times;
extern uint8_t cfg_bt_a2dp_feature;
extern uint8_t cfg_bt_a2dp_feature1;
extern uint8_t cfg_bt_hfp_feature;
extern uint8_t cfg_bt_hfp_feature1;
extern uint8_t cfg_bt_hci_disc_only_spp;
extern const uint8_t cfg_bb_bt_opt;
#define BT_OPT_AFH_UNKNOW_DIS               BIT(0)
#define BT_OPT_SCO_ACK_EN                   BIT(1)

void bt_audio_bypass(void);                     //蓝牙A2DP/SCO通路关闭，必须与bt_audio_enable成对使用
void bt_audio_enable(void);                     //蓝牙A2DP/SCO通路使能，bypass n次后需要enable n次才能出声
void bt_get_stack_local_name(char* name);
void bt_set_stack_local_name(const char* name);
void bt_set_sco_far_delay(void *buf, uint size, uint delay);


//蓝牙连接
void bt_set_scan(uint8_t scan_en);              //打开/关闭可被发现和可被连接, bit0=可被发现, bit1=可被连接
uint8_t bt_get_scan(void);                      //获取设置的可被发现可被连接状态（已连接时设置完不会立即生效，需要等断开连接）
uint8_t bt_get_curr_scan(void);                 //获取当前可被发现可被连接状态
void bt_set_link_max(uint8_t max);              //设置最大连接数量，1=允许连接1个设备，2=允许连接2个设备
void bt_connect(void);                          //蓝牙设备回连, 回连次数在cfg_bt_connect_times配置
void bt_disconnect(void);                       //蓝牙设备断开
void bt_connect_address(void);                  //蓝牙设备回连地址, 回连地址在bt_get_connect_addr函数设置
void bt_disconnect_address(void);               //蓝牙设备断开地址, 断开地址在bt_get_disconnect_addr函数设置
void bt_release(uint reason);                   //蓝牙设备断开, reason: 0=单独断开（入仓）; 1=断开并同步关机（按键/自动关机）;用户单独调用断开，并不关机reason=0xff
#define bt_is_scan_ctrl()                       cfg_bt_scan_ctrl_mode
#define bt_scan_enable()                        bt_set_scan(0x03)                       //打开扫描
#define bt_scan_disable()                       bt_set_scan(0x00)                       //关闭扫描
#define bt_set_scan_param(ps_intv,is_intv)      bt_ctrl_msg(BT_CTRL_SET_SCAN_INTV, (ps_intv << 8) | is_intv)  //设置scan 参数，0x100*(intv+1)
#define bt_abort_reconnect()                    bt_ctrl_msg(BT_CTRL_ABORT_RECONNECT, BT_MSG_INVL_PARAM)        //终止回接
#define bt_abort_reconnect_silence(feat)        bt_ctrl_msg(BT_CTRL_ABORT_RECONNECT, (u16)feat)     //终止回接，没有消息回调。feat:0=手机, BT_FEAT_TWS=TWS


//status
void bt_dump_status(void);
uint bt_get_disp_status(void);                  //获取蓝牙的当前显示状态, V060
uint bt_get_status(void);                       //获取蓝牙的当前状态
uint8_t bt_get_scan(void);                      //判断当前可被连接可被扫描状态
uint8_t bt_get_curr_scan(void);                 //获取实时可被连接可被扫描状态
uint bt_get_call_indicate(void);                //获取通话的当前状态
uint bt_get_siri_status(void);                  //获取SIRI当前状态, 0=SIRI已退出, 1=SIRI已唤出
bool bt_is_calling(void);                       //判断是否正在通话
bool bt_is_playing(void);                       //判断是否正在播放
bool bt_is_tbox_testing(void);                  //判断当前蓝牙是否处于测试盒测试状态
#define bt_is_testmode()        bt_is_tbox_testing()
bool bt_is_dut_testing(void);                   //判断当前蓝牙是否处于DUT测试状态
bool bt_is_sleep(void);                         //判断蓝牙是否进入休眠状态
bool bt_is_allow_sleep(void);                   //判断蓝牙是否允许进入休眠状态
bool bt_is_connected(void);                     //判断蓝牙是否已连接（TWS副耳被连接，或主耳与手机已连接）
bool bt_is_ios_device(void);                    //判断当前播放的是否ios设备
bool bt_is_support_vol_ctrl(void);              //判断当前播放的是否支持音量同步
uint32_t bt_sleep_proc(void);
void bt_enter_sleep(void);
void bt_exit_sleep(void);
void bt_updata_local_name(char *bt_name);
uint8_t bt_sco_get_status(void);
bool bt_sco_is_connected(void);
bool bt_sco_is_msbc(void);                      //判断当前通话是否是宽带通话
bool bt_a2dp_is_bypass(void);
bool bt_is_low_latency(void);                   //判断蓝牙是否在低延时状态
bool bt_is_silence(void);
bool bt_decode_is_aac(void);                    //判断蓝牙解码是否是aac
bool bt_decode_is_lhdc(void);                   //判断蓝牙解码是否是LHDC
bool bt_decode_is_ldac(void);                   //判断蓝牙解码是否是LDAC
uint8_t bt_get_connected_num(void);             //一拖二获取当前连接了几台设备
uint8_t bt_get_cur_a2dp_media_index(void);      //一拖二获取当前播放设备的index
u8 bt_call_get_ring_index(void);                //一拖二获取当前响铃设备的index
u8 bt_get_connect_status(u8 *bd_addr);          //获取当前地址设备的连接状态，已连接返回BT_STA_CONNECTED，连接中BT_STA_CONNECTING，未连接BT_STA_DISCONNECTING

//info
//删除配对信息TAG
#define BT_INFO_TAG_NOR                 "NULL"   //删除手机
#define BT_INFO_TAG_UNPAIR              "UPAR"   //删除TWS，并断开
#define BT_INFO_TAG_QTEST               "QTET"   //快测删除TWS配对信息
#define BT_INFO_TAG_IODM                "IODM"   //IODM删除TWS配对信息
#define BT_INFO_TAG_CHARGE_BOX          "CBOX"   //充电仓删除TWS配对信息
#define BT_INFO_TAG_USER                "USER"   //用户删除TWS配对信息

bool bt_get_link_btname(uint8_t index, char *name, uint8_t max_size);   //index: 0=link0, 1=link1, 0xff=auto(default link0)
bool bt_get_link_btaddr(uint8_t index, u8 *addr);                       //index: 0=link0, 1=link1, 0xff=auto(default link0)
uint8_t bt_get_link_index_for_handle(uint16_t con_handle);              //根据连接handle获取index，返回值0=link0, 1=link1, 0xff获取失败
void bt_get_local_bd_addr(u8 *addr);
bool bt_get_link_info_addr(uint8_t *bd_addr);    //获取手机配对信息，bd_addr=NULL时仅查询是否存在回连信息
bool bt_get_link_info_addr_for_order(uint8_t *bd_addr, uint8_t order);    //获取第n个手机配对信息，bd_addr=NULL时仅查询是否存在回连信息
bool bt_get_link_info_name(uint8_t *bd_addr, char *name, uint8_t max_size); //获取配对信息中的蓝牙名
int bt_reorder_for_addr(uint8_t *bd_addr);  //根据地址把设备的回连顺序调整到最前面
void bt_delete_link_info_for_addr(uint8_t *bd_addr); //根据地址删除配对信息
void bt_delete_link_info(void);             //删除手机配对信息
void bt_unpair_device(void);                //删除手机配对信息并断开

//
#define bt_ctl_nr_sta_change()
void bt_update_ear_sta(void);                   //更新耳机入耳状态
void bt_set_ear_sta(bool ear_sta);              //设置耳机入耳状态

//----------------------------------------------------------------------------
//sdp
uint8_t sdp_add_service(void *item);

//hfp
typedef struct {
    uint8_t callsetup;
    uint8_t callheld;
    uint8_t call;
    uint8_t ccwa;
} hfp_indicators_status_t;

void hfp_hf_init(void);
uint bt_get_hfp_feature(void);

//hsp
void hsp_hs_init(void);
void hsp_hs_init_var(void);
void bt_hsp_call_switch(void);                  //挂断/接听
void bt_hsp_sco_conn(void);                     //建立HSP SCO连接
void bt_hsp_sco_disconn(void);                  //断开HSP SCO连接

//蓝牙通话控制
#define bt_call_redial_last_number()            bt_hfp_msg(BT_HFP_CALL_REDIAL,BT_MSG_INVL_PARAM)         //电话回拨（最后一次通话）
#define bt_call_answer_incoming()               bt_hfp_msg(BT_HFP_CALL_ANSWER_INCOM,BT_MSG_INVL_PARAM)   //接听电话，三通时挂起当前通话
#define bt_call_answer_incom_rej_other()        bt_hfp_msg(BT_HFP_CALL_ANSWER_INCOM_REJ_OTHER,BT_MSG_INVL_PARAM)     //接听电话，三通时挂断当前通话，1拖2时挂断当前的手机通话
#define bt_call_answer_incom_hold_other()       bt_hfp_msg(BT_HFP_CALL_ANSWER_INCOM_HOLD_OTHER,BT_MSG_INVL_PARAM)    //接听电话，三通时挂起当前通话，1拖2时挂起当前的手机通话
#define bt_call_terminate()                     bt_hfp_msg(BT_HFP_CALL_TERMINATE,BT_MSG_INVL_PARAM)      //挂断电话
#define bt_call_swap()                          bt_hfp_msg(BT_HFP_CALL_SWAP,BT_MSG_INVL_PARAM)           //切换三通电话
#define bt_call_private_switch()                bt_hfp_msg(BT_HFP_CALL_PRIVATE_SWITCH,BT_MSG_INVL_PARAM) //切换私密通话
#define bt_call_redial_number()                 bt_hfp_msg(BT_HFP_CALL_REDIAL_NUMBER,BT_MSG_INVL_PARAM)
#define bt_hfp_siri_switch()                    bt_hfp_msg(BT_HFP_SIRI_SW,BT_MSG_INVL_PARAM)         //开关SIRI, android需要在语音助手中打开“蓝牙耳机按键启动”, ios需要打开siri功能
#define bt_hfp_report_bat()                     bt_hfp_msg(BT_HFP_REPORT_BAT,BT_MSG_INVL_PARAM)
#define bt_hfp_send_at_cmd()                    bt_hfp_msg(BT_HFP_AT_CMD,BT_MSG_INVL_PARAM)
#define bt_hfp_send_custom_at_cmd()             bt_hfp_msg(BT_HFP_CUSTOM_AT_CMD,BT_MSG_INVL_PARAM)
#define bt_call_vol_change()                    bt_hfp_msg(BT_HFP_CALL_VOL_CHANGE,BT_MSG_INVL_PARAM)              //调节音频音量
#define bt_call_vol_up()                        bt_hfp_msg(BT_HFP_CALL_VOL_ADJ,1)                  //音频加音量
#define bt_call_vol_down()                      bt_hfp_msg(BT_HFP_CALL_VOL_ADJ,0)                //音频减音量

//a2dp
void a2dp_dump_status(void);
void a2dp_init(void);
bool a2dp_is_playing(void);
uint8_t a2dp_vol_reverse(uint vol);                 //将系统音量转换为a2dp_vol
uint8_t a2dp_vol_conver(uint8_t a2dp_vol);          //将a2dp_vol转换为系统音量级数
#define bt_a2dp_is_vol_ctrl()                   (cfg_bt_a2dp_feature & A2DP_AVRCP_VOL_CTRL)

//蓝牙音乐控制
#define bt_music_play()                         bt_a2dp_msg(BT_A2DP_CTRL,A2DP_PLAY)               //播放
#define bt_music_pause()                        bt_a2dp_msg(BT_A2DP_CTRL,A2DP_PAUSE)              //暂停
#define bt_music_stop()                         bt_a2dp_msg(BT_A2DP_CTRL,A2DP_STOP)               //停止
#define bt_music_prev()                         bt_a2dp_msg(BT_A2DP_CTRL,A2DP_BACKWARD)           //上一曲
#define bt_music_next()                         bt_a2dp_msg(BT_A2DP_CTRL,A2DP_FORWARD)            //下一曲
#define bt_music_rewind()                       bt_a2dp_msg(BT_A2DP_CTRL,A2DP_REWIND)             //开始快退
#define bt_music_rewind_end()                   bt_a2dp_msg(BT_A2DP_CTRL,A2DP_REWIND_END)         //结束快退
#define bt_music_fast_forward()                 bt_a2dp_msg(BT_A2DP_CTRL,A2DP_FAST_FORWARD)       //开始快进
#define bt_music_fast_forward_end()             bt_a2dp_msg(BT_A2DP_CTRL,A2DP_FAST_FORWARD_END)   //结束快进
#define bt_music_play_pause()                   bt_a2dp_msg(BT_A2DP_PLAY_PAUSE,BT_MSG_INVL_PARAM)              //切换播放/暂停
#define bt_music_play_switch()                  bt_a2dp_msg(BT_A2DP_2ACL_PALY_SWITCH,BT_MSG_INVL_PARAM)        //一拖二切换播放手机
#define bt_low_latency_enable()                 bt_a2dp_msg(BT_A2DP_LOW_LATENCY_SET,1)          //蓝牙使能低延时
#define bt_low_latency_disable()                bt_a2dp_msg(BT_A2DP_LOW_LATENCY_SET,0)         //蓝牙关闭低延时
#define bt_music_vol_change()                   bt_a2dp_msg(BT_A2DP_VOL_CHANGE,BT_MSG_INVL_PARAM)              //调节音频音量
#define bt_music_vol_up()                       bt_a2dp_msg(BT_A2DP_VOL_ADJ,1)                  //音频加音量
#define bt_music_vol_down()                     bt_a2dp_msg(BT_A2DP_VOL_ADJ,0)                //音频减音量

//spp
enum {
    SPP_SERVICE_CH0,     //channel0，是默认SPP UUID 服务通路
    SPP_SERVICE_CH1,     //channel1，自定义UUID,默认用GFPS
    SPP_SERVICE_CH2,     //channel2, 自定义UUID，未使用
};
void spp_init(void);
extern struct txpkt_tag spp_tx;
void spp_send_kick(void);
int bt_spp_tx_for_index(uint8_t index, uint8_t ch, uint8_t *packet, uint16_t len);//index: 0=link0, 1=link1, 0xff=auto(default link0)
#define bt_spp_tx(a, b, c)    bt_spp_tx_for_index(0xff, a, b, c)
void spp_support_mul_server(uint8_t support);
bool spp_is_connect(void);
bool spp_is_connected_with_channel(uint8_t ch);  //判断某一个SPP通路是否连接，目前只有 0,1,2 共3个通路
void spp_disconnect(void);

//voice assistant
enum {
    VA_STA_IDLE = 0,
    VA_STA_CONNECT,
    VA_STA_STOP,
    VA_STA_START,
    VA_STA_END,
};

void aap_init(void);
void bt_menu_va_stop(void);
void bt_menu_va_operate(u8 enble);
u8 bt_menu_va_is_connected(void);

//hid
void hid_device_init(void);
bool bt_hid_send(void *buf, uint len, bool auto_release);                                           //自定义HID数组
bool bt_hid_send_key(uint type, uint keycode);                                                      //标准HID按键
void bt_hid_connect(void);                      //蓝牙HID服务回连
void bt_hid_disconnect(void);                   //蓝牙HID服务断开
int bt_hid_is_connected(void);
bool bt_hid_is_ready_to_discon(void);
#define bt_hid_key(keycode)                     bt_hid_send_key(HID_KEYBOARD, keycode)              //标准HID键, 如Enter
#define bt_hid_consumer(keycode)                bt_hid_send_key(HID_CONSUMER, keycode)              //自定义HID键, 如VOL+ VOL-
#define bt_hid_touch_screen(keycode)            bt_hid_send_key(HID_TOUCH_SCREEN, keycode)          //触屏
bool bt_hid_touch_screen_set_key(void *ts);

//pbap
void pbap_client_init(void);
#define bt_pbap_connect()                       bt_pbap_msg(BT_PBAP_CTRL, 1)
#define bt_pbap_disconnect()                    bt_pbap_msg(BT_PBAP_CTRL, 0)
// sim - 1:选择SIM卡，0:本机
// book- 0:pb, 1:fav, 2-ich, 3:och, 4-mch, 5-cch, 6-spd
// 若不配置，则选择默认值为本机pb
#define bt_pbap_select_phonebook(book, sim)     bt_pbap_msg(BT_PBAP_SELECT_PHONEBOOK, (sim<<8) | (u8)book)
// 按编号获取联系人信息
// idx不为零，如果为零则直接获取整个电话本信息
#define bt_pbap_get_phonebook_size()            bt_pbap_msg(BT_PBAP_GET_PHONEBOOK_SIZE, 0)
#define bt_pbap_pull_phonebook_whole()          bt_pbap_msg(BT_PBAP_PULL_PHONEBOOK, 0)
#define bt_pbap_pull_phonebook_single(idx)      bt_pbap_msg(BT_PBAP_PULL_PHONEBOOK, idx)

void map_client_init(void);
void goep_client_init(void);

//gatt over bredr
void latt_send_kick(void);
int latt_tx_notify_for_index(uint8_t index, uint16_t att_handle, const uint8_t *value, uint16_t value_len);//index: 0=link0, 1=link1, 0xff=auto(default link0)
#define latt_tx_notify(a, b, c)    latt_tx_notify_for_index(0xff, a, b, c)
extern struct txpkt_tag latt_notify_tx;

#endif // _API_BT_H

