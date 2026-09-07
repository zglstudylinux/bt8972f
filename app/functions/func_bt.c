#include "include.h"
#include "func.h"
#include "func_bt.h"


func_bt_t f_bt;
void uart_cmd_process(void);


ALIGNED(128)
u16 func_bt_chkclr_warning(u16 bits)
{
    u16 value;
    GLOBAL_INT_DISABLE();
    value = f_bt.warning_status & bits;
    if(value != 0) {
        f_bt.warning_status &= ~value;
        GLOBAL_INT_RESTORE();
        return value;
    }
    GLOBAL_INT_RESTORE();
    return value;
}


#if FUNC_BT_EN
void func_bt_set_dac(u8 enable)
{
    if (bsp_dac_off_for_bt_conn()) {
        if (enable) {
            if (!dac_get_pwr_sta()) {
                dac_restart();
            }
        } else {
            if (dac_get_pwr_sta()) {
                dac_power_off();
            }
        }
    }
}

//切换提示音语言
void func_bt_switch_voice_lang(void)
{
#if (LANG_SELECT == LANG_EN_ZH)
    if (xcfg_cb.lang_id >= LANG_EN_ZH) {
        sys_cb.lang_id = (sys_cb.lang_id) ? 0 : 1;
        multi_lang_init(sys_cb.lang_id);
        param_lang_id_write();
        param_sync();

        bsp_res_play(RES_IDX_LANGUAGE_EN + sys_cb.lang_id);                     //同步播放语言提示音
    }
#endif
}

u8 func_bt_tws_get_channel(void)
{
    return false;
}

void func_bt_warning_do(void)
{
#if QTEST_EN
    if(qtest_get_mode()){
        func_bt_chkclr_warning(0xffff);
        return;
    }
#endif

    if(func_bt_chkclr_warning(BT_WARN_DISCON)) {
#if WARNING_BT_DISCONNECT
        if(!bt_tws_is_slave()) {
            bsp_res_play(RES_IDX_DISCONNECT);
            return;
        }
#endif // WARNING_BT_DISCONNECT
    }

    if(func_bt_chkclr_warning(BT_WARN_PAIRING)) {
        if(!bt_tws_is_slave()) {
            bsp_res_play(RES_IDX_PAIRING);
            return;
        }
    }

	if(func_bt_chkclr_warning(BT_WARN_CON)) {
#if WARNING_BT_CONNECT
        if(!bt_tws_is_slave()) {
            bsp_res_play(RES_IDX_CONNECTED);
            return;
        }
#endif
    }

#if BT_HID_MANU_EN
    //按键手动断开HID Profile的提示音
    if (xcfg_cb.bt_hid_manu_en) {
    #if WARNING_BT_HID_MENU
        if (func_bt_chkclr_warning(BT_WARN_HID_CON)) {
            bsp_res_play(RES_IDX_CAMERA_ON);
        }
    #endif

    #if WARNING_BT_HID_MENU
        if (func_bt_chkclr_warning(BT_WARN_HID_DISCON)) {
            bsp_res_play(RES_IDX_CAMERA_OFF);
        }
    #endif

    #if BT_HID_DISCON_DEFAULT_EN
        if (f_bt.hid_discon_flag) {
            if (bt_hid_is_ready_to_discon()) {
                f_bt.hid_discon_flag = 0;
                bt_hid_disconnect();
            }
        }
    #endif
    }
#endif // BT_HID_MANU_EN
}

AT(.text.func.bt.process)
void func_bt_warning(void)
{
    if(f_bt.warning_status != 0 && !bsp_res_is_full()) {
        func_bt_warning_do();
    }
}

void func_bt_vol_fade(void)
{
    //已经在music线程加了dac_fade_in/out处理，原则上不用在此处理
    //若担心其它地方乱调fade_out导致无声，可以打开以下dac_fade_in代码
//    if(dac_get_pwr_sta()) {
//        if(f_bt.disp_status > BT_STA_CONNECTED) {
//            if(!dac_is_fade_in()) {
//                dac_fade_in();
//            }
//        } else {
////            if(dac_is_fade_in()) {
////                dac_fade_out();
////            }
//        }
//    }
}

void func_bt_disp_status_do(void)
{
    if(!bt_is_connected()) {
        en_auto_pwroff();
    } else {
        dis_auto_pwroff();
    }
    sys_cb.sleep_en = 1;

    printf("bt_status: %d\n", f_bt.disp_status);

    switch (f_bt.disp_status) {
    case BT_STA_CONNECTING:
        if (BT_RECONN_LED_EN) {
            led_bt_reconnect();
            break;
        }
    case BT_STA_INITING:
    case BT_STA_IDLE:
        led_bt_idle();
#if WARNING_BT_PAIR
            if(f_bt.need_pairing && f_bt.disp_status == BT_STA_IDLE) {
                f_bt.need_pairing = 0;
            }
#endif
        break;
    case BT_STA_SCANNING:
        led_bt_scan();
        break;

    case BT_STA_DISCONNECTING:
        led_bt_connected();
        break;

    case BT_STA_CONNECTED:
        led_bt_connected();
        break;
    case BT_STA_PLAYING:
        led_bt_play();
        break;
    }

    if(f_bt.disp_status >= BT_STA_CONNECTED) {
        f_bt.need_pairing = 1;
        func_bt_set_dac(1);
    } else {
        func_bt_set_dac(0);
    }

    if(!bt_a2dp_is_bypass() && !bsp_res_is_vol_busy()) {
        func_bt_vol_fade();
    }

#if BT_BACKSTAGE_EN
    if (f_bt.disp_status < BT_STA_PLAYING && func_cb.sta_break != FUNC_NULL) {
        func_cb.sta = func_cb.sta_break;
//        func_cb.sta_break = FUNC_NULL;
    }
#endif
}

AT(.text.func.bt.process) ALIGNED(128)
void func_bt_disp_status(void)
{
    uint status = bt_get_disp_status();

    GLOBAL_INT_DISABLE();
    if(f_bt.disp_status != status || f_bt.disp_update) {
        f_bt.disp_status = status;
        f_bt.disp_update = 0;
        GLOBAL_INT_RESTORE();

        func_bt_disp_status_do();
    } else {
        GLOBAL_INT_RESTORE();
    }
}

AT(.text.func.bt.process)
void func_bt_status(void)
{
    func_bt_disp_status();

    {
        func_bt_warning();
    }
}

//借用读参数区做load flash的动作，使flash一直busy
static void func_bt_load_flash(void)
{
    static u32 tick = 0;
    u8 load_buf[32];
    if (tick_check_expire(tick, 1000)) {
//        printf("load flash test\n");
        tick = tick_get();
    }
    cm_read(load_buf, PAGE0(0), 32);
    cm_read(load_buf, PAGE1(0), 32);
    cm_read(load_buf, PAGE2(0), 32);
}

AT(.text.func.bt.process)
void func_bt_sub_process(void)
{
    func_bt_status();
#if TKEY_DEBUG_EN
    bsp_tkey_spp_tx();
#endif
    if ((func_cb.sta == FUNC_BT_DUT || BT_DUT_MODE_EN) && bt_is_dut_testing()) {
        func_bt_load_flash();       //如果在DUT模式就让flash动起来，测试flash对rf的影响
    }
}

AT(.text.func.bt.process)
void func_bt_process(void)
{
    func_process();
    func_bt_sub_process();

    if(f_bt.disp_status == BT_STA_INCOMING) {
        key_set_msg_tbl(bt_call_key_msg_tbl);
        sfunc_bt_ring();
        key_set_msg_tbl(bt_music_key_msg_tbl);
        reset_sleep_delay();
        reset_pwroff_delay();
    } else if(f_bt.disp_status == BT_STA_OTA) {
        sfunc_bt_ota();
        reset_sleep_delay();
        reset_pwroff_delay();
    } else if(f_bt.disp_status >= BT_STA_OUTGOING) {
        key_set_msg_tbl(bt_call_key_msg_tbl);
        sfunc_bt_call();
        key_set_msg_tbl(bt_music_key_msg_tbl);
        reset_sleep_delay();
        reset_pwroff_delay();
    }

    if(sys_cb.pwroff_delay == 0) {
        sys_cb.pwroff.timeout_ind = 1;
        return;
    }
    if(sleep_process(bt_is_allow_sleep)) {
        f_bt.disp_status = 0xff;
    }

#if WIRELESS_CON_AND_BT_PAIRING_EN
    if(bt_is_connected() && sys_cb.wl_scan_flag == 1) {
        printf("wireless scan disable\n");
        sys_cb.wl_scan_flag = 0;
        wireless_scan_set_enable(0);
    }

    if(bt_is_connected() && sys_cb.bond_mode != BONDING_MODE_BT) {
        sys_cb.bond_mode = BONDING_MODE_BT;
        param_bond_mode_write();
    }
#endif
}

AT(.text.func.bt)
void func_bt_init(void)
{
    if (!sys_cb.bt_is_inited) {
        msg_queue_clear();
        func_bt_set_dac(0);
        bsp_bt_init();
        sys_cb.bt_is_inited = 1;
    }
}

AT(.text.func.bt)
bool func_bt_chk_off(u8 reason)
{
#if !BT_BACKSTAGE_EN
    bool off_flag = (bool)(func_cb.sta != FUNC_BT
#if FUNC_ADAPTER_EN
                           && func_cb.sta != FUNC_ADAPTER
#endif
#if FUNC_DEVICE_EN
                           && func_cb.sta != FUNC_DEVICE
#endif
                           );
#else
    bool off_flag = (bool)(func_cb.sta == FUNC_PWROFF
                      || func_cb.sta == FUNC_BT_DUT
                      || func_cb.sta == FUNC_BT_FCC
                      || func_cb.sta == FUNC_BT_IODM
                      || func_cb.sta == FUNC_CHARGE
                    #if FUNC_USBDEV_EN
                      || func_cb.sta == FUNC_USBDEV
                    #endif
                      );
#endif

    if (off_flag && sys_cb.bt_is_inited) {
#if BT_PWRKEY_5S_DISCOVER_EN
        bsp_bt_pwrkey5s_clr();
#endif
        bt_release(reason);
        bt_off();
        func_bt_set_dac(1);
        sys_cb.bt_is_inited = 0;

        return true;
    } else if(func_cb.sta == FUNC_DEVICE && sys_cb.bt_is_inited) {
        if(bt_is_connected()){
            bt_release(reason);
        }
        bt_scan_disable();
        return true;
    }

    return false;
}

bool func_bt_charge_dcin(void)
{
#if QTEST_EN
    u32 qtest_5v_tick = tick_get();
    if(qtest_get_mode()){
        return false;
    }

    if(qtest_cb.sta) {
        while(!tick_check_expire(qtest_5v_tick, 1000)) {
            if(!CHARGE_DC_IN()) {
                return false;
            }
            if(func_cb.sta == FUNC_BT) {
                bt_thread_check_trigger();
                bsp_res_process();
            }
        }
        qtest_cb.sta = 0;
    }
#endif

    if ((func_cb.sta != FUNC_BT)) {
		sys_cb.discon_reason = 0;

		//开启UART2检测VUSB KEY，避免func_bt_exit过程太久，无法进入VUSB升级
		sys_clk_set(SYS_24M);
		uart2_key_mode();
        if (bt_is_connected()){
            func_bt_exit();
        }
        sw_reset_kick(SW_RST_DC_IN);                    //直接复位进入充电
        while(1);
    }
    if(sys_cb.discon_reason == 0xff) {
        sys_cb.discon_reason = 0;   //不同步关机
    }

    return true;
}

AT(.text.func.bt)
void func_bt_enter(void)
{
    bool break_flag = false;
#if BT_BACKSTAGE_EN
    //后台播放进入蓝牙模式不需要播提示音
    break_flag = (bool)(func_cb.sta_break != FUNC_NULL);
#endif

    u8 addr[6];
    bt_get_local_bd_addr(addr);
    printf("BT ADDR: ");
    print_r(addr, 6);

    func_bt_enter_display();
    led_bt_init();
    func_bt_init();
    //en_auto_pwroff();

    if(!break_flag) {
#if WARNING_FUNC_BT
        bsp_res_play(RES_IDX_BT_MODE);
#endif

#if WARNING_BT_WAIT_CONNECT
        bsp_res_play(RES_IDX_WAIT4CONN);
#endif
    }

    f_bt.disp_status = 0xfe;
    f_bt.rec_pause = 0;
    f_bt.pp_2_unmute = 0;
    sys_cb.key2unmute_cnt = 0;

    if(sys_cb.vol != sys_cb.a2dp_vol && (sys_cb.incall_flag == 0)) {
        bsp_change_volume(sys_cb.a2dp_vol);
    }

    bt_redial_init();
    bt_audio_enable();

#if BT_PWRKEY_5S_DISCOVER_EN
    if(bsp_bt_pwrkey5s_check()) {
        f_bt.need_pairing = 0;  //已经播报了
        func_bt_disp_status();
        bsp_res_play(RES_IDX_PAIRING);
    } else {
        func_bt_disp_status();
#if WARNING_BT_PAIR
        if (xcfg_cb.warning_bt_pair && !xcfg_cb.bt_tws_en) {
            bsp_res_play(RES_IDX_PAIRING);
        }
#endif // WARNING_BT_PAIR
    }
#endif

#if WIRELESS_CON_AND_BT_PAIRING_EN
    sys_cb.wl_scan_flag = 0;

    if(sys_cb.bond_mode == BONDING_MODE_BT) {
        //已经绑定经典蓝牙的情况,直接打开bt_scan进行回连
        bt_scan_enable();
    } if(func_cb.last == FUNC_DEVICE) {
        //上一次FUNC_DEVICE, 现转到BT, 说明是按键控制进入配对状态
        msg_enqueue(MSG_PAIRING);
    } else {
        //否则等按键控制进入配对状态的时候,再打开scan(这种情况一般是烧录完程序, 第一次上电)
        bt_scan_disable();
    }
#endif
    sys_cb.device_usb_init_flag = true;
}

AT(.text.func.bt)
void func_bt_exit(void)
{
#if !BT_BACKSTAGE_EN
    bsp_res_w4_finish(true);
    bsp_res_set_enable(false);
    bsp_res_cleanup();
#endif

#if DEVICE_RECORD_EN
    sfunc_record_exit();           //切模式，停止录音
#endif

    dac_fade_out();

#if BT_PWRKEY_5S_DISCOVER_EN
    bsp_bt_pwrkey5s_clr();
#endif
    func_bt_exit_display();
    bt_audio_bypass();

#if !BT_BACKSTAGE_EN
    if(sys_cb.discon_reason == 0xff && func_cb.sta == FUNC_PWROFF) {
        sys_cb.discon_reason = 1;   //默认同步关机
    }
    func_bt_chk_off(sys_cb.discon_reason);
#else
    if(bt_tws_is_slave()) {
        if(sys_cb.discon_reason == 0xff && func_cb.sta == FUNC_PWROFF) {
            sys_cb.discon_reason = 0;   //蓝牙后台，不同步关机
        }
        func_bt_chk_off(sys_cb.discon_reason);
    } else {
        if (bt_get_status() == BT_STA_PLAYING && !bt_is_testmode()) {        //蓝牙退出停掉音乐
            delay_5ms(10);
            if(bt_get_status() == BT_STA_PLAYING) {     //再次确认play状态
                u32 timeout = 250; //2.5s
                bt_music_pause();
                while (bt_get_status() == BT_STA_PLAYING && timeout > 0) {
                    func_process();
                    timeout--;
                    delay_5ms(2);
                }
            }
        }
    }
#endif
    if(sys_cb.vol != sys_cb.a2dp_vol) {
        bsp_change_volume(sys_cb.vol);
    }

    f_bt.rec_pause = 0;
    f_bt.pp_2_unmute = 0;
    sys_cb.key2unmute_cnt = 0;
    func_bt_set_dac(1);
    func_cb.last = FUNC_BT;
}

AT(.text.func.bt)
void func_bt(void)
{
    printf("%s\n", __func__);

    func_bt_enter();

    while (func_cb.sta == FUNC_BT) {
        func_bt_process();
        func_bt_message(msg_dequeue());
        func_bt_display();
    }

    func_bt_exit();
}

#endif //FUNC_BT_EN
