#include "include.h"
#include "func.h"

void func_udisk_insert(u8 func_sta);
void func_udisk_remove(u8 func_sta);
void func_sdcard_insert(u8 func_sta);
void func_sdcard_remove(u8 func_sta);
void func_sdcard1_insert(u8 func_sta);
void func_sdcard1_remove(u8 func_sta);


static const struct {
    const void *msg_tbl;
    u8 dev_online;      //0=不检查，!0=检查对应bit的dev_online
} func_info[] = {
#if FUNC_BT_EN
    [FUNC_BT]       = {bt_music_key_msg_tbl,    0},
#endif
#if FUNC_ADAPTER_EN
    [FUNC_ADAPTER]  = {adapter_key_msg_tbl,     0},
#endif
#if FUNC_DEVICE_EN
    [FUNC_DEVICE]   = {device_key_msg_tbl,      0},
#endif
#if FUNC_USBDEV_EN
    [FUNC_USBDEV]   = {usbdev_key_msg_tbl,      BIT(DEV_USBPC),},
#endif
    [FUNC_CHARGE]   = {NULL, 0},
    [FUNC_PWROFF]   = {NULL, 0},
#if FUNC_BT_DUT_EN
    [FUNC_BT_DUT]   = {NULL, 0},
#endif
#if FUNC_BT_FCC_EN
    [FUNC_BT_FCC]   = {NULL, 0},
#endif
    [FUNC_BT_IODM]  = {NULL, 0},
    [FUNC_IDLE]     = {NULL, 0},
};

static const struct dev_evt_tag evt_tbl[DEV_TOTAL_NUM] = {
#if MUSIC_UDISK_EN
    [DEV_UDISK] = {
        func_udisk_insert,
        func_udisk_remove,
    },
#endif

#if MUSIC_SDCARD_EN || UDE_STORAGE_EN
    [DEV_SDCARD] = {
        func_sdcard_insert,
        func_sdcard_remove,
    },
#endif

#if AUX_DETECT_EN
    [DEV_AUX] = {
        func_aux_insert,
        func_aux_remove,
    },
#endif

#if FUNC_USBDEV_EN || ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN
    [DEV_USBPC] = {
        func_usbdev_insert,
        func_usbdev_remove,
    },
#endif
};

func_cb_t func_cb AT(.buf.func_cb);

#if VBAT_DETECT_EN
void lowbat_warning_proc(void)
{
    sys_cb.lbat_warning_delay = 0;          //延时计数清零

    if (sys_cb.lbat_warning_times) {        //低电语音提示次数
        if (RLED_LOWBAT_FOLLOW_EN) {
            led_lowbat_follow_warning();
        }

        bsp_res_play_local(RES_IDX_LOW_BATTERY);

        if (sys_cb.lbat_warning_times != 0xff) {
            sys_cb.lbat_warning_times--;
        }
    }
}

void lowbat_warning_start(void)
{
    sys_cb.lowbat_flag = 1;
    plugin_lowbat_vol_reduce();         //低电降低音乐音量

    lowbat_warning_proc();
}

void lowbat_warning_stop(void)
{
    sys_cb.lowbat_flag = 0;
    sys_cb.lbat_warning_times = LPWR_WARING_TIMES;
    plugin_lowbat_vol_recover();    //离开低电, 恢复音乐音量
}

AT(.text.func.msg)
void lowbat_process(void)
{
    uint vbat_sta = vbat_get_lpwr_sta();

    switch(vbat_sta) {
    case 2:
        if(sys_cb.lpwr_status == LBAT_STA_W4_ENTER) {
            if(tick_check_expire(sys_cb.lpwr_tick, 1000)) {
                sys_cb.lpwr_status = LBAT_STA_WARNING;

                if (LPWR_OFF_VBAT) {                    //是否关机
                    sys_cb.pwroff.low_bat_ind = 1;
                }
            }
        } else {
            sys_cb.lpwr_status = LBAT_STA_W4_ENTER;
            sys_cb.lpwr_tick = tick_get();
        }

        if(sys_cb.vbat_warning <= sys_cb.vbat_pwroff) {
            goto __lbat_process_sta;
        }
        break;

    case 1:
        sys_cb.lpwr_status = LBAT_STA_IDLE;

__lbat_process_sta:
        if(sys_cb.lbat_status == LBAT_STA_W4_ENTER) {
            if(tick_check_expire(sys_cb.lbat_tick, 1000)) {
                sys_cb.lbat_status = LBAT_STA_WARNING;

                lowbat_warning_start();                 //进入低电模式
            }
        } else if(sys_cb.lbat_status == LBAT_STA_W4_EXIT) {
            sys_cb.lbat_tick = tick_get();
        } else if(sys_cb.lbat_status != LBAT_STA_WARNING) {
            sys_cb.lbat_status = LBAT_STA_W4_ENTER;
            sys_cb.lbat_tick = tick_get();
        }
        break;

    case 0:
        sys_cb.lpwr_status = LBAT_STA_IDLE;

        if(sys_cb.lbat_status == LBAT_STA_W4_EXIT) {
            if(sys_cb.vbat > 3800) {                    //等待恢复到多少v电压
                sys_cb.lbat_tick = tick_get();
            } else if(tick_check_expire(sys_cb.lbat_tick, 500)) {
                sys_cb.lbat_status = LBAT_STA_IDLE;

                lowbat_warning_stop();                  //退出低电模式
            }
        } else if(sys_cb.lbat_status == LBAT_STA_WARNING) {
            sys_cb.lbat_status = LBAT_STA_W4_EXIT;
            sys_cb.lbat_tick = tick_get();
        } else {
            sys_cb.lbat_status = LBAT_STA_IDLE;
        }
        break;
    }

#if TRACE_EN
    static u8 lpwr_sta = 0xff;
    static u8 lbat_sta = 0xff;
    static u32 tick = 0;
    if(lpwr_sta != sys_cb.lpwr_status || lbat_sta != sys_cb.lbat_status || tick_check_expire(tick, 1000)) {
        lpwr_sta = sys_cb.lpwr_status;
        lbat_sta = sys_cb.lbat_status;
        tick = tick_get();

        printf("@@@@@@ lpwr_sta: %d(%d), %d(%d), vbat=%d\n", sys_cb.lpwr_status, sys_cb.vbat_pwroff, sys_cb.lbat_status, sys_cb.vbat_warning, sys_cb.vbat);
    }
#endif

    if(sys_cb.lbat_status >= LBAT_STA_WARNING) {
        //重复播报低电提示音
        if (sys_cb.lbat_warning_delay >= xcfg_cb.lpwr_warning_period) {
            lowbat_warning_proc();
        }
    }

#if LED_LOWBAT_EN
    if (xcfg_cb.rled_lowbat_en) {
        if(sys_cb.lbat_status >= LBAT_STA_WARNING) {
            if ((!CHARGE_DC_IN()) && (!RLED_LOWBAT_FOLLOW_EN)) {
                led_lowbat();
            }
        } else {
            led_lowbat_recover();
        }
    }
#endif
}
#endif  //VBAT_DETECT_EN

AT(.text.func.process)
void pwroff_process(void)
{
    struct pwroff_tag *pwroff = &sys_cb.pwroff;

    if(pwroff->key_state == PWROFF_W4_TIMEOUT) {
        if(tick_check_expire(pwroff->key_ticks, pwroff->delay_ticks)) {
            pwroff->key_state = PWROFF_END;
            pwroff->pwr_key_ind = 1;
        }
    }

    if(pwroff->all_flag != 0) {
        if(pwroff->charge_full_ind) {
#if AUX_MODE_2_PWROFF_TONE_EN
        } else if(pwroff->aux_insert_ind) {
            pwroff->tone_en = 2;
#endif
        } else {
            pwroff->tone_en = 1;
        }

        //rtc_set_alarm_wakeup(20);

        func_cb.sta = FUNC_PWROFF;
    }
}

//提示音过程复用资源时暂停相关功能
void func_audio_bypass(u8 func_sta, u8 res_type)
{
    switch(func_sta) {
#if FUNC_BT_EN
    case FUNC_BT:
        bt_audio_bypass();
        break;
#endif

    default:
#if WARNING_SYSVOL_ADJ_EN
    {
        //其他模式资源不复用，可以系统音量退避
        u8 vol = (sys_cb.incall_flag != 0)? bsp_bt_get_hfp_vol(sys_cb.hfp_vol) : bsp_get_vol();
        if(vol > SYS_BACKSTAGE_VOLUME) {
            bsp_change_volume(SYS_BACKSTAGE_VOLUME);
        }
    }
#endif
        break;
    }
}

//提示音过程复用资源时恢复相关功能
void func_audio_restore(u8 func_sta, u8 res_type)
{
    switch(func_sta) {
#if FUNC_BT_EN
    case FUNC_BT:
        bt_audio_enable();
        func_bt_vol_fade();
        break;
#endif
    }
}

//音量加消息
void func_volume_up(void)
{
    printf("vol_up: %d, (a2dp=%d, hfp=%d)\n", sys_cb.vol, sys_cb.a2dp_vol, sys_cb.hfp_vol);

    if (func_cb.sta == FUNC_BT) {
        if (sys_cb.incall_flag) {
            bsp_bt_call_volume_msg(MSG_VOL_UP);
        } else {
#if WARNING_MAX_VOLUME
            u8 vol = sys_cb.a2dp_vol;
#endif

#if BT_HID_VOL_CTRL_EN
            if(bsp_bt_hid_vol_change(HID_KEY_VOL_UP)) {
                return;
            }
#endif

            bt_music_vol_up();

#if WARNING_MAX_VOLUME
            if (vol >= (VOL_MAX-1)) {
                maxvol_tone_play();
            }
#endif
        }
    } else {
        bsp_set_volume(bsp_volume_inc(sys_cb.vol));
        if (sys_cb.vol == VOL_MAX) {
            maxvol_tone_play();
        }
    }

    if (func_cb.set_vol_callback) {
        func_cb.set_vol_callback(1);
    }
}

//音量减消息
void func_volume_down(void)
{
    printf("vol_down: %d, (a2dp=%d, hfp=%d)\n", sys_cb.vol, sys_cb.a2dp_vol, sys_cb.hfp_vol);

    if (func_cb.sta == FUNC_BT) {
        if (sys_cb.incall_flag) {
            bsp_bt_call_volume_msg(MSG_VOL_DOWN);
        } else {
#if WARNING_MIN_VOLUME
            u8 vol = sys_cb.a2dp_vol;
#endif

#if BT_HID_VOL_CTRL_EN
            if(bsp_bt_hid_vol_change(HID_KEY_VOL_DOWN)) {
                return;
            }
#endif

            bt_music_vol_down();

#if WARNING_MIN_VOLUME
            if (vol <= 1) {
                minvol_tone_play();
            }
#endif
        }
    } else {
        bsp_set_volume(bsp_volume_dec(sys_cb.vol));
        if (sys_cb.vol == 0) {
            minvol_tone_play();
        }
    }

    if (func_cb.set_vol_callback) {
        func_cb.set_vol_callback(0);
    }
}

#if MUSIC_UDISK_EN
void func_udisk_insert(u8 func_sta)
{
    udisk_insert();
}

void func_udisk_remove(u8 func_sta)
{
    udisk_remove();
}
#endif

#if ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN
void func_usbdev_insert(u8 func_sta)
{
    if(!sys_cb.usbdev_insert) {
        printf("EVT_PC_INSERT\n");
        sys_cb.usbdev_insert = true;
        memset(&f_ude, 0, sizeof(f_ude));
        f_ude.db_level = bsp_volume_vol2lev(sys_cb.vol);
        f_ude.vol = sys_cb.vol;         //restore system volume
        ude_info_init();
        usb_device_enter(UDE_ENUM_TYPE);
        sys_cb.ude_flag = true;
    }
}

void func_usbdev_remove(u8 func_sta)
{
    if(sys_cb.usbdev_insert) {
        printf("EVT_PC_REMOVE\n");
        sys_cb.usbdev_insert = false;
        sys_cb.ude_flag = false;
        usb_device_exit();
    }
}
#endif // ADAPTER_USB_MIC_RX_EN

#if MUSIC_SDCARD_EN || UDE_STORAGE_EN
void func_sdcard_insert(u8 func_sta)
{
    sd_insert();

#if UDE_STORAGE_EN
    if(func_sta == FUNC_USBDEV) {
        ude_sdcard_switch(DEV_SDCARD);
        return;
    }
#endif
}

void func_sdcard_remove(u8 func_sta)
{
    sd_remove();
}
#endif

//设备插拔事件处理
void func_dev_process(uint dev_change)
{
    u8 func_sta = func_cb.sta;

    printf("====>dev_change: %x, %x\n", dev_change, dev_get_online());

    for(uint i=0; i<DEV_TOTAL_NUM; i++) {
        if(dev_change & BIT(i)) {
            if (dev_get_online() & BIT(i)) {
                if(evt_tbl[i].insert != NULL) {
                    evt_tbl[i].insert(func_sta);
                }
            } else {
                if(evt_tbl[i].remove != NULL) {
                    evt_tbl[i].remove(func_sta);
                }
#if BT_BACKSTAGE_EN
                u8 chk_dev = func_info[func_sta].dev_online;
                if(func_cb.sta_break == func_sta && chk_dev != 0 && (dev_get_online() & chk_dev) == 0) {
                    func_cb.sta_break = FUNC_NULL;  //清除打断恢复
                }
#endif
            }
        }
    }
}

//func common event process
AT(.text.func.process)
void func_process(void)
{
    uint dev_change;
    static u8 recursion = 0;
    recursion++;
#if UART2_COM_EN
    bsp_uart2_com_process();                            //普通串口测试口回显
#endif
    if(sys_cb.usbdev_insert) {
#if ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN
        usb_device_process();
#endif
    }
    WDT_CLR();
    if(!CHARGE_WORKING_WHILE_CHARGING) {   //边充电边工作模式考虑插入复位一下情况
#if !TBOX_TEST_EN
        vusb_reset_clr();
#endif
    }


    sys_cb.sys_delay = 0;
    if(recursion > 1) {     //检查递归
        printf("func_err, %d, %x\n", recursion, (uint32_t)__builtin_return_address(0));
    }

    //关机也需要处理的事件
#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        bsp_charge_process();
    }
    #if CHARGE_BOX_EN
    charge_box_process();
    #endif
#endif // CHARGE_EN

    bsp_res_process();
    if(func_cb.curr == FUNC_PWROFF) {
        recursion--;
        return;
    }

    //以下是开机状态处理的事件
    dev_change = sys_cb.dev_active ^ dev_get_online();
    if(dev_change != 0 && (sys_cb.device_usb_init_flag || !DEVICE_USB_EN)) {
        sys_cb.dev_active ^= dev_change;
        func_dev_process(dev_change);
    }

#if CHARGE_EN
    if (charge_get_dc_sta() && CHARGE_DC_IN() && func_cb.sta != FUNC_CHARGE) {
        if(!CHARGE_WORKING_WHILE_CHARGING) {   //边充电边工作模式不需要要跑入专门的充电模式
            if (func_bt_charge_dcin()) {
                sys_cb.pwroff.tone_en = 0;
                func_cb.sta = FUNC_CHARGE;
            }
        }
    }
#endif

#if VBAT_DETECT_EN
    lowbat_process();
#endif // VBAT_DETECT_EN

    pwroff_process();

#if QTEST_EN
    if(QTEST_IS_ENABLE()) {
        qtest_process();
    }
#endif

#if DEVICE_RECORD_EN
    sfunc_mic_rec_process();
#endif

#if BT_BACKSTAGE_EN
    if (func_cb.curr != FUNC_BT && func_cb.sta != FUNC_BT) {
        func_bt_warning();

        if(func_cb.sta != FUNC_PWROFF && func_cb.sta != FUNC_CHARGE) {      //pwroff和charge优先级更高
            uint status = bt_get_status();
#if BT_BACKSTAGE_PLAY_DETECT_EN
            if (status >= BT_STA_PLAYING) {
#else
            if (status > BT_STA_PLAYING) {
#endif
                func_cb.sta_break = func_cb.sta;
                func_cb.sta = FUNC_BT;
            }
        }
    }
#endif


    app_process();

#if OFFLINE_LOG_EN
    offline_log_process();
#endif
    gsensor_process();

    recursion--;
}

//func common message process
AT(.text.func.msg)
void func_message(u16 msg)
{
    switch (msg) {
        case MSG_PWR_HOLD:
            if(sys_cb.pwroff.key_state == PWROFF_IDLE) {
                sys_cb.pwroff.key_ticks = tick_get();
                sys_cb.pwroff.key_state = PWROFF_W4_TIMEOUT;
            }
            break;
        case MSG_PWR_RELEASE:
            if(sys_cb.pwroff.key_state == PWROFF_W4_TIMEOUT) {
                sys_cb.pwroff.key_state = PWROFF_IDLE;
                bsp_key_recover_msg();
            }
            break;
        case MSG_PWR_OFF:
            sys_cb.pwroff.pwr_key_ind = 1;
            break;

#if FUNC_DEVICE_EN && WIRELESS_CON_AND_BT_PAIRING_EN
        case MSG_PAIRING:
            printf("MSG_PAIRING\n");
            if(!wireless_role_is_adapter()) {
                if(func_cb.sta == FUNC_DEVICE) {
                    //在device模式下,长按配对按键,先转到BT模式, 再进入配对状态
                    func_cb.sta = FUNC_BT;
                } else if(func_cb.sta == FUNC_BT && !sys_cb.wl_scan_flag) {
                    //进入配对状态
                    wireless_device_pairing_enable();
                }
            }
            break;

        case MSG_FUNC_DEVICE:
            if(!wireless_role_is_adapter()) {
                func_cb.sta = FUNC_DEVICE;
            }
            break;
#endif
#if DEVICE_RECORD_EN
    case MSG_VOL_UP:
        if (sfunc_is_recording()) {
            sfunc_record_stop();
        } else if(sys_cb.sd_card_init_flag){
            if(!wireless_cb.alg_en) {         //没启动算法和麦，但SD卡在线，启动算法和麦之后再开始录音
                kick_record_start();
            }
            if(!sfunc_record_start()) {
                printf("record fail\n");
            }
        }
        break;
#endif
#if MUSIC_SDCARD_EN
    case EVT_SD_INSERT:
        sd_record_init();
        break;
    case EVT_SD_REMOVE:
        sys_cb.sd_card_init_flag = 0;
#if DEVICE_LOCAL_REC_EN
        if((wireless_cb.connected_sta == 0) && (func_cb.sta == FUNC_DEVICE)) {
            wireless_cb.alg_en = 0;
            wireless_device_exit();
        }
#endif
        sys_clk_free(INDEX_MUSIC);
        break;
#endif
//
//        case MSG_VOL_DOWN:
//            func_volume_down();
//            break;

#if CHANGE_MODE_MSG_EN
        case MSG_CHANGE_MODE:
            if (bt_get_call_indicate() == 0) {      //通话中不允许切模式
                func_cb.sta = FUNC_NULL;        //退出当前模式，切换到下一个
            }
            break;
#endif
#if EQ_MODE_EN
        case MSG_CHANGE_EQ:
            sys_set_eq();
            break;
#endif // EQ_MODE_EN

        case MSG_VOL_MUTE:
            if (sys_cb.mute) {
                bsp_sys_unmute();
            } else {
                bsp_sys_mute();
            }
            break;

#if FUNC_BT_DUT_EN
        case MSG_ENTER_DUT:
            if(func_cb.sta != FUNC_BT_DUT){
                func_cb.sta = FUNC_BT_DUT;
                sys_cb.discon_reason = 0;
            }
            break;
#endif

        case EVT_RECORD_1S:
#if DEVICE_RECORD_EN
            if(!wireless_role_is_adapter()) {
                if (!sfunc_is_recording()) {
                        break;
                    }
                sys_cb.fs_recording_cnt++;
                my_printf("%d\n",sys_cb.fs_recording_cnt);
                if(!sfunc_fwrite_sync()) {
                    my_printf("fwrite sync fail\n");
                    sfunc_record_stop();
                }
            }
#endif
            break;

        case EVT_A2DP_SET_VOL:
        case EVT_TWS_INIT_VOL:
            if((sys_cb.incall_flag & INCALL_FLAG_SCO) == 0) {
                if((func_cb.sta == FUNC_BT) && !bsp_res_is_vol_busy()) {
                    bsp_change_volume(sys_cb.a2dp_vol);
                }
            }
            //no break
        case EVT_TWS_SET_VOL:
            if (sys_cb.incall_flag == 0) {
                if(func_cb.sta == FUNC_BT) {
                    printf("A2DP_VOL: %d\n", sys_cb.a2dp_vol);
                    if(msg != EVT_TWS_INIT_VOL) {   //TWS同步音量，不需要显示
                        gui_box_show_vol();
                    }
                }
            }
            //no break
        case EVT_DEV_SAVE_VOL:
            sys_cb.cm_times = 0;
            sys_cb.cm_vol_change = 1;
            break;

        case EVT_BT_SCAN_START:
            if (bt_get_status() < BT_STA_SCANNING) {
                bt_scan_enable();
            }
            break;
#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
        case EVT_ONLINE_SET_EQ:
            bsp_eq_parse_cmd();
            break;
#if EFFECT_DBG_ADJUST_IN_UART
        case EVT_ONLINE_SET_EFFECT:
            toolkit_process();
            break;
#endif
#endif

#if ENC_DBG_EN
        case EVT_ONLINE_SET_ENC:
            bsp_enc_parse_cmd();
            break;
#endif

#if LANG_SELECT == LANG_EN_ZH
        case EVT_BT_SET_LANG_ID:
            param_lang_id_write();
            param_sync();
            break;
#endif

#if EQ_MODE_EN
        case EVT_BT_SET_EQ:
            music_eq_set_by_num(sys_cb.eq_mode);
            break;
#endif

#if CHARGE_BOX_EN && (UART0_PRINTF_SEL != PRINTF_VUSB)
        //耳机入仓关机
        case EVT_CHARGE_INBOX:
            if(sys_cb.discon_reason == 0xff) {
                sys_cb.discon_reason = 0;   //不同步关机
            }
            charge_box_inbox_wakeup_enable();
            sys_cb.pwroff.tone_en = 0;
            func_cb.sta = FUNC_PWROFF;
            break;
#endif

#if QTEST_EN
        case EVT_QTEST_PICKUP_PWROFF:
            func_cb.sta = FUNC_PWROFF;
            break;
#endif

        case EVT_HFP_SET_VOL:
            if(sys_cb.incall_flag & INCALL_FLAG_SCO){
                bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
                sys_cb.cm_times = 0;
                sys_cb.cm_vol_change = 1;
            }
            break;

#if IODM_TEST_EN
        case EVT_IODM_TEST:
            iodm_reveice_data_deal();
            break;
#endif

#if OFFLINE_LOG_EN
        case EVT_OFFLINE_LOG_DUMP:
            offline_log_dump_huart();
            break;
#endif
#if AUDIO_DUMP_EN
        case EVT_ONLINE_SET_BTMIC_DUMP:
            bsp_btmic_dump_parse_cmd();
            break;
#endif
        default:
            break;
    }

    //调节音量，3秒后写入flash
    if ((sys_cb.cm_vol_change) && (sys_cb.cm_times >= 3)) {
        sys_cb.cm_vol_change = 0;
        param_hfp_vol_write();
        param_sys_vol_write();
        cm_sync();
    }
}

///进入一个功能的总入口
AT(.text.func)
void func_enter(void)
{
    u8 func_sta = func_cb.sta;

    printf("====>func%d: %d->%d\n", 0, func_cb.curr, func_cb.sta);
    func_cb.curr = FUNC_NULL;       //退出当前模式清掉，避免bsp_res_w4时恢复错误
    bsp_res_w4_finish(false);       //切到下一模式前，先等当前提示音播完，此处可能有新设备插入
    if(func_sta != func_cb.sta) {
        printf("====>func%d: %d->%d\n", 1, func_sta, func_cb.sta);
        func_sta = func_cb.sta;
    }

    if (sys_cb.cm_vol_change) {
        sys_cb.cm_vol_change = 0;
        param_sys_vol_write();
    }
    param_sync();
    gui_box_clear();
    reset_sleep_delay();
    reset_pwroff_delay();
    func_cb.set_vol_callback = NULL;
    bsp_clr_mute_sta();
    sys_cb.voice_evt_brk_en = 1;    //播放提示音时，快速响应事件。

    if(func_cb.sta < FUNC_MAX) {
        key_set_msg_tbl(func_info[func_cb.sta].msg_tbl);
    } else {
        key_set_msg_tbl(NULL);
    }

#if BT_BACKSTAGE_EN
    func_cb.back_flag = (bool)((func_cb.sta_break != FUNC_NULL) && (func_cb.sta == func_cb.sta_break));
    if(func_bt_chk_off(0)) {
        bsp_res_w4_finish(false);   //切到下一模式前，先等当前提示音播完，此处可能有新设备插入
        if(func_sta != func_cb.sta) {
            printf("====>func%d: %d->%d\n", 2, func_sta, func_cb.sta);
            func_sta = func_cb.sta;
        }
    } else {
        func_bt_init();             //避免切到某些模式关掉了蓝牙，退出时恢复蓝牙初始化
    }
#endif

    func_cb.curr = func_cb.sta;     //即将进入的模式
}

AT(.text.func)
void func_exit(void)
{
    u8 chk_dev;
    u8 func_num, func_sta = func_cb.last;
    u8 funcs_total = get_funcs_total();

    for (func_num = 0; func_num != funcs_total; func_num++) {
        if (func_cb.last == func_sort_table[func_num]) {
            break;
        }
    }

    for(uint i=0; i<funcs_total; i++) {
        func_num++;                                     //切换到下一个任务
        if (func_num >= funcs_total) {
            func_num = 0;
        }
        func_sta = func_sort_table[func_num];           //获取新的任务

        chk_dev = func_info[func_sta].dev_online;       //检查设备是否在线，不在线时跳过
        if(chk_dev == 0 || (chk_dev&dev_get_online()) != 0) {
            break;
        }
    }

    func_cb.sta = func_sta;

#if BT_BACKSTAGE_EN
    func_cb.sta_break = FUNC_NULL;  //切模式后清除打断恢复
#endif
}

#if WIRELESS_CON_AND_BT_PAIRING_EN
AT(.text.func)
void func_device_mode_select(void)
{
    //已经绑定无线麦,则进入DEVICE模式
    if(sys_cb.wl_bond_nb > 0) {
        func_cb.sta = FUNC_DEVICE;
    } else {
        func_cb.sta = FUNC_BT;
    }
}
#endif // WIRELESS_CON_AND_BT_PAIRING_EN

AT(.text.func)
void func_run(void)
{
    printf("%s\n", __func__);

#if WIRELESS_MIC_ROLE == 0
    if(func_cb.sta == FUNC_NULL) {
        func_cb.sta = FUNC_DEVICE;
    }
#elif WIRELESS_MIC_ROLE == 1
    if(func_cb.sta == FUNC_NULL) {
        func_cb.sta = FUNC_ADAPTER;
    }
#elif WIRELESS_MIC_ROLE == 2
    if(func_cb.sta == FUNC_NULL) {
        func_cb.sta = cfg_wireless_role? FUNC_ADAPTER : FUNC_DEVICE;
    }
#else
    if (PWRON_ENTER_BTMODE_EN || func_cb.sta == FUNC_NULL) {
        func_cb.sta = FUNC_BT;
    }
#endif

#if WIRELESS_CON_AND_BT_PAIRING_EN && FUNC_BT_EN
    if(func_cb.sta == FUNC_DEVICE) {
        func_device_mode_select();
    }
#endif

    func_bt_chk_off(0);
    while (1) {
        func_enter();
        switch (func_cb.sta) {
#if FUNC_ADAPTER_EN
        case FUNC_ADAPTER:
            func_adapter();
            break;
#endif

#if FUNC_DEVICE_EN
        case FUNC_DEVICE:
            func_device();
            break;
#endif

#if FUNC_BT_EN
        case FUNC_BT:
            func_bt();
            break;
#endif

#if FUNC_USBDEV_EN
        case FUNC_USBDEV:
            func_usbdev();
            break;
#endif

#if FUNC_IDLE_EN
        case FUNC_IDLE:
            func_idle();
            break;
#endif

#if FUNC_BT_DUT_EN
        case FUNC_BT_DUT:
            func_bt_dut();
            break;
#endif
#if FUNC_BT_FCC_EN
        case FUNC_BT_FCC:
            func_bt_fcc();
            break;
#endif
#if IODM_TEST_EN
        case FUNC_BT_IODM:
            func_bt_iodm();
            break;
#endif
#if CHARGE_EN
        case FUNC_CHARGE:
            func_charge();
            break;
#endif
        case FUNC_PWROFF:
            func_pwroff(sys_cb.pwroff.tone_en);
            break;

        default:
            func_exit();
            break;
        }
    }
}
