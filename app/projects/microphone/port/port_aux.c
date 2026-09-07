#include "include.h"

#if FUNC_AUX_EN

bool is_aux_enter_enable(void)
{
    if ((!xcfg_cb.func_aux_en)) {
        return false;
    }

    if (xcfg_cb.aux_mode_2_pwroff_en) {
        return false;
    }

#if AUX_DETECT_EN
    if (xcfg_cb.mode_2_aux_en) {
        return true;
    }
    if ((xcfg_cb.aux_det_iosel) && (!dev_is_online(DEV_AUX))) {
        return false;
    }
#endif // AUX_DETECT_EN

    return true;
}

#endif // FUNC_AUX_EN


#if AUX_DETECT_EN
///通过配置工具来选择AUX检测GPIO

static gpio_t aux_gpio;

void aux_detect_init(void)
{
    gpio_t *g = &aux_gpio;
    u8 io_num = xcfg_cb.aux_det_iosel;
    bsp_gpio_cfg_init(&aux_gpio, xcfg_cb.aux_det_iosel);

    if (!io_num) {
        return;
    } else if (io_num == IO_MUX_MICL) {
        //复用MICL检测
        GPIOFDE |= BIT(2);
        GPIOFPU |= BIT(2);
        GPIOFDIR |= BIT(2);
    } else if (io_num == IO_MUX_SDCLK) {
        SD_MUX_DETECT_INIT();
    } else if (io_num <= IO_MAX_NUM) {
        g->sfr[GPIOxDE] |= BIT(g->num);
        g->sfr[GPIOxPU] |= BIT(g->num);
        g->sfr[GPIOxDIR] |= BIT(g->num);
    }
}

AT(.com_text.aux.det)
bool aux_is_online(void)
{
    gpio_t *g = &aux_gpio;
    u8 io_num = xcfg_cb.aux_det_iosel;

    //无AUX检测
    if (!io_num) {
        return false;
    }

    if (io_num == IO_MUX_SDCLK) {
        return SD_MUX_IS_ONLINE();
    } else if (io_num == IO_MUX_SDCMD) {
        return SD_CMD_MUX_IS_ONLINE();
    } else if (io_num == IO_MUX_PWRKEY) {
        return pwrkey_detect_flag;
    } else {
        return (!(g->sfr[GPIOx] & BIT(g->num)));
    }
}

AT(.com_text.aux.det)
bool aux_detect_is_busy(void)
{
    u8 io_num = xcfg_cb.aux_det_iosel;

    //无AUX检测
    if (!io_num) {
        return true;
    }

    //复用SDCMD或SDCLK检测
    if ((io_num == IO_MUX_SDCMD) || (io_num == IO_MUX_SDCLK)) {
        return sdcard_detect_is_busy();
    }

    return false;
}

AT(.com_text.aux.det)
bool is_aux_det_mux_micl(void)
{
    if (xcfg_cb.aux_det_iosel == IO_MUX_MICL) {
        return true;
    } else {
        return false;
    }
}
#endif
