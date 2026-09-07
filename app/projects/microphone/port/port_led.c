#include "include.h"
#include "led.h"

#if LED_DISP_EN
led_func_t bled_func AT(.buf.led);
led_func_t rled_func AT(.buf.led);
gpio_t bled_gpio AT(.buf.led);
gpio_t rled_gpio AT(.buf.led);

AT(.rodata.led.cfg)
const led_cfg_t led_cfg_poweron     = {0x00, 0xff, 1, 0};   //¿∂µ∆≥£¡¡
AT(.rodata.led.cfg)
const led_cfg_t led_cfg_poweroff    = {0x00, 0x00, 1, 0};   //∫Ïµ∆£¨¿∂µ∆√
AT(.rodata.led.cfg)
const led_cfg_t led_cfg_idle        = {0x00, 0xff, 1, 0};   //¿∂µ∆≥£¡¡
AT(.rodata.led.cfg.bt)
const led_cfg_t led_cfg_btinit      = {0x00, 0xaa, 10, 0};  //¿∂µ∆500ms÷‹∆⁄…¡À∏£¨ƒ¨»œ∏˙w4pwrkey“ª—˘
AT(.rodata.led.cfg.bt)
const led_cfg_t led_cfg_pairing     = {0xf0, 0x0f, 1, 3};   //¡¡300ms√450ms£¨¿∂∫ÏΩªÃÊ…¡
AT(.rodata.led.cfg.bt)
const led_cfg_t led_cfg_reconnect   = {0x00, 0xaa, 6, 0};   //¿∂µ∆300ms÷‹∆⁄…¡À∏
AT(.rodata.led.cfg.bt)
const led_cfg_t led_cfg_connected   = {0x00, 0xff, 1, 0};   //¿∂µ∆≥£¡¡
AT(.rodata.led.cfg.bt)
const led_cfg_t led_cfg_playing     = {0x00, 0x0f, 5, 0};   //¡¡4*250ms, √4*250ms
AT(.rodata.led.cfg.bt)
const led_cfg_t led_cfg_ring        = {0x00, 0x02, 2, 86};  //¡¡100ms, √5S
AT(.rodata.led.cfg.bt)
const led_cfg_t led_cfg_calling     = {0x00, 0x0a, 2, 86};  //¡¡100ms, √5S, ¡¡¡Ω¥Œ
AT(.rodata.led.cfg.music)
const led_cfg_t led_cfg_music_busy  = {0x00, 0xaa, 4, 0};   //¿∂µ∆200ms÷‹∆⁄øÏ…¡À∏
AT(.rodata.led.cfg.music)
const led_cfg_t led_cfg_music_play  = {0x00, 0xaa, 20, 0};  //¿∂µ∆1s÷‹∆⁄…¡À∏
AT(.rodata.led.cfg.aux)
const led_cfg_t led_cfg_aux_play    = {0x00, 0xaa, 20, 0};  //¿∂µ∆1s÷‹∆⁄…¡À∏
AT(.rodata.led.cfg.fm)
const led_cfg_t led_cfg_fm_play     = {0x00, 0xaa, 20, 0};  //¿∂µ∆1s÷‹∆⁄…¡À∏
AT(.rodata.led.cfg.fm)
const led_cfg_t led_cfg_fm_scan     = {0x00, 0xaa, 20, 0};  //¿∂µ∆1s÷‹∆⁄…¡À∏
AT(.rodata.led.cfg.rec)
const led_cfg_t led_cfg_record      = {0x00, 0xaa, 20, 0};  //¿∂µ∆1s÷‹∆⁄…¡À∏
AT(.rodata.led.cfg.lowbat)
const led_cfg_t led_cfg_lowbat          = {0xaa, 0x00, 0x06, 0x00};     //∫Ïµ∆300ms÷‹∆⁄…¡À∏
AT(.rodata.led.cfg.lowbat)
const led_cfg_t led_cfg_lowbat_follow   = {0x0a, 0x00, 0x02, 0xff};     //∫Ïµ∆º‰∏Ù100ms…¡2¥Œ

void led_xcfg_init(void)
{
    if(xcfg_cb.led_sta_config_en == 0) {
        memcpy(&xcfg_cb.led_poweron,    &led_cfg_poweron,   sizeof(led_cfg_t));
        memcpy(&xcfg_cb.led_poweroff,   &led_cfg_poweroff,  sizeof(led_cfg_t));
        memcpy(&xcfg_cb.led_btinit,     &led_cfg_btinit,    sizeof(led_cfg_t));
        memcpy(&xcfg_cb.led_reconnect,  &led_cfg_reconnect, sizeof(led_cfg_t));
        memcpy(&xcfg_cb.led_pairing,    &led_cfg_pairing,   sizeof(led_cfg_t));
        memcpy(&xcfg_cb.led_connected,  &led_cfg_connected, sizeof(led_cfg_t));
        memcpy(&xcfg_cb.led_playing,    &led_cfg_playing,   sizeof(led_cfg_t));
#if (LED_LOWBAT_EN || RLED_LOWBAT_FOLLOW_EN)
        memcpy(&xcfg_cb.led_lowbat,     &led_cfg_lowbat,    sizeof(led_cfg_t));
#endif
    }
}

AT(.text.led_disp)
void led_power_up(void)
{
    led_set_sta((led_cfg_t *)&xcfg_cb.led_poweron);
}

AT(.text.led_disp)
void led_power_down(void)
{
    led_set_sta((led_cfg_t *)&xcfg_cb.led_poweroff);
}

AT(.text.led_disp)
void led_idle(void)
{
    led_set_sta(&led_cfg_idle);
}

AT(.text.led_disp.bt)
void led_bt_init(void)
{
    led_set_sta((led_cfg_t *)&xcfg_cb.led_btinit);
}

AT(.text.led_disp.bt)
void led_bt_idle(void)
{
    led_set_sta((led_cfg_t *)&xcfg_cb.led_pairing);
}

AT(.text.led_disp.bt)
void led_bt_reconnect(void)
{
    led_set_sta((led_cfg_t *)&xcfg_cb.led_reconnect);
}

AT(.text.led_disp.bt)
void led_bt_connected(void)
{
    led_set_sta((led_cfg_t *)&xcfg_cb.led_connected);
}

AT(.text.led_disp.bt)
void led_bt_play(void)
{
    led_set_sta((led_cfg_t *)&xcfg_cb.led_playing);
}

AT(.text.led_disp.music)
void led_music_busy(void)
{
     led_set_sta(&led_cfg_music_busy);
}

AT(.text.led_disp.music)
void led_music_play(void)
{
    led_set_sta(&led_cfg_music_play);
}

AT(.text.led_disp.aux)
void led_aux_play(void)
{
     led_set_sta(&led_cfg_aux_play);
}

AT(.text.led_disp.fm)
void led_fm_play(void)
{
     led_set_sta(&led_cfg_fm_play);
}

AT(.text.led_disp.fm)
void led_fm_scan(void)
{
    led_set_sta(&led_cfg_fm_scan);
}

AT(.text.led_disp.rec)
void led_record(void)
{
    led_set_sta(&led_cfg_record);
}

AT(.text.led_disp.bt)
void led_bt_scan(void)
{
    led_bt_idle();
}

#if (LED_LOWBAT_EN || RLED_LOWBAT_FOLLOW_EN)
AT(.text.led_disp.lowbat)
void led_lowbat_do(void)
{
    led_set_sta_lowbat((const led_cfg_t *)&xcfg_cb.led_lowbat);
}

AT(.text.led_disp.lowbat)
void led_lowbat_follow_do(void)
{
    led_set_sta_lowbat(&led_cfg_lowbat_follow);
}
#endif

AT(.sleep_text.led)
bool led_bt_sleep(void)
{
    static u8 cnt = 0;

    cnt++;
    if (cnt >= 10) {
        cnt = 0;
        LED_INIT();
        bled_set_on();
        return true;
    } else {
        bled_set_off();
        return false;
    }
}

void led_func_init(void)
{
    led_xcfg_init();

#if UART0_PRINTF_SEL == PRINTF_PB3
    if (xcfg_cb.bled_io_sel == 12) {
        xcfg_cb.bled_io_sel = 0;
    }
    if (xcfg_cb.rled_io_sel == 12) {
        xcfg_cb.rled_io_sel = 0;
    }
#endif
    bsp_gpio_cfg_init(&bled_gpio, xcfg_cb.bled_io_sel);
    bsp_gpio_cfg_init(&rled_gpio, xcfg_cb.rled_io_sel);

#if LED_PWR_EN
    if (xcfg_cb.bled_io_sel == xcfg_cb.rled_io_sel) {
        //1∏ˆIOÕ∆¡Ω∏ˆµ∆
        if (bled_gpio.sfr != NULL) {
            sys_cb.port2led_en = 1;
        }
        bled_func.port_init = led2_port_init;
        bled_func.set_on = bled2_set_on;
        bled_func.set_off = bled2_set_off;

        rled_func.port_init = led2_port_init;
        rled_func.set_on = rled2_set_on;
        rled_func.set_off = rled2_set_off;
    } else
#endif // LED_PWR_EN
    {
        sys_cb.port2led_en = 0;
        bled_func.port_init = led_cfg_port_init;
        bled_func.set_on = led_cfg_set_on;
        bled_func.set_off = led_cfg_set_off;
#if ADKEY_MUX_LED_EN
        bled_func.port_init = led_mux_cfg_port_init;
        bled_func.set_on = led_mux_cfg_set_on;
        bled_func.set_off = led_mux_cfg_set_off;
#endif // ADKEY_MUX_LED_EN

        rled_func.port_init = led_cfg_port_init;
        rled_func.set_on = led_cfg_set_on;
        rled_func.set_off = led_cfg_set_off;
    }
}

u8 led_get_cfg_port(void)
{
    if(xcfg_cb.led_disp_en) {
        return xcfg_cb.bled_io_sel; //”≈œ»—°‘Ò¿∂µ∆
    } else if(xcfg_cb.led_pwr_en) {
        return xcfg_cb.rled_io_sel; //∆‰¥Œ—°‘Ò∫Ïµ∆
    }
    return 0;                       //≤ªœ‘ æ
}
#endif
