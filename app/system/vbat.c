#include "include.h"

#if VBAT_DETECT_EN

#define VBAT_CACL_VOLTAGE()     (u32)((VBAT_VALUE() * adc_cb.vbat2_coef / adc_cb.vbg) * adc_cb.vbg_volt / 10000)

//AT(.com_rodata.bat)
//const char bat_str[] = "VBAT: %d.%03dV\n";
AT(.com_text.port.vbat)
uint16_t vbat_get_voltage(void)
{
    static u16 vbat_bak = 0;

    u32 vbat = VBAT_CACL_VOLTAGE();
    u32 delta;

    //不同方案可能采用不同 vbat 滤波算法, 在方案对应的plugin.c中处理
    plugin_vbat_filter(&vbat);
    //默认的取平均值算法.
    adc_cb.vbat_total = adc_cb.vbat_total - adc_cb.vbat_val + vbat; //均值
    adc_cb.vbat_val = adc_cb.vbat_total>>5;

    if(adc_cb.vbat_val > vbat_bak) {
        delta = adc_cb.vbat_val - vbat_bak;
    } else {
        delta = vbat_bak - adc_cb.vbat_val;
    }
    if (delta >= 30) {   //偏差大于一定值则更新
        vbat_bak = adc_cb.vbat_val;
//        printf(bat_str, adc_cb.vbat_val/1000, adc_cb.vbat_val%1000);
    }

    return vbat_bak;
}

void vbat_init(void)
{
    saradc_set_channel(BIT(ADCCH_BGOP) | BIT(ADCCH_VBAT));
}

ALIGNED(128)
void vbat_voltage_init(void)
{
    //先初始化vbg_volt、vbat2_coef
    adc_cb.vbg_volt = vbg_voltage_get();
    adc_cb.vbat2_coef = vbat2_voltage_get();

    //先获取初值vbat
    GLOBAL_INT_DISABLE();
    sys_cb.vbat = VBAT_CACL_VOLTAGE();
    adc_cb.vbat_val =  sys_cb.vbat;
    adc_cb.vbat_total = adc_cb.vbat_val << 5;
    GLOBAL_INT_RESTORE();
}

AT(.text.func.msg)
uint vbat_get_lpwr_sta(void)
{
    if (sys_cb.vbat <= sys_cb.vbat_pwroff) {
        return 2;
    } else {
        return (sys_cb.vbat <= sys_cb.vbat_warning)? 1 : 0;
    }
}

#endif  //VBAT_DETECT_EN
