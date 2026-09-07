#ifndef _API_CHARGE_H
#define _API_CHARGE_H

//充电状态
enum {
    CHAG_STA_UNINIT = 0,        //未初始化
    CHAG_STA_W4_DCIN,           //初始化完，等待5V插入
    CHAG_STA_ON_TRICKLE,        //正在涓流充电
    CHAG_STA_ON_CONSTANT,       //正在恒流充电
    CHAG_STA_OFF,               //充电关闭，5V电源已拔出（出仓）
    CHAG_STA_OFF_VBUS_PATH,     //充电关闭，但是还是插着5V电源（充满仓5V不掉电）
    CHAG_STA_OFF_INBOX,         //充电关闭，但还在仓内（充满仓5V掉到维持电压）
};

//充电截止控制
typedef struct {
    u8 delay_cnt;               //充电截止计数
    u8 curr_cnt;                //电流截止计数
    u8 curr_cnt_sub;            //电压截止后电流截止计数
    u8 volt_cnt;                //电压截止计数
    u32 volt_delay;             //电压充满后计数
} c_off_t;

typedef struct {
    u8 chag_sta         : 3,    //充电状态
       dcin_reset       : 1,    //DC插入复位
       leakage          : 3,    //漏电电流档
       inbox_voltage    : 1;    //充电仓维持电压

    u8 const_curr;              //恒流充电电流
    u8 trick_curr;              //涓流充电电流
    u8 stop_curr;               //充电截止电流
    u8 stop_volt;               //充电截止电压

    u8 trick_curr_en    : 1;    //是否使能涓流充电
    u8 trick_stop_volt  : 1;    //涓流充电截止电压
    u8 ldo_en           : 1;    //充电是否使用LDO mode
    u8 volt_follow_en   : 1;    //充电是否使用电压跟随模式
    u8 volt_follow_diff;        //恒压差充电差值
    volatile u32 stop_time;

    c_off_t c_off;
    u8 trick_curr_cnt;          //涓流模式计数
    u8 vddio_bak;
} charge_ctl_t;

void charge_set_leakage(u8 sta, u8 level);
bool charge_off_detect(charge_ctl_t *ctl);
void charge_start(charge_ctl_t *ctl, u8 mode);       //mdoe，1：恒流充电， 0：涓流充电
void charge_stop(charge_ctl_t *ctl, u8 mode);
void charge_init_do(charge_ctl_t *ctl);


#endif // _API_CHARGE_H
