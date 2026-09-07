#include "include.h"


void knob_init(void);
void knob_process(void);

struct {
    u8 msg_tbl[KEY_TBL_MAX_NB][KEY_MSG_MAX_IDX];
    u16 hold_msg;
    u16 save_msg;
    u8 ignore_id;
} key_scan;


AT(.com_text.key.table)
const key_shake_tbl_t key_shake_table = {
    .scan_cnt = KEY_SCAN_TIMES,
    .up_cnt   = KEY_UP_TIMES,
    .long_cnt = KEY_LONG_TIMES,
    .hold_cnt = KEY_LONG_HOLD_TIMES,
};

AT(.text.bsp.key.init) ALIGNED(128)
void key_set_msg_tbl(const void *msg_tbl)
{
    u8 msg_buf[KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX];

    if(msg_tbl == NULL) {
        memset(msg_buf, MSG_NO, KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX);
    } else {
        memcpy(msg_buf, msg_tbl, KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX);
        key_replace_msg_tbl_cb(msg_buf, msg_tbl);   //重定义配置的按键消息
    }

    GLOBAL_INT_DISABLE();
    memcpy(key_scan.msg_tbl, msg_buf, KEY_TBL_MAX_NB*KEY_MSG_MAX_IDX);
    GLOBAL_INT_RESTORE();
}

AT(.com_text.bsp.key)
static u16 bsp_key_get_msg(u16 key_evt)
{
    u16 key_id  = key_evt & KEY_ID_MASK;
    u16 evt_type = key_evt & KEY_EVT_MASK;
    if(key_id < KEY_TBL_MAX_NB) {
        u8 msg_idx = key_evt_idx_tbl[evt_type >> KEY_EVT_POS];
        if(msg_idx < KEY_MSG_MAX_IDX) {
            return key_scan.msg_tbl[key_id][msg_idx];
        }
#if IRKEY_EN
    } else if(key_id >= KEY_POWER) {
        //IR按键较多且功能单一，不做转换处理
        return key_evt & ~KEY_TYPE_MASK;        //只保留key_id和key_evt
#endif
    }

    return NO_KEY;
}

void key_set_ignore(u8 usage_id)
{
    key_scan.ignore_id = usage_id;
}

AT(.text.bsp.key.init)
void key_init(void)
{
    key_var_init();
    key_set_msg_tbl(NULL);

#if IOKEY_EN
    io_key_init();
#endif

#if ADKEY_EN || ADKEY2_EN || USER_NTC
    adkey_init();
#endif

#if KNOB_KEY_EN
    knob_init();
#endif

#if PWRKEY_EN
    pwrkey_init();
#endif

#if VBAT_DETECT_EN
    vbat_init();
#endif

#if IRKEY_EN
    irkey_init();
#endif

    bsp_saradc_init();

#if TKEY_EN
    bsp_tkey_init();
#else
    bsp_tkey_off();
#endif
}

AT(.text.bsp.key)
u8 get_double_key_time(void)
{
    if(DOUBLE_KEY_TIME > 7) {
        return 60;
    } else {
        return (u8)((u8)(DOUBLE_KEY_TIME + 2) * 20 + 1);
    }
}

AT(.com_text.bsp.key)
static u16 bsp_key_process(u8 key_uid)
{
    u16 key_evt = key_process(key_uid);
    return key_evt;
}

AT(.com_rodata.bsp.key.str)
const char key_enqueue_str[] = "enqueue: %04x, %x, %x\n";

AT(.com_text.bsp.key)
u8 bsp_key_scan_do(void)
{
    u8 key_uid = KEY_NULL;

    if (!bsp_saradc_process()) {
        return key_uid;
    }

#if KNOB_KEY_EN
    knob_process();
#endif

#if TKEY_EN
    key_uid = bsp_tkey_scan();
#endif

#if PWRKEY_EN
    if ((key_uid == KEY_NULL) && (!PWRKEY_2_HW_PWRON)) {
        key_uid = pwrkey_get_val();
    }
#endif

#if IOKEY_EN
    if (key_uid == KEY_NULL) {
        key_uid = get_iokey();
    }
#endif

#if IRKEY_EN
    if (key_id == KEY_NULL) {
        key_id = irkey_get_val();
    }
#endif

#if ADKEY_EN || ADKEY2_EN ||ADKEY_MUX_SDCLK_EN
    if (key_uid == KEY_NULL) {
        key_uid = adkey_get_val();
    }
#endif
    return key_uid;
}

AT(.com_text.bsp.key)
u8 bsp_key_scan(void)
{
    u8 key_uid = bsp_key_scan_do();
    u16 key_evt = bsp_key_process(key_uid);

    if (key_evt != NO_KEY) {
        u16 key_msg = bsp_key_get_msg(key_evt);
        u16 evt_type = key_evt & KEY_EVT_MASK;
        u8 key_uid = (key_evt & KEY_UID_MASK);

        if(key_scan.ignore_id == key_uid) {     //过滤掉第1次上电长按PWR键消息
            if(evt_type == KEY_LONG_UP || evt_type == KEY_SHORT_UP) {
                key_scan.ignore_id = MSG_NO;
            }
            return key_uid;
        } else if(pwr_usage_id == key_uid) {    //长按关机过程中长按键消息延后处理
            if(evt_type == KEY_LONG) {
                key_scan.save_msg = key_msg;
                return key_uid;
            }
        }

        if (evt_type == KEY_HOLD) {
            //避免在ring和call模式下不响应关机按键时没有长按键消息
            if(pwr_usage_id == key_uid && key_msg != MSG_PWR_HOLD) {
                bsp_key_recover_msg();
            }

            //防止enqueue多个HOLD消息
            key_scan.hold_msg = key_msg;
            if(msg_queue_check(key_msg) > 0) {
                key_msg = MSG_NO;
            }
        } else {
            if(key_scan.hold_msg != key_msg) {
                if(key_scan.hold_msg == MSG_PWR_HOLD) {
                    msg_enqueue(MSG_PWR_RELEASE);
                }
                key_scan.hold_msg = MSG_NO;
            }
        }

        if(key_msg != MSG_NO) {
            //printf(key_enqueue_str,key_uid,key_evt,key_msg);
            msg_enqueue(key_msg);
        }
    }
    return key_uid;
}

AT(.com_text.bsp.key)
void bsp_key_recover_msg(void)
{
    GLOBAL_INT_DISABLE();
    if(key_scan.save_msg != NO_MSG) {
        msg_enqueue(key_scan.save_msg);
        key_scan.save_msg = NO_MSG;
    }
    GLOBAL_INT_RESTORE();
}

uint8_t bsp_get_pwrkey_uid(void)
{
    uint8_t key_uid = KEY_NULL;

#if TKEY_EN
    key_uid = bsp_tkey_scan();
#endif

#if PWRKEY_EN
    if (key_uid == KEY_NULL) {
        key_uid = pwrkey_get_val();
    }
#endif // PWRKEY_EN
    return key_uid;
}

uint8_t bsp_pwrkey_scan(struct pwrkey_scan_tag *pkey)
{
    bsp_key_scan_do();

    if (bsp_get_pwrkey_uid() == pwr_usage_id) {
        pkey->up_cnt = 0;
        if (pkey->state == PWRKEY_IDLE) {
            pkey->state = PWRKEY_W4_PRESS;
            pkey->ticks = tick_get();
        } else if(pkey->state == PWRKEY_W4_PRESS) {
            if (tick_check_expire(pkey->ticks, pkey->press_time)) {
                pkey->state = PWRKEY_PRESS;
                return true;
            }
        } else {
            pkey->state = PWRKEY_IDLE;
        }
    } else {
        if (pkey->up_cnt < 3) {
            pkey->up_cnt++;
            if (pkey->up_cnt == 3) {
                pkey->state = PWRKEY_RELEASE;
                return true;
            }
        }
    }

    return false;
}
