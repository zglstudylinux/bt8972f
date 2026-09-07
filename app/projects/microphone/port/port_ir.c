#include "include.h"

#if IRKEY_EN

#if IR_ADDR_FF00_EN
AT(.com_text.ir.table)
const u8 ir_tbl_FF00[96] =
{
//     0            1               2              3             4               5             6                7
/*0*/  KEY_NULL,    KEY_NULL,       KEY_NULL,      KEY_NULL,     KEY_NULL,       KEY_NULL,     KEY_NULL,        KEY_VOL_DOWN,
       KEY_NUM_4,   KEY_EQ,         KEY_NULL,      KEY_NULL,     KEY_NUM_1,      KEY_REPEAT,   KEY_NULL,        KEY_NULL,
/*1*/  KEY_NULL,    KEY_NULL,       KEY_NULL,      KEY_NULL,     KEY_NULL,       KEY_VOL_UP,   KEY_NUM_0,       KEY_NULL,
       KEY_NUM_2,   KEY_NULL,       KEY_NULL,      KEY_NULL,     KEY_NUM_5,      KEY_NULL,     KEY_NULL,        KEY_NULL,
/*2*/  KEY_NULL,    KEY_NULL,       KEY_NULL,      KEY_NULL,     KEY_NULL,       KEY_NULL,     KEY_NULL,        KEY_NULL,
       KEY_NULL,    KEY_NULL,       KEY_NULL,      KEY_NULL,     KEY_NULL,       KEY_NULL,     KEY_NULL,        KEY_NULL,
/*3*/  KEY_NULL,    KEY_NULL,       KEY_NULL,      KEY_NULL,     KEY_NULL,       KEY_NULL,     KEY_NULL,        KEY_NULL,
       KEY_NULL,    KEY_NULL,       KEY_NULL,      KEY_NULL,     KEY_NULL,       KEY_NULL,     KEY_NULL,        KEY_NULL,
/*4*/  KEY_NEXT,    KEY_NULL,       KEY_NUM_7,     KEY_PLAY,     KEY_PREV,       KEY_POWER,    KEY_MODE,        KEY_MUTE,
       KEY_NULL,    KEY_NULL,       KEY_NUM_9,     KEY_NULL,     KEY_NULL,       KEY_NULL,     KEY_NULL,        KEY_NULL,
/*5*/  KEY_NULL,    KEY_NULL,       KEY_NUM_8,     KEY_NULL,     KEY_NULL,       KEY_NULL,     KEY_NULL,        KEY_NULL,
       KEY_NULL,    KEY_NULL,       KEY_NUM_6,     KEY_NULL,     KEY_NULL,       KEY_NULL,     KEY_NUM_3,       KEY_NULL,
};
#endif // IR_ADDR_FF00_EN

#if IR_ADDR_BF00_EN
AT(.com_text.ir.table)
const u8 ir_tbl_BF00[32] =
{
   //  0            1               2              3             4               5             6                 7
/*0*/  KEY_POWER,   KEY_PLAY,       KEY_MODE,      KEY_NULL,       KEY_NUM_1,      KEY_NUM_2,    KEY_NUM_3,        KEY_NULL,
       KEY_NUM_4,   KEY_NUM_5,      KEY_NUM_6,     KEY_NULL,       KEY_NUM_7,      KEY_NUM_8,    KEY_NUM_9,        KEY_NULL,
/*1*/  KEY_NUM_0,   KEY_NULL,       KEY_VOL_UP,    KEY_NULL,       KEY_PREV,       KEY_NEXT,     KEY_VOL_DOWN,     KEY_NULL,
       KEY_NULL,    KEY_NULL,       KEY_NULL,      KEY_NULL,       KEY_NULL,       KEY_NULL,     KEY_NULL,         KEY_NULL,
};
#endif // IR_ADDR_BF00_EN

#if IR_ADDR_FD02_EN
AT(.com_text.ir.table)
const u8 ir_tbl_FD02[32] =
{
   //  0            1              2              3             4               5               6               7
/*0*/  KEY_POWER,   KEY_MODE,      KEY_MUTE,    KEY_NULL,       KEY_PLAY,       KEY_PREV,       KEY_NEXT,       KEY_NULL,
       KEY_EQ,      KEY_VOL_DOWN,  KEY_VOL_UP,  KEY_NULL,       KEY_NUM_0,      KEY_REPEAT,     KEY_USB_SD,     KEY_NULL,
/*1*/  KEY_NUM_1,   KEY_NUM_2,     KEY_NUM_3,   KEY_NULL,       KEY_NUM_4,      KEY_NUM_5,      KEY_NUM_6,      KEY_NULL,
       KEY_NUM_7,   KEY_NUM_8,     KEY_NUM_9,   KEY_NULL,       KEY_NULL,       KEY_NULL,       KEY_NULL,       KEY_NULL,
};
#endif // IR_ADDR_FD02_EN

#if IR_ADDR_FE01_EN
AT(.com_text.ir.table)
const u8 ir_tbl_FE01[32] =
{
   // 0             1              2               3             4              5               6               7
/*0*/ KEY_NULL,     KEY_EQ,        KEY_VOL_DOWN,   KEY_VOL_UP,   KEY_NUM_0,     KEY_PREV,       KEY_NEXT,       KEY_NUM_1,
      KEY_NUM_2,    KEY_NUM_3,     KEY_NUM_4,      KEY_NULL,     KEY_NUM_7,     KEY_NUM_8,      KEY_NUM_9,      KEY_POWER,
/*1*/ KEY_NULL,     KEY_NULL,      KEY_PLAY,       KEY_NULL,     KEY_NULL,      KEY_NULL,       KEY_NULL,       KEY_NULL,
      KEY_NULL,     KEY_MODE,      KEY_CH_DOWN,    KEY_NUM_5,    KEY_NULL,      KEY_NULL,       KEY_CH_UP,      KEY_NUM_6,
};
#endif // IR_ADDR_FE01_EN

#endif // (IRRX_SW_EN || IRRX_HW_EN)
