#ifndef _WIRELESS_CMD_API_H
#define _WIRELESS_CMD_API_H

#define CMD_MAX_NB          4
#define CMD_MAX_SIZE        9

#define TX_MAX_BUF_SIZE     (CMD_MAX_SIZE - 1)


//内部定义
typedef enum{
    PRIVATE_WS_MIC_CMD      = 0,
    PRIVATE_USB_CMD,
    PRIVATE_SYNC_CMD,           ///tx端发送变化信息给rx端，rx端再把信息发给另一个tx端实现两个tx同步
    PRIVATE_USER_DATA,
    PRIVATE_PWR_CTR_CMD,
} cmd_t;

typedef struct wireless_cmd{
    u8 cmd;
    u8 buf[TX_MAX_BUF_SIZE];
} wireless_cmd_t;

//无线麦控制
typedef enum{
    //adapter opcode
    ADAPTER_GET_VERS,
    ADAPTER_GET_SN,
    ADAPTER_GET_MAC,
    ADAPTER_GET_BAT,
    ADAPTER_SET_NR_STA,                 //设置降噪开关、等级等参数
    ADAPTER_SET_ECHO_STA,               //设置混响等级参数
    ADAPTER_SET_MAGIC_STA,              //设置魔音等级参数

    //device opcode
    MUSIC_CTL_1ST           = 0x80,
    MUSIC_CTL_VOL_UP = MUSIC_CTL_1ST,   //音乐音量加
    MUSIC_CTL_VOL_DOWN,                 //音乐音量减
    MUSIC_CTL_PREV,                     //上一曲
    MUSIC_CTL_NEXT,                     //下一曲
    MUSIC_CTL_PLAY_PAUSE,               //播放/暂停
    MUSIC_CTL_VOICE_REMOVE,             //人声消除切换
    ECHO_DELAY_LEVEL_UP,                //混响等级加
    ECHO_DELAY_LEVEL_DOWN,              //混响等级减
    MAGIC_LEVEL_CHANGE,                 //切换魔音音效
    WL_DISCONNECT,                      //无线麦断开连接

    AUDIO_CRT_MUTE,                     //通知底层音频断开
} sub_ws_mic_cmd_t;

//USB播放控制
typedef enum{
    USB_SET_SPK_VOLUME      = 0,
    USB_CTL_MIC_STA,
    USB_CTL_PLAY_PAUSE,
    USB_CTL_VOLUME_UP,
    USB_CTL_VOLUME_DOWN,
    USB_CTL_PREVFILE,
    USB_CTL_NEXTFILE,
    USB_CTL_WDT_RESET,
} sub_usb_cmd_t;


//device <==> adapter私有命令api
void wireless_cmd_init(void);
void wireless_rx_cmd(u8 index, u8 *ptr, u8 len);
void wireless_tx_mic_cmd0(u8 index, u8 opcode, u8 param);
bool wireless_tx_mic_cmd(u8 index, u8 opcode, u8 *buf, u8 size);
void wireless_tx_usb_cmd(u8 msg, u8 param);
void wireless_tx_user_cmd(u8 *ptr, u8 len);
void wireless_tx_pwr_ctr_cmd(u8 index, u8 pwr_level);
bool wireless_cmd_is_full(u8 index);
bool wireless_cmd_is_empty(u8 index);

//device <==> adpater USB播放控制api
#if !ADAPTER_USB_SPK_TX_EN
    #define wireless_music_play_pause()
    #define wireless_music_volup()
    #define wireless_music_voldown()
    #define wireless_music_prev()
    #define wireless_music_next()
    #define wireless_set_usbspk_vol(vol)
#else
    #define wireless_music_play_pause()             wireless_tx_usb_cmd(USB_CTL_PLAY_PAUSE, 0)      //切换播放/暂停
    #define wireless_music_volup()                  wireless_tx_usb_cmd(USB_CTL_VOLUME_UP, 0)       //音量加
    #define wireless_music_voldown()                wireless_tx_usb_cmd(USB_CTL_VOLUME_DOWN, 0)     //音量减
    #define wireless_music_prev()                   wireless_tx_usb_cmd(USB_CTL_PREVFILE, 0)        //上一曲
    #define wireless_music_next()                   wireless_tx_usb_cmd(USB_CTL_NEXTFILE, 0)        //下一曲
    #define wireless_set_usbspk_vol(vol)            wireless_tx_usb_cmd(USB_SET_SPK_VOLUME, vol)    //spk_vol
#endif

#if !ADAPTER_USB_MIC_RX_EN
    #define wireless_set_usbmic_status(start)
#else
    #define wireless_set_usbmic_status(start)       wireless_tx_usb_cmd(USB_SET_SPK_VOLUME, start)  //mic_status
#endif

//device ==> adapter无线麦命令
#define wirless_echo_level_up()                     wireless_tx_mic_cmd0(0, ECHO_DELAY_LEVEL_UP, 1)
#define wirless_echo_level_down()                   wireless_tx_mic_cmd0(0, ECHO_DELAY_LEVEL_DOWN, 1)
#define wirless_magic_change()                      wireless_tx_mic_cmd0(0, MAGIC_LEVEL_CHANGE, 1)
#define wirless_music_vol_up()                      wireless_tx_mic_cmd0(0, MUSIC_CTL_VOL_UP, 1)
#define wirless_music_vol_down()                    wireless_tx_mic_cmd0(0, MUSIC_CTL_VOL_DOWN, 1)
#define wirless_music_prev()                        wireless_tx_mic_cmd0(0, MUSIC_CTL_PREV, 1)
#define wirless_music_next()                        wireless_tx_mic_cmd0(0, MUSIC_CTL_NEXT, 1)
#define wirless_music_play_pause()                  wireless_tx_mic_cmd0(0, MUSIC_CTL_PLAY_PAUSE, 1)
#define wirless_music_voice_remove()                wireless_tx_mic_cmd0(0, MUSIC_CTL_VOICE_REMOVE, 1)
#define wirless_adapter_disconnect()                wireless_tx_mic_cmd0(0, WL_DISCONNECT, 1)

//device <== adapter无线麦命令

#if ADAPTER_USB_MIC_RX_EN
    #define wireless_wdt_reset()                    wireless_tx_usb_cmd(USB_CTL_WDT_RESET, 0)       //控制看门狗复位
#endif

//外部函数声明
void wireless_cmd_buf_init(void);
bool wireless_send_cmd(u8 index, u8 *cmd, u8 len);
void wireless_rx_user_cmd(u8 index, u8 *ptr, u8 len);

#endif
