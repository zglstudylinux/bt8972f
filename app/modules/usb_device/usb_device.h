#ifndef _USB_DEVICE_H
#define _USB_DEVICE_H

typedef struct {
    u8 vol;
    u8 cur_dev;
    u8 db_level;
    u8 db_level_l;
    u8 db_level_r;
    u8 dev_change;
    u8 rw_sta;
} f_ude_t;

extern f_ude_t f_ude;

extern uint8_t cfg_get_conf_len;
extern uint8_t udm_vol_default_sel;

#endif
