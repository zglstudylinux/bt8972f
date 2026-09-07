#ifndef __BT_CALL
#define __BT_CALL

extern const int mic_gain_tbl[16];
void bt_sco_code_init(void);
void bt_sco_nr_init(u8 *sysclk, nr_cb_t *nr);
void bt_sco_nr_exit(void);
void bt_sco_agc_init(void);

void bt_sco_dnn_init(u8 *sysclk, nr_cb_t *nr);
void bt_sco_dnn_pro_init(u8 *sysclk, nr_cb_t *nr);

#if BT_SCO_SMIC_AI_EN
    #define bt_sco_near_nr_init(a, b)       bt_sco_dnn_init(a, b)
    #define bt_sco_near_nr_exit()           bt_dnn_exit()
#elif BT_SCO_SMIC_AI_PRO_EN
    #define bt_sco_near_nr_init(a, b)       bt_sco_dnn_pro_init(a, b)
    #define bt_sco_near_nr_exit()           bt_dnn_pro_exit()
#else
    #define bt_sco_near_nr_init(a, b)
    #define bt_sco_near_nr_exit()
#endif

#endif
