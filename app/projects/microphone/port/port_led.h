#ifndef _PORT_LED_H
#define _PORT_LED_H

typedef struct {
    void (*port_init)(gpio_t *g);
    void (*set_on)(gpio_t *g);
    void (*set_off)(gpio_t *g);
} led_func_t;

extern led_func_t bled_func;
extern led_func_t rled_func;
extern gpio_t bled_gpio;
extern gpio_t rled_gpio;

void led_func_init(void);
#if LED_DISP_EN
    #define is_led_scan_enable()        sys_cb.led_scan_en
#else
    #define is_led_scan_enable()        0
#endif

#endif // _PORT_LED_H
