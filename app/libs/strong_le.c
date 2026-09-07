/**********************************************************************
*
*   strong_le.c
*   定义库里面le部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"


void strong_le_include(void) {}
#if !LE_EN && !BT_ATT_EN
void btstack_gatt_init(void) {}
void gatt_init_att(void **cb_info_table_p, uint16_t cb_info_tb_size) {}
bool gatt_init_att_info(uint16_t att_handle, gatt_characteristic_cb_info_t *att_cb_info) {return false;}
#endif

#if !BT_ATT_EN
void latt_establish_service_level_connection(uint8_t * bd_addr) {}
void latt_release_service_level_connection(uint8_t * bd_addr) {}
void latt_send_kick(void) {}
void latt_att_profile_init(void * latt_con) {}
int latt_tx_notify_for_index(uint8_t index, uint16_t att_handle, const uint8_t *value, uint16_t value_len) {return -1;}
void * provide_latt_connection_context_for_bd_addr(void * bd_addr) {return NULL;}
void remove_latt_connection_context(void * latt_con) {}
int latt_send_notify_packet(void *context, struct txbuf_tag *buf) {return -1;}
void latt_event_server_send(uint16_t cid) {}
void btstack_latt_send(void) {}
void latt_init_register_service(void) {}
void latt_init_do(void) {}
#endif

#if !LE_EN && !WIRELESS_EN
typedef uint8_t (*ll_cntl_func_t)(uint8_t opcode);

void ll_init(uint8_t init_type) {}
uint8_t ll_start(uint8_t index, void *param) {return -1;}
void ll_stop(uint8_t index) {}
void ll_cntl_state_set(uint8_t index, uint8_t txrx, uint8_t state) {}
void ll_proc_timer_set(uint8_t index, uint8_t type, bool enable) {}
void ll_proc_timer_set_state(uint8_t index, uint8_t type, bool enable) {}
void ll_cntl_send(uint8_t index, void *pdu, ll_cntl_func_t tx_func) {}
void ll_cntl_tx_check(uint8_t index) {}
bool ble_adv_end_con_ind(void const *param) { return false; }
AT(.com_text.weak.bb.ble.chs)
void ble_channel_assess(uint8_t channel, bool rx_ok, uint32_t ts) {}
AT(.com_text.weak.bb.ble.end)
void ble_con_rx_end(uint8_t index, bool rx_ok, uint16_t rxchass) {}
int hci_le_create_con_cancel_cmd_func(void const *param, uint16_t id) {return 0;}
#endif

#if !LE_EN
AT(.com_text.stack.ble.mtu)
u16 ble_get_gatt_mtu(void) { return 0; }
int ble_tx_notify_do(void *att, u8 idx, u16 handle, u8* buf, u8 len) {return 0x0c;}
int ble_tx_notify(u16 handle, u8* buf, u8 len) {return 0x0c;}
int ble_tx_indication(u16 handle, u8* buf, u8 len) {return 0x0c;}
void ble_update_conn_param(u16 interval, u16 latency, u16 timeout) {}
void ble_set_adv_interval(u16 interval) {}
void *btstack_get_app_ble_att_connect(void) {return NULL;}
void btstack_ble_send_req(uint param) {}
int ble_send_notify_packet(void *context, void *buf) {return -1;}
void att_event_server_send(uint16_t con_handle) {}
//void hci_run_le_connection(void) {}
bool ble_event_cmd_complete(uint8_t *packet, int size) { return false; }
void ble_event_meta(uint8_t *packet, int size) {}
void btstack_ble_init(void) {}
void btstack_ble_update_conn_param(uint param) {}
void btstack_ble_set_adv_interval(uint param) {}
void btstack_ble_set_adv_data(uint param) {}
void btstack_ble_set_scan_rsp_data(uint param) {}
void btstack_ble_disconnect(uint param) {}
void btstack_ble_send_sm_req(uint param) {}

typedef uint8_t (*bb_msg_func_t)(uint16_t index, void const *param);
typedef uint8_t (*bb_cmd_func_t)(uint16_t cmd, void const *param);
typedef uint8_t (*ll_cntl_func_t)(uint8_t opcode);

struct nanos_task_desc
{
    u8* tab;
    void* state;
    uint16_t idx_max;
    uint16_t msg_cnt;
};

struct ll_cntl_pdu_info
{
    ll_cntl_func_t  func;
    const char*     desc;
    uint16_t        length;
    uint8_t         flag;
};

struct bb_msg_info
{
    uint16_t index;
    bb_msg_func_t func;
};

struct bb_cmd_info
{
    uint16_t index;
    bb_cmd_func_t func;
};

#if !LE_BQB_RF_EN
struct hci_cmd_desc_tag
{
    uint8_t cmd[12];
};
#endif

struct hci_evt_desc_tag
{
    uint8_t evt[8];
};

AT(.rodata.le.ll_proc)
const struct nanos_task_desc ll_msg_env;
AT(.rodata.le.ll_proc)
const struct bb_cmd_info ll_hci_cmd_tbl[1] = {0};
AT(.rodata.le.ll_proc)
const struct bb_msg_info ll_msg_tbl[1] = {0};
AT(.rodata.le.ll_cntl)
const struct ll_cntl_pdu_info ll_cntl_pdu_tbl[1] = {0};
AT(.rodata.le.ll_proc)
const struct bb_msg_info mgr_hci_cmd_tbl[1] = {0};
#if !LE_BQB_RF_EN
AT(.rodata.le.hci)
const struct hci_cmd_desc_tag hci_cmd_desc_tab_le[1] = {0};
#endif
AT(.rodata.le.hci)
const struct hci_evt_desc_tag hci_evt_le_desc_tab[1] = {0};

AT(.com_text.weak.bb.ble.isr)
void ble_pwr_ctrl_hook(uint8_t index) {}
AT(.com_text.weak.bb.ble.adj)
void ble_sched_prio_time_adj(void *item) {}
void aes_init(uint8_t init_type) {};
void aes_result_handler(uint8_t status, uint8_t* result) {};
void ble_ecpy(uint8_t *key, uint8_t *enc_data) {}
void mgr_clk_acc_set(uint8_t aid, bool clk_acc) {}
void mgr_reset(void) {}
void mgr_1st_reset(void) {}
int ble_adv_end_ind_do(uint16_t const mid, void const *param, uint16_t const did, uint16_t const sid) {return 0;}
int ble_scan_end_ind_do(uint16_t const mid, void const *param, uint16_t const did, uint16_t const sid) {return 0;}

void ble_adv_rand_addr_update(uint8_t index, void *addr) {}
void ble_adv_scan_rsp_data_update(uint8_t index, uint8_t len, uint16_t buffer) {}
void ble_adv_adv_data_update(uint8_t index, uint8_t len, uint16_t buffer) {}
void ble_adv_restart(uint8_t index, uint16_t dura, uint8_t max_ext) {}
void * get_ble_link_ctrl_context_for_bd_addr(uint8_t *bd_addr) {return NULL;}
void * alloc_ble_link_ctrl_context_for_bd_addr(uint8_t *bd_addr, uint16_t provide_flag) {return NULL;}
void * provide_ble_link_ctrl_context_for_bd_addr(uint8_t *bd_addr, uint16_t provide_flag) {return NULL;}
void remove_ble_link_ctrl_context(void * link_ctrl, uint16_t provide_flag) {}
void *get_ble_link_ctrl_for_index(uint8_t index) {return NULL;}
bool ble_link_provide_check(uint8_t index, uint16_t provide_flag) {return false;}
void *get_ble_link_ctrl_for_handle(uint16_t con_handle) {return NULL;}

///llcp
void ll_con_move_cb(uint16_t id) {}
void ll_ping_restart(uint8_t id) {}
uint8_t ll_ping_set(uint8_t id, uint16_t au_to) {return 0;}
#endif

#if WIRELESS_EN && !ADAPTER_SAVE_PARAM_EN
int wl_hci_le_con_upd_cmd_func(uint8_t id, void *param, uint16_t opcode) {return 0;}
uint8_t ll_con_update_ind_func(uint8_t id, void *param, uint16_t opcode) {return 0;}
int ll_op_con_upd_ind_func(uint8_t aid, uint *ret, uint8_t rid, uint8_t index) {return 0;}
int ble_con_param_upd_cfm_func(uint8_t aid, uint *ret, uint8_t rid, uint8_t index) {return 0;}
int ble_con_offset_upd_ind_func(uint8_t aid, uint *ret, uint8_t rid, uint8_t index) {return 0;}
#endif

#if ((!BT_TWS_EN) || (!LE_EN))
uint16_t tws_get_ble_service(uint8_t *data_ptr){return 0;}
uint16_t tws_ble_get_adv_info(uint8_t *data_ptr){return 0;}
void tws_ble_set_adv_info(uint8_t *data_ptr, uint16_t size){}
void tws_set_ble_service(uint8_t *data_ptr, uint16_t size){}
void tws_send_ble_service_cfm(void) {}
void tws_send_ble_service_continue_cfm(void) {}
uint16_t tws_get_ble_link(uint8_t *data_ptr) { return 0; }
uint16_t tws_set_ble_link(uint8_t *data_ptr, uint16_t size) { return 0; }
bool tws_ble_get_link_info_bb(uint16_t *ble_conhdl, uint8_t *bdaddr) {return false;}
uint8_t tws_get_ble_service_bb(uint8_t *data_ptr) {return 0;}
void tws_send_ble_service_cfm_bb(void) {}
void tws_send_ble_service_continue_cfm_bb(void) {}
void tws_ble_switch_pend(void) {}
uint16_t tws_get_ble_service_continue_len(void) {return 0;}
void tws_ble_restore_tx(void) {}
void tws_ble_con_stop(void) {}
void tws_set_ble_service_continue(uint8_t *data_ptr, uint16_t size) {}
bool tws_ble_switch_save_pend(uint8_t type, uint8_t flag, uint32_t buf_ptr, uint16_t len) { return false; }
uint16_t tws_get_ble_service_continue(uint8_t *data_ptr) { return 0;}
int ll_ble_switch_to_func(uint16_t id, void *param, uint16_t did, uint16_t sid) {return 0;}
uint8_t tws_ble_switch_is_active(void) {return 0;}
void tws_le_init(bool reset) {}
void tws_ble_switch(void) {}
bool hci_acl_data_hook(u16 const did, void *param) {return false;}
bool ble_acl_data_rx_hook(u16 const did, void const *param) {return false;}
void hct_flush_le_acl_rx_pend(uint8_t index) {}
AT(.com_text.weak.bb.ble.sw_nb)
uint8_t ble_get_con_nb(void) {return 0;}
int lc_op_vs_le_switch_func(u16 const mid, void const *param, u16 const dest_id, u16 const sid) {return 0;}
AT(.text.stk.tws.send)
bool tws_ble_l2cap_send_req(void) {return false;}
#endif

#if !LE_PAIR_EN
#if (LE_ADDRESS_TYPE == GAP_RANDOM_ADDRESS_TYPE_OFF)
void le_sm_init(void) {}
void gap_random_address_set(void) {}
void gap_random_address_set_mode(void) {}
void le_device_db_init(void){}
int gap_authorization_state(uint16_t con_handle){return 3;}
void sm_get_rand_addr(uint8_t * addr) {}
#endif
void sm_just_works_confirm(void) {}
void sm_request_pairing(void) {}
void sm_send_security_request(void) {}
int sm_authorization_state(void) { return 0; }
int sm_authenticated(void) { return 0; }
int sm_encryption_key_size(void) { return 0; }
void sm_add_event_handler(void) {}
void sm_cmac_signed_write_start(void){ }
int sm_cmac_ready(void) { return 0; }
uint16_t sm_set_setup_context(uint8_t *ptr){ return 0;}
uint16_t sm_get_setup_context(uint8_t *ptr){ return 0;}
uint16_t tws_get_sm_db_info(uint8_t *ptr){ return 0;}
uint16_t tws_set_sm_db_info(uint8_t *ptr, uint16_t len){ return 0;}
#endif

#if !LE_ADV0_EN
void ble_adv0_init(uint8_t init_type) {}
uint8_t adv0_data[1];
void btstack_adv0_ctrl(uint opcode) {}
bool ble_adv0_end_ind(void *param) { return false;}
void ble_adv0_set_en(bool adv_en) {}
void ble_adv0_update_adv_data(const uint8_t *adv_ptr, u32 size) {}
void ble_adv0_update_scan_rsp_data(const uint8_t *scan_rsp_ptr, u32 size) {}
#endif

#if !LE_PRIV_EN
void btstack_priv_con_ctrl(uint parms) {}
int ble_priv_tx_notify(u16 att_handle, u8* buf, u8 len) {return 0;}
void ble_priv_con_ctrl(u16 opcode) {}
bool ble_priv_is_connect(void) {return false;}
uint8_t priv_adv_data[1];
void ble_priv_adv_init(uint8_t init_type) {};
void ble_priv_adv_set_en(bool adv_en) {}
bool ble_priv_end_ind(void *param) {return false;}
bool ble_is_priv_adv_id(u8 id) {return false;}
bool ble_priv_adv_get_acc_addr(u8 act_id, u32 *acc_addr) {return false;}
void ble_priv_adv_update_adv_data(const uint8_t *adv_ptr, u32 size) {}
void ble_priv_adv_update_scan_data(const uint8_t *scan_rsp_ptr, u32 size) {}
void ble_priv_adv_set_interval(u16 interval) {}
#endif

#if WIRELESS_EN
uint16_t ble_con_get_con_type(uint8_t link);
void wireless_con_evt_start_do(uint8_t link);
///v3
void ble_con_end_cb_v3(uint8_t link);
void ble_t2r_v3_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt);
void ble_t2r_v3_init(uint8_t init_type);
uint8_t ble_t2r_v3_start(uint16_t con_handle, uint8_t test_rx, uint8_t second_con_vers_en, uint16_t second_con_offset);
uint8_t ble_t2r_v3_stop(uint16_t con_handle);
uint16_t ble_t2r_v3_link_duration_get(void);
void ble_t2r_v3_skip_first_set_enable(uint8_t index, bool enable);
void ble_t2r_v3_pwr_ctr(uint8_t index, uint8_t pwr_level);
///v4
void ble_con_end_cb_v4(uint8_t link);
void ble_t2r_v4_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt);
void ble_t2r_v4_calc_anchor_ts_diff(uint8_t link);
void ble_t2r_v4_init(uint8_t init_type);
uint8_t ble_t2r_v4_start(uint16_t con_handle, uint8_t test_rx, void *pdu);
uint8_t ble_t2r_v4_stop(uint16_t con_handle);
bool ble_t2r_v4_evt_cnt_is_free(void);
bool ble_t2r_v4_evt_cnt_alloc(uint8_t link, uint16_t instant, uint16_t *evt_cnt_ret);
void ble_t2r_v4_evt_cnt_free(uint8_t link);
void ble_t2r_v4_ch_map_update(uint8_t link, void *map, uint16_t instant);
bool ble_t2r_v4_param_get(uint16_t *sw, void *map, uint16_t *offset);
void ll_tis_req(uint8_t index, uint8_t test_rx);
///v6&v7
void ble_t2r_v6_init(uint8_t init_type);
uint8_t ble_t2r_v6_start(uint16_t con_handle, bool test_rx, uint8_t second_con_vers_en, uint16_t second_con_offset);
uint8_t ble_t2r_v6_stop(uint16_t con_handle);
void ble_con_end_cb_v6(uint8_t link);
void wireless_con_evt_start_do(uint8_t link);
void ble_t2r_v6_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt);
uint16_t ble_t2r_v6_link_duration_get(void);
void ble_t2r_v6_pwr_ctr(uint8_t index, uint8_t pwr_level);
void ble_t2r_v6_link_all_ctr(uint8_t enable);
///v10
void ble_con_end_cb_v10(uint8_t link);
void ble_t2r_v10_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt);
void ble_t2r_v10_init(uint8_t init_type);
uint8_t ble_t2r_v10_start(uint16_t con_handle, uint8_t test_rx);
uint8_t ble_t2r_v10_stop(uint16_t con_handle);
uint16_t ble_t2r_v10_link_duration_get(void);
void ble_t2r_v10_skip_first_set_enable(uint8_t index, bool enable);
void ble_t2r_v10_pwr_ctr(uint8_t index, uint8_t pwr_level);
void ble_t2r_v10_send_audio_setup(uint8_t index, uint8_t test_rx);


AT(.com_text.ble.isr.con)
void ble_con_evt_start_do_cb(uint8_t link)
{
    wireless_con_evt_start_do(link);
}

AT(.com_text.ble.isr.con)
void ble_con_end_cb(uint8_t link)
{
    uint16_t con_type = ble_con_get_con_type(link);

    switch(con_type&0xff) {
        case CON_VERS3:
            ble_con_end_cb_v3(link);
            break;
        case CON_VERS4:
            ble_con_end_cb_v4(link);
            break;
        case CON_VERS6:
        case CON_VERS7:
            ble_con_end_cb_v6(link);
            break;
        case CON_VERS10:
            ble_con_end_cb_v10(link);
            break;
    }
}

void ble_con_established_cb(uint8_t link)
{
//    ble_con_established_cb_v3(link);
}

AT(.com_text.ble.isr.con)
void ble_con_anchor_ts_update_cb(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt)
{
    uint16_t con_type = ble_con_get_con_type(link);

    switch(con_type&0xff) {
        case CON_VERS3:
            ble_t2r_v3_anchor_ts_update(link, anchor_ts, anchor_hus, evt_cnt);
            break;
        case CON_VERS4:
            ble_t2r_v4_anchor_ts_update(link, anchor_ts, anchor_hus, evt_cnt);
            break;
        case CON_VERS6:
        case CON_VERS7:
            ble_t2r_v6_anchor_ts_update(link, anchor_ts, anchor_hus, evt_cnt);
            break;
        case CON_VERS10:
            ble_t2r_v10_anchor_ts_update(link, anchor_ts, anchor_hus, evt_cnt);
            break;
    }
}

void wl_audio_init_cb(uint8_t init_type)
{
    ///没使能的放在weak函数
    ble_t2r_v3_init(init_type);
    ble_t2r_v4_init(init_type);
    ble_t2r_v6_init(init_type);
    ble_t2r_v10_init(init_type);
}

uint8_t wl_audio_start_cb(uint16_t con_type, uint16_t con_handle, uint8_t test_rx, void *param)
{
    uint8_t ret = 0;
    uint8_t second_con_vers_en = 0;
    uint16_t second_con_offset = 0;
#if ADAPTER_SECOND_CON_VERS_EN
    if (con_type == ADAPTER_SECOND_CON_VERS) {
        second_con_vers_en = ADAPTER_SECOND_CON_VERS_EN;
        second_con_offset  = wl_single_link_duration_get(ADAPTER_FIRST_CON_VERS) + 200;
    }
#endif
    printf("wl_audio_start_cb, %d\n", con_type);
    switch(con_type&0xff) {
        case CON_VERS3:
            ret = ble_t2r_v3_start(con_handle, test_rx,second_con_vers_en,second_con_offset);
            break;
        case CON_VERS4:
            ret = ble_t2r_v4_start(con_handle, test_rx, param);
            break;
        case CON_VERS6:
        case CON_VERS7:
            ret = ble_t2r_v6_start(con_handle, test_rx,second_con_vers_en,second_con_offset);
            break;
        case CON_VERS10:
            ret = ble_t2r_v10_start(con_handle, test_rx);
            break;
    }

    return ret;
}

uint8_t wl_audio_stop_cb(uint16_t con_type, uint16_t con_handle)
{
    uint8_t ret = 0;

    printf("wl_audio_stop_cb, %d\n", con_type);
    switch(con_type&0xff) {
        case CON_VERS3:
            ret = ble_t2r_v3_stop(con_handle);
            break;
        case CON_VERS4:
            ret = ble_t2r_v4_stop(con_handle);
            break;
        case CON_VERS6:
        case CON_VERS7:
            ret = ble_t2r_v6_stop(con_handle);
            break;
        case CON_VERS10:
            ret = ble_t2r_v10_stop(con_handle);
            break;
    }

    return ret;
}

#if !WIRELESS_MIC_BROADCAST_EN
uint16_t wl_single_link_duration_get(uint8_t con_type)
{
    uint16_t ret = 0;

    printf("wl_single_link_duration_get, %d\n", con_type);
    switch(con_type&0xff) {
        case CON_VERS3:
            ret = ble_t2r_v3_link_duration_get();
            break;
        case CON_VERS4:
            break;
        case CON_VERS6:
        case CON_VERS7:
            ret = ble_t2r_v6_link_duration_get();
            break;
        case CON_VERS10:
            ret = ble_t2r_v10_link_duration_get();
            break;
    }

    return ret;
}
#endif

void wl_skip_first_set(uint8_t link, uint8_t index, uint8_t enable)
{
    uint16_t con_type = ble_con_get_con_type(link);

    switch(con_type&0xff) {
        case CON_VERS3:
            ble_t2r_v3_skip_first_set_enable(index, enable);
            break;
        case CON_VERS4:
            break;
        case CON_VERS6:
        case CON_VERS7:
            break;
        case CON_VERS10:
            break;
    }
}

void wl_audio_ctr_cb(uint8_t con_type, uint8_t enable)
{
    switch(con_type&0xff) {
        case CON_VERS3:
            break;
        case CON_VERS4:
            break;
        case CON_VERS6:
        case CON_VERS7:
            ble_t2r_v6_link_all_ctr(enable);
            break;
        case CON_VERS10:
            break;
    }
}

void wl_con_change_high_prio(uint8_t index)
{
}

void wl_con_audio_stop(uint8_t index)
{
}

#if !WIRELESS_MIC_BROADCAST_EN
void wl_audio_pwr_ctr_set(uint8_t con_type, uint8_t idx, uint8_t pwr_level)
{
    printf("wl_audio_pwr_ctr_set, %d, %d, %d\n", con_type, idx, pwr_level);

    switch(con_type&0xff) {
        case CON_VERS3:
            break;
        case CON_VERS4:
            break;
        case CON_VERS6:
        case CON_VERS7:
            ble_t2r_v6_pwr_ctr(idx, pwr_level);
            break;
        case CON_VERS10:
            ble_t2r_v10_pwr_ctr(idx, pwr_level);
            break;
    }
}
#endif

void wl_con_send_audio_setup(uint8_t con_type, uint8_t index, uint8_t test_rx)
{
    switch(con_type&0xff) {
        case CON_VERS3:
            break;
        case CON_VERS4:
            ll_tis_req(index, test_rx);
            break;
        case CON_VERS6:
        case CON_VERS7:
            break;
        case CON_VERS10:
            ble_t2r_v10_send_audio_setup(index, test_rx);
            break;
    }
}
///v4 only
AT(.com_text.ble.isr.con)
void ble_con_calc_anchor_ts_diff_cb(uint8_t link)
{
    ble_t2r_v4_calc_anchor_ts_diff(link);
}

bool ble_tis_req_param_get(uint16_t *sw, void *map, uint16_t *offset)
{
    return ble_t2r_v4_param_get(sw, map, offset);
}

AT(.text.ble.chmap)
void ble_tis_ch_map_update(uint8_t link, void *map, uint16_t instant)
{
    ble_t2r_v4_ch_map_update(link, map, instant);
}

bool ble_tis_evt_cnt_is_free(void)
{
    return ble_t2r_v4_evt_cnt_is_free();
}

bool ble_tis_evt_cnt_alloc(uint8_t link, uint16_t instant, uint16_t *evt_cnt_ret)
{
    return ble_t2r_v4_evt_cnt_alloc(link, instant, evt_cnt_ret);
}

void ble_tis_evt_cnt_free(uint8_t link)
{
    ble_t2r_v4_evt_cnt_free(link);
}
#endif


#if (WIRELESS_CON_VERS != 2)
void ble_con_end_cb_v3(uint8_t link) {}
void ble_t2r_v3_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt) {}
void ble_t2r_v3_init(uint8_t init_type) {}
uint8_t ble_t2r_v3_start(uint16_t con_handle, uint8_t test_rx, uint8_t second_con_vers_en, uint16_t second_con_offset){return 0;}
uint8_t ble_t2r_v3_stop(uint16_t con_handle) {return 0;}
uint16_t ble_t2r_v3_link_duration_get(void) {return 0;}
void ble_t2r_v3_skip_first_set_enable(uint8_t index, bool enable) {}
#endif

#if (WIRELESS_CON_VERS != 3)
void ble_con_end_cb_v4(uint8_t link) {}
void ble_t2r_v4_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt) {}
void ble_t2r_v4_calc_anchor_ts_diff(uint8_t link) {}
void ble_t2r_v4_init(uint8_t init_type) {}
uint8_t ble_t2r_v4_start(uint16_t con_handle, uint8_t test_rx, void *param) {return 0;}
uint8_t ble_t2r_v4_stop(uint16_t con_handle) {return 0;}
uint16_t ble_t2r_v4_link_duration_get(void) {return 0;}
void ll_tis_req(uint8_t index, uint8_t test_rx) {}
void mgr_ch_class_set_bad(void* ch_map) {}
void mgr_ch_class_update(void) {}
int ll_op_tis_req_ind_func(uint16_t msg, void *param, uint16_t par0, uint16_t par1) {return 0;}
void ll_tis_req_ind_func(uint8_t link, void *pdu, uint16_t evt_cnt) {}
#endif

#if ((WIRELESS_CON_VERS != 5) && (WIRELESS_CON_VERS != 6))
void ble_con_end_cb_v6(uint8_t link) {}
void ble_t2r_v6_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt) {}
void ble_t2r_v6_init(uint8_t init_type) {}
uint8_t ble_t2r_v6_start(uint16_t con_handle, bool test_rx, uint8_t second_con_vers_en, uint16_t second_con_offset) {return 0;}
uint8_t ble_t2r_v6_stop(uint16_t con_handle) {return 0;}
uint16_t ble_t2r_v6_link_duration_get(void) {return 0;}
void ble_t2r_v6_link_all_ctr(uint8_t enable) {}
void ble_t2r_v6_pwr_ctr(uint8_t index, uint8_t pwr_level) {}
AT(.com_text.ble.isr.con)
void ble_t2r_v6_hop_accel_isr(void) {}
AT(.com_text.ble.isr.con)
void ble_t2r_v6_ch_map_update(uint8_t id, void *p_map) {}
AT(.com_text.ble.isr.con)
bool ble_t2r_v6_need_hold(uint8_t id) {return false;}
#endif

#if (WIRELESS_CON_VERS != 9)
void ble_con_end_cb_v10(uint8_t link) {}
void *ble_t2r_v10_info_get(void) {return NULL;}
void ble_t2r_v10_info_set(void *param1, uint8_t param2) {}
bool ble_t2r_v10_chanel_map_compare(void *param1) {return 0;}
uint16_t ble_t2r_v10_next_instant_get(void) {return 0;}
void ble_t2r_v10_chanel_map_upd_req(void *param1, uint16_t param2, uint8_t param3) {}
uint16_t ble_t2r_v10_next_evt_cnt_get(void) {return 0;}
bool ble_t2r_v10_is_established(void) {return 0;}
void ble_t2r_v10_anchor_ts_update(uint8_t link, uint32_t anchor_ts, uint16_t anchor_hus, uint evt_cnt) {}
void ble_t2r_v10_init(uint8_t init_type) {}
uint8_t ble_t2r_v10_start(uint16_t con_handle, uint8_t test_rx) {return 0;}
uint8_t ble_t2r_v10_stop(uint16_t con_handle) {return 0;}
uint16_t ble_t2r_v10_link_duration_get(void) {return 0;}
void ble_t2r_v10_pwr_ctr(uint8_t index, uint8_t pwr_level) {}
void ble_t2r_v10_skip_first_set_enable(uint8_t index, bool enable) {}
void ble_t2r_v10_send_audio_setup(uint8_t index, uint8_t test_rx) {}
#endif

#if !WIRELESS_SPK_EN
AT(.com_text.ble.isr.con)
void ble_tick1_alloc(void (*tick_isr_cb)(uint)) {}
AT(.com_text.ble.isr.con)
void ble_tick1_set(uint tmr_hus) {}
AT(.com_text.ble.isr.con)
void ble_tick1_free(void) {}
AT(.com_text.ble.isr.con)
void ble_a2d_rx_tick_alloc(uint8_t index, bool con_sta) {}
AT(.com_text.ble.isr.con)
void ble_a2d_rx_tick_free(uint8_t index) {}
AT(.com_text.ble.isr.con)
void ble_a2d_rx_tick_set_sta(uint8_t index, bool con_sta) {}
#endif

#if !(WIRELESS_MIC_BROADCAST_EN)
void wl_per_adv_init(void) {}
uint8_t wl_per_adv_start(const void *params) {return 0;}
void wl_per_adv_stop(void) {}
bool wl_per_adv_end_ind(void *param) {return false;}
void wl_per_scan_params_set(const void *params) {}
void wl_per_scan_adv_rep_ind(const void *ind) {}
AT(.com_text.ble.isr.per_scan)
bool wl_per_scan_rx_ind(uint8_t id, uint8_t idx) {return false;}
bool wl_per_scan_end_ind(uint8_t id) {return false;}
void wl_per_scan_init(void) {}
void wl_per_scan_init_hook(void *params, uint8_t act_id) {}

void ble_chan_scan_anchor_ts_update(uint32_t anchor_ts, uint16_t anchor_hus) {};
#endif

#if WIRELESS_MIC_BROADCAST_EN
uint16_t wl_single_link_duration_get(uint8_t con_type) {return 0;}
void wl_audio_pwr_ctr_set(uint8_t con_type, uint8_t idx, uint8_t pwr_level) {}
#else

void wl_bc_adv_init(void) {}
void wl_bc_adv_evt_start(void *ev) {}
void wl_bc_adv_ctrl(uint8_t enable) {}
void wl_bc_scan_init(void) {}
bool wl_bc_scan_rx_ind(uint8_t aid, uint8_t rxidx, uint16_t rxptr, uint8_t rxlen) {return false;}
void wl_bc_scan_adv_rep_cb(const void *ind) {}
void wl_bc_scan_ctrl(bool enable) {}
void wl_bc_scan_set_param(uint intv, uint window) {}
void wl_bc_sync_ctrl(uint8_t enable) {}
int wl_bc_ctl_disc_ind_func(uint16_t mid, void *param, uint16_t did, uint16_t sid) {return false;}
int wl_bc_ctl_rx_user_data_ind_func(uint16_t mid, void *param, uint16_t did, uint16_t sid) {return false;}
int wl_bc_ctl_tx_req_func(uint16_t mid, void *param, uint16_t did, uint16_t sid) {return false;}
int wl_bc_ctl_tx_cfm_func(uint16_t mid, void *param, uint16_t did, uint16_t sid) {return false;}
int wl_bc_ctl_chmap_tx_ind_func(uint16_t mid, void *param, uint16_t did, uint16_t sid) {return false;}
int wl_bc_ctl_chmap_rx_ind_func(uint16_t mid, void *param, uint16_t did, uint16_t sid) {return false;}
void wl_bc_chmap_update(void){};
void wl_bc_chmap_tx_req(uint8_t index, uint8_t *map) {}
const uint8_t *ble_t2r_broadcast_channel_map_get(void) {return NULL;}
#endif
