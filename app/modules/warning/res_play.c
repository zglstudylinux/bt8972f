#include "include.h"
#include "res_play.h"


#if WARNING_TONE_EN


#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

#define TRACE_ERR(...)          printf(__VA_ARGS__)
#define TRACE_INFO(...)         printf(__VA_ARGS__)


//add res_list
//send alarm
//wait alarm
//audio bypass
//res init
//wait kick
//loop, wait end
//res exit
//if no more res, goto audio enable
//wait done
//send next alarm
//wait next alarm
//goto res init
//audio enable
//finish


static uint8_t res_lable_cnt = 0;
res_list_t res_list;
res_play_t res_play;



bool mp3_res_play_init(u32 addr, u32 len, bool kick_start);
void wav_sync_kick(void);
bool wav_res_play_init(u32 addr, u32 len, bool kick_start);
void wav_res_dec_process(void);
bool wav_res_is_play(void);
bool wav_res_stop(void);
void mp3_res_play_exit(void);

void res_play_before_cb(uint8_t res_idx);
void res_play_after_cb(uint8_t res_idx);


//----------------------------------------------------------------------------
//将res_idx加入到res队列尾部
ALIGNED(128) NO_INLINE
static bool res_list_add(uint8_t res_idx, uint8_t res_lable, uint8_t res_flag)
{
    GLOBAL_INT_DISABLE();
    if(res_list.len < res_list.total) {
        if(res_list.wpos >= res_list.total) {
            res_list.wpos = 0;
        }
        res_list.item[res_list.wpos].res_idx = res_idx;
        res_list.item[res_list.wpos].res_lable = res_lable;
        res_list.item[res_list.wpos].res_flag = res_flag;
        res_list.wpos++;
        res_list.len++;
        GLOBAL_INT_RESTORE();
        return true;
    }
    GLOBAL_INT_RESTORE();

    return false;
}

//从res队列头部删除item
ALIGNED(128) NO_INLINE
static bool res_list_delete(res_item_t *item)
{
    bool ret = false;

    GLOBAL_INT_DISABLE();
    if(res_list.len > 0) {
        if(res_list.rpos >= res_list.total) {
            res_list.rpos = 0;
        }

        if(memcmp(&res_list.item[res_list.rpos], item, sizeof(res_item_t)) == 0) {
            res_list.len--;
            res_list.rpos++;

            ret = true;
        }
    }
    GLOBAL_INT_RESTORE();

    return ret;
}

//观察res队列头部的item
ALIGNED(128) NO_INLINE
static bool res_list_peek(res_item_t *item)
{
    bool ret = false;

    GLOBAL_INT_DISABLE();
    if(res_list.len > 0) {
        if(res_list.rpos >= res_list.total) {
            res_list.rpos = 0;
        }
        *item = res_list.item[res_list.rpos];
        ret = true;
    }
    GLOBAL_INT_RESTORE();

    return ret;
}

//从res队列取出一个item
ALIGNED(128) NO_INLINE
static bool res_list_get(res_item_t *item)
{
    bool ret = false;

    GLOBAL_INT_DISABLE();
    if(res_list.len > 0) {
        if(res_list.rpos >= res_list.total) {
            res_list.rpos = 0;
        }
        if(item != NULL) {
            *item = res_list.item[res_list.rpos];
        }

        res_list.len--;
        res_list.rpos++;

        ret = true;
    }
    GLOBAL_INT_RESTORE();

    return ret;
}

//从res队列查找res_idx是否存在
ALIGNED(128) NO_INLINE
static bool res_list_search(uint8_t res_idx)
{
    bool ret = false;

    GLOBAL_INT_DISABLE();
    uint8_t rpos = res_list.rpos;
    uint8_t len = res_list.len;

    while(len--) {
        if(rpos >= res_list.total) {
            rpos = 0;
        }
        if(res_list.item[rpos].res_idx == res_idx) {
            ret = true;
            break;
        }
        rpos++;
    }
    GLOBAL_INT_RESTORE();

    return ret;
}

//----------------------------------------------------------------------------
void res_play_add_do(uint16_t param)
{
    uint8_t res_idx = (uint8_t)param;
    uint8_t res_lable = (uint8_t)(param>>8) & 0xf;
    uint8_t res_flag = (uint8_t)(param>>12) & 0xf;

    if (!res_play.enable) {
        return;
    }

    TRACE_INFO("res_add(%c): %d(%d)%x\n", bt_tws_is_slave()? 'S': 'M', res_idx, res_lable, res_flag);

    if(res_idx == RES_IDX_CALL_HANGUP) {
        res_play.break_ring |= BIT(0);
    } else if(res_idx == RES_IDX_CALL_REJECT) {
        res_play.break_ring |= BIT(1);
    }

    res_list_add(res_idx, res_lable, res_flag);
}

void res_play_done_do(uint16_t param)
{
    uint8_t res_lable = (uint8_t)(param>>8);

    if(res_play.res_lable == res_lable) {
        res_play.done_flag = true;
    }

#if TRACE_EN
    uint8_t res_idx = (uint8_t)param;
    TRACE("res_done: %d(%d--%d)\n", res_idx, res_lable, res_play.res_lable);
#endif
}

ALIGNED(128) NO_INLINE
static void res_play_alarm_start(res_item_t *item, uint8_t role)
{
    GLOBAL_INT_DISABLE();
    res_play.res_idx = item->res_idx;
    res_play.res_lable = item->res_lable;
    res_play.res_flag  = item->res_flag;
    res_play.kick_flag = false;
    res_play.done_flag = (role == 1)? true : false;
    res_play.play_kick_cb = NULL;
    GLOBAL_INT_RESTORE();

    res_play.play_err = 0;
    res_play.res_type = res_tbl_get_type(item->res_idx);

#if BT_TWS_EN
    if(role == 0 && bt_is_tws_mode()) {     //master send alarm_req to slave
        bt_tws_req_alarm_res((item->res_lable<<8) | item->res_idx);
    }
#endif
    TRACE("res_start: %d(%d),%x --role=%d\n", res_play.res_idx, res_play.res_lable, res_play.res_flag, role);
}

static void res_play_dec_prepare(void)
{
    uint8_t res_type = res_play.res_type;

    TRACE_INFO("res_item: %d, %d, %d\n", res_play.res_idx, res_play.res_lable, res_play.res_type);
    res_play.func_sta = func_cb.curr;
    if(res_type == RES_TYPE_MP3 || res_type == RES_TYPE_WSBC) {
        res_play.vol_busy = RES_VOL_ADJ;    //提示音bypass过程中，不允许调音量
        func_audio_bypass(res_play.func_sta, res_type);
    } else {
#if WARNING_SYSVOL_ADJ_EN
        u8 vol = (sys_cb.incall_flag != 0)? bsp_bt_get_hfp_vol(sys_cb.hfp_vol) : bsp_get_vol();

        res_play.vol_busy = RES_VOL_ADJ;    //提示音退避过程中，不允许调音量
        if(vol > SYS_BACKSTAGE_VOLUME) {
            bsp_change_volume(SYS_BACKSTAGE_VOLUME);
        }
#endif
    }

#if DAC_DNR_EN
    res_play.dnr_sta = dac_dnr_get_sta();
    dac_dnr_set_sta(0);
#endif
    res_play.dac_sta = dac_get_pwr_sta();
    func_bt_set_dac(1);

#if LOUDSPEAKER_MUTE_EN
    res_play.mute_bak = sys_cb.mute;
    if (sys_cb.mute) {
        bsp_loudspeaker_unmute();
    }
#endif
}

static void res_play_dec_init(void)
{
    uint8_t res_idx  = res_play.res_idx;
    uint8_t res_type = res_play.res_type;
    uint32_t addr, len;

    if(!bt_is_tws_mode() || !bt_is_enable() || !bt_tws_is_connected()) {
        res_play.kick_flag = true;                              //非tws模式，直接kick_start
    } else if (res_play.alarm_lable != res_play.res_lable) {    //暂时先规避lable不一致时，没有kick的问题
        res_play.kick_flag = true;
        TRACE_ERR("res_alarm: lable miss ###\n");
    }

    if (!res_tbl_get_addr(res_idx, &addr, &len)) {
        res_play.play_err |= PLAY_ERR_INVALID;
        return;
    }

    if((res_play.res_flag & RES_FLAG_LOCAL) == 0) {
        res_play.vol_busy = RES_VOL_MUTE;
    } else {
        if(WARNING_SYSVOL_ADJ_EN || res_type == RES_TYPE_MP3 || res_type == RES_TYPE_WSBC) {
            res_play.vol_busy = RES_VOL_ADJ;
        }
    }

    switch(res_type) {
#if WARNING_MP3_RES_EN
    case RES_TYPE_MP3:
        if(get_music_dec_sta() != MUSIC_STOP) {         //避免来电响铃/报号未完成，影响get_music_dec_sta()状态
            music_control(MUSIC_MSG_STOP);
        }
        if((res_play.res_flag & RES_FLAG_LOCAL) == 0) { //RES_FLAG_LOCAL属性为0，则不播出声音
            bsp_change_volume(0);
        } else {
            bsp_change_volume(WARNING_VOLUME);
        }
        mp3_res_play_init(addr, len, res_play.kick_flag);
        res_play.play_kick_cb = music_sync_kick;
        break;
#endif
#if WARNING_WSBC_RES_EN
    case RES_TYPE_WSBC:
        sys_clk_req(INDEX_RES_PLAY, SYS_160M);
        res_play.play_kick_cb = warning_play_kick;
        if(warning_play_init((u8 *)addr, len, res_play.kick_flag, res_play.res_flag) != 0) {
            res_play.play_err |= PLAY_ERR_FORMAT;
        }
        break;
#endif
#if WARNING_WAV_RES_EN
    case RES_TYPE_WAV:
        sys_clk_req(INDEX_RES_PLAY, SYS_120M);
        res_play.play_kick_cb = wav_sync_kick;
        if(!wav_res_play_init(addr, len, res_play.kick_flag)) {
            res_play.play_err |= PLAY_ERR_FORMAT;
        }
        break;
#endif
#if WARNING_PIANO_RES_EN
    case RES_TYPE_TONE:
    case RES_TYPE_PIANO:
        int type = (res_type == RES_TYPE_TONE)? WARNING_TONE : WARNING_PIANO;
        res_play.play_kick_cb = piano_sync_kick;
        if(piano_tone_init(type, (void *)addr, res_play.kick_flag, res_play.res_flag) != 0) {
            res_play.play_err |= PLAY_ERR_FORMAT;
        }
        break;
#endif
    }
}

static void res_play_dec_exit(void)
{
    uint8_t res_type = res_play.res_type;

    if(res_play.res_idx == RES_IDX_CALL_HANGUP) {
        res_play.break_ring &= ~BIT(0);
    } else if(res_play.res_idx == RES_IDX_CALL_REJECT) {
        res_play.break_ring &= ~BIT(1);
    }
    if(res_play.play_err & PLAY_ERR_INVALID) {
        return;
    }

    switch(res_type) {
#if WARNING_MP3_RES_EN
    case RES_TYPE_MP3:
        music_control(MUSIC_MSG_STOP);
        mp3_res_play_exit();
        bsp_change_volume(bsp_get_vol());
        break;
#endif
#if WARNING_WSBC_RES_EN
    case RES_TYPE_WSBC:
        warning_play_exit();
        sys_clk_free(INDEX_RES_PLAY);
        break;
#endif
#if WARNING_WAV_RES_EN
    case RES_TYPE_WAV:
        wav_res_stop();
        sys_clk_free(INDEX_RES_PLAY);
        break;
#endif
#if WARNING_PIANO_RES_EN
    case RES_TYPE_TONE:
    case RES_TYPE_PIANO:
        piano_tone_exit();
        break;
#endif
    }
}

static void res_play_dec_restore(void)
{
    uint8_t res_type = res_play.res_type;
    uint8_t vol;

#if LOUDSPEAKER_MUTE_EN
    sys_cb.mute = res_play.mute_bak;
    if (sys_cb.mute) {
        bsp_loudspeaker_mute();
    }
#endif
    func_bt_set_dac(res_play.dac_sta);
#if DAC_DNR_EN
    dac_dnr_set_sta(res_play.dnr_sta);
#endif

    //先恢复音量
    vol = (sys_cb.incall_flag != 0)? bsp_bt_get_hfp_vol(sys_cb.hfp_vol) : bsp_get_vol();
    bsp_change_volume(vol);

    //再恢复系统音乐
    res_play.vol_busy = false;
    if(res_type == RES_TYPE_MP3 || res_type == RES_TYPE_WSBC) {
        func_audio_restore(res_play.func_sta, res_type);
    }
}

static bool res_play_check_break(void)
{
#if WARNING_BREAK_EN
    if( res_play.break_ring && (res_play.res_idx <= RES_IDX_RING && res_play.res_idx >= RES_IDX_NUM_0) ) {
        res_play.play_err |= PLAY_ERR_BREAK;
        return true;
    }
#endif
#if BT_TWS_EN
    if(bt_tws_is_slave()) {
        if((res_play.res_flag&RES_FLAG_BREAK) != 0 && res_play.break_flag) {
            res_play.play_err |= PLAY_ERR_BREAK;
            return true;
        }
    }
#endif
    return false;
}

//return: true=dec finish, false=dec busy
static bool res_play_dec_proc(void)
{
    uint8_t res_type = res_play.res_type;

    if(res_play.play_err & PLAY_ERR_INVALID) {
        return true;
    }

    if(res_play_check_break()) {
        return true;
    }

    switch(res_type) {
#if WARNING_MP3_RES_EN
    case RES_TYPE_MP3:
        if(get_music_dec_sta() == MUSIC_STOP) {
            return true;
        }
        break;
#endif
#if WARNING_WSBC_RES_EN
    case RES_TYPE_WSBC:
        if(warning_play_proc() != 0) {
            return true;
        }
        break;
#endif
#if WARNING_WAV_RES_EN
    case RES_TYPE_WAV:
        if(!wav_res_is_play()) {
            return true;
        }
        wav_res_dec_process();
        break;
#endif
#if WARNING_PIANO_RES_EN
    case RES_TYPE_TONE:
    case RES_TYPE_PIANO:
	     if (!piano_tone_is_playing()) {
            return true;
	     }
	     break;
#endif
    default:
        return true;
    }

    return false;
}

#if BT_TWS_EN
//提示音开始播放处理，需要tws_time_proc_cb调用
AT(.com_text.tws.res)
void res_play_tws_kick(u32 tickn)
{
    if(res_play.play_tickn != -1L && res_play.alarm_lable == res_play.res_lable) {
        if(TICK_GEQ(tickn, res_play.play_tickn)) {
            res_play.play_tickn = -1L;
            res_play.kick_flag = true;
            if(res_play.play_kick_cb != NULL) {
                res_play.play_kick_cb();
            }
        }
    }
}
#endif

static bool res_play_list_is_empty(uint8_t res_type)
{
    res_item_t item;

    if(res_list_peek(&item)) {
        if(res_tbl_get_type(item.res_idx) == res_type) {
            return false;
        }
    }

    return true;
}

static bool res_play_is_busy(void)
{
    if(res_play.state != RES_STA_IDLE) {
        return (bool)(res_play.state <= RES_STA_PROC);
    }
    return false;
}

#if BT_TWS_EN
static bool res_play_slave_w4_start(void)
{
    res_item_t item;
    uint retry = ITEM_MAX_NB + 4;
    uint8_t req_lable = res_play.req_lable;
    uint8_t req_idx = res_play.req_idx;

    while(retry--) {
        if(res_play.rd_lable == req_lable) {
            break;
        }
        if(!res_list_peek(&item)) {
            break;
        }

        uint diff = req_lable - item.res_lable;
        if(diff == 0) {
            if(req_idx != item.res_idx) {
                res_list_delete(&item);         //discard
                TRACE_ERR("res_miss: %d(%d), %d(%d) ####\n", item.res_idx, item.res_lable, req_idx, req_lable);
            } else {
                res_play.rd_lable = req_lable;
                return true;
            }
        } else if(diff < ITEM_MAX_NB/2) {
            res_list_delete(&item);             //discard
            TRACE_ERR("res_miss: %d(%d), %d(%d) ####\n", item.res_idx, item.res_lable, req_idx, req_lable);
        }
    }

    return false;
}
#endif

void res_play_process_do(void)
{
    uint16_t delay = 200;
    uint8_t  role = 0;
    res_item_t item;

    if(!res_play.enable) {
        if(res_play.state >= RES_STA_DELAY && res_play.state <= RES_STA_NEXT_RES_INIT) {
            res_play.state = RES_STA_RES_EXIT;
        }
    }

    switch(res_play.state) {
    case RES_STA_IDLE:
        if(!res_play.enable) {
            break;
        }
#if BT_TWS_EN
        if(bt_tws_is_slave()) {
            if(!res_play_slave_w4_start()) {
                break;
            }
            role = 1;
        }
#endif
        //因为主耳会先取出来再发命令通知副耳播放，副耳时间长会稍后一些
        //在队列满时，为避免主耳取出后又立即插入新提示音，此处先peek同步后再delete
        if(res_list_peek(&item)) {
            res_play_alarm_start(&item, role);
            res_play.tickn = TICK_ADD(TICKN_GET(), 500);
            res_play.state = RES_STA_WAIT_ALARM;
        }
        break;

    case RES_STA_WAIT_ALARM:
        if( !bt_is_tws_mode() || !bt_is_enable()
           || res_play.alarm_lable == res_play.res_lable || TICKN_IS_EXPIRE(res_play.tickn) ) {
            res_play_dec_prepare();
            res_play.state = RES_STA_AUDIO_BYPASS;
        }
        break;

    case RES_STA_AUDIO_BYPASS:
        res_play_dec_init();
        res_play_before_cb(res_play.res_idx);
        res_play.state = RES_STA_WAIT_KICK;
        res_play.tickn = TICK_ADD(TICKN_GET(), 500);
        break;

    case RES_STA_WAIT_KICK:
        if(res_play.kick_flag || TICKN_IS_EXPIRE(res_play.tickn)) {
            res_play.state = RES_STA_PROC;

            //同步后才从队列中删除，此处不能用res_list_get
            //因为RES_STA_WAIT_KICK过程可能执行过res_play_cleanup
            item.res_idx = res_play.res_idx;
            item.res_lable = res_play.res_lable;
            item.res_flag = res_play.res_flag;
            res_list_delete(&item);
        }
        break;

    case RES_STA_PROC:
        if(res_play_dec_proc()) {
            res_play_dec_exit();
            res_play_after_cb(res_play.res_idx);
#if BT_TWS_EN
            if(bt_tws_is_slave()) {
                bt_tws_res_done(res_play.res_idx | (res_play.res_lable<<8));
            }
#endif
            res_play.state = RES_STA_RES_EXIT;
        }
        break;

    case RES_STA_RES_EXIT:
        if(!res_play.enable || res_play_list_is_empty(res_play.res_type)) {         //no more res
            res_play_dec_restore();
            res_play_set_break(false); //播空后清掉break_flag
            res_play.state = RES_STA_IDLE;
            break;
        }

#if BT_TWS_EN
        if(bt_tws_is_connected() && !bt_tws_is_slave()) {
            delay = 300;
        }
#endif
        res_play.tickn = TICK_ADD(TICKN_GET(), delay);
        res_play.state = RES_STA_DELAY;
        //no break;

    case RES_STA_DELAY:
        if(!bt_tws_is_connected()) {
            res_play.done_flag = true;
        }

        res_play_check_break();
        if(res_play.play_err != 0) {
            res_play.state = RES_STA_NEXT_RES_INIT;
            break;
        }
        if(TICKN_IS_EXPIRE(res_play.tickn)/* || res_play.done_flag*/) {
#if BT_TWS_EN
            if(!res_play.done_flag) {
                res_play.tickn = TICK_ADD(TICKN_GET(), 300);
                res_play.state = RES_STA_DELAY_MORE;
                break;
            }
#endif
            res_play.state = RES_STA_NEXT_RES_INIT;
        }
        break;

    case RES_STA_DELAY_MORE:
        res_play_check_break();
        if(res_play.done_flag || res_play.play_err || TICKN_IS_EXPIRE(res_play.tickn)) {
            res_play.state = RES_STA_NEXT_RES_INIT;
        }
        break;

    case RES_STA_NEXT_RES_INIT:
#if BT_TWS_EN
        if(bt_tws_is_slave()) {
            if(!res_play_slave_w4_start()) {
                break;
            }
            role = 1;
        }
#endif
        if(res_list_peek(&item)) {
            res_play_alarm_start(&item, role);
            res_play.tickn = TICK_ADD(TICKN_GET(), 500);
            res_play.state = RES_STA_NEXT_WAIT_ALARM;
        } else {
            res_play.state = RES_STA_RES_EXIT;
        }
        break;

    case RES_STA_NEXT_WAIT_ALARM:
        if( !bt_is_tws_mode() || !bt_is_enable()
           || res_play.alarm_lable == res_play.res_lable || TICKN_IS_EXPIRE(res_play.tickn) ) {
            res_play.state = RES_STA_AUDIO_BYPASS;
        }
        break;
    }
}

//等待当前task的提示音播放完
//参数all: true=所有提示音，false=当前提示音
void res_play_w4_finish(bool all)
{
    u8 func_sta = func_cb.sta;

    func_process();

    TRACE("res_w4: sta=%d, flag=(%d,%d), (%d,%d)\n", res_play.state, all, res_play.enable, func_sta, func_cb.curr);
    while(bsp_res_is_playing() && (res_play.state <= RES_STA_WAIT_ALARM)) {
        func_process();
    }

    func_sta = res_play.func_sta;
    while(bsp_res_is_playing() && ((all && res_play.enable) || res_play.func_sta == func_sta)) {
        func_process();
    }

    TRACE("res_finish: sta=%d, %d\n", res_play.state, res_list_is_empty());
}

AT(.com_text.tws.res)
void res_play_process(void)
{
#if TRACE_EN
    static uint8_t state = -1;
    if(state != res_play.state) {
        state = res_play.state;
        TRACE("res_state: %d\n", res_play.state);
    }
#endif

    if(res_play.state == RES_STA_IDLE && res_list_is_empty()) {
        return;
    }

   res_play_process_do();
}

void res_play_cleanup(void)
{
    res_item_t item;
    uint retry = ITEM_MAX_NB + 4;

    TRACE_INFO("res_play_cleanup, %d\n", res_list.len);
    while(retry--) {
        if(!res_list_get(&item)) {
            break;
        }
    }

    if(res_play.state == RES_STA_WAIT_ALARM) {  //避免在RES_STA_IDLE时已经peek取出来了
        res_play.state = RES_STA_IDLE;
        res_play_set_break(false);              //播空后清掉break_flag
    }
}

bool res_play_search(uint8_t res_idx)
{
    if(res_play_is_busy() && res_play.res_idx == res_idx) {
        return true;
    } else {
        return res_list_search(res_idx);
    }
}

uint8_t res_play_add(uint8_t res_idx, uint8_t flag)
{
    u32 addr, len;
    uint8_t res_type = res_tbl_get_type(res_idx);

    if(sys_cb.dac_disable) {
        return RES_ERR_NO_ERR;
    }

    if (res_type == RES_TYPE_INVALID) {
        return RES_ERR_INVALID;
    } else {
        if(res_play.break_ring && (res_play.res_idx <= RES_IDX_RING && res_play.res_idx >= RES_IDX_NUM_0)) {
            return RES_ERR_NO_ERR;
        }

        if (!res_tbl_get_addr(res_idx, &addr, &len)) {
            return RES_ERR_INVALID;
        }
    }

    if(res_idx == RES_IDX_MAX_VOL) {
        if(res_play_search(res_idx)) {
            return RES_ERR_NO_ERR;
        }
    } else if((res_idx >= RES_IDX_POWERON && res_idx <= RES_IDX_BT_MODE) || res_idx == RES_IDX_PAIRING) {
        flag |= RES_FLAG_BREAK;
    }

    if(res_list_is_full()) {
        TRACE_ERR("res_full: %d(%d) ####\n", res_idx);
        return RES_ERR_LIST_FULL;
    }
    res_lable_cnt++;
    res_lable_cnt &= 0xf;

    TRACE_INFO("res_play: %d(%d),%x --sta=%d, en=%d, brk=%d\n", res_idx, res_lable_cnt, flag, res_play.state, res_play.enable, res_play.break_flag);
#if BT_TWS_EN
    if(bt_is_tws_mode() && bt_is_enable()) {
        bt_tws_res_add(res_idx | (res_lable_cnt<<8) | ((flag&0xf)<<12));
    } else
#endif
    {
        res_play_add_do(res_idx | (res_lable_cnt<<8) | ((flag&0xf)<<12));
    }
    return RES_ERR_NO_ERR;
}

void res_play_var_init(void)
{
    memset(&res_play, 0x00, sizeof(res_play));
    memset(&res_list, 0x00, sizeof(res_list));
    res_list.total = ITEM_MAX_NB;
    res_play.rd_lable = -1;
    res_play.req_lable = -1;
    res_play.alarm_lable = -1;
    res_play.play_tickn = -1L;
    res_play.enable = true;
}

void res_play_req_lable_clr(void)
{
    res_play.rd_lable = -1;
    res_play.req_lable = -1;
    res_play.alarm_lable = -1;
    res_play.play_tickn = -1L;
}

//----------------------------------------------------------------------------
//同步提示音播报前，发控制消息到手机
void res_play_before_cb(uint8_t res_idx)
{
    if(!bt_tws_is_slave()) {
        switch(res_idx) {
        case RES_IDX_CALL_HANGUP:
            bt_call_answer_incoming();  //接听
            break;
        case RES_IDX_CALL_REJECT:
            bt_call_terminate();        //拒接
            break;
        case RES_IDX_REDIALING:
            bt_call_redial_last_number(); //回拨电话
            break;
        }
    }
}

//同步提示音播报完毕，设置状态
void res_play_after_cb(uint8_t res_idx)
{
    switch(res_idx) {
#if BT_A2DP_LOW_LATENCY_EN
    case RES_IDX_MUSIC_MODE:
        printf("music mode\n");
        bt_low_latency_disable();
        break;
    case RES_IDX_GAME_MODE:
        printf("game mode\n");
        bt_low_latency_enable();
        break;
#endif
    }
}

#endif // WARNING_TONE_EN
