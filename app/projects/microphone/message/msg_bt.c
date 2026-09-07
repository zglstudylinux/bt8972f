#include "include.h"
#include "func.h"
#include "func_bt.h"


#if BT_HID_MANU_EN
//双击VOL-, VOL+的功能处理
void bt_hid_vol_msg(u8 sel)
{
    if (sel == 1) {
        bsp_bt_hid_photo(HID_KEY_VOL_UP);
    } else if (sel == 2) {
        bsp_bt_hid_photo(HID_KEY_VOL_DOWN);
    } else if (sel == 3) {
        bsp_bt_hid_tog_conn();
    }
}
#endif

bool bt_tws_pair_mode(u8 method)
{
    return false;
}

AT(.text.func.bt.msg)
void func_bt_message_do(u16 msg)
{
    switch (msg) {
    case MSG_MUSIC_PLAY:
        bt_music_play();
        break;
    case MSG_MUSIC_PAUSE:
        bt_music_pause();
        break;
    case MSG_MUSIC_PLAY_PAUSE:
        bt_music_play_pause();
        break;
    case MSG_MUSIC_PREV:
        bt_music_prev();
        sys_cb.key2unmute_cnt = 15 * sys_cb.mute;
        break;
    case MSG_MUSIC_NEXT:
        bt_music_next();
        sys_cb.key2unmute_cnt = 15 * sys_cb.mute;
        break;

    case MSG_MUSIC_FB_START:
        bt_music_rewind();
        break;
    case MSG_MUSIC_FB_END:
        bsp_clr_mute_sta();
        bt_music_rewind_end();
        break;
    case MSG_MUSIC_FF_START:
        bt_music_fast_forward();
        break;
    case MSG_MUSIC_FF_END:
        bsp_clr_mute_sta();
        bt_music_fast_forward_end();
        break;

    case MSG_CALL_REDIAL_LAST:
        if (bt_is_connected()) {
            if(bsp_res_play(RES_IDX_REDIALING) == RES_ERR_INVALID) {
                bt_call_redial_last_number();       //回拨电话
            }
        }
        break;

    case MSG_MUSIC_LOW_LATENCY:
        bool low_latency = bt_is_low_latency();
        if (low_latency) {
            bsp_res_play(RES_IDX_MUSIC_MODE);
        } else {
            bsp_res_play(RES_IDX_GAME_MODE);
        }
        break;

    case MSG_CHANGE_LANGUAGE:
        func_bt_switch_voice_lang();
        break;

    case MSG_SWITCH_SIRI:
        if (bt_is_connected()) {
            bt_hfp_siri_switch();
        }
        break;

    case MSG_HID_HOME:
        bt_hid_consumer(HID_KEY_IOS_HOME);
        break;

    case MSG_SYS_1S:
//        bt_dump_status();
//        a2dp_dump_status();
//        dac_dump_vol();

        bt_hfp_report_bat();
        break;

    case EVT_KEY_2_UNMUTE:
        bsp_clr_mute_sta();
        break;

    default:
        func_message(msg);
        break;
    }
}

AT(.text.func.bt.msg)
void func_bt_message(u16 msg)
{
    if(msg){
        func_bt_message_do(msg);
    }
}

AT(.text.func.btring.msg)
void sfunc_bt_ring_message_do(u16 msg)
{
    switch (msg) {
    case MSG_CALL_ANSWER:
    case MSG_CALL_ANSWER_AND_HOLD:
    case MSG_CALL_ANSWER_AND_REJECT:
    case MSG_KEY_PP_KU:
        if(bsp_res_play(RES_IDX_CALL_HANGUP) == RES_ERR_INVALID) {
            bt_call_answer_incoming();  //接听
        }
        break;

    case MSG_CALL_REJECT:
    case MSG_KEY_PP_KL:
        if(bsp_res_play(RES_IDX_CALL_REJECT) == RES_ERR_INVALID) {
            bt_call_terminate();        //拒接
        }
        break;

    case MSG_PWR_HOLD:
    case MSG_PWR_OFF:
        //ring不响应关机消息，解决关机时间1.5时长按拒接偶尔触发关机的问题。
        break;

    case MSG_VOL_UP:
    case MSG_VOL_DOWN:
        //屏蔽来电响铃调音量
        break;

    case MSG_SYS_1S:
        bt_hfp_report_bat();
        break;

    default:
        func_message(msg);
        break;
    }
}

AT(.text.func.btring.msg)
void sfunc_bt_ring_message(u16 msg)
{
    if(msg){
        sfunc_bt_ring_message_do(msg);
    }
}

void sfunc_bt_call_message_do(u16 msg)
{
    u8 call_status;

    switch (msg) {

    ///短按
    case MSG_CALL_ANSWER_AND_HOLD:
    case MSG_KEY_PP_KU:
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_answer_incoming();                  //接听第2路通话，挂起当前通话
        } else {
            bt_call_terminate();                        //挂断当前通话
        }
        break;

    ///长按，拒接第2路通话, 或私密接听切换
    case MSG_CALL_SWITCH_PRIV:
    case MSG_KEY_PP_KL:
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_terminate();                        //拒接第2路通话
        } else {
            bt_call_private_switch();
        }
        break;

    ///双击，保持当前通话并接通第2路通话，或者切换两路通话
    case MSG_CALL_SWITCH_3WAY_CALL:
    case MSG_KEY_PP_DOUBLE:
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_answer_incom_hold_other();          //接听第2路通话
        } else if(call_status >= BT_CALL_ACTIVE) {
            bt_call_swap();                             //切换两路通话或切换私密通话
        }
        break;
    case MSG_CALL_ANSWER_AND_REJECT:
        call_status = bt_get_call_indicate();
        if(call_status == BT_CALL_INCOMING) {
            bt_call_answer_incom_rej_other();           //接听第2路通话，挂断当前通话
        } else if(call_status >= BT_CALL_ACTIVE) {
            bt_call_terminate();                        //挂断当前通话
        }
        break;
    case EVT_HFP_SET_VOL:
        if(sys_cb.incall_flag & INCALL_FLAG_SCO) {
            bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
            sys_cb.cm_times = 0;
            sys_cb.cm_vol_change = 1;
        }
        break;

    case MSG_SYS_1S:
        bt_hfp_report_bat();
        break;

    default:
        func_message(msg);
        break;
    }
}

void sfunc_bt_call_message(u16 msg)
{
    if(msg){
        sfunc_bt_call_message_do(msg);
    }
}
