#include "include.h"
#include "func.h"
#include "func_bt.h"

#if FUNC_BT_EN && BT_HFP_EN
static call_cfg_t call_cfg AT(.sco_data);


AT(.rodata.mic_gain)
const int mic_gain_tbl[16] = {
    AEC_DIG_P0DB,
    AEC_DIG_P1DB,
    AEC_DIG_P2DB,
    AEC_DIG_P3DB,
    AEC_DIG_P4DB,
    AEC_DIG_P5DB,
    AEC_DIG_P6DB,
    AEC_DIG_P7DB,
    AEC_DIG_P8DB,
    AEC_DIG_P9DB,
    AEC_DIG_P10DB,
    AEC_DIG_P11DB,
    AEC_DIG_P12DB,
    AEC_DIG_P13DB,
    AEC_DIG_P14DB,
    AEC_DIG_P15DB,
};

#if BT_SCO_DBG_EN
void sco_audio_set_param(u8 type, u16 value)
{
//    aec_cfg_t *aec = &call_cfg.aec;
//    nr_cb_t *nr = &call_cfg.nr;
    //printf("set param[%d]:%d\n", type, value);
    if (type == AEC_PARAM_NOISE) {
//        xcfg_cb.bt_noise_threshoid = value;
//        nr->threshoid = value;
    } else if (type == AEC_PARAM_OFFSET) {
//       aec->far_offset = value;
    } else if (type == AEC_PARAM_MIC_ANL_GAIN) {
        xcfg_cb.mic_anl_gain = value;
//        set_mic_analog_gain(value, (CHANNEL_0 | CHANNEL_1 | CHANNEL_2));
    } else if (type == AEC_PARAM_MIC_DIG_GAIN) {
        xcfg_cb.bt_mic_dig_gain = value;
//        sdadc_set_digital_gain((CHANNEL_1 | CHANNEL_2 | CHANNEL_3), (value&0x3f));
    } else if (type == AEC_PARAM_MIC_POST_GAIN) {
//        xcfg_cb.bt_sco_post_gain = value & 0x0f;
    }
}
#endif

static void bt_sco_eq_init(call_cfg_t *p)
{
    if(!sys_cb.dac_disable) {
#ifdef RES_BUF_EQ_CALL_NORMAL_EQ
        music_eq_set_by_res(EQ_CHL|EQ_CHR, RES_BUF_EQ_CALL_NORMAL_EQ, RES_LEN_EQ_CALL_NORMAL_EQ);
#else
        music_eq_off();
#endif

#if DAC_DRC_EN
        music_drc_off();
#endif
    }

#if BT_SCO_EQ_DRC_EN
    if(mic_sco_pacc_init(bt_sco_is_msbc())) {
        call_cfg.mic_eq_en = 1;
//        pacc_eq_set_post_gain(mic_gain_tbl[xcfg_cb.bt_sco_post_gain] << 8);
    }
#endif
}

static void bt_sco_eq_exit(void)
{
#if BT_SCO_EQ_DRC_EN
    mic_sco_pacc_exit();
#endif

    if(!sys_cb.dac_disable) {
        music_eq_set_by_num(sys_cb.eq_mode);

        if(app_eq_set() == 0){
            music_eq_set_by_num(sys_cb.eq_mode);
        }
#if DAC_DRC_EN
        music_drc_on();
#endif
    }
}

static void bt_call_alg_init(void)
{
    u8 sysclk = SYS_24M;
    memset(&call_cfg, 0, sizeof(call_cfg_t));

    sys_clk_req(INDEX_VOICE, SYS_100M);

    clkgat_hwmath_en();     //算法初始化会用到，需要放前面
    clkgat_plc_en();        //硬件sqrt64需要用到

    ///通话算法code初始化
    bt_sco_code_init();

    ///通话MIC端降噪算法初始化
    bt_sco_nr_init(&sysclk, &call_cfg.nr);

    ///DAC、MIC EQ初始化
    bt_sco_eq_init(&call_cfg);

    if (SYS_100M != sysclk) {
        sys_clk_req(INDEX_VOICE, sysclk);
    }

    bt_call_init(&call_cfg);
}

static void bt_call_alg_exit(void)
{
    bt_sco_nr_exit();

    bt_call_exit();

    bt_sco_eq_exit();

    clkgat_hwmath_dis();
    clkgat_plc_dis();       //plc和sqrt64同时关闭clkgate

    sys_clk_free(INDEX_VOICE);
}

ALIGNED(64)
void sco_set_incall_flag(u8 bit)
{
    GLOBAL_INT_DISABLE();
    sys_cb.incall_flag |= bit;
    GLOBAL_INT_RESTORE();
}

ALIGNED(128)
bool sco_clr_incall_flag(u8 bit)
{
    bool ret = false;
    GLOBAL_INT_DISABLE();
    if(sys_cb.incall_flag == INCALL_FLAG_FADE) {
        ret = true;
    }
    sys_cb.incall_flag &= ~bit;
    GLOBAL_INT_RESTORE();
    return ret;
}

#define bt_sco_karaok_is_en(n)      false
void sco_audio_init(void)
{
    fot_update_pause();

    sco_set_incall_flag(INCALL_FLAG_SCO);
    if (!bt_sco_karaok_is_en(1)) {
        bt_call_alg_init();
        if(!sys_cb.dac_disable) {
            dac_set_anl_offset(1);
        }

        audio_path_init(AUDIO_PATH_BTMIC);
        audio_path_start(AUDIO_PATH_BTMIC);
        call_cfg.audio_init_flag = 1;
        if(!sys_cb.dac_disable) {
            bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
            dac_fade_in();
        }
    }
}

void sco_audio_exit(void)
{
#if DEVICE_RECORD_EN
    sfunc_record_exit();         //切模式，停止录音
#endif

    sco_clr_incall_flag(INCALL_FLAG_SCO);
    if (!bt_sco_karaok_is_en(0)) {
        if(!sys_cb.dac_disable) {
            dac_fade_out();
            dac_aubuf_clr();
            dac_set_anl_offset(0);
            bsp_change_volume(sys_cb.a2dp_vol);
        }
        audio_path_exit(AUDIO_PATH_BTMIC);
        call_cfg.audio_init_flag = 0;
    }
    bt_call_alg_exit();

    fot_update_continue();
}

AT(.com_text.sco.audio_init)
bool sco_audio_is_init(void)
{
    return (bool)call_cfg.audio_init_flag;
}

static void sfunc_bt_call_process(void)
{
    func_process();
    func_bt_sub_process();
    func_bt_status();
}

static void sfunc_bt_call_enter(void)
{
    if(sys_cb.pwroff.key_state == PWROFF_W4_TIMEOUT) {  //响铃/通话过程长按不关机时，清掉关机状态
        sys_cb.pwroff.key_state = PWROFF_IDLE;
    }

    sco_set_incall_flag(INCALL_FLAG_CALL);
    if(sys_cb.incall_flag == INCALL_FLAG_FADE) {
        bsp_change_volume(bsp_bt_get_hfp_vol(sys_cb.hfp_vol));
        dac_fade_in();
    }
}

static void sfunc_bt_call_exit(void)
{
    bool vol_change = sco_clr_incall_flag(INCALL_FLAG_CALL);
    if(vol_change) {
        bsp_change_volume(sys_cb.a2dp_vol);
    }
}

AT(.text.func.bt)
void sfunc_bt_call(void)
{
    printf("%s\n", __func__);

    sfunc_bt_call_enter();

    while ((f_bt.disp_status >= BT_STA_OUTGOING) && (func_cb.sta == FUNC_BT)) {
        sfunc_bt_call_process();
        sfunc_bt_call_message(msg_dequeue());
        func_bt_display();
    }
    sfunc_bt_call_exit();
}
#else
void sco_audio_init(void){}
void sco_audio_exit(void){}
bool sco_audio_is_init(void){ return false;}
void sfunc_bt_call(void){}
void sco_audio_set_param(u8 type, u16 value){};
#endif
