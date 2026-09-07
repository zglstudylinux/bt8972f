#include "include.h"

volatile int pwrkey_detect_flag;            //pwrkey 820K用于复用检测的标志。


void plugin_init(void)
{
    CLKGAT0 = 0xffffffff;
    CLKGAT1 = 0xffffffff;
    CLKGAT2 = 0xffffffff;
    CLKGAT3 = 0xffffffff;

//    CLKGAT0 = BIT(0)|BIT(12);
//    CLKGAT1 = BIT(1)|BIT(2)|BIT(3)|BIT(18)|BIT(19)|BIT(20)|BIT(21)|BIT(22)|BIT(23)|BIT(24)|BIT(25)|BIT(26)|BIT(27)|BIT(28)|BIT(29)|BIT(30);
//    CLKGAT2 = BIT(2)|BIT(10)|BIT(11)|BIT(12)|BIT(15)|BIT(17)|BIT(23)|BIT(24)|BIT(25)|BIT(26)|BIT(29);
//    CLKGAT3 = 0;
    CLKCON2 &= ~(BIT(29)|BIT(30));  //USB

#if UART0_PRINTF_SEL != PRINTF_NONE
    CLKGAT0 |= BIT(6);
#endif

#if (LANG_SELECT == LANG_EN_ZH)
    multi_lang_init(sys_cb.lang_id);
#endif

#if USB_SUPPORT_EN
    CLKGAT0 |= BIT(15);
#endif
#if SD_SUPPORT_EN
    CLKGAT0 |= BIT(16);
#endif

}

void plugin_var_init(void)
{
    pwrkey_detect_flag = 0;

#if FUNC_AUX_EN
    if (!xcfg_cb.func_aux_en) {
        xcfg_cb.aux_mode_2_pwroff_en = 0;
        xcfg_cb.aux_det_iosel = 0;
        xcfg_cb.auxl_sel = 0;
        xcfg_cb.auxr_sel = 0;
        xcfg_cb.mode_2_aux_en = 0;
    }
#endif // FUNC_AUX_EN

    if (!xcfg_cb.led_disp_en) {
        xcfg_cb.bled_io_sel = 0;
        xcfg_cb.charge_full_bled = 0;
        xcfg_cb.led_sta_config_en = 0;
    }

#if FUNC_AUX_EN
    if (!AUX_DETECT_EN) {
        xcfg_cb.aux_det_iosel = 0;
    }
#endif

    if (!xcfg_cb.led_pwr_en) {
        xcfg_cb.rled_io_sel = 0;
    }

    if (xcfg_cb.buck_mode_en) {
        xcfg_cb.vddbt_capless_en = 0;
    }

    if (!xcfg_cb.charge_en) {
        xcfg_cb.chbox_en = 0;
    }

    if (!xcfg_cb.chbox_en) {
        xcfg_cb.ch_box_type_sel            = 3;
        xcfg_cb.ch_out_auto_pwron_en       = 0;
        xcfg_cb.ch_leakage_sel             = 0;
        xcfg_cb.chg_inbox_pwrdwn_en        = 0;
        xcfg_cb.chbox_out_delay            = 0;
    }

#if (CHARGE_BOX_TYPE == CBOX_SSW)
    if (xcfg_cb.chbox_en) {
        xcfg_cb.ch_box_type_sel            = 2;     //昇生微智能充电仓为维持电压仓
        xcfg_cb.chg_inbox_pwrdwn_en        = 0;
    }
#endif

#if TKEY_SOFT_PWR_EN
    sys_cb.tkey_pwrdwn_en = 1;
#endif

    //PWRKEY模拟硬开关,需要关闭长按10S复位， 第一次上电开机， 长按5秒进配对等功能。
    if (PWRKEY_2_HW_PWRON) {
        xcfg_cb.powkey_10s_reset = 0;
//        xcfg_cb.pwron_frist_bat_en = 0;
        xcfg_cb.bt_pwrkey_nsec_discover = 0;
    }

#if LED_DISP_EN
	led_func_init();
#endif // LED_DISP_EN
}

AT(.com_text.plugin)
void plugin_tmr5ms_isr(void)
{
#if ENERGY_LED_EN
    energy_led_level_calc();
#endif
}

AT(.com_text.plugin)
void plugin_tmr1ms_isr(void)
{
#if ENERGY_LED_EN
    energy_led_scan();
#endif
}

//蓝牙休眠时，唤醒IO配置
void sleep_wakeup_config(void)
{
#if ADKEY_EN
    wakeup_gpio_config(get_adc_gpio_num(ADKEY_CH), 0, 0);           //配置ADKEY IO下降沿唤醒。
#endif // ADKEY_EN

#if ADKEY2_EN
    wakeup_gpio_config(get_adc_gpio_num(ADKEY2_CH), 0, 0);          //配置ADKEY1 IO下降沿唤醒。
#endif // ADKEY2_EN

#if ADKEY_MUX_SDCLK_EN
    wakeup_gpio_config(get_adc_gpio_num(SDCLK_AD_CH), 0 ,0);        //IO下降沿唤醒。
#endif // ADKEY_MUX_SDCLK_EN

#if IOKEY_EN && IOKEY_GPIO_SEL_EN
    //不用工具配置IOKEY时，根据实际使用的IOKEY IO进行修改。
    wakeup_gpio_config(IOKEY_GPIO_SEL0, 0, 1);       //配置IO下降沿唤醒。
    wakeup_gpio_config(IOKEY_GPIO_SEL1, 0, 1);
#endif // IOKEY_EN

#if SC7A20_EN
	wakeup_wko_config(); /*配置PB5作为GPIO唤醒功能*/
	//printf("%s config pb5 wakeup\n", __func__);
#endif

#if PWRKEY_EN
    if ((!PWRKEY_2_HW_PWRON) && !bsp_tkey_wakeup_en()) {
        wakeup_wko_config();
    }
#endif // PWRKEY_EN
}

AT(.com_text.bsp.sys)
bool is_sd_support(void)
{
#if (SD0_MAPPING == SD0MAP_G2) && EQ_DBG_IN_UART
    if (xcfg_cb.huart_en && xcfg_cb.huart_sel == 1) {
        return false;
    }
#endif
    return (SD_SUPPORT_EN);
}

AT(.com_text.bsp.sys)
bool is_usb_support(void)
{
#if EQ_DBG_IN_UART
    if (xcfg_cb.huart_en && xcfg_cb.huart_sel == 2) {
        return false;
    }
#endif
    if(xcfg_cb.wireless_adapter_en) {     //接收端才开usb,要不然不用usb又init的化PB4会有亮一下的问题
        return USB_SUPPORT_EN;
    }else {
        return DEVICE_USB_EN;             //发射端使用USB功能才开起来
    }

}

bool is_sleep_dac_off_enable(void)
{
#if FUNC_AUX_EN
    if (xcfg_cb.aux_det_iosel == IO_MUX_MICL) {
        return false;
    }
#endif
    return true;
}

//设置piano提示音播放的数字音量 (0 ~ 0x7fff)
u32 piano_get_digvol_cb(void)
{
    return bsp_volume_vol2dvol(WARNING_VOLUME);
}

//设置提示音播放的数字音量 (0 ~ 0x7fff)
u32 waring_get_digvol_cb(void)
{
    return bsp_volume_vol2dvol(WARNING_VOLUME);
}

void maxvol_tone_play(void)
{
	bsp_res_play(RES_IDX_MAX_VOL);
}

void minvol_tone_play(void)
{

}

void plugin_playmode_warning(void)
{

}

void plugin_lowbat_vol_reduce(void)
{
#if LPWR_REDUCE_VOL_EN
    music_src_set_volume(0x50c0);       //设置音乐源音量达到整体降低系统音量 (范围：0~0x7fff)
#endif // LPWR_REDUCE_VOL_EN
}

void plugin_lowbat_vol_recover(void)
{
#if LPWR_REDUCE_VOL_EN
    music_src_set_volume(0x7fff);       //还原音量
#endif // LPWR_REDUCE_VOL_EN
}

//用于调DAC音质接口函数
void plugin_music_eq(void)
{
    bsp_eq_init();
    sys_cb.eq_mode = 0;
    music_eq_set_by_num(sys_cb.eq_mode);
}

AT(.com_text.port.vbat)
void plugin_vbat_filter(u32 *vbat)
{
#if  VBAT_FILTER_USE_PEAK
    //电源波动比较大的音箱方案, 取一定时间内的电量"最大值"或"次大值",更能真实反应电量.
    #define VBAT_MAX_TIME  (3000/5)   //电量峰值判断时间 3S
    static u16 cnt = 0;
	static u16 vbat_max_cnt = 0;
    static u32 vbat_max[2] = {0,0};
    static u32 vbat_ret = 0;
    u32 vbat_cur = *vbat;
    if (cnt++  < VBAT_MAX_TIME) {
        if (vbat_max[0] < vbat_cur) {
            vbat_max[1] = vbat_max[0];  //vbat_max[1] is less max (次大值)
            vbat_max[0] = vbat_cur;     //vbat_max[0] is max(最大值)
            vbat_max_cnt = 0;
        } else if (vbat_max[0] == vbat_cur) {
            vbat_max_cnt ++;
        }
    } else {
        if (vbat_max_cnt >= VBAT_MAX_TIME/5) {  //总次数的(1/5)都采到最大值,则返回最大值.
            vbat_ret = vbat_max[0];
        } else if (vbat_max[1] != 0) {   //最大值次数较少,则返回次大值(舍弃最大值)
            vbat_ret = vbat_max[1];
        }
        vbat_max[0] = 0;
        vbat_max[1] = 0;
        vbat_max_cnt = 0;
        cnt = 0;
    }
    //返回值
    if (vbat_ret != 0) {
        *vbat = vbat_ret;
    }
#endif
}

//初始化完成, 各方案可能还有些不同参数需要初始化,预留接口到各方案
void plugin_sys_init_finish_callback(void)
{
#if ENERGY_LED_EN
    energy_led_init();
#endif
}

bool plugin_func_idle_enter_check(void)
{
    //可以在这里决定否需要进入idle
    return true;
}



