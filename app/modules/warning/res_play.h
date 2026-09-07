/*
 *  res_play.h
 *
 *  Created by zoro on 2023-9-20.
 */
#ifndef __RES_PLAY_H
#define __RES_PLAY_H

#include "res_tbl.h"

#define ITEM_MAX_NB         8

//res_flag
enum {
    RES_FLAG_REMOTE     = BIT(0),       //远端提示音
    RES_FLAG_LOCAL      = BIT(1),       //本地提示音
    RES_FLAG_BREAK      = BIT(2),       //可被打断
    RES_FLAG_NOT_DELAY  = BIT(3),       //播报完不delay

    RES_FLAG_TWS        = RES_FLAG_LOCAL|RES_FLAG_REMOTE,   //同步播报
};

typedef struct {
    uint8_t res_idx;
    uint8_t res_lable   : 4;
    uint8_t res_flag    : 4;
} res_item_t;

typedef struct {
    res_item_t item[ITEM_MAX_NB];
    uint8_t wpos;
    uint8_t rpos;
    uint8_t len;
    uint8_t total;
} res_list_t;

//state
enum {
    RES_STA_IDLE,
    RES_STA_WAIT_ALARM,
    RES_STA_AUDIO_BYPASS,
    RES_STA_WAIT_KICK,
    RES_STA_PROC,
    RES_STA_RES_EXIT,
    RES_STA_DELAY,
    RES_STA_DELAY_MORE,
    RES_STA_NEXT_RES_INIT,
    RES_STA_NEXT_WAIT_ALARM,
};

//play_err
enum {
    PLAY_ERR_BREAK      = BIT(0),
    PLAY_ERR_INVALID    = BIT(1),
    PLAY_ERR_FORMAT     = BIT(2),
};

//vol_busy
enum {
    RES_VOL_IDLE        = 0,    //提示音空闲
    RES_VOL_ADJ,                //提示音调节了系统音量
    RES_VOL_MUTE,               //提示音MUTE住了系统音量
};

typedef struct {
    uint8_t state;
    uint8_t res_idx;
    uint8_t res_type;
    uint8_t res_lable;
    uint8_t res_flag;
    uint8_t rd_lable;
    uint8_t req_idx;
    uint8_t req_lable;
    uint8_t alarm_lable;

    uint8_t play_err;
    uint8_t break_ring;
    uint8_t vol_busy;
    bool kick_flag;
    bool done_flag;
    bool break_flag;
    volatile bool enable;
    uint32_t tickn;

    uint32_t play_tickn;
    void (*play_kick_cb)(void);

    volatile uint8_t func_sta;
    uint8_t dnr_sta;
    uint8_t dac_sta;
    uint8_t mute_bak;
} res_play_t;

extern res_play_t res_play;
extern res_list_t res_list;


enum {
    RES_ERR_NO_ERR,
    RES_ERR_LIST_FULL,
    RES_ERR_INVALID,
};

#define res_list_is_empty()                     (bool)(res_list.len == 0)
#define res_list_is_full()                      (bool)(res_list.len >= ITEM_MAX_NB)

#define res_play_set_alarm_lable(param, tickn)  res_play.alarm_lable = ((param)>>8);\
                                                res_play.play_tickn = TICK_ADD(tickn, 300)
#define res_play_set_req_lable(param)           res_play.req_idx = ((param) & 0xff);\
                                                res_play.req_lable = ((param)>>8)
#define res_play_set_enable(en)                 res_play.enable = en
#define res_play_set_break(en)                  res_play.break_flag = en
#define res_play_is_playing()                   (bool)(res_play.state != RES_STA_IDLE)
#define res_play_is_prepare()                   (bool)(res_play.state > RES_STA_IDLE && res_play.state < RES_STA_AUDIO_BYPASS)
#define res_play_is_vol_busy()                  (bool)(res_play.vol_busy != RES_VOL_IDLE)
#define res_play_is_empty()                     res_list_is_empty()

void res_play_var_init(void);                   //提示音初始化
void res_play_process(void);                    //提示音处理循环，需要在主循环调用
void res_play_w4_finish(bool all);              //等待提示音播放完成，all: true=所有提示音，false=当前提示音
void res_play_tws_kick(u32 tickn);              //提示音开始播放处理，需要tws_time_proc_cb调用
void res_play_cleanup(void);                    //清除提示音播放列表
bool res_play_search(uint8_t res_idx);          //查找列表是否存在某个提示音
uint8_t res_play_add(uint8_t res_idx, uint8_t flag);            //添加一个提示音到播放列表，返回值：RES_ERR_NO_ERR/RES_ERR_LIST_FULL/RES_ERR_INVALID
void res_play_req_lable_clr(void);              //清除req_lable


//提示音API函数
#define bsp_res_init()                      res_play_var_init()         //提示音初始化
#define bsp_res_process()                   res_play_process()          //提示音处理循环，需要在主循环调用
#define bsp_res_w4_finish(all)              res_play_w4_finish(all)     //等待提示音播放完成，all: true=所有提示音，false=当前提示音
#define bsp_res_set_enable(en)              res_play_set_enable(en)     //是否使能提示音播放，关闭后可以加播放列表但不播放
#define bsp_res_set_break(en)               res_play_set_break(en)      //设置打断副耳独立的提示音
#define bsp_res_is_full()                   res_list_is_full()          //提示音列表是否存满
#define bsp_res_is_empty()                  res_list_is_empty()         //提示音列表是否播空
#define bsp_res_is_playing()                res_play_is_playing()       //是否有提示音正在播放
#define bsp_res_is_vol_busy()               res_play_is_vol_busy()      //提示音是否正在控制音量
#define bsp_res_cleanup()                   res_play_cleanup()          //清除提示音播放列表
#define bsp_res_search(idx)                 res_play_search(idx)        //查找列表是否存在某个提示音
#define bsp_res_play_do(idx, flag)          res_play_add(idx, flag)     //添加一个提示音到播放列表，返回值：RES_ERR_NO_ERR/RES_ERR_LIST_FULL/RES_ERR_INVALID
#define bsp_res_play(idx)                   bsp_res_play_do(idx, RES_FLAG_TWS)      //添加同步提示音
#define bsp_res_play_local(idx)             bsp_res_play_do(idx, RES_FLAG_LOCAL)    //添加本地提示音
#define bsp_res_play_remote(idx)            bsp_res_play_do(idx, RES_FLAG_REMOTE)   //添加远端提示音

#define bsp_res_tone_play(n)                bt_tws_req_alarm_tone(n)


#endif // __RES_PLAY_H

