#include "include.h"
#include "func.h"

#if CHARGE_EN

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if TRACE_EN
AT(.rodata.charge.str)
const char charge_on_str[] = "charge: on\n";
AT(.rodata.charge.str)
const char charge_off_str[] = "charge: off: %d\n";
AT(.rodata.charge.str)
const char charge_dc_out_str[] = "charge: dc out\n";
AT(.rodata.charge.str)
const char charge_full_str[] = "charge: full\n";
#endif

//充满电亮蓝灯时间table表
AT(.rodata.charge)
const u8 ch_bled_timeout_tbl[8] = {0, 10, 20, 30, 60, 120, 180, 255};

//充满电亮蓝灯时间控制
AT(.text.charge.led)
void charge_bled_ctrl(void)
{
    if (sys_cb.charge_bled_flag) {
        if (sys_cb.ch_bled_cnt) {
            if (sys_cb.ch_bled_cnt != 255) {
                sys_cb.ch_bled_cnt--;
            }
        } else {
            sys_cb.charge_bled_flag = 0;
            bled_set_off();
        }
    }
}

AT(.text.charge.led)
void charge_status_disp_cb(u8 sta)
{
    if (sta == 1) {
        //充电开启
        sys_cb.charge_sta = 1;
        sys_cb.charge_bled_flag = 0;
        charge_led_on();
        TRACE(charge_on_str);
    } else {
        //充电关闭
        TRACE(charge_off_str, sta);
        sys_cb.charge_sta = 0;
        sys_cb.charge_bled_flag = 0;
        charge_led_off();
        if (sta == 2) {
            sys_cb.charge_sta = 2;
            //充电满亮蓝灯
            if (BLED_CHARGE_FULL) {
                sys_cb.charge_bled_flag = 1;
                sys_cb.ch_bled_cnt = ch_bled_timeout_tbl[BLED_CHARGE_FULL];
                charge_bled_on();
            }
        }
    }
}

AT(.text.func.charge)
void sfunc_charge_enter(bool chbox_out2pwr_en)
{
    lock_code_charge();
#if CHARGE_BOX_EN
    charge_box_enter(chbox_out2pwr_en);
#endif
    vusb4s_reset_dis();
	charge_power_save();
}

AT(.text.func.charge)
void sfunc_charge_exit(void)
{
    charge_power_recover();
    charge_off();
#if CHARGE_BOX_EN
    charge_box_reinit();
#endif
    unlock_code_charge();

#if TKEY_EN
    tkey_stop_calibration_in_charge();
#endif
}

AT(.text.func.charge)
u8 sfunc_charge_process(u32 counter)
{
    u8 charge_sta = 0;

    sys_cb.sys_delay = 0;

    if ((counter % 20) == 0) {
        charge_process();
    }

    if ((counter % 200) == 0) {
        vusb4s_reset_dis();
        vusb_reset_clr();
        charge_bled_ctrl();
    }

#if TKEY_EN
    if (CHARGE_DC_IN()) {
        if ((counter % 1000) == 0) {                           //5秒
            bsp_charge_bcnt_calibration((CHARGE_BOX_EN) ? 100 : 500);
        }
    }
    tkey_proc_calibration_in_charge();
#endif

#if CHARGE_BOX_EN
    charge_sta = charge_box_charge_on_process();
#else
    charge_sta = charge_get_ch_sta();
#endif

    return charge_sta;
}

AT(.text.func.charge)
void func_charge_process(void)
{
    u32 counter = 0;

    while (1) {
        WDT_CLR();

#if QTEST_EN
        if(qtest_get_mode() || qtest_cb.sta){
            func_cb.sta = func_cb.last;
            break;;
        }
#endif

#if CHARGE_LOW_POWER_EN
        delay_us(200);
#else
        delay_5ms(1);
#endif

        u8 charge_sta = sfunc_charge_process(counter++);
        if(charge_sta == CHAG_STA_OFF) {
            TRACE(charge_dc_out_str);
            func_cb.sta = func_cb.last;             //拔掉电源，恢复开机
            break;
        } else if(charge_sta == CHAG_STA_OFF_VBUS_PATH || charge_sta == CHAG_STA_OFF_INBOX) {
            if(xcfg_cb.ch_full_auto_pwrdwn_en) {    //充满关机
                TRACE(charge_full_str);
                sfunc_charge_exit();
                bsp_saradc_exit();
                sfunc_pwrdown(0);
//                sys_cb.pwroff.charge_full_ind = 1;
//                sys_cb.pwroff.tone_en = 0;
//                func_cb.sta = FUNC_PWROFF;
                break;
            }
        }
    }
}

AT(.text.func.charge)
static void func_charge_enter(void)
{
    bsp_res_w4_finish(true);
    bsp_res_set_enable(false);
	dac_power_off();

	sfunc_charge_enter(1);
}

AT(.text.func.charge)
static void func_charge_exit(void)
{
    sfunc_charge_exit();

    bsp_res_set_enable(true);

    if (!bsp_dac_off_for_bt_conn()) {
        dac_restart();
    }
}

AT(.text.func.charge)
void func_charge(void)
{
    printf("%s\n", __func__);

    func_charge_enter();
    func_charge_process();
    func_charge_exit();
}
#endif
