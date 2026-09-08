#include "include.h"


xcfg_cb_t xcfg_cb;
sys_cb_t sys_cb AT(.buf.bsp.sys_cb);
uint8_t cfg_spiflash_speed_up_en = FLASH_SPEED_UP_EN | (FLASH_DUAL_READ * 0x02) | (FLASH_QUAD_READ * 0x04);     //SPI FLASH提速。部份FLASH不支持提速
uint8_t cfg_pmu_vddio_lp_enable = SYS_VDDIO_LP_EN;

u32 getcfg_pmu_mode(void);

#if MUSIC_SDCARD_EN
AT(.com_text.detect)
void sd_detect(void)
{
    if (SD_IS_ONLINE()) {
        if (dev_online_filter(DEV_SDCARD)) {
            msg_enqueue(EVT_SD_INSERT);
        }
    } else {
        if (dev_offline_filter(DEV_SDCARD)) {
            msg_enqueue(EVT_SD_REMOVE);
        }
    }
}
#endif

#if BT_PWRKEY_5S_DISCOVER_EN
AT(.com_text.detect)
void pwrkey_5s_on_check(void)
{
    if (!xcfg_cb.bt_pwrkey_nsec_discover) {
        return;
    }
    if (sys_cb.pwrkey_5s_check) {
        if (PWRKEY_IS_PRESS() || TKEY_IS_PRESS()) {
            if (tick_check_expire(sys_cb.ms_ticks, 1000 * xcfg_cb.bt_pwrkey_nsec_discover)) {
                sys_cb.pwrkey_5s_flag = 1;
                sys_cb.pwrkey_5s_check = 0;
            }
        } else {
            sys_cb.pwrkey_5s_check = 0;
        }
    }
}
#endif // BT_PWRKEY_5S_DISCOVER_EN

#if PWRKEY_2_HW_PWRON
//软开机模拟硬开关，松开PWRKEY就关机。
AT(.com_text.detect)
void pwrkey_2_hw_pwroff_detect(void)
{
    static int off_cnt = 0;

    if (PWRKEY_IS_PRESS()) {
        off_cnt = 0;
    } else {
        if (off_cnt < 10) {
            off_cnt++;
        } else if (off_cnt == 10) {
            //pwrkey已松开，需要关机
            off_cnt = 20;
            sys_cb.pwroff.hw_pwrdwn = 1;
            sys_cb.poweron_flag = 0;
        }
    }
}
#endif // PWRKEY_2_HW_PWRON

#if USER_NTC
AT(.com_text.ntc)
u8 sys_ntc_check(void)
{
    if (!xcfg_cb.ntc_en) {
        return  0;
    }
    static int n_cnt = 0;
    if (xcfg_cb.ntc_en) {
        if (saradc_get_value8(ADCCH_NTC) <= xcfg_cb.ntc_thd_val) {
            if (n_cnt >= 20) {
                sys_cb.pwroff_flag.ntc_ind = 1;
                return 1;
            } else {
                n_cnt++;
            }
        } else {
            n_cnt = 0;
        }
    }
    return 0;
}
#endif

//timer tick interrupt(1ms)
AT(.com_text.timer)
void usr_tmr1ms_isr(void)
{
#if (GUI_SELECT & DISPLAY_LEDSEG)
    gui_scan();                     //7P屏按COM扫描时，1ms间隔
#endif

#if LED_DISP_EN
    port_2led_scan();
#endif // LED_DISP_EN

    plugin_tmr1ms_isr();

    led_scan();
}

//timer tick interrupt(5ms)
AT(.com_text.timer)
void usr_tmr5ms_thread(void)
{
    uint8_t ret;

    tmr5ms_cnt++;
    //5ms timer process
    ret = dac_fade_process();
#if LOUDSPEAKER_MUTE_EN
    if(ret == 1) {
        dac_set_mute_callback(0);               //unmute before fade_in
    } else if(ret == 3) {
        dac_set_mute_callback(1);   //mute after fade_out
    }
#else
    ret = ret;
#endif

    bsp_key_scan();

#if PWRKEY_2_HW_PWRON
    pwrkey_2_hw_pwroff_detect();
#endif

#if (CHARGE_BOX_TYPE == CBOX_SSW)
    charge_box_heart_beat_ack();
#endif
    plugin_tmr5ms_isr();

#if USER_NTC
    sys_ntc_check();
#endif

#if USB_SUPPORT_EN
    usb_detect();
#endif // USB_SUPPORT_EN

#if AUX_DETECT_EN
    aux_detect();
#endif // AUX_DETECT_EN

#if WARNING_WSBC_RES_EN
    warning_dec_check_kick();
#endif

    //50ms timer process
    if ((tmr5ms_cnt % 10) == 0) {
#if BT_PWRKEY_5S_DISCOVER_EN
        pwrkey_5s_on_check();
#endif // BT_PWRKEY_5S_DISCOVER_EN
    }

    //100ms timer process
    if ((tmr5ms_cnt % 20) == 0) {
        lowpwr_tout_ticks();
#if UDE_HID_EN
        if (sys_cb.ude_flag) {
            ude_tmr_isr();              //func_usbdev才调用，避免ram复用问题
        }
#endif // UDE_HID_EN
        gui_box_isr();                  //显示控件计数处理

        if (sys_cb.key2unmute_cnt) {
            sys_cb.key2unmute_cnt--;
            if (!sys_cb.key2unmute_cnt) {
                msg_enqueue(EVT_KEY_2_UNMUTE);
            }
        }
        dac_fifo_detect();
    }
#if MUSIC_SDCARD_EN
    if ((func_cb.sta == FUNC_DEVICE || func_cb.sta == FUNC_BT) && !wireless_role_is_adapter()) {
        sd_detect();
    }
#endif
    //500ms timer process
    if ((tmr5ms_cnt % 100) == 0) {
        msg_enqueue(MSG_SYS_500MS);
    }
    //1s timer process
    if ((tmr5ms_cnt % 200) == 0) {
        msg_enqueue(MSG_SYS_1S);
        msg_enqueue(EVT_RECORD_1S);
        sys_cb.cm_times++;
        sys_cb.lbat_warning_delay++;
    }
    //10s timer process
    if ((tmr5ms_cnt % 6000) == 0) {
        msg_enqueue(EVT_MSG_SYS_30S);
        tmr5ms_cnt = 0;
    }
}

uint bsp_get_bat_level(void)
{
#if VBAT_DETECT_EN
    //计算方法：level = (实测电压 - 关机电压) / ((满电电压 - 关机电压) / 100)
    u16 bat_off = sys_cb.vbat_pwroff;
    if (bat_off > sys_cb.vbat) {
        return 0;
    }
    uint bat_level = (sys_cb.vbat - bat_off) / ((4200 - bat_off) / 100);
    //printf("bat level: %d %d\n", sys_cb.vbat, bat_level);
    if (bat_level > 100) {
        bat_level = 100;
    }
    return bat_level;
#else
    return 100;
#endif
}

AT(.text.bsp.sys.init)
void rtc_32k_configure(void)
{
    u32 temp = RTCCON0;

//    //xosc_div768_rtc
//    temp &= ~BIT(6);
//    temp |= BIT(9) | BIT(8);                        //sel xosc_div768_rtc
//    RTCCON0 = temp;
//    RTCCON4 |= BIT(17);                             //xosc24m low power clk enable
//    RTCCON2 = 31249;

    //clk2m_rtc_div32
    temp &= ~BIT(6);
    temp &= ~(BIT(9) | BIT(8));
    temp |= BIT(8);
    temp |= BIT(2) | BIT(0);
    RTCCON0 = temp;
    RTCCON2 = sys_get_rc2m_rtc_clk() / 64 - 1;

}

AT(.text.bsp.sys.init)
bool rtc_init(void)
{
    rtc_32k_configure();
    if (sys_cb.rst_reason & RST_RTC_PWRUP) {  //rtc 1st pwrup
        return false;
    }

    return true;
}

//UART0打印信息输出GPIO选择，UART0默认G1(PA7)
void uart0_mapping_sel(void)
{
    //等待uart0发送完成
    if(UART0CON & BIT(0)) {
        while (!(UART0CON & BIT(8)));
    }

    GPIOEDE  &= ~BIT(13);
    GPIOEPU  &= ~BIT(13);
    GPIOBPU  &= ~(BIT(2) | BIT(3));
    FUNCMCON0 = (0xf << 12) | (0xf << 8);           //clear uart0 mapping

#if (UART0_PRINTF_SEL == PRINTF_PA7)
    GPIOADE  |= BIT(7);
    GPIOAPU  |= BIT(7);
    GPIOADIR |= BIT(7);
    GPIOAFEN |= BIT(7);
    GPIOADRV |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PA7;           //RX0 Map To TX0, TX0 Map to G1
#elif (UART0_PRINTF_SEL == PRINTF_PB2)
    GPIOBDE  |= BIT(2);
    GPIOBPU  |= BIT(2);
    GPIOBDIR |= BIT(2);
    GPIOBFEN |= BIT(2);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB2;           //RX0 Map To TX0, TX0 Map to G2
#elif (UART0_PRINTF_SEL == PRINTF_PB3)
    GPIOBDE  |= BIT(3);
    GPIOBPU  |= BIT(3);
    GPIOBDIR |= BIT(3);
    GPIOBFEN |= BIT(3);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PB3;           //RX0 Map To TX0, TX0 Map to G3
#elif (UART0_PRINTF_SEL == PRINTF_PE7)
    GPIOEDE  |= BIT(7);
    GPIOEPU  |= BIT(7);
    GPIOEDIR |= BIT(7);
    GPIOEFEN |= BIT(7);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE7;           //RX0 Map To TX0, TX0 Map to G4
#elif (UART0_PRINTF_SEL == PRINTF_PE0)
    GPIOEDE  |= BIT(0);
    GPIOEPU  |= BIT(0);
    GPIOEDIR |= BIT(0);
    GPIOEFEN |= BIT(0);
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_PE0;           //RX0 Map To TX0, TX0 Map to G5
#elif (UART0_PRINTF_SEL == PRINTF_VUSB)
    FUNCMCON0 = URX0MAP_TX | UTX0MAP_VUSB;          //RX0 Map To TX0, TX0 Map to G8
#endif
}

//开user timer前初始化的内容
AT(.text.bsp.sys.init)
static void sys_var_init(void)
{
    u32 rst_reason = sys_cb.rst_reason;
    memset(&sys_cb, 0, sizeof(sys_cb));
    sys_cb.rst_reason = rst_reason;
    sys_cb.loudspeaker_mute = 1;
    sys_cb.ms_ticks = tick_get();
    sys_cb.pwrkey_5s_check = 1;
    xcfg_cb.vol_max = (xcfg_cb.vol_max) ? 32 : 16;
    sys_cb.hfp2sys_mul = (xcfg_cb.vol_max + 2) / 16;
    sys_cb.init_vol         = (xcfg_cb.vol_max * xcfg_cb.sys_init_vol_perc + 50)/100;           //SYS_INIT_VOLUME
    sys_cb.recover_initvol  = (xcfg_cb.vol_max * xcfg_cb.sys_recover_initvol_perc + 50)/100;    //SYS_LIMIT_VOLUME
    sys_cb.warning_volume   = (xcfg_cb.vol_max * xcfg_cb.warning_volume_perc + 50)/100;         //WARNING_VOLUME


    sys_cb.pwroff.delay_ticks = xcfg_cb.pwroff_press_time * 500 + 1500 - (KEY_LONG_TIMES*5);
    sys_cb.sleep_time = -1L;
    sys_cb.pwroff_time = -1L;
    if (xcfg_cb.sys_sleep_time != 0) {
        sys_cb.sleep_time = (u32)xcfg_cb.sys_sleep_time * 10;   //100ms为单位
    }
    if (xcfg_cb.sys_off_time != 0) {
        sys_cb.pwroff_time = (u32)xcfg_cb.sys_off_time * 10;    //100ms为单位
    }

    sys_cb.sleep_delay = -1L;
    sys_cb.pwroff_delay = -1L;
    sys_cb.sleep_en = 0;
    sys_cb.led_scan_en = 1;

    if(xcfg_cb.osci_cap == 0 && xcfg_cb.osco_cap == 0) {        //没有过产测时，使用自定义OSC电容
        xcfg_cb.osci_cap = xcfg_cb.uosci_cap;
        xcfg_cb.osco_cap = xcfg_cb.uosco_cap;
    }
    if(xcfg_cb.ft_rf_param_en == 0 && xcfg_cb.bt_rf_param_en) { //使用自定义参数时，不需要微调
        xcfg_cb.bt_rf_pwrdec = 0;
    }

    sys_cb.lbat_warning_times = LPWR_WARING_TIMES;
    sys_cb.vbat_pwroff = ((u16)LPWR_OFF_VBAT*100+2700);
    sys_cb.vbat_warning = ((u16)LPWR_WARNING_VBAT*100+2800);

    saradc_var_init();
    key_var_init();
    plugin_var_init();

    msg_queue_init();
    bsp_res_init();

    dev_init(((u8)is_sd_support()) | ((u8)is_usb_support() * 0x02));

    sdadc_var_init();

    music_stream_var_init();
    msc_pcm_out_var_init();

    u8 vcm_sel = (DAC_MAXOUT_EN)? DAC_MAXOUT_VCM_SEL : DAC_VCM_SEL;
    dac_cb_init((DAC_CH_SEL & 0x0f) | (vcm_sel << 4) | (0x200 * DAC_FAST_SETUP_EN) \
                | (0x400 * DAC_VCM_CAPLESS_EN) | (0x800 * DAC_MAXOUT_EN), \
                LOUDSPEAKER_UNMUTE_DELAY * LOUDSPEAKER_MUTE_EN);
}

AT(.text.bsp.sys.init)
static void sys_io_init(void)
{
    //全部设置成模拟GPIO，防止漏电。使用时，自行配置对应数字GPIO
    GPIOADE = 0;
    GPIOBDE = 0;
    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F; //MCP FLASH

    uart0_mapping_sel();        //调试UART IO选择或关闭

#if AUX_DETECT_EN
    AUX_DETECT_INIT();
#endif // AUX_DETECT_EN

    LOUDSPEAKER_MUTE_INIT();

    MIC_LDO_INIT();
}

void xosc_get_cfg_cap(u8 *osci_cap, u8 *osco_cap, u8 *both_cap)
{
    *osci_cap = xcfg_cb.osci_cap;
    *osco_cap = xcfg_cb.osco_cap;
    *both_cap = xcfg_cb.osc_both_cap;
}

AT(.text.bsp.power)
static bool power_on_check_pend(u32 rtccon9)
{
#if PWRON_FRIST_BAT_EN
    if (sys_cb.rst_reason & RST_RTC_PWRUP) {        //第一次上电是否直接开机
        return true;
    }
#endif
    if (sys_cb.rst_reason & RST_WKO10S) {           //is wko 10s reset pending
        return true;                                //长按PWRKEY 10S复位后直接开机。
    }
    if(sys_cb.sw_rst_flag == SW_RST_FLAG){
        return true;
    }

    if (CHARGE_DC_IN() && (!CHARGE_WORKING_WHILE_CHARGING)&& (CHARGE_DC_NOT_PWRON) && (xcfg_cb.charge_en)) {   //VUSB充电禁止开机
        return false;
    }

    if (sys_cb.rst_reason & (RST_WDT|RST_RTC_WDT|RST_LVD)) {
        return true;                                //WDT、LVD复位后直接开机。
    }

    //清除不允许直接开机的pending
    rtccon9 &= ~BIT(4);                             //charge inbox wakeup
    rtccon9 &= ~BIT(6);                             //TK Wakeup pending
#if WKO_SOURCE == WK0
        rtccon9 &= ~BIT(2);           //WK0 wakeup
#else
        rtccon9 &= ~(BIT(7) << WKO_SOURCE);//WK1~3 wakeup
#endif // WKO_SOURCE

    return (rtccon9 != 0);
}

static bool power_on_check_reset(u32 rtccon9)
{
    bool ret = false;
    if (power_on_check_pend(rtccon9)) {
        ret = true;
    }
#if QTEST_EN
    if(qtest_get_mode()){
        ret = true;
    }
#endif

#if IODM_TEST_EN
    if (cm_read8(PARAM_RST_FLAG) == RST_FLAG_MAGIC_VALUE) {
        cm_write8(PARAM_RST_FLAG, 0);
        cm_sync();
        printf("iodm rst power_on\n");
        ret = true;
    }
#endif
#if !PWRKEY_EN
    if ((!PWRKEY_2_HW_PWRON) && (!sys_cb.tkey_pwrdwn_en)) {
        ret = true;
    }
#endif
#if CHARGE_BOX_EN
    if ((rtccon9 & BIT(4)) && (xcfg_cb.ch_out_auto_pwron_en) && (xcfg_cb.ch_box_type_sel != 3)) {    //charge inbox wakeup直接开机(5V完全掉电无维持电压的仓不开机)
        if (charge_box_outbox_stable_check()) {
            sys_cb.outbox_pwron_flag = 1;
            printf("inbox wakeup\n");
            return true;
        }
    }
#endif
    return ret;
}

AT(.text.bsp.power)
void power_on_check(void)
{
    u8 charge_sta = CHAG_STA_UNINIT;
    bool pwron_flag = false;
    struct pwrkey_scan_tag pwrkey;
    u32 rtccon9 = RTCCON9;                          //wakeup pending

    printf("power_on_check: %08x\n", rtccon9);

    RTCCON9 = 0xffff;                               //Clr pending
    RTCCON10 = BIT(10) | BIT(1) | BIT(0);           //Clr pending
    CRSTPND = 0x1ff0000;                            //clear reset pending
    LVDCON &= ~(0x1f << 8);                         //clear software reset
    //RTCCON13 &= ~BIT(13);                           //wko pin0 low level wakeup

    if (power_on_check_reset(rtccon9)) {
        return;
    }



#if CHARGE_EN
    u32 counter = 0;
    if(CHARGE_DC_NOT_PWRON && xcfg_cb.charge_en) {
        u8 chbox_out2pwr_en = 0;
        //是否需要拿起开机
        if ((sys_cb.rst_reason & RST_VUSB) || (rtccon9 & BIT(3)) || (sys_cb.sw_rst_flag == SW_RST_DC_IN) || (sys_cb.inbox_wken_flag)) {    //vusb reset or wakeup
            chbox_out2pwr_en = xcfg_cb.ch_out_auto_pwron_en;
        }
        sfunc_charge_enter(chbox_out2pwr_en);
    }

    if(CHARGE_WORKING_WHILE_CHARGING && xcfg_cb.charge_en) {
        if (sys_cb.rst_reason & RST_VUSB) {
		     //边充电边工作模式,接入就开机
             return;
        }
    }
#endif

    memset(&pwrkey, 0x00, sizeof(pwrkey));
    pwrkey.press_time = PWRON_PRESS_TIME;
    if (pwrkey.press_time == 0) {
        pwrkey.press_time = 15;                     //最小开机时间在100ms左右
    }

    while (1) {
        WDT_CLR();
#if CHARGE_LOW_POWER_EN
        delay_us(350);
#else
        delay_5ms(1);
#endif

        if(bsp_pwrkey_scan(&pwrkey) && pwrkey.state == PWRKEY_PRESS) {
            sys_cb.ms_ticks = pwrkey.ticks;         //记录PWRKEY按键按下的时刻
        }

#if CHARGE_EN
        if ((CHARGE_DC_NOT_PWRON) && xcfg_cb.charge_en) {
            charge_sta = sfunc_charge_process(counter++);
            if(charge_sta == CHAG_STA_OFF) {
#if CHARGE_BOX_EN
                if(xcfg_cb.ch_out_auto_pwron_en) {      //出充电仓，拿起开机
                    pwron_flag = true;
                    break;
                }
#endif
            } else if(charge_sta == CHAG_STA_OFF_VBUS_PATH || charge_sta == CHAG_STA_OFF_INBOX) {
                if(xcfg_cb.ch_full_auto_pwrdwn_en) {    //充满关机
                    break;
                }
            }
            if(CHARGE_DC_IN() || (charge_sta > CHAG_STA_W4_DCIN)) {    //充电情况下需要等待状态稳定
                continue;
            }
        }
#endif

        if (pwrkey.state == PWRKEY_PRESS) {         //PWKKEY已长按
#if VBAT_DETECT_EN
            if (sys_cb.vbat <= 2950) {              //电压小于2.95v不开机
                continue;
            }
#endif
            key_set_ignore(pwr_usage_id);           //忽略开机时第1次按键直到松开
            pwron_flag = true;
            break;
        } else if(pwrkey.state == PWRKEY_RELEASE) { //PWKKEY已松开
            if ((!SOFT_POWER_ON_OFF) || ((!PWRKEY_EN) && (!TKEY_SOFT_PWR_EN))) {
                pwron_flag = true;                  //没有软开关机键，直接开机
            }
            break;
        }
    }
#if CHARGE_EN
    sfunc_charge_exit();
#endif

    if(!pwron_flag) {                               //重新关机
        bsp_saradc_exit();
        sfunc_pwrdown((charge_sta==CHAG_STA_OFF_VBUS_PATH)? 0 : 1);
        while(1);
    }
}

AT(.text.bsp.sys.init)
void sys_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
        WDT_RST();
        while(1);
    }

//    sys_stack_dump();       //打印线程堆栈情况

    // io init
    sys_io_init();

    // var init
    sys_var_init();

    // power init
    pmu_init(getcfg_pmu_mode());

    //audio pll init
    adpll_init(DAC_OUT_SPR);

    // clock init
    sys_clk_set(SYS_CLK_SEL);

    // peripheral init
    rtc_init();
    param_init(sys_cb.rst_reason & RST_RTC_PWRUP);

#if OFFLINE_LOG_EN
    offline_log_init(OFFLINE_LOG_START, OFFLINE_LOG_SIZE, OFFLINE_LOG_SIZE/2);
    sys_log_info("hello, reset reson %x\n", sys_cb.rst_reason);
#endif

    //晶振配置
    xosc_init();

    plugin_init();

    if (POWKEY_10S_RESET) {
        WKO_10SRST_EN(0);                                   //10s reset source select  0: wko pin press, 1: touch key press
    } else {
        WKO_10SRST_DIS();
    }

    led_init();

#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_init();
    }
#endif
    key_init();

#if QTEST_EN
    if(QTEST_IS_ENABLE()) {
        qtest_init();
    }
#endif

#if HUART_EN
    if(xcfg_cb.huart_en){
        bsp_huart_init();
    }
#endif // HUART_EN

#if UART1_EN
    if (xcfg_cb.uart1_en) {
        bsp_uart1_init(9600);
    }
#endif

#if UART2_COM_EN
    bsp_uart2_com_init(UART2_COM_BAUD);
#endif

#if TBOX_TEST_EN
    bsp_vusb_test_process();
#endif

#if TBOX_TEST_EN
    if (!vusb_test_is_sucess())
#endif
    {
        power_on_check();               //在key_init之后
    }
    gui_init();

    gsensor_init();

    en_auto_pwroff();

    /// enable user timer for display & dac
    sys_set_tmr_enable(1, 1);

    led_power_up();
    gui_display(DISP_POWERON);

    bt_init();
    if (bsp_dac_off_for_bt_conn()) {
        dac_init();
//        func_bt_init();
    } else {
//        func_bt_init();
        dac_init();
    }
    bsp_res_set_enable(true);           //dac初始化后使能播提示音

    app_init_do();

    bsp_change_volume(sys_cb.vol);

#if WARNING_POWER_ON
    if ((!sys_cb.outbox_pwron_flag)) {
        bsp_res_play(RES_IDX_POWERON);
    }
    sys_cb.outbox_pwron_flag = 0;

    //等待开机提示音播报完，并完成设备插入检测
    do {
        func_process();
    } while(bsp_res_is_playing());
#endif // WARNING_POWER_ON

#if WIRELESS_EN
    wireless_init();
#endif

#if !DEVICE_DAC_OUTPUT_EN && !DEVICE_INTERPHONE_EN
    if(!wireless_role_is_adapter()) {
        dac_power_off();
        sys_cb.dac_disable = 1;
    }
#endif

#if SPI_HW_EN
    bsp_spi_init();
#endif

#if MUSIC_SDCARD_EN
    SD_DETECT_INIT();
#endif
#if DEVICE_RECORD_EN
    record_var_init();
#endif

#if EQ_DBG_IN_UART || EQ_DBG_IN_SPP
    eq_dbg_init();
#endif // EQ_DBG_IN_UART

#if PLUGIN_SYS_INIT_FINISH_CALLBACK
    plugin_sys_init_finish_callback(); //初始化完成, 各方案可能还有些不同参数需要初始化,预留接口到各方案
#endif
}


AT(.text.bsp.sys.init)
void sys_update_init(void)
{
    /// config
    if (!xcfg_init(&xcfg_cb, sizeof(xcfg_cb))) {           //获取配置参数
        printf("xcfg init error\n");
    }

    // io init
    sys_io_init();

    // var init
    sys_var_init();
    sys_cb.lang_id = 0;

    // power init
    pmu_init(getcfg_pmu_mode());

    // peripheral init
    rtc_init();
    param_init(sys_cb.rst_reason & RST_RTC_PWRUP);

    //晶振配置
    xosc_init();

    plugin_init();
    sys_set_tmr_enable(1, 1);

    adpll_init(DAC_OUT_SPR);
    dac_init();
    bsp_res_play(RES_IDX_UPDATE_DONE);
}
