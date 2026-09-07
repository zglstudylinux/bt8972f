#ifndef _FUNC_H
#define _FUNC_H

#include "func_adapter.h"
#include "func_device.h"
#include "func_bt.h"
#include "func_bt_dut.h"
#include "func_idle.h"
//#include "func_music.h"
#include "func_usbdev.h"
//#include "func_aux.h"
//#include "func_speaker.h"
#include "func_lowpwr.h"
#include "func_update.h"
#include "sfunc_record.h"
//#include "sfunc_adapter.h"

//task number
enum {
    FUNC_NULL = 0,
    FUNC_ADAPTER,
    FUNC_DEVICE,
    FUNC_BT,
    FUNC_USBDEV,
    FUNC_PWROFF,
    FUNC_CHARGE,
    FUNC_BT_DUT,                        //CBT测试模式
    FUNC_BT_FCC,                        //FCC测试模式
    FUNC_BT_IODM,                       //IODM测试模式
    FUNC_IDLE,
    FUNC_MAX,
};

struct dev_evt_tag {
    void (*insert)(u8 func_sta);
    void (*remove)(u8 func_sta);
};

//task control block
typedef struct {
    u8 sta;                                         //new task number
    u8 curr;                                        //current working task number
    u8 last;                                        //lask task number
#if BT_BACKSTAGE_EN
    bool back_flag;
    u8 sta_break;                                   //被中断的任务
#endif
    void (*set_vol_callback)(u8 dir);               //设置音量的回调函数，用于各任务的音量事件处理。
} func_cb_t;
extern func_cb_t func_cb;

extern const u8 func_sort_table[];     //任务切换排序table
extern const u8 adapter_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 device_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 bt_music_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 bt_call_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
extern const u8 usbdev_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];

u8 get_funcs_total(void);
void func_process(void);
void func_message(u16 msg);

void func_run(void);
void func_music(void);
void func_idle(void);
void func_clock(void);
void func_fmrx(void);
void func_bt(void);
void func_bthid(void);
void func_usbdev(void);
void func_aux(void);
void func_speaker(void);
void func_charge(void);

void sfunc_charge_enter(bool chbox_out2pwr_en);
u8 sfunc_charge_process(u32 counter);
void sfunc_charge_exit(void);

void func_exspifalsh_music(void);
void func_bt_warning(void);
void func_bt_vol_fade(void);
void func_audio_bypass(u8 func_sta, u8 res_type);
void func_audio_restore(u8 func_sta, u8 res_type);

#define bsp_get_vol()       (func_cb.sta == FUNC_BT)? sys_cb.a2dp_vol : sys_cb.vol

#endif // _FUNC_H
