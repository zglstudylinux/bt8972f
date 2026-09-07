#ifndef _WIRELESS_PROC_H
#define _WIRELESS_PROC_H


#define WIRELESS_CON_STA_MASK               ((1<<WIRELESS_CON_LINK_NB)-1)

struct wireless_cb_tag {
    uint8_t change_flag;
    uint8_t change_sta;

    volatile uint8_t connected_sta;
    bool alg_en;
    uint8_t bypass_cnt;

#if WIRELESS_MIC_2TNR_EN
    volatile uint8_t device_con_sta;
    volatile uint8_t device_con_role;           //0 : master, 1 : slave
#endif
};

extern struct wireless_cb_tag wireless_cb;

void wireless_var_init(void);
void wireless_sta_proc(void);
void wireless_audio_bypass(void);
void wireless_audio_enable(void);
u8 wireless_get_status(void);

void wireless_cmd_reset(u8 index);

#endif // _WIRELESS_PROC_H
