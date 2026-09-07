#ifndef _BSP_TKEY_H
#define _BSP_TKEY_H

#define TKC_MAX_BITS               2
#define TKC_MAX_SIZE              (1 << TKC_MAX_BITS)

#if TKEY_EN
void bsp_tkey_init(void);
u8 bsp_tkey_scan(void);
bool tkey_is_pressed(void);
bool bsp_tkey_wakeup_en(void);
void tkey_press_timeout_clr(void);
void tkey_press_timeout_process(void);

void bsp_charge_bcnt_calibration(u32 min_avg_cnt);
void tkey_stop_calibration_in_charge(void);
void tkey_proc_calibration_in_charge(void);

void bsp_tebcnt_temp_calibration_stop(void);
void bsp_tebcnt_temp_calibration_start(void);
void bsp_tebcnt_temp_calibration(void);
void bsp_tebcnt_temp_calibration_stop(void);
void bsp_tkey_bcnt_calibration_timeout(void);
#else
#define bsp_tkey_off()              tkey_sw_reset()
#define bsp_tkey_wakeup_en()        false
#define bsp_charge_bcnt_calibration(x)
#define tkey_proc_calibration_in_charge()
#define tkey_stop_calibration_in_charge()

#define bsp_tebcnt_temp_calibration_stop()
#define bsp_tebcnt_temp_calibration_start()
#define bsp_tebcnt_temp_calibration()
#define bsp_tebcnt_temp_calibration_stop()
#define bsp_tkey_bcnt_calibration_timeout()
#endif // TKEY_EN

#if TKEY_DEBUG_EN
void bsp_tkey_spp_tx(void);
void bsp_tkey_str_spp_tx(char *str);
#endif

#if TKEY_INEAR_EN
u8 tkey_is_inear(void);     //入耳检测，0：出耳, 1：入耳
#else
#define tkey_is_inear()     (0)
#endif // TKEY_INEAR_EN

#endif
