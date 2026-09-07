#ifndef _CHARGE_H
#define _CHARGE_H

#define CHARGE_DC_IN_FILTER                     1

//充电DC插入检测
typedef struct dc_ctl_tag {
    u8 dc_in_cnt;
    u8 dc_sta;                  //DC插入状态，0=拔出，1=插入
} dc_ctl_t;

extern dc_ctl_t dc_ctl;
extern charge_ctl_t charge_ctl;

bool charge_dc_detect(void);
#define charge_get_dc_sta()     dc_ctl.dc_sta
#define charge_get_ch_sta()     charge_ctl.chag_sta

void charge_init(void);
void charge_off(void);
void charge_set_stop_time(u32 stop_time);
void charge_power_save(void);
void charge_power_recover(void);

void charge_status_disp_cb(u8 sta);
void charge_process(void);          //充电模式调用
void bsp_charge_process(void);      //正常模式调用（高速时钟）
void sleep_charge_process(void);    //sleep模式调用（休眠时钟）

#endif
