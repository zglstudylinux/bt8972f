#include "include.h"

AT(.com_text.bsp.key)
void pwrkey10s_counter_clr(void)
{
    if (POWKEY_10S_RESET) {
        RTCCON10 = BIT(10);                 //clear pwrkey10s pending and counter
    }
}

void pwrkey_off(void)
{
    GPIOBDE &= ~BIT(WKO_SOURCE);
    GPIOBDIR |= BIT(WKO_SOURCE);
    GPIOBPU &= ~BIT(WKO_SOURCE);
    GPIOBPD &= ~BIT(WKO_SOURCE);
#if WKO_SOURCE == WK0
    RTCCON13 &= ~(BIT(0) | BIT(6) | BIT(12) | BIT(18));   //wk pin0 disable,
#else
    RTCCON13 &= ~((BIT(1) | BIT(7) | BIT(13) | BIT(19)) << WKO_SOURCE);//wk pin1~3 disable
#endif // WKO_SOURCE
}

void pwrkey_init(void)
{
#if PWRKEY_EN
    pwr_usage_id = pwrkey_table[0].usage_id;
#if WKO_SOURCE == WK0
    adcch_io_pu10k_enable(ADCCH_WKO);
    RTCCON13 |= BIT(0) | BIT(6) | BIT(18);  //wk pin0 wakeup, input, pullup10k enable
    saradc_set_channel(BIT(ADCCH_WKO));
#elif WKO_SOURCE == WK1
    adcch_io_pu10k_enable(ADCCH_PB0);
    RTCCON13 |= BIT(1) | BIT(7) | BIT(19);  //wk pin0 wakeup, input, pullup10k enable
    saradc_set_channel(BIT(ADCCH_PB0));
#elif WKO_SOURCE == WK2
    adcch_io_pu10k_enable(ADCCH_PB1);
    RTCCON13 |= BIT(2) | BIT(8) | BIT(20);  //wk pin0 wakeup, input, pullup10k enable
    saradc_set_channel(BIT(ADCCH_PB1));
#elif WKO_SOURCE == WK3
    adcch_io_pu10k_enable(ADCCH_PB2);
    RTCCON13 |= BIT(3) | BIT(9) | BIT(21);  //wk pin0 wakeup, input, pullup10k enable
    saradc_set_channel(BIT(ADCCH_PB2));
#endif // WKO_SOURCE
#else
    pwrkey_off();
#endif
}

#if PWRKEY_EN
AT(.com_text.port.key)
uint8_t pwrkey_get_val(void)
{
#if PWRKEY_EN || PWRKEY_2_HW_PWRON
    uint8_t num = 0;
#if WKO_SOURCE == WK0
    while (saradc_get_value8(ADCCH_WKO) > pwrkey_table[num].adc_val) {
#elif WKO_SOURCE == WK1
    while (saradc_get_value8(ADCCH_PB0) > pwrkey_table[num].adc_val) {
#elif WKO_SOURCE == WK2
    while (saradc_get_value8(ADCCH_PB1) > pwrkey_table[num].adc_val) {
#elif WKO_SOURCE == WK3
    while (saradc_get_value8(ADCCH_PB2) > pwrkey_table[num].adc_val) {
#endif // WKO_SOURCE
        num++;
    }

    return pwrkey_table[num].usage_id;
#else
    return KEY_NULL;
#endif
}

bool pwrkey_get_status(void)
{
    u32 delay = 20;
    u32 pre_sta = 0;
    u32 sta_cnt = 0;
    while(delay--) {
        u32 tmp_sta = (RTCCON & BIT(19));
        if(pre_sta == tmp_sta) {
            sta_cnt++;
        } else {
            pre_sta = tmp_sta;
            sta_cnt = 0;
        }
        delay_ms(1);
        if(sta_cnt > 4) {
            break;
        }
    }

    return (pre_sta == 0)? true : false;
}
#endif
