#ifndef __SYSTEM_H
#define __SYSTEM_H

#include "mute.h"
#include "param.h"
#include "device.h"
#include "vbat.h"

#define VOL_MAX                         xcfg_cb.vol_max   //最大音量级数

//pwroff key status
enum {
    PWROFF_IDLE,
    PWROFF_W4_TIMEOUT,
    PWROFF_END,
};

//low battery status
enum {
    LBAT_STA_IDLE,
    LBAT_STA_W4_ENTER,
    LBAT_STA_WARNING,
    LBAT_STA_W4_EXIT,
};

struct pwroff_tag {
    volatile union {
        struct {
            u8 charge_full_ind  : 1;
            u8 ntc_ind          : 1;
            u8 low_bat_ind      : 1;
            u8 aux_insert_ind   : 1;
            u8 pwr_key_ind      : 1;
            u8 timeout_ind      : 1;
            u8 user_ind         : 1;    //app、tws等操作
        };
        u8 all_flag;
    };
    u8 tone_en;                         //是否播放pwroff提示音
    u8 key_state;                       //PWR键状态
    u32 key_ticks;                      //PWR键按下的时刻
    u32 delay_ticks;                    //PWR键长按超时时间
};

typedef struct {
    u32 rst_reason;
    u8  sys_delay;
    u8  play_mode;
    u8  vol;                    //非蓝牙模式系统音量级数
    u8  a2dp_vol;               //蓝牙音乐音量级数
    u8  hfp_vol;                //蓝牙通话音量级数
    u8  hfp2sys_mul;            //系统音量与HFP音量倍数，手机HFP音量只有16级。
    s8  gain_offset;            //用于动态修改音量表，均衡音乐和通话音量
    u8  eq_mode;
    u8  cur_dev;
    u8  dev_active;
    u8  lang_id;
    u8  lbat_warning_delay;     //播报间隔延迟
    u8  lbat_warning_times;     //播报低电次数，0xff表示一直播
    u8  lpwr_status;
    u8  lbat_status;
    u32 lpwr_tick;
    u32 lbat_tick;

    u8 vusb_uart_flag;          //vusb uart打开标志
    u8 discon_reason;
volatile u8  incall_flag;

    u8 msc_bypass;
    bool ude_flag;
    struct pwroff_tag pwroff;

    u8 inbox_wken_flag;
    u8 outbox_pwron_flag;
    u16 vbat;
    u16 vbat_pwroff;            //关机电压值
    u16 vbat_warning;           //低电提醒电压值
    u32 sleep_time;
    u32 pwroff_time;
    u32 sleep_delay;
    u32 pwroff_delay;
    u32 sleep_wakeup_time;
    u32 ms_ticks;               //ms为单位
    u32 rand_seed;
    u8 tkey_pwrdwn_en;
    u8 sw_rst_flag;

#if CHARGE_LOW_POWER_EN
    u32 charge_timr0_pr;
#endif
volatile u8  cm_times;
volatile u8  loudspeaker_mute;  //功放MUTE标志
volatile u8  pwrkey_5s_check;   //是否进行开机长按5S的PWRKEY检测
volatile u8  pwrkey_5s_flag;    //长按5秒PWRKEY开机标志
volatile u8  charge_sta;        //0:充电关闭， 1：充电开启， 2：充满
volatile u8  charge_bled_flag;  //charge充满蓝灯常亮标志
volatile u8  ch_bled_cnt;       //charge充满蓝灯亮时间计数
volatile u8  poweron_flag;      //pwrkey开机标志
volatile u8  key2unmute_cnt;    //按键操作延时解mute

#if REC_FAST_PLAY
    u16 rec_num;                //记录最新的录音文件编号
    u32 ftime;
#endif

    ///位变量不要用于需要在中断改值的变量。 请谨慎使用位变量，尽量少定义位变量。
    u8  mute             : 1,   //系统MUTE控制标志
        cm_factory       : 1,   //是否第一次FLASH上电
        cm_vol_change    : 1,   //音量级数是否需要更新到FLASH
        bthid_mode       : 1,   //独立HID拍照模式标志
        port2led_en      : 1,   //1个IO推两个灯
        voice_evt_brk_en : 1,   //播放提示音时，U盘、SD卡、AUX等插入事件是否立即响应.
        karaok_init      : 1;   //karaok初始化标志

    u8  sleep_en                : 1,   //用于控制是否进入sleep
		lowbat_flag             : 1,
        hfp_karaok_en           : 1,   //通话模式是否打开K歌功能。
        eq_app_total_gain_en    : 1;   //用于控制是否使用app eq总增益
    u8 led_scan_en;
    u8 bt_is_inited;

    u8 init_vol;                //系统默认音量
    u8 recover_initvol;         //开机最小音量，避免开机时音量太小误认为没开机
    u8 warning_volume;          //播放提示音的音量级数

    u8 phone_type;              //设备类型标志位
    u8 dac_disable;             //关闭DAC标志位
    u8 usbdev_insert;           //USB设备插入检测标志位
    u8 rec_format;              //录音标志位
    u8 sd_card_init_flag;       //SD卡初始化标志位
    u8 device_usb_init_flag;    //发射端USB启动初始化标志位
    u32 fs_recording_cnt;       //录音时长计数
    u32 recording_size_cnt;     //录音文件大小计算
#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
    u8 bond_mode;               //0:未绑定任何模式, 1:绑定无线麦模式, 2:绑定经典蓝牙模式
    u8 wl_scan_flag;            //无线麦扫描标志
    u8 wl_bond_nb;              //无线麦绑定数量
    u16 wl_bd_key;              //无线麦绑定秘钥
#endif
} sys_cb_t;
extern sys_cb_t sys_cb;

#define SYS_INIT_VOLUME                 sys_cb.init_vol         //系统默认音量
#define SYS_LIMIT_VOLUME                sys_cb.recover_initvol  //开机最小音量，避免开机时音量太小误认为没开机
#define WARNING_VOLUME                  sys_cb.warning_volume   //播放提示音的音量级数
#define SYS_BACKSTAGE_VOLUME            (SYS_LIMIT_VOLUME+3)    //播放提示音时，系统退到后台的音量级数

void power_on_check(void);
void sys_init(void);
void sys_update_init(void);
void timer1_irq_init(void);
void bsp_sys_mute(void);
void bsp_sys_unmute(void);
void bsp_clr_mute_sta(void);
#if LOUDSPEAKER_MUTE_EN
void bsp_loudspeaker_mute(void);
void bsp_loudspeaker_unmute(void);
#else
#define bsp_loudspeaker_mute()
#define bsp_loudspeaker_unmute()
#endif
void uart0_mapping_sel(void);
void aux_detect(void);
bool aux_micl_is_online(void);
void get_usb_chk_sta_convert(void);
uint bsp_get_bat_level(void);
void sd_soft_cmd_detect(u32 check_ms);
void bsp_i2s_init(void);

void cpu_set_sfr(psfr_t sfr, u32 value);
void cpu_clr_sfr(psfr_t sfr, u32 value);

#endif // __SYSTEM_H
