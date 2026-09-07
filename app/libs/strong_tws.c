/**********************************************************************
*
*   strong_tws.c
*   定义库里面tws部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

void strong_tws_include(void) {}
#if !BT_TWS_EN
void btstack_tws_init(void){}
void a2dp_play_init(void){}
AT(.com_text.weak.sbc.play)
void a2dp_play_reset(bool force){}
AT(.com_text.weak.sbc.play)
void a2dp_play_reset_do(void){}

AT(.btdec.tws.a2dp_play)
bool tws_get_delta_ticks(int32_t *delta_ticks, uint32_t ticks) { return false; }

AT(.com_text.weak.sbc_cache)
void avdtp_fill_tws_chk_close(void) {}

AT(.com_text.weak.sbc_cache)
bool a2dp_cache_fill(uint8_t *packet, uint16_t size) {
    return true;
}

AT(.com_text.weak.sbc_cache)
uint8_t avdtp_fill_tws_buffer(u8 *ptr, uint len) {
    return 0;
}

AT(.com_text.weak.sbc_cache)
uint8_t a2dp_cache_before_rx(uint8_t *data_ptr, uint16_t data_len) {
    return 0;
}

AT(.sbcdec.code)
void sbc_cache_free(void) {
}

AT(.sbcdec.code)
size_t sbc_cache_read(uint8_t **buf) {
    return 0;
}

AT(.com_text.weak.bb.tws)
void bt_tws_ticks_isr(void) {
}

AT(.com_text.weak.bb.btisr)
void bt_tws_ticks_instant(void) {
}

AT(.com_text.weak.bb.btisr)
uint8_t tws_get_ticks_status(void) {
    return 0x80;
}

uint8_t bt_tws_set_spr(uint8_t index, uint spridx, uint32_t instant) {
    return 0;
}

AT(.com_text.weak.sbc.play)
void tws_ticks_trigger(uint32_t ticks) {
}

AT(.com_text.weak.sbc_play)
uint32_t tws_get_play_ticks(uint16_t seq_num, uint32_t duration) {
    return 0;
}

AT(.com_text.weak.sbc_play)
void tws_trigger_isr(void) {
}

AT(.com_text.weak.sbc_play)
bool tws_cache_is_empty(void) {
    return false;
}

AT(.com_text.weak.sbc_play)
void sbc_cache_env_reset(void)
{
}

AT(.com_text.weak.sbc_play)
void tws_send_pkt(void)
{
}

AT(.com_text.weak.tws.a2dp_play)
bool tws_send_is_pend(void) {
    return false;
}

AT(.com_text.weak.bb.btisr)
void bt_tws_ticks_next_instant(void) {}

AT(.com_text.weak.lc.tws)
uint8_t tws_get_acl_status(uint8_t lid)
{
    return 0;
}
AT(.com_text.weak.lc.tws)
uint16_t tws_get_snoop_status(uint8_t index) {
    return 0;
}
AT(.com_text.weak.sbc.play)
void a2dp_set_play_ticks(uint32_t e_ticks, uint16_t seqn) {}
AT(.com_text.weak.sbc.play)
bool a2dp_is_rx_stop(uint8_t index) { return false; }
AT(.com_text.weak.a2dp.lost)
uint32_t a2dp_lost_get_cache(void) { return 0; }
AT(.com_text.weak.sbc)
void tws_ticks_set(void) {}

AT(.com_text.weak.bb.tws_switch)
bool bt_tws_need_switch(bool calling) { return false; }
AT(.com_text.weak.bb.bttx)
bool tws_evs_rx_remote(u8 *buf) { return false; }
AT(.com_text.weak.bb.bttx)
void tws_seqn_update_local(u32 e_ticks, uint16_t seqn, bool first) {}
AT(.com_text.weak.sbc.play)
uint8_t avdtp_check_play_reset(void) { return 0; }
AT(.com_text.weak.sbc.play)
size_t a2dp_play_read_do(uint8_t **buf) { return 0; }
AT(.com_text.weak.sbc.play)
void a2dp_play_free_do(void) {}
AT(.com_text.weak.sbc.play)
void a2dp_fix_slv_ticks(u32 m_ticks, u16 m_seqn) {}
AT(.com_text.weak.sbc.play)
bool a2dp_fix_slv_ticks_cb(void) { return false; }
AT(.com_text.weak.sbc.play)
bool tws_is_first_pkt(void) { return false; }
AT(.com_text.weak.sbc.play)
bool tws_next_pkt_is_exist(void) { return true; }
uint16_t tws_set_snoop_ticks(uint8_t *data_ptr, uint16_t size) { return false; }
AT(.com_text.weak.tws)
void tws_evs_send_proc(void) {}
AT(.com_text.weak.tws)
bool tws_evs_check_tx_ptr(uint16_t buf_ptr) { return false; }
//AT(.com_text.weak.tws)
//uint32_t tws_time_get(tws_time_t *time, uint32_t inst) { return 0; }
//AT(.com_text.weak.tws)
//uint32_t tws_time_inc(void) { return 0;}
//AT(.com_text.weak.tws)
//void tws_time_fix_proc(void) {}
AT(.com_text.weak.tws)
void tws_time_calc_proc(void) {}
AT(.com_text.weak.tws)
void tws_time_calc_proc_cb(void) {}
AT(.com_text.weak.tws)
void tws_time_proc_cb(u32 tickn) {}
AT(.com_text.weak.tws)
void tws_alarm_proc(uint32_t tickn) {}
AT(.com_text.weak.tws)
void tws_time_alarm_cb(uint32_t params, uint32_t alarm_tickn) {}
void tws_local_alarm_req_cb(uint32_t tickn, uint16_t param) {}
void tws_remote_alarm_req_cb(uint32_t tick, uint16_t param) {}
void tws_local_res_add_cb(uint16_t param) {}
void tws_remote_res_add_cb(uint16_t param) {}
void tws_remote_res_done_cb(uint16_t param) {}
void tws_slave_res_add_req_cb(uint16_t param) {}
void btstack_tws_api(uint param) {}
void bt_wait_tws_switch_end(void) {}
bool bt_tws_is_en(void) {return 0;}
void bt_tws_set_scan(uint8_t scan_en) {}
bool btstack_tws_key(uint8_t key, uint16_t opcode) {return 0;}
uint16_t btstack_tws_get_nego_feat(void) {return 0;}
uint8_t btstack_tws_get_save_feature(uint8_t *bd_addr) {return 0;}
bool btstack_tws_is_connected_for_handle(uint16_t handle) {return 0;}
void btstack_tws_enter_sniff(bool tws_flag, bool tws_role) {}
uint8_t btstack_tws_get_force_feature(void) {return 0;}
bool btstack_set_tws_info(uint8_t *param) {return 0;}
void btstack_tws_get_guid(uint8_t *buf, uint8_t buf_len) {}
bool btstack_get_tws_info(uint8_t *param, uint8_t tws_info) {return 0;}
bool btstack_tws_cmp_guid(uint8_t *buf, uint8_t buf_len) {return 0;}
void btstack_tws_remote_set_codec(uint8_t *param) {}
void btstack_tws_disconnect_do(uint16_t timeout) {}
void btstack_tws_disc_continue(void) {}
uint8_t btstack_get_tws_key(uint msg) {return 0;}
uint btstack_get_tws_msg(uint8_t key) {return 0;}
bool tws_nor_connect_req(uint8_t times, uint32_t connect_flag) {return 0;}
bool btstack_tws_verify_connect(uint8_t *bd_addr) {return 0;}
void btstack_tws_get_tws_name(uint8_t *tws_name) {}
void tws_pair_id_init(void) {}
bool tws_l2cap_set_send_flag(u32 flag) {return 0;}
bool tws_l2cap_msg_send_req(u32 flag) {return 0;}
bool tws_l2cap_msg_mst_set_send_flag(u32 flag) {return 0;}
bool tws_l2cap_msg_mst_send_req(u32 flag) {return 0;}
void tws_establish_service_level_connection(u8 address[]) {}
bool tws_acl_tx_cfm_cb(uint8_t opcode) {return 0;}
bool tws_loc_snoop_is_allow(uint8_t index) {return false;}
void tws_con_init(void) {}
void tws_switch_restort_pend_cb(uint16_t con_handle, uint8_t num_pkts_sent) {}
uint16_t tws_get_snoop_ticks_cb(uint8_t *data_ptr) {return 0;}
uint16_t tws_set_snoop_ticks_cb(uint8_t *data_ptr, uint8_t size) {return 0;}
bool tws_prepare_snoop_service(uint8_t index) {return 0;}
void tws_send_snoop_service(void) {}
void tws_update_local_addr(uint8_t *address) {}
bool tws_loc_switch_is_allow(void) {return 0;}
AT(.com_text.weak.tws)
bool tws_rem_switch_is_allow(void) {return 0;}
void tws_set_new_avdtp_cid(uint16_t avdtp_cid) {}
void tws_remote_set_discon(uint8_t reason) {}
uint8_t tws_remote_vs_msg_cb(uint8_t opcode, uint8_t *pdu) {return 0;}
void tws_remote_snoop_setup_cb(void) {}
void tws_remote_snoop_lost_cb(void) {}
void tws_remote_ticks_fail_cb(void) {}
void tws_send_set_volume(uint8_t *bd_addr, uint8_t index, uint8_t a2dp_vol) {}
void tws_send_a2dp_feat_record(uint8_t *bd_addr, uint8_t a2dp_feat) {}
bool tws_send_set_key(uint8_t key, uint16_t opcode) { return false; }
void tws_send_set_status(uint8_t status) {}
void tws_send_set_disc_reason(uint8_t reason) {}
void tws_send_set_profile_status(void) {}
void tws_send_set_call_indicate(uint8_t *c_sta, uint8_t siri_sta) {}
bool tws_send_set_latency(uint8_t latency) {return 0;}
void tws_send_set_aap_info(void) {}
void tws_send_set_fota_rsp(void) {}
void tws_send_avdtp_cid(u16 l2cap_cid) {}
void tws_send_avdtp_start_discard(u16 discard_cnt) {}
void tws_set_discon_tx_cfm_cb(void) {}
bool tws_vs_msg_pack_cb(const uint8_t *param) {return 0;}
void tws_vs_msg_free_cb(const uint8_t *param, bool send_msg) {}
void tws_vs_msg_tx_cfm_cb(uint8_t opcode) {}
void tws_send_set_vol_ctrl_status(uint8_t vol_ctrl) {}
void tws_send_ios_flag(uint8_t ios_flag) {}
bool tws_get_snoop_info_bb(uint16_t *acl_conhdl, uint16_t *sco_conhdl, uint8_t *bdaddr) {return 0;}
uint16_t bt_get_snoop_link(uint8_t *data_ptr) {return 0;}
void tws_remote_snoop_lost(void) {}
void tws_remote_snoop_setup(void) {}
void tws_remote_set_codec(uint8_t *param) {}
void tws_remote_ticks_fail(void) {}
void tws_remote_set_speed(uint8_t *param) {}
void tws_remote_snoop_sta(uint8_t status) {}
bool tws_sco_remote_setup(uint8_t *pdu) {return false;}
void tws_sco_remote_kill(uint8_t *pdu) {}
void tws_vs_msg_flush(void) {}
void tws_vs_msg_free(void const *msg_ptr, bool send_msg) {}
bool tws_vs_msg_pack(void const *msg_ptr) {return 0;}
void *tws_vs_msg_alloc(uint8_t opcode) {return NULL;}
void tws_vs_msg_send(uint8_t *pdu) {}
void tws_send_set_media_codec(uint16_t media_lcid, void *media_codec, bool scms_en) {}
void tws_send_set_speed(uint8_t speed) {}
void tws_send_set_snoop_sta(uint8_t status) {}
void tws_send_speed_miss(void) {}
void tws_send_set_disconnect(uint8_t reason) {}
void tws_send_switch_req(void) {}
void tws_msg_tx_cfm(uint8_t opcode) {}
void *tws_vs_msg_peek_do(void) {return NULL;}
AT(.com_text.weak.tws)
uint8_t tws_acl_set_tx_ptr(uint16_t buf_ptr) {return 0;}
AT(.com_text.weak.tws)
void tws_acl_free_tx_ptr(uint8_t i) {}
AT(.com_text.weak.tws)
bool tws_acl_check_tx_ptr(uint16_t buf_ptr) {return 0;}
AT(.com_text.weak.tws)
bool tws_evs_get_snoop_offset_do(uint8_t index_s, uint8_t index_t, uint16_t *bitoff, uint32_t *clkoff) {return 0;}
AT(.com_text.weak.tws)
void tws_evs_set_snoop_offset_do(uint8_t index_s, uint8_t index_t, uint32_t clkoff_s, uint16_t bitoff_s) {}
AT(.com_text.weak.tws)
uint8_t tws_acl_tx_lost_pkt(uint16_t rcid, uint8_t *packet, uint8_t flag, uint16_t data_len) {return 0;}
AT(.com_text.weak.tws)
bool tws_acl_tx_is_vaild(void) {return 0;}
void tws_ctl_con_cmp(uint8_t index) {}
void tws_ctl_con_end(void) {}
void tws_ctl_init(bool reset) {}
void tws_set_local_ticks(uint16_t con_handle, uint8_t spr_idx) {}
void tws_free_snoop_link(void) {}
void tws_kill_snoop_link(uint8_t index) {}
void tws_enter_sniff(uint16_t pid) {}
AT(.com_text.weak.tws)
void tws_evs_send_last_seqn_req(void) {}
AT(.com_text.weak.tws)
void tws_evs_send_first_seqn_req(uint32_t e_ticks, uint16_t seqn) {}
AT(.com_text.weak.tws)
void tws_evs_send_sco_req(void) {}
AT(.com_text.weak.tws)
bool tws_evs_send_avdtp_close_req(uint32_t ticks_ms, uint16_t avdtp_cid) {return 0;}
AT(.com_text.weak.tws)
void tws_seqn_clr_local(void) {}
AT(.com_text.weak.tws)
void tws_set_rx_ack2(uint8_t id) {}
void tws_clr_txrx_ack2(void) {}
AT(.com_text.weak.tws)
bool tws_acl_rx_ack2tx(uint16_t stat, uint16_t rh) {return 0;}
AT(.com_text.weak.tws)
void tws_acl_rx_ack2rx(uint16_t rstat, uint16_t rh, uint16_t tstat) {}
AT(.com_text.weak.tws)
void tws_acl_rx_media_pkt(uint8_t *ptr) {}
AT(.com_text.weak.tws)
bool tws_sco_is_active(void) {return 0;}
bool tws_sco_is_slave(void) {return 0;}
void tws_sco_suspend_ack2(uint8_t id) {}
void tws_sco_resume_ack2(uint8_t id) {}
void tws_sco_start_link(uint16_t id, uint8_t sid, uint8_t *ptr) {}
void tws_sco_stop_link(uint16_t id, uint8_t sid) {}
void tws_sco_kill_link(uint8_t id) {}
void tws_sco_buf_init(void) {}
AT(.com_text.weak.tws)
void tws_sco_buf_add(void *pkt_buf, bool pkt_stat, uint pkt_len, uint pkt_frame) {}
AT(.com_text.weak.tws)
bool tws_sco_rx_lost_pkt(uint8_t *ptr, uint16_t len) {return 0;}
AT(.com_text.weak.tws)
void tws_sco_tx_lost_pkt(void) {}
AT(.com_text.weak.tws)
uint8_t tws_vs_switch_is_active(void) {return 0;}
void tws_vs_switch_kill(uint8_t id) {}
void tws_vs_switch_finish(uint8_t id) {}
AT(.com_text.weak.tws)
bool tws_switch_acl_is_pause(uint32_t clock, uint8_t id) {return 0;}
AT(.com_text.weak.tws)
void tws_switch_restore_aclseqn(void) {}
AT(.com_text.weak.tws)
void tws_switch_check_inst(uint8_t *ptr, uint8_t tid, uint32_t clkn) {}
void tws_set_switch_inst(uint32_t instant) {}
bool tws_switch_save_pend(uint8_t type, uint8_t flag, uint8_t *buf, uint16_t len) {return 0;}
bool tws_switch_save_acl_rx(uint8_t *ptr) {return 0;}
void tws_switch_save_acl_tx(uint8_t id) {}
void lc_flush_lmp_tx_pend(uint8_t id) {}
void nanos_flush_pend(uint16_t did) {}
AT(.com_text.weak.ble.tws)
bool tws_scan0_rx_ind(uint8_t sid, uint8_t rid) {return 0;}
bool tws_scan0_end_ind(uint8_t aid) {return 0;}
void tws_page_init(uint8_t init_type) {}
AT(.com_text.weak.ble.tws)
uint8_t tws_page_start(uint8_t index, uint8_t *ptr, uint8_t feature) {return 0;}
AT(.com_text.weak.ble.tws)
uint8_t tws_page_stop(void) {return 0;}
void tws_page_con_init(uint8_t index, void **slave_timing) {}
AT(.com_text.weak.ble.tws)
void tws_page_con_end(uint8_t index) {}
AT(.com_text.weak.ble.tws)
uint8_t ble_inq_start(void) {return 0;}
AT(.com_text.weak.tws)
uint8_t ble_inq_stop(void) {return 0;}
void tws_pscan_init(uint8_t init_type) {}
AT(.com_text.weak.tws)
bool tws_pscan_rx_ind(uint8_t aid, uint8_t rid, uint *ret) {return 0;}
void tws_pscan_set_interval(uint32_t interval) {}
bool tws_pscan_end_ind(uint8_t aid) {return 0;}
void ble_con_adv_restore(void) {}
void tws_pscan_con_end(uint8_t index) {}
void tws_pscan_con_init(uint8_t index, uint8_t lt_addr) {}
void tws_w_scan_ctrl(uint8_t scan_en) {}
#endif

//----------------------------------------------------------------------------------------------------
typedef int (*lc_hci_cmd_hdl_func_t)(void const *param, uint16_t const dest_id, uint16_t opcode);
typedef int (*lc_lmp_msg_hdl_func_t)(void const *param, uint16_t const dest_id);
typedef int (*nanos_msg_func_t)(uint16_t const msgid, void const *param, uint16_t const dest_id, uint16_t const src_id);

struct lc_lmp_msg_handler
{
    uint8_t opcode;
    lc_lmp_msg_hdl_func_t func;
};

struct lc_hci_cmd_item
{
    uint16_t opcode;
    lc_hci_cmd_hdl_func_t func;
};

typedef struct nanos_task_desc
{
    u8 *tab;
    u8 *state;
    uint16_t idx_max;
    uint16_t msg_cnt;
} nanos_task_desc_t;

struct nanos_msg_handler
{
    uint16_t id;
    nanos_msg_func_t func;
};

#if !BT_HFP_EN
const struct lc_lmp_msg_handler lc_lmp_func_tab[1] = {0};
const struct lc_lmp_msg_handler lc_lmp_ext_func_tab[1] = {0};
const struct lc_hci_cmd_item lc_hci_cmd_tbl[1] = {0};
const struct nanos_msg_handler lc_msg_tbl[1] = {0};

void lc_sco_init(void){}
void lc_sco_reset(void){}
uint8_t lc_sco_detach(uint16_t pid, uint8_t reason){ return 0;}
void lc_sco_release(uint16_t pid, uint8_t reason){}
void lc_sco_free_link(uint16_t pid, uint8_t sync_linkid){}
uint8_t lc_sco_start_snoop(uint8_t lid, void *params, uint8_t *sco_lid){ return 0;}
uint8_t lc_sco_host_request(uint16_t pid, bool old_style, bool host_initiated, uint16_t conhdl, void * p_host_params){ return 0;}
uint8_t lc_sco_host_accept(uint16_t pid, bool old_style, bool very_old_style, void * p_host_params){ return 0;}
void lc_sco_host_reject(uint16_t pid, bool very_old_style, uint8_t status){}
uint8_t lc_sco_host_request_disc(uint16_t pid, uint16_t conhdl, uint8_t reason){ return 0;}
void lc_sco_peer_request(uint16_t pid, uint8_t req_type, void * p_peer_params){}
void lc_sco_peer_accept(uint16_t pid){}
void lc_sco_peer_reject(uint16_t pid, uint8_t reason){}
void lc_sco_peer_request_disc(uint16_t pid, uint8_t req_type, uint8_t scohdl, uint8_t reason){}
void lc_sco_peer_accept_disc(uint16_t pid){}
void lc_sco_peer_reject_disc(uint16_t pid, uint8_t reason){}
void lc_sco_baseband_ack(uint16_t pid, uint8_t lmp_opcode){}
void lc_sco_timeout(uint16_t pid){}
void lc_sco_offset_update(uint8_t sync_linkid, uint16_t sco_offset){}
bool lc_sco_is_connect(void){ return false;}
bool lc_sco_is_connect_for_index(uint8_t link_id){ return false;}
int8_t lc_sco_get_air_param_wesco(void){ return 0;}

void lm_init_sync(void){}
void lm_reset_sync(void){}
bool lm_check_active_sync(void){ return false;}
uint8_t lm_get_synchdl(uint8_t sco_link_id) {return 0;}
bool lm_look_for_sync(uint8_t link_id, uint8_t type){ return false;}
uint8_t lm_get_nb_sync_link(void){ return 0;}
uint8_t lm_get_min_sync_intv(void){ return 0;}

bool lc_sco_codec_setup(void){ return false;}
void lc_sco_codec_kill(void){}
void lc_sco_codec_pkt(uint16_t sco_hdl, uint8_t *sco_tx_buf, uint8_t *sco_rx_buf, uint16_t sco_len, bool pkt_stat, uint8_t cur_frame){}
void bt_sco_rx_process(void){}
AT(.com_text.weak.bb_sco)
void bt_codec_isr(void){}
#else
const struct lc_lmp_msg_handler lc_lmp_dut_func_tab[1] = {0};
const struct lc_lmp_msg_handler lc_lmp_ext_dut_func_tab[1] = {0};
const struct lc_hci_cmd_item lc_dut_hci_cmd_tbl[1] = {0};
const struct nanos_msg_handler lc_dut_msg_tbl[1] = {0};
#endif
