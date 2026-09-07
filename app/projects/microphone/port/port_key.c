#include "include.h"


//------------------------------------------------------------------------------------------
AT(.com_rodata.key.tbl)
const u8 key_evt_idx_tbl[] = {
    [KEY_EVT_IDX(KEY_SHORT_UP)] = 0,
    [KEY_EVT_IDX(KEY_LONG)]     = 1,
    [KEY_EVT_IDX(KEY_HOLD)]     = 2,
    [KEY_EVT_IDX(KEY_LONG_UP)]  = 3,
    [KEY_EVT_IDX(KEY_DOUBLE)]   = 4,
    [KEY_EVT_IDX(KEY_THREE)]    = 5,
    [KEY_EVT_IDX(KEY_FOUR)]     = 6,
    [KEY_EVT_IDX(KEY_FIVE)]     = 7,
    [KEY_EVT_IDX(KEY_SHORT)]    = KEY_MSG_MAX_IDX,
    [KEY_EVT_IDX(KEY_LHOLD)]    = KEY_MSG_MAX_IDX,
};

//ADAPTER模式按键消息
const u8 adapter_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX] = {
                //单击,                 长按下,                 HOLD,               长按抬起,               双击,                   三击                四击            五击
    [KEY_1] =   {MSG_MUSIC_PLAY_PAUSE,  MSG_NO,                 MSG_PWR_HOLD,       MSG_NO,                 MSG_NO,                 MSG_NO,             MSG_NO,         MSG_ENTER_DUT},
#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
    [KEY_2] =   {MSG_VOL_DOWN,          MSG_PAIRING,            MSG_NO,             MSG_NO,                 MSG_CHANGE_NR,          MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_3] =   {MSG_VOL_UP,            MSG_PAIRING,            MSG_NO,             MSG_NO,                 MSG_CHANGE_NR,          MSG_NO,             MSG_NO,         MSG_NO},
#else
    [KEY_2] =   {MSG_VOL_DOWN,          MSG_CHANGE_NR,          MSG_NO,             MSG_NO,                 MSG_VOL_DOWN,           MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_3] =   {MSG_VOL_UP,            MSG_CHANGE_NR,          MSG_NO,             MSG_NO,                 MSG_VOL_UP,             MSG_NO,             MSG_NO,         MSG_NO},
#endif
};

//DEVICE模式按键消息
const u8 device_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX] = {
                //单击,                 长按下,                 HOLD,               长按抬起,               双击,                   三击                四击            五击
    [KEY_1] =   {MSG_MUSIC_PLAY_PAUSE,  MSG_NO,                 MSG_PWR_HOLD,       MSG_NO,                 MSG_NO,                 MSG_NO,             MSG_NO,         MSG_ENTER_DUT},
    [KEY_2] =   {MSG_VOL_DOWN,          MSG_PAIRING,            MSG_NO,             MSG_NO,                 MSG_VOL_DOWN,           MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_3] =   {MSG_VOL_UP,            MSG_PAIRING,            MSG_NO,             MSG_NO,                 MSG_VOL_UP,             MSG_NO,             MSG_NO,         MSG_NO},
};

//BT音乐模式按键消息
const u8 bt_music_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX] = {
                //单击,                 长按下,                 HOLD,               长按抬起,               双击,                   三击                四击            五击
    [KEY_1] =   {MSG_MUSIC_PLAY_PAUSE,  MSG_SWITCH_SIRI,        MSG_PWR_HOLD,       MSG_NO,                 MSG_NO,                 MSG_CALL_REDIAL_LAST, MSG_NO,       MSG_ENTER_DUT},
    [KEY_2] =   {MSG_MUSIC_PREV,        MSG_PAIRING,            MSG_NO,             MSG_MUSIC_FB_END,       MSG_VOL_DOWN,           MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_3] =   {MSG_VOL_UP,            MSG_PAIRING,            MSG_NO,             MSG_MUSIC_FF_END,       MSG_VOL_UP,             MSG_NO,             MSG_NO,         MSG_NO},
};

//BT通话模式按键消息
const u8 bt_call_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX] = {
                //单击,                 长按下,                 HOLD,               长按抬起,               双击,                   三击                四击            五击
    [KEY_1] =   {MSG_KEY_PP_KU,         MSG_KEY_PP_KL,          MSG_NO,             MSG_KEY_PP_KLU,         MSG_KEY_PP_DOUBLE,      MSG_KEY_PP_THREE,   MSG_NO,         MSG_NO},
    [KEY_2] =   {MSG_NO,                MSG_PAIRING,            MSG_NO,             MSG_NO,                 MSG_VOL_DOWN,           MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_3] =   {MSG_VOL_UP,            MSG_PAIRING,            MSG_NO,             MSG_NO,                 MSG_VOL_UP,             MSG_NO,             MSG_NO,         MSG_NO},
};

//USBDEV模式按键消息
const u8 usbdev_key_msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX] = {
                //单击,                 长按下,                 HOLD,               长按抬起,               双击,                   三击                四击            五击
#if USB_IPHONE_PREMOTE_EN
    [KEY_1] =   {MSG_KEY_PP_KU,         MSG_KEY_PP_KL,          MSG_NO,             MSG_KEY_PP_KLU,         MSG_NO,                 MSG_NO,             MSG_NO,         MSG_NO},
#else
    [KEY_1] =   {MSG_MUSIC_PLAY_PAUSE,  MSG_NO,                 MSG_NO,             MSG_NO,                 MSG_NO,                 MSG_NO,             MSG_NO,         MSG_NO},
#endif
    [KEY_2] =   {MSG_MUSIC_PREV,        MSG_NO,                 MSG_NO,             MSG_NO,                 MSG_VOL_DOWN,           MSG_NO,             MSG_NO,         MSG_NO},
    [KEY_3] =   {MSG_VOL_UP,            MSG_NO,                 MSG_NO,             MSG_NO,                 MSG_VOL_UP,             MSG_NO,             MSG_NO,         MSG_NO},
};


//------------------------------------------------------------------------------------------
//KEY_1消息重定义
const u16 key1_msc_cfg_tab[] = {
    MSG_NO,                 MSG_MUSIC_PLAY_PAUSE,       MSG_MUSIC_PREV,                 MSG_MUSIC_NEXT,
    MSG_VOL_UP,             MSG_VOL_DOWN,               MSG_CALL_REDIAL_LAST,           MSG_SWITCH_SIRI,
    MSG_MUSIC_LOW_LATENCY,  MSG_CHANGE_ANC,             MSG_CHANGE_LANGUAGE,            MSG_CHANGE_MODE,
    MSG_HID_KEY,            MSG_HID_HOME,               MSG_ENTER_DUT,
};

const u16 key1_call_cfg_tab[] = {
    MSG_NO,                 MSG_CALL_ANSWER_AND_HOLD,   MSG_CALL_ANSWER_AND_REJECT,     MSG_CALL_REJECT,
    MSG_CALL_SWITCH_PRIV,   MSG_CALL_SWITCH_3WAY_CALL,  MSG_VOL_UP,                     MSG_VOL_DOWN,
};

//重定义配置的按键消息
void key_replace_msg_tbl_cb(u8 *msg_buf, const void *msg_tbl)
{
    u8 *msg = &msg_buf[KEY_1*KEY_MSG_MAX_IDX];
    if(msg_tbl == bt_music_key_msg_tbl && xcfg_cb.key1_msc_cfg_en) {
        msg[0] = key1_msc_cfg_tab[xcfg_cb.key1_msc_ks_sel];
        msg[1] = key1_msc_cfg_tab[xcfg_cb.key1_msc_kl_sel];
        msg[4] = key1_msc_cfg_tab[xcfg_cb.key1_msc_kd_sel];
        msg[5] = key1_msc_cfg_tab[xcfg_cb.key1_msc_kth_sel];
        msg[6] = key1_msc_cfg_tab[xcfg_cb.key1_msc_kfour_sel];
        msg[7] = key1_msc_cfg_tab[xcfg_cb.key1_msc_kfive_sel];
    }

    if(msg_tbl == bt_call_key_msg_tbl && xcfg_cb.key1_call_cfg_en) {
        msg[0] = key1_call_cfg_tab[xcfg_cb.key1_call_ks_sel];
        msg[1] = key1_call_cfg_tab[xcfg_cb.key1_call_kl_sel];
        msg[4] = key1_call_cfg_tab[xcfg_cb.key1_call_kd_sel];
        msg[5] = key1_call_cfg_tab[xcfg_cb.key1_call_kth_sel];
        msg[6] = key1_msc_cfg_tab[xcfg_cb.key1_call_kfour_sel];
        msg[7] = key1_msc_cfg_tab[xcfg_cb.key1_call_kfive_sel];
    }
}

//------------------------------------------------------------------------------------------

#if (ADKEY_EN || ADKEY_MUX_SDCLK_EN)
#if ADKEY_PU10K_EN
/******************************************************************
*                   内部10K上拉的table表
* 1、不复用SDCLK时，最多支持7个按键，按键电阻0R, 2K, 5.1K, 10K, 22K, 56K, 150K
* 2、复用SDCLK时, 需要从2K电阻的按键开始，最多6个按键
* 3、150K电阻也可以用于插入检测
*******************************************************************/
AT(.com_text.adkey.table)
const adkey_tbl_t adkey_table[] = {
//    {0x0A, KEY_NORMAL(KEY_1)},          //PLAY          0R
//    {0x3F, KEY_NORMAL(KEY_2)},          //PREV/VOL-     2K
//    {0x6C, KEY_NORMAL(KEY_3)},          //NEXT/VOL+     5.1K,
//    {0x97, KEY_NORMAL(KEY_4)},          //MODE          10K,
//    {0xC1, KEY_NORMAL(KEY_5)},          //HSF           22K,
//    {0xE7, KEY_NORMAL(KEY_6)},          //REPEAT        56K,
//    {0xF7, KEY_NORMAL(KEY_7)},          //REC           150K
//    {0xFF, KEY_NULL},                   //              END

    {0x0A, KEY_MULTI(KEY_1)},           //PP            0R
    {0x9D, KEY_MULTI(KEY_2)},           //KEY1          12K
    {0xE4, KEY_MULTI(KEY_3)},           //KEY1          47K
    {0xFF, KEY_NULL},                   //              END
};
#else

#if ADKEY_MUX_LED_EN
/******************************************************************
*                   ADKEY复用LED的table表（外部100K上拉, 蓝灯）
* 1、最多支持7个按键
*******************************************************************/
AT(.com_text.adkey.table)
const adkey_tbl_t adkey_table[] = {
    {0x76, KEY_NORMAL(KEY_1)},          //PLAY          75K,
    {0x8C, KEY_NORMAL(KEY_2)},          //PREV          100K,
    {0xA1, KEY_NORMAL(KEY_3)},          //NEXT          150K,
    {0xB6, KEY_NORMAL(KEY_4)},          //MODE          200K,
    {0xCE, KEY_NORMAL(KEY_5)},          //HSF           330K,
    {0xE0, KEY_NORMAL(KEY_6)},          //VOL-          560K,
    {0xF3, KEY_NORMAL(KEY_7)},          //VOL+          1M,
    {0xFF, KEY_NULL},                   //              END
};
#else
/******************************************************************
*                   外部10K上拉的table表
* 1、最多支持12个按键
* 2、复用SDCLK时, 需要从2K电阻的按键开始或ADKEY先串个10K电阻到IO
* 3、100K电阻也可以用于插入检测
*******************************************************************/
AT(.com_text.adkey.table)
const adkey_tbl_t adkey_table[] = {
    {0x0A, KEY_NORMAL(KEY_1)},          //PLAY          0R
    {0x20, KEY_NORMAL(KEY_2)},          //PREV          1K
    {0x35, KEY_NORMAL(KEY_3)},          //NEXT          2K
    {0x4A, KEY_NORMAL(KEY_4)},          //MODE          3.3K
    {0x62, KEY_NORMAL(KEY_5)},          //HSF           5.1K,
    {0x76, KEY_NORMAL(KEY_6)},          //VOL-          7.5K,
    {0x8C, KEY_NORMAL(KEY_7)},          //VOL+          10K,
    {0xA8, KEY_NORMAL(KEY_8)},          //EQ            15K,
    {0xB8, KEY_NORMAL(KEY_9)},          //REPEAT        20K,
    {0xCB, KEY_NORMAL(KEY_10)},         //MINUS         33K,
    {0xE4, KEY_NORMAL(KEY_11)},         //PLUS          56K
    {0xF4, KEY_NORMAL(KEY_12)},         //REC           100K,
    {0xFF, KEY_NULL},                   //              END
};
#endif // ADKEY_MUX_LED_EN
#endif // ADKEY_PU10K_EN

#endif // ADKEY_EN

#if ADKEY2_EN
///最多支持12个按键, 以0xff结束
AT(.com_text.adkey2.table)
const adkey_tbl_t adkey2_table[] = {
    {0x0A, KEY_SINGLE(KEY_NUM_0)},
    {0x20, KEY_SINGLE(KEY_NUM_1)},
    {0x35, KEY_SINGLE(KEY_NUM_2)},
    {0x4A, KEY_SINGLE(KEY_NUM_3)},
    {0x68, KEY_SINGLE(KEY_NUM_4)},
    {0x78, KEY_SINGLE(KEY_NUM_5)},
    {0x8E, KEY_SINGLE(KEY_NUM_6)},
    {0xA8, KEY_SINGLE(KEY_NUM_7)},
    {0xB8, KEY_SINGLE(KEY_NUM_8)},
    {0xCB, KEY_SINGLE(KEY_NUM_9)},
    {0xE4, KEY_NULL},
    {0xF4, KEY_NULL},
    {0xFF, KEY_NULL},
};
#endif // ADKEY2_EN

#if PWRKEY_EN
///最多支持5个按键。数组元数总数请保持不变。不需要的按键改为KEY_NULL
AT(.com_text.pwrkey.table)
const adkey_tbl_t pwrkey_table[6] = {
    //TWS DEV, PCB.19-050D
    {0x0A, KEY_MULTI(KEY_1)},           //KEY1          0R
    {0xAF, KEY_MULTI(KEY_2)},           //KEY2          15K
    {0xE1, KEY_MULTI(KEY_3)},           //KEY3          33K
    {0xFF, KEY_NULL},                   //              END
    {0xFF, KEY_NULL},                   //              END
    {0xFF, KEY_NULL},                   //              END
};

#endif // PWRKEY_EN

#if IOKEY_EN
#if IOKEY_GPIO_SEL_EN
gpio_t iokey0_gpio;
gpio_t iokey1_gpio;

//工具配置的IO初始化
AT(.com_text.key.init)
void iokey_cfg_port_init(gpio_t *g)
{
    if ((g == NULL) || (g->sfr == NULL)) {
        return;
    }
    g->sfr[GPIOxDE] |= BIT(g->num);
    g->sfr[GPIOxPU] |= BIT(g->num);     //上拉
    g->sfr[GPIOxDIR] |= BIT(g->num);    //输入
}

AT(.com_text.port.key)
bool iokey_cfg_port_pressed(gpio_t *g)
{
    if ((g == NULL) || (g->sfr == NULL)) {
        return false;
    }
    return (!(g->sfr[GPIOx] & BIT(g->num)));
}

AT(.com_text.port.key)
void iokey_cfg_port_out_low(gpio_t *g)
{
    if ((g == NULL) || (g->sfr == NULL)) {
        return;
    }
    g->sfr[GPIOxDE] |= BIT(g->num);
    g->sfr[GPIOxDIR] &= ~BIT(g->num);   //输出
    g->sfr[GPIOxCLR] = BIT(g->num);     //低电平
}

AT(.com_text.port.key)
bool iokey_cfg_midkey_pressed(gpio_t *g0, gpio_t *g1)
{
    bool pressed_flag = false;

    iokey_cfg_port_out_low(g0);
    delay_us(6);
    if (iokey_cfg_port_pressed(g1)) {
        pressed_flag = true;
    }
    iokey_cfg_port_init(g0);
    return pressed_flag;
}
#endif

AT(.text.key.init)
void io_key_init(void)
{
    //工具配置了IOKEY的按键定义？
#if IOKEY_GPIO_SEL_EN
    bsp_gpio_cfg_init(&iokey0_gpio, IOKEY_GPIO_SEL0);
    bsp_gpio_cfg_init(&iokey1_gpio, IOKEY_GPIO_SEL1);

    iokey_cfg_port_init(&iokey0_gpio);
    iokey_cfg_port_init(&iokey1_gpio);
#else
    GPIOFDE  |= BIT(0) | BIT(1);
    GPIOFDIR |= BIT(0) | BIT(1);
    GPIOFPU  |= BIT(0) | BIT(1);
#endif
}

AT(.com_text.port.key)
u8 get_iokey(void)
{
    u8 key_val = KEY_NULL;

#if IOKEY_GPIO_SEL_EN
    if (iokey_cfg_port_pressed(&iokey0_gpio)) {
        key_val = KEY_NORMAL(KEY_1);
    } else if (iokey_cfg_port_pressed(&iokey1_gpio)) {
        key_val = KEY_NORMAL(KEY_2);
  #if IOKEY_MID01_EN
    } else if (iokey_cfg_midkey_pressed(&iokey0_gpio, &iokey1_gpio)) {
        key_val = KEY_NORMAL(KEY_3);
  #endif
    }
#else
    if (!(GPIOF & BIT(0))) {
        //KEY0
        key_val = KEY_NORMAL(KEY_1);
    } else if (!(GPIOF & BIT(1))) {
        //KEY1
        key_val = KEY_NORMAL(KEY_2);
    }
#endif
    return key_val;
}
#endif // IOKEY_EN

