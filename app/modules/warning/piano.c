#include "include.h"
#include "res_play.h"

//          1/8  4/4   3/4   2/4      1/4  4/4   3/4   2/4
//Rhythm = [    0.062 0.094 0.125         0.125 0.187 0.250];
//           Do     Re     Mi     Fa     Sol    La    Si
//Fin    = [261.63 293.67 329.63 349.23 391.99 440 493.88 0 0 0];
#define PIANO_BASEKEYDLY        (int)(48000*0.125)
#define TONE_DELAY(x)           (int)(48*x)
#define TONE_FREQUENCY(a, b)    (u32)((a << 16) | (u16)((1 << 15) / (48000 / b)))


u32 piano_get_digvol_cb(void);
bool piano_init(bool kick_start);
void piano_isr(void);
void piano_exit(void);


//频率：    380Hz  间隔40mS  380Hz
//持续时间：103mS            103mS
//窗函数：  fade out         fade out
//窗持续：  1/2              1
AT(.rodata.piano.tbl)
static const tone_tbl_t tone_disconnected_tbl[3] = {
    {TONE_FREQUENCY(0x7ff, 380),        TONE_DELAY(103)},
    {0,                                 TONE_DELAY(40)},    //静音
    {TONE_FREQUENCY(0x7ff, 380),        TONE_DELAY(103)},
};

//频率：    760Hz
//持续时间：100mS
//窗函数：  fade out
//窗持续：  1/2
AT(.rodata.piano.tbl)
static const tone_tbl_t tone_connected_tbl[1] = {
    {TONE_FREQUENCY(0x7ff, 760),        TONE_DELAY(100)},
};

//频率：    501Hz
//持续时间：100mS
//窗函数：  fade out
//窗持续：  1/2
AT(.rodata.piano.tbl)
static const tone_tbl_t tone_pair_tbl[1] = {
    {TONE_FREQUENCY(0x7ff, 501),        TONE_DELAY(100)},
};

//频率：    200Hz  间隔20mS  302Hz   间隔20mS  400Hz   间隔20mS  501Hz
//持续时间：160mS            160mS             157mS             157mS
//窗函数：  fade out         fade out          fade out          fade out
//窗持续：  1/2              1/2               1/2               1
AT(.rodata.piano.tbl)
static const tone_tbl_t tone_power_on_tbl[7] = {
    {TONE_FREQUENCY(0x7ff, 200),        TONE_DELAY(160)},
    {0,                                 TONE_DELAY(20)},    //静音
    {TONE_FREQUENCY(0x7ff, 302),        TONE_DELAY(160)},
    {0,                                 TONE_DELAY(20)},    //静音
    {TONE_FREQUENCY(0x7ff, 400),        TONE_DELAY(157)},
    {0,                                 TONE_DELAY(20)},    //静音
    {TONE_FREQUENCY(0x7ff, 501),        TONE_DELAY(157)},
};

//频率：    501Hz  间隔20mS  400Hz   间隔20mS  302Hz   间隔20mS  200Hz
//持续时间：160mS            160mS             157mS             157mS
//窗函数：  fade out         fade out          fade out          fade out
//窗持续：  1/2              1/2               1/2               1
AT(.rodata.piano.tbl)
static const tone_tbl_t tone_power_off_tbl[7] = {
    {TONE_FREQUENCY(0x7ff, 501),        TONE_DELAY(160)},
    {0,                                 TONE_DELAY(20)},    //静音
    {TONE_FREQUENCY(0x7ff, 400),        TONE_DELAY(160)},
    {0,                                 TONE_DELAY(20)},    //静音
    {TONE_FREQUENCY(0x7ff, 302),        TONE_DELAY(157)},
    {0,                                 TONE_DELAY(20)},    //静音
    {TONE_FREQUENCY(0x7ff, 200),        TONE_DELAY(157)},
};

//频率：    450Hz
//持续时间：1365mS
//窗函数：  sine
//窗调制：  15Hz
AT(.rodata.piano.tbl)
static const tone_tbl_t tone_ring_tbl[2] = {
    {TONE_FREQUENCY(0x800a, 450),       (TONE_DELAY(1364)+24)},
    {0,                                 TONE_DELAY(20)},    //静音
};

////频率：    2600Hz
////持续时间：100mS
////窗函数：  fade out
////窗调制：  1
//AT(.rodata.piano.tbl)
//static const tone_tbl_t tone_maxvol_tbl[1] = {
//    {TONE_FREQUENCY(0x7ff, 2600),        TONE_DELAY(100)},
//};

//频率：    210Hz  间隔50mS  260Hz   间隔50mS  400Hz
//持续时间：145mS            437mS             145mS
//窗函数：  fade out         fade out          fade out
//窗调制：  1                1                 1
AT(.rodata.piano.tbl)
static const tone_tbl_t tone_maxvol_tbl[5] = {
    {TONE_FREQUENCY(0x7ff, 210),        TONE_DELAY(145)},
    {0,                                 TONE_DELAY(50)},    //静音
    {TONE_FREQUENCY(0x7ff, 260),        TONE_DELAY(437)},
    {0,                                 TONE_DELAY(50)},    //静音
    {TONE_FREQUENCY(0x7ff, 400),        TONE_DELAY(145)},
};

//频率：    848Hz  间隔50mS  639Hz
//持续时间：207mS            208mS
//窗函数：  fade out         fade out
//窗调制：  1                1
AT(.rodata.piano.tbl)
static const tone_tbl_t tone_low_battery_tbl[3] = {
    {TONE_FREQUENCY(0x7ff, 848),        TONE_DELAY(207)},
    {0,                                 TONE_DELAY(50)},    //静音
    {TONE_FREQUENCY(0x7ff, 639),        TONE_DELAY(208)},
};

AT(.rodata.piano.tbl)
static const tone_tbl_t tone_redialing_tbl[1] = {
    {TONE_FREQUENCY(0x7ff, 524),       TONE_DELAY(100)},
};

AT(.rodata.piano.tbl)
static const tone_tbl_t tone_bt_mode_tbl[3] = {
    {TONE_FREQUENCY(0x7ff, 392),        TONE_DELAY(100)},
    {0,                                 TONE_DELAY(53)},    //静音
    {TONE_FREQUENCY(0x7ff, 586),        TONE_DELAY(98)},
};

//piano提示音
AT(.rodata.piano.tbl)
static const piano_tbl_t piano_bt_mode_tbl[1] = {
    0x00236521
};

AT(.rodata.piano.tbl)
static const piano_tbl_t piano_camera_mode_tbl[1] = {
    0x00030201
};

AT(.rodata.piano.tbl)
static const piano_tbl_t piano_aux_mode_tbl[1] = {
    0x00131203
};

AT(.rodata.piano.tbl)
static const piano_tbl_t piano_power_on_tbl[1] = {
    0x07050301
};

AT(.rodata.piano.tbl)
static const piano_tbl_t piano_power_off_tbl[1] = {
    0x01030507
};

AT(.rodata.piano.tbl)
static const piano_tbl_t piano_finemy_tbl[1] = {
    0x07050301
};

AT(.rodata.piano.table)
const warning_tone_tbl_t warning_tone_tbl[] = {
    [TONE_POWER_ON]     = {sizeof(tone_power_on_tbl)  / sizeof(tone_tbl_t),      tone_power_on_tbl       },
    [TONE_POWER_OFF]    = {sizeof(tone_power_off_tbl) / sizeof(tone_tbl_t),      tone_power_off_tbl      },
    [TONE_PAIR]         = {sizeof(tone_pair_tbl)      / sizeof(tone_tbl_t),      tone_pair_tbl           },
    [TONE_BT_DISCONNECT] = {sizeof(tone_disconnected_tbl) / sizeof(tone_tbl_t),  tone_disconnected_tbl   },
    [TONE_BT_CONNECT]   = {sizeof(tone_connected_tbl) / sizeof(tone_tbl_t),      tone_connected_tbl      },
    [TONE_BT_RING]      = {sizeof(tone_ring_tbl)      / sizeof(tone_tbl_t),      tone_ring_tbl           },
    [TONE_MAX_VOL]      = {sizeof(tone_maxvol_tbl)    / sizeof(tone_tbl_t),      tone_maxvol_tbl         },
    [TONE_LOW_BATTERY]  = {sizeof(tone_low_battery_tbl) / sizeof(tone_tbl_t),    tone_low_battery_tbl    },
    [TONE_REDIALING]    = {sizeof(tone_redialing_tbl) / sizeof(tone_tbl_t),      tone_redialing_tbl      },
    [TONE_BT_MODE]      = {sizeof(tone_bt_mode_tbl) / sizeof(tone_tbl_t),        tone_bt_mode_tbl        },
};

AT(.rodata.piano.table)
const warning_piano_tbl_t warning_piano_tbl[] = {
    [PIANO_POWER_ON]    = {sizeof(piano_power_on_tbl) / sizeof(piano_tbl_t),     piano_power_on_tbl},
    [PIANO_POWER_OFF]   = {sizeof(piano_power_off_tbl) / sizeof(piano_tbl_t),    piano_power_off_tbl},
    [PIANO_BT_MODE]     = {sizeof(piano_bt_mode_tbl) / sizeof(piano_tbl_t),      piano_bt_mode_tbl},
    [PIANO_CAMERA_MODE] = {sizeof(piano_camera_mode_tbl) / sizeof(piano_tbl_t),  piano_camera_mode_tbl},
    [PIANO_AUX_MODE]    = {sizeof(piano_aux_mode_tbl) / sizeof(piano_tbl_t),     piano_aux_mode_tbl},
    [PIANO_FINEMY_SOUND] = {sizeof(piano_finemy_tbl) / sizeof(piano_tbl_t),      piano_finemy_tbl},
};

piano_cb_t piano_cb AT(.bss.piano);


AT(.text.piano)
u8 piano_tone_init(uint type, void *res, bool kick, uint8_t res_flag)
{
    if(piano_is_playing()) {
        return -3;
    }

    piano_cb.type = type;
    piano_cb.spr = SPR_48000;

    if (type == WARNING_PIANO) {
        const warning_piano_tbl_t *p = (const warning_piano_tbl_t *)res;
        if(p->len > PIANO_MAX_LEN*2) {
            return -2;
        }

        memcpy(piano_cb.res_buf, p->res_tbl, sizeof(piano_tbl_t)*p->len);
        piano_cb.len = p->len;
        piano_cb.base_key = 4;
        piano_cb.base_delay = PIANO_BASEKEYDLY;
        piano_cb.harmen = 1;
    } else {
        const warning_tone_tbl_t *p = (const warning_tone_tbl_t *)res;
        if(p->len > PIANO_MAX_LEN) {
            return -2;
        }

        memcpy(piano_cb.res_buf, p->res_tbl, sizeof(tone_tbl_t)*p->len);
        piano_cb.len = p->len;
        piano_cb.harmen = 0;
    }

//    piano_cb.dac1_en = (bool)(DACDIGCON0 & BIT(8));       //SRC1
    dac1_aubuf_init();

    if(res_flag & RES_FLAG_LOCAL) {
        dac1_vol_set(piano_get_digvol_cb());
    } else {
        dac1_vol_set(0);
    }
    dac1_spr_set(piano_cb.spr);

    dac1_fade_in();
    dac1_fade_wait();

    return piano_init(kick);                    //0=ok, -1=fail
}

void piano_tone_exit(void)
{
    dac1_fade_out();
    dac1_aubuf_w4_empty();
    dac1_put_zero(80);
    dac1_aubuf_w4_empty();
//    if (!piano_cb.dac1_en) {
//        DACDIGCON0 &= ~BIT(8);         //SRC1 disable
//    }
    dac1_vol_set(0);
    dac1_fade_wait();

    piano_exit();
}
