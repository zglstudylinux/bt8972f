#include "include.h"
#include "func.h"


AT(.text.pwroff.save)
void sfunc_power_save_enter(void)
{
    LOUDSPEAKER_MUTE_DIS();
#if CHARGE_EN
    if (xcfg_cb.charge_en) {
        charge_off();
    }
#endif // CHARGE_EN
    pmu_set_mode(getcfg_pmu_mode() & BIT(3));
    //usb_disable();
    GPIOADE = 0;

    GPIOBDE = BIT(WKO_SOURCE);           //pwrkey

    GPIOEDE = 0;
    GPIOFDE = 0;
    GPIOGDE = 0x3F;                         //MCP FLASH

    PICCONCLR = BIT(0);                     //Globle IE disable
    CLKCON0 |= BIT(0);                      //enable RC
    CLKCON0 = (CLKCON0 & ~(0x3 << 2));      //system clk select RC
    PLL0CON0 &= ~BIT(18);                   //pll10 sdm enable disable
    PLL0CON0 &= ~BIT(6);                    //pll0 analog diable
    PLL0CON0 &= ~BIT(12);                   //pll0 ldo disable

    XOSCCON &= ~(0xf<<24);                  //X26 output DIS
    XOSCCON &= ~BIT(10);                    //X26 DIS
}

///充满关机前等待仓休眠5V掉电
AT(.text.pwroff)
u8 sfunc_pwrdown_w4_vusb_offline(void)
{
    u32 timeout = 12000, off_cnt = 0;
    if (xcfg_cb.ch_box_type_sel != 1) {
        while (timeout--) {
            WDT_CLR();
            delay_us(330);                  //5ms
            if (!CHARGE_DC_IN()) {
                off_cnt++;
            } else {
                off_cnt = 0;
            }
            if (off_cnt > 5) {
                return 1;                   //VUSB已掉线，打开VUSB唤醒
            }
        }
    }
    return 0;
}

//硬开关方案，低电时，进入省电状态
AT(.text.pwroff.lowbat)
void sfunc_lowbat(void)
{
    lock_code_pwrsave();
    sfunc_power_save_enter();
    WDT_DIS();
    RTC_WDT_DIS();
    asm("nop");asm("nop");asm("nop");

    WPTCON &= ~BIT(22);                 //cpu_dq_tout_en=0
    LPMCON |= BIT(0);                   //Sleep mode enable
    asm("nop");asm("nop");asm("nop");
    while(1);
}

//软开关方案，POWER键/低电时，进入关机状态
AT(.text.pwroff.pwrdown)
void sfunc_pwrdown_do(u8 vusb_wakeup_en)
{
#if TKEY_EN
    u32 tkey_wakeup_en = sys_cb.tkey_pwrdwn_en;
#if QTEST_EN
    if (qtest_cb.pdn_boat_flag) {
        tkey_wakeup_en = 0;
    }
#endif
#endif

    printf("pwr_down: %d\n", vusb_wakeup_en);
    vusb_reset_clr();
    RTCCON3 &= ~BIT(8);                             //rtc alarm wakeup disable
    RTCCON8 &= ~BIT(15);                            //RI_EN_VUSBDIV = 0
    RTCCON9 = 0xffff;                               //Clr pending
#if TKEY_LOWPWR_WAKEUP_DIS                     //电池无保护板且有内置触摸开关机功能方案，需要打开此宏
    if (sys_cb.vbat < 3050) {
        tkey_wakeup_en = 0;                         //低电关机, 关掉触模唤醒
    }
    if (tkey_wakeup_en) {
        RTCALM = RTCCNT + 300;                      //定时5分钟唤醒检查电池电量
        RTCCON3 |= BIT(8);
    }
#endif
    sfunc_power_save_enter();
    if (!vusb_wakeup_en) {
        RTCCON8 = (RTCCON8 & ~BIT(6)) | BIT(1);     //disable charger function
#if (CHARGE_BOX_TYPE == CBOX_NOR)
        vusb_wakeup_en = sfunc_pwrdown_w4_vusb_offline();
        if (xcfg_cb.ch_box_type_sel == 3) {         //5V完全掉电的仓
            RTCCON3 &= ~BIT(12);                    //RTCCON3[12], INBOX Wakeup disable
        }
#endif
    }

    WDT_DIS();
    vusb4s_reset_dis();                             //VUSB 4s reset disable
    vusb4s_reset_recover_clr();
    RTCCON11 |= BIT(4);                             //VUSB 4s reset select VUSB pull out

    RTCCON11 = (RTCCON11 & ~0x03) | BIT(2);         //WK PIN filter select 8ms
    uint rtccon3 = RTCCON3 & ~BIT(11);
    uint rtccon13 = RTCCON13 & ~(0x3f << 18);       //WK pin5~0 wakeup disable
    uint rtccon4 = RTCCON4 & ~(0x7 << 22);
#if CHARGE_EN
    if ((xcfg_cb.charge_en) && (vusb_wakeup_en)) {
        rtccon3 |= BIT(11);                         //VUSB wakeup enable
        RTCCON3 |= BIT(11);                         //VUSB wakeup enable
        RTCCON11 |= BIT(6);                         //VUSB pull out filter VUSB拔出滤波
        delay_us(100);
        RTCCON10 = BIT(3);
    }
#endif

#if TBOX_TEST_EN
    rtccon3 |= BIT(11);                             //VUSB wakeup enable  //产测模式需要支持5v唤醒
#endif // TBOX_TEST_EN

    RTCCON3 = rtccon3 & ~(BIT(10) | BIT(14));       //关WK PIN，再打开，以清除Pending
    PWRCON1 &= ~(0x1F<<14);                         //disable Flash Power Gate
    PWRCON1 |= BIT(18);                             //pdown flash power gate
#if TKEY_EN
    if ((RTCCON3 & BIT(12)) || sys_cb.lowbat_flag) {//仓内或低电关机？
        tkey_wakeup_en = 0;
    }

    if (tkey_wakeup_en) {
        RTCCON1 |= BIT(9);                          //tk digital voltage enable
        RTCCON3 |= BIT(14);                         //Touch key long press wakeup
        rtccon4 |= 0x4 << 22;                       //VRTC 0.96V
    } else {
        //保持电源，保证tkey通道能顺利关闭
        RTCCON0 |= BIT(5);                          //tk rst disable
        TKACON0 = 0;
        TKACON1 = 0;
        RTCCON0 &= ~BIT(4);                         //tk interface disable
#if !OFFLINE_LOG_EN
        RTCCON0 &= ~BIT(0);                         //RC2M_RTC Disable
#endif
        RTCCON0 &= ~BIT(5);                         //tk rst enable
        RTCCON1 &= ~BIT(9);                         //tk digital voltage disable
        rtccon4 |= 0x2 << 22;                       //VRTC 0.7V
    }
#else
#if !OFFLINE_LOG_EN
    RTCCON0 &= ~BIT(0);                             //RC2M_RTC Disable
#endif
    rtccon4 |= 0x2 << 22;                           //VRTC 0.7V
#endif
    RTCCON4 = rtccon4;

    RTCCON1 |= BIT(11);                             //VRTC control by hw
    RTCCON0 &= ~(BIT(22) | BIT(23));

    rtccon3 = RTCCON3 & ~0x17;                      //Disable VDDCORE VDDIO VDDBUCK, VDDXOEN
    rtccon3 |= BIT(6);                              //Core power down enable, VDDCORE short disable
    rtccon3 &= ~BIT(22);                            //LVCORE_DIS
    RTCCON3 &= ~BIT(22);                            //LVCORE_DIS, lvcore掉电
    rtccon3 |= BIT(7);                              //RI_EN_VDDIO_AON   RTC 2.9V LDO enable
    rtccon3 |= BIT(4)|BIT(19);                      //pd_core
#if PWRKEY_EN
    rtccon3 |= BIT(10);                             //WK pin wake up enable

#if WKO_SOURCE == WK0
    rtccon13 |= BIT(0) | BIT(6) | BIT(18);          //wk pin0 wakeup, input, pullup10k enable
#else
    rtccon13 |= ((BIT(1) | BIT(7) | BIT(19)) << WKO_SOURCE);       //0x40041 == rtccon13 |= BIT(0) | BIT(6) | BIT(18);
                                                    //wk pin1~3 wakeup, input, pullup10k enable
#endif // WKO_SOURCE
#endif // PWRKEY_EN
#if QTEST_EN
    if (qtest_cb.pdn_boat_flag) {                                         //若船运模式关机，只保留5V唤醒
        rtccon3 &= ~(BIT(14) | BIT(12) | BIT(10) | BIT(9) | BIT(8));      //touch key long press, inbox, wk pin, RTC one second, RTC alarm wakeup disable
        rtccon3 |= BIT(11);                                               //VUSB wakeup enable
    }
#endif
    RTC_WDT_DIS();
    LPMCON |= BIT(3);
    RTCCON &= ~(3 << 1);                            //rtc选择sys_clk div2, 避免关机时rtc配置来不及生效，导致关机功耗偏大1.5uA
    RTCCON |= BIT(5);                               //PowerDown Reset，如果有Pending，则马上Reset
    RTCCON13 = rtccon13;
    RTCCON3 = rtccon3;
    WPTCON &= ~BIT(22);                             //cpu_dq_tout_en=0
    LPMCON |= BIT(0);
    LPMCON |= BIT(1);                               //idle mode
    asm("nop");asm("nop");asm("nop");
    while (1);
}


void sfunc_pwrdown(u8 vusb_wakeup_en)
{
#if OFFLINE_LOG_EN
    rtc_32k_configure();
#endif
#if (CHARGE_BOX_TYPE == CBOX_SSW) || QTEST_EN
    bsp_vusb_uart_dis();
#endif
    lock_code_pwrsave();
    sfunc_pwrdown_do(vusb_wakeup_en);
}

AT(.com_text.wko)
static bool pwrkey_get_pressed(void)        //获取pwrkey是否按下的状态
{
    if (0 == (GPIOB & BIT(WKO_SOURCE))) {    //通过GPIO检测电平变化
        return true;
    }
    return false;
}

AT(.text.lowpwr.pwroff)
void func_pwroff(int pwroff_tone_en)
{
    printf("%s: %d\n", __func__, pwroff_tone_en);

#if OFFLINE_LOG_EN
    offline_log_end();
#endif
    bsp_res_w4_finish(false);
    bsp_res_cleanup();

#if BT_BACKSTAGE_EN
    func_bt_chk_off(sys_cb.discon_reason);
#endif

#if (CHARGE_BOX_TYPE == CBOX_SSW)
    if (vhouse_cb.inbox_sta) {
        charge_box_inbox_wakeup_enable();
    }
#endif
    gsensor_lowpwr();
    led_power_down();

#if WARNING_POWER_OFF
    bsp_res_set_enable(true);
    if (pwroff_tone_en == 1) {
        bsp_res_play(RES_IDX_POWEROFF);
    } else if (pwroff_tone_en == 2) {
#if WARNING_FUNC_AUX
        bsp_res_play(RES_IDX_AUX_MODE);
#endif
    }
#endif

    bsp_res_w4_finish(true);
    bsp_res_set_enable(false);
    bsp_res_cleanup();

    gui_off();

#if SOFT_POWER_ON_OFF
#if !PWRKEY_2_HW_PWRON
    {
        u8  dcin_cnt = 0;
        #if TKEY_EN
            tkey_press_timeout_clr();
        #endif
        while (pwrkey_get_pressed() || TKEY_IS_PRESS()) {      //等待PWRKWY松开
            if (CHARGE_DC_IN()) {
                dcin_cnt++;
                if (dcin_cnt > 3) {
                    sw_reset_kick(SW_RST_DC_IN);            //直接复位进入充电
                }
            } else {
                dcin_cnt = 0;
            }
    #if TKEY_EN
            if (sys_cb.tkey_pwrdwn_en) {
                tkey_press_timeout_process();
            }
    #endif
            delay_5ms(2);
            WDT_CLR();
        }
    }
#endif

    dac_power_off();                    //dac power down
    bsp_saradc_exit();                  //close saradc及相关通路模拟

    if (CHARGE_DC_IN()) {
        WDT_RST();                      //DC_IN, RST to power_on_check()进入假关机
    }

    if ((PWRKEY_2_HW_PWRON) && (sys_cb.poweron_flag)) {
#if WKO_SOURCE == WK0
        RTCCON13 |= BIT(24);            //WK0 High level wakeup
#else
        RTCCON13 |= (BIT(25) << WKO_SOURCE);//WK1~3 High level wakeup
#endif // WKO_SOURCE
    }
    sfunc_pwrdown(1);
#else // SOFT_POWER_ON_OFF
    dac_power_off();                    //dac power down
    bsp_saradc_exit();                  //close saradc及相关通路模拟

    sfunc_lowbat();                     //低电关机进入Sleep Mode
#endif // SOFT_POWER_ON_OFF
}
