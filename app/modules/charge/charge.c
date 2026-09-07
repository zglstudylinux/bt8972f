#include "include.h"

#if CHARGE_EN

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

charge_ctl_t charge_ctl;
dc_ctl_t dc_ctl;
#if CHARGE_LOW_POWER_EN
u32 charge_pll0con0;
#endif


AT(.text.charge.proc)
void charge_set_stop_time(u32 stop_time)
{
    charge_ctl.stop_time = stop_time;
}

AT(.text.init.charge)
void charge_init(void)
{
    charge_ctl_t *ctl = &charge_ctl;
    memset(ctl, 0, sizeof(charge_ctl_t));
    memset(&dc_ctl, 0, sizeof(dc_ctl_t));

    ctl->trick_curr_en    = CHARGE_TRICK_EN;
    ctl->const_curr       = CHARGE_CONSTANT_CURR;
    if (ctl->const_curr > 15) {
        ctl->const_curr = (ctl->const_curr - 15) * 2 + 15;
        if (ctl->const_curr > 75) {                                //5ma一个档位，最大电流配到380ma
            ctl->const_curr = 75;
        }
    }
    ctl->const_curr       = ctl->const_curr & 0x7f;
    ctl->trick_curr       = CHARGE_TRICKLE_CURR & 0xf;
    ctl->stop_curr        = CHARGE_STOP_CURR & 0x0f;
    ctl->stop_volt        = CHARGE_STOP_VOLT & 0x03;
    ctl->leakage          = CHARGE_LEAKAGE_CURR;
    ctl->inbox_voltage    = CHARGE_BOX_KEEP_VOLT;
    ctl->dcin_reset       = 0x01;
    ctl->trick_stop_volt  = CHARGE_TRICK_STOP_VOLT;
    ctl->stop_time        = (u32)xcfg_cb.charge_stop_time *10 *60 * 10;     //默认30分钟, 100ms为单位
    ctl->ldo_en           = 1;                          //默认普通充电使用LDO mode，
    ctl->volt_follow_en   = CHARGE_VOLT_FOLLOW_EN;
    ctl->volt_follow_diff = CHARGE_VOLT_FOLLOW_DIFF & 0x03;
    if (CHARGE_VOLT_FOLLOW_EN) {
        ctl->ldo_en       = 0;                          //开启电压跟随模式，强制不能开LD0 充电mode
    }

    if (RTCCON3 & BIT(12)) {
        sys_cb.inbox_wken_flag = 1;                     //修正触摸唤醒拿出不能开机问题
    }
    RTCCON3 &= ~BIT(12);                                //RTCCON3[12], INBOX Wakeup disable
    RTCCON11 &= ~BIT(6);                                //VUSB pull out filter

    charge_init_do(ctl);

#if CHARGE_BOX_EN
    charge_box_init();
#endif

    vusb4s_reset_en();

    if(CHARGE_WORKING_WHILE_CHARGING || TBOX_TEST_EN) {   //边充电边工作模式不考虑插入4s复位一下情况
//        RTCCON12 |= BIT(3)|BIT(2);         //关闭VUSB INSERT 4S RESET软件使能
//        RTCCON11 &= ~BIT(4);               //关闭VUSB INSERT 4S RESET硬件使能
        vusb4s_reset_dis();
    }
}

AT(.text.charge.proc)
void charge_off(void)
{
    if (charge_ctl.chag_sta >= CHAG_STA_ON_TRICKLE) {
        charge_stop(&charge_ctl, 0);
        charge_status_disp_cb(0);
    }
}

AT(.text.charge.det)
bool charge_dc_detect(void)
{
    u8 dc_sta = CHARGE_DC_IN();

    //5V插入检测
    if (dc_sta != dc_ctl.dc_sta) {
        dc_ctl.dc_in_cnt++;
        if (dc_ctl.dc_in_cnt > 5) {
            dc_ctl.dc_sta = dc_sta;
            dc_ctl.dc_in_cnt = 0;
        }
    } else {
        dc_ctl.dc_in_cnt = 0;
    }

    return dc_ctl.dc_sta;
}

AT(.text.charge.proc)
void charge_dc_insert(void)
{
    if (charge_ctl.chag_sta == CHAG_STA_W4_DCIN || charge_ctl.chag_sta == CHAG_STA_OFF) { //5v插入，但是还没开始充电
        if (!charge_ctl.trick_curr_en || (PWRCON1 & BIT(29))) {    //判断电压是否达到2.9/3
            charge_start(&charge_ctl, 1);
        } else {
            charge_start(&charge_ctl, 0);        //涓流充电
        }
        charge_status_disp_cb(1);
    } else {    //已经开始充电
        if (charge_ctl.chag_sta == CHAG_STA_ON_TRICKLE) {   //当前是涓流充电
            if (PWRCON1 & BIT(29)) {//判断电压是否达到2.9/3
                charge_ctl.trick_curr_cnt++;
            } else {
                charge_ctl.trick_curr_cnt = 0;
            }
            if (charge_ctl.trick_curr_cnt > 10) {
                TRACE("charge switch: trick -> const\n");
                charge_start(&charge_ctl, 1);    //切换到恒流充电
                charge_status_disp_cb(1);
            }
        } else {    //充电结束检测
            if ((charge_ctl.chag_sta == CHAG_STA_ON_CONSTANT) && charge_off_detect(&charge_ctl)) {
                charge_stop(&charge_ctl, 1);
                charge_status_disp_cb(2);
            }
        }
    }
}

ALWAYS_INLINE static bool charge_is_tout(u8 delay_mode)
{
    static u32 chage_tick = 0;

    if (charge_ctl.chag_sta == CHAG_STA_UNINIT) {   //未初始化
        return false;
    }

    if (delay_mode) {
        if (!tick_check_expire(chage_tick, 100)) {  //100ms
            return false;
        }
        chage_tick = tick_get();
    }

    return true;
}

#if TRACE_EN
void dump_charge(void)
{
    static u8 chag_sta = 0xff;
    static u8 dc_in = 0xff;

    u32 tmp = PWRCON1;
    bool vpend = (bool)(tmp & BIT(28));
    bool ipend = (bool)(tmp & BIT(30));

    if(charge_ctl.chag_sta != chag_sta || dc_in != CHARGE_DC_IN()) {
        chag_sta = charge_ctl.chag_sta;
        dc_in = CHARGE_DC_IN();
        printf("charge: sta=%d, dc_sta=%d(%d,%d), pend=i=%d, v=%d\n", charge_ctl.chag_sta, dc_ctl.dc_sta, CHARGE_DC_IN(), CHARGE_INBOX(), ipend, vpend);
    }
}
#endif

ALWAYS_INLINE static void charge_process_p(u8 delay_mode)
{
    if(charge_is_tout(delay_mode)) {
        charge_dc_detect();

        if (dc_ctl.dc_sta) {
            charge_dc_insert();
        } else {
            if (charge_ctl.chag_sta == CHAG_STA_W4_DCIN) {
                //还在等待插入稳定
            } else if (charge_ctl.chag_sta != CHAG_STA_OFF) {
                charge_stop(&charge_ctl, 0);
                charge_status_disp_cb(0);
            }
        }

#if TRACE_EN
        dump_charge();
#endif
    }
}

//------------------------------------------------------------------------------------------
///降低充电时主控耗电
AT(.text.charge.lowpwr)
void charge_power_save(void)
{
    printf("pwr_save\n");

    sys_clk_free_all();                     //释放其它模块sys_clk_req的系统时钟

#if CHARGE_LOW_POWER_EN
    sys_clk_set(SYS_RC2M);                  //跑RC2M
#if (CHARGE_BOX_TYPE == CBOX_SSW)
    set_uart1_baud_rc2m_clk(9600);
#endif
    sys_cb.charge_timr0_pr = TMR0PR;
    GLOBAL_INT_DISABLE();
    TMR0CON &= ~(7 << 1);                   //timer0 clk Select system clock
//    TMR0CON |= BIT(3);                      //timer0 CLK Select RC
    TMR0CNT = 0;
    TMR0PR = 2000000/1000 - 1;              //2M Hz, 1ms_isr
    CLKCON1 &= ~(BIT(25)|BIT(26));          //saradc clk select RC
    GLOBAL_INT_RESTORE();
    clk_xosc_dis();
#else
    sys_clk_set(SYS_24M);
    dac_clk_source_sel(2);                  //dac clk select xosc_clk
#endif

#if CHARGE_LOW_POWER_EN
    RSTCON0 &= ~BIT(4);                     //PLLSDM reset control
    charge_pll0con0 = PLL0CON0;
    PLL0CON0 = 0;                           //disable PLL
#else
//    clk_pll0_dis();
#endif
}

AT(.text.charge.lowpwr.recover)
void charge_power_recover(void)
{
#if CHARGE_LOW_POWER_EN
    clk_xosc_en();

    PLL0CON0 = charge_pll0con0;
    adpll_init(DAC_OUT_SPR);                //关了pll_ldo后需要重新init trim回频率
#else
//    clk_pll0_en();
#endif

    sys_clk_set(SYS_CLK_SEL);
#if !CHARGE_LOW_POWER_EN
    dac_clk_source_sel(1);                  //dac clk select adda_clk48
#else
    GLOBAL_INT_DISABLE();
    CLKCON1  |= BIT(25);                    //saradc clk select x26m_clkdiv6
    TMR0CNT = 0;
    TMR0PR = sys_cb.charge_timr0_pr;
    TMR0CON &= ~(7 << 1);                   //timer0 clk
    TMR0CON |= 6 << 1;                      //tmr_inc select x26m_div_clk = 1M
    GLOBAL_INT_RESTORE();
#if (CHARGE_BOX_TYPE == CBOX_SSW)
    set_uart1_baud(9600);
#endif
#endif

    printf("pwr_normal\n");
}

//------------------------------------------------------------------------------------------
//内联到不同代码段，可减少flash缺页

//func_charge调用
AT(.text.func.charge)
void charge_process(void)
{
    charge_process_p(0);
}

//func_process调用
AT(.text.func.process)
void bsp_charge_process(void)
{
    charge_process_p(1);
}

//sleep调用
AT(.sleep_text.charge.process)
void sleep_charge_process(void)
{
    charge_process_p(0);
}

#endif // CHARGE_EN
