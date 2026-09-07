#include "include.h"

AT(.rodata.func.table)
const u8 func_sort_table[] = {
#if FUNC_BT_EN
    FUNC_BT,
#endif

#if FUNC_DEVICE_EN
    FUNC_DEVICE,
#endif // FUNC_AUX_EN

#if FUNC_USBDEV_EN
    FUNC_USBDEV,
#endif // FUNC_USBDEV_EN

#if FUNC_IDLE_EN
    FUNC_IDLE,
#endif
};

AT(.text.func)
u8 get_funcs_total(void)
{
    return sizeof(func_sort_table);
}

u32 getcfg_vddio_sel(void)
{
    return xcfg_cb.vddio_sel;
}

u32 getcfg_pmu_mode(void)
{
    u32 pmu_cfg = 0;

    if (BUCK_MODE_EN) {
        pmu_cfg |= BIT(0);      //vddbt buck en
        if (xcfg_cb.vcore_buck_en) {
            pmu_cfg |= BIT(1);  //vddcore buck en
        } else {
            pmu_cfg |= BIT(2);  //vddbt buck sido

            cfg_pmu_vddio_lp_enable = 0;
        }
    } else {
        if(xcfg_cb.vddbt_capless_en) {
            pmu_cfg |= BIT(3);  //vddbt capless
        }
    }

    if(xcfg_cb.dac_maxout_en) {
        pmu_cfg |= (DAC_MAXOUT_VCM_SEL<<4);     //DAC大功率
    } else {
        pmu_cfg |= DAC_VCM_SEL<<4;              //DAC正常功率
    }

    pmu_cfg |= VDDCORE_LIMIT_SEL<<8;            //vddcore limit, bit8~10
    pmu_cfg |= VDDBT_LIMIT_SEL<<11;             //vddbt limit, bit11~13
    pmu_cfg |= VDDIO_LIMIT_SEL<<14;             //vddio limit, bit14~16
    return pmu_cfg;
}

u32 getcfg_mic_bias_method(u8 mic_ch)
{
    return xcfg_cb.mic_pwr_sel << 4 | xcfg_cb.mic_bias_method;
}

u32 getcfg_mic_gain(u8 mic_ch)
{
    return xcfg_cb.mic_anl_gain << 8 | xcfg_cb.bt_mic_dig_gain;
}

u32 getcfg_mic_power_level(void)
{
    return xcfg_cb.mic_pwr_level;
}

#if BT_A2DP_LDAC_AUDIO_EN
u8 *getcfg_soft_key(void)
{
    return xcfg_cb.soft_key;
}
#endif
