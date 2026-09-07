/*
 *  tws_res.h
 *
 *  Created by zoro on 2023-9-20.
 */
#ifndef __RES_TBL_H
#define __RES_TBL_H


enum {
    //一般由主耳发起的提示音
    RES_IDX_NUM_0,
    RES_IDX_NUM_1,
    RES_IDX_NUM_2,
    RES_IDX_NUM_3,
    RES_IDX_NUM_4,
    RES_IDX_NUM_5,
    RES_IDX_NUM_6,
    RES_IDX_NUM_7,
    RES_IDX_NUM_8,
    RES_IDX_NUM_9,

    RES_IDX_RING,
    RES_IDX_CONNECTED,          //STA
    RES_IDX_DISCONNECT,         //STA
    RES_IDX_PAIRING,            //STA

    //副耳也可以请求的提示音（根据需要添加到tws_slave_res_add_req_cb）
    RES_IDX_LOW_BATTERY,        //STA
    RES_IDX_MAX_VOL,            //STA
    RES_IDX_LANGUAGE_EN,        //STA
    RES_IDX_LANGUAGE_ZH,        //STA

    RES_IDX_CALL_HANGUP,        //hangup
    RES_IDX_CALL_REJECT,        //reject
    RES_IDX_REDIALING,          //redial
    RES_IDX_MUSIC_MODE,         //music_mode
    RES_IDX_GAME_MODE,          //game_mode

    RES_IDX_POWERON,            //break
    RES_IDX_POWEROFF,           //break
    RES_IDX_SDCARD_MODE,        //break
    RES_IDX_USB_MODE,           //break
    RES_IDX_AUX_MODE,           //break
    RES_IDX_CLOCK_MODE,         //break
    RES_IDX_FM_MODE,            //break
    RES_IDX_SPK_MODE,           //break
    RES_IDX_PC_MODE,            //break
    RES_IDX_BT_MODE,            //break
    RES_IDX_LEFT_CH,
    RES_IDX_RIGHT_CH,
    RES_IDX_WAIT4CONN,
    RES_IDX_CAMERA_MODE,
    RES_IDX_CAMERA_ON,
    RES_IDX_CAMERA_OFF,
    RES_IDX_TAKE_PHOTO,
    RES_IDX_FINDMY_SOUND,
    RES_IDX_UPDATE,
    RES_IDX_UPDATE_DONE,
    RES_IDX_TWS_CONNECTED,
    RES_IDX_MAX,
};

extern const u32 res_tone_len;
extern const u32 res_piano_len;

#define RES_CONST_CAT(a, b, c)      a ## b ## _ ## c

#define RES_BUF_MP3(name)           RES_CONST_CAT(RES_BUF_, name, MP3)
#define RES_LEN_MP3(name)           RES_CONST_CAT(RES_LEN_, name, MP3)
#define RES_BUF_WAV(name)           RES_CONST_CAT(RES_BUF_, name, WAV)
#define RES_LEN_WAV(name)           RES_CONST_CAT(RES_LEN_, name, WAV)
#define RES_BUF_SBC(name)           RES_CONST_CAT(RES_BUF_, name, SBC)
#define RES_LEN_SBC(name)           RES_CONST_CAT(RES_LEN_, name, SBC)
#define RES_BUF_TONE(n)             warning_tone_tbl[n]
#define RES_LEN_TONE(n)             res_tone_len
#define RES_BUF_PIANO(n)            warning_piano_tbl[n]
#define RES_LEN_PIANO(n)            res_piano_len

u8 res_tbl_get_type(u32 index);
bool res_tbl_get_addr(u32 index, u32 *addr, u32 *len);

#endif // __RES_TBL_H
