#ifndef _API_DRC_H
#define _API_DRC_H

//DRC初始化相关结构体
typedef struct {
    int threshold;
    int gain;
    int slope;
} drc_v3_coeff_t;

typedef struct {
    //预置值
    int in_attack;
    int in_release_his;
    int out_attack;
    int out_release;
    int in_tav;
    drc_v3_coeff_t coeffs[4];

    //中间值
    int in_level;
    int in_level_r;
    int in_level_l;
    int out_gain;
} drc_v3_cb_t;

bool drc_v3_init(const void *bin, int bin_size, drc_v3_cb_t *drc_cb);
s16 drc_v3_calc(s32 sample, drc_v3_cb_t *drc_cb);

#endif
