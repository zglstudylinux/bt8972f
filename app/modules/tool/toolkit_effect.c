#include "include.h"
#include "toolkit_effect.h"

/*
 * 文件名称: effect_adjust_by_res.c
 * 功能描述: 使用上位机修改音效相关处理
 ****************************************************************************************
    code : 待测
    buf  : 待测
    time : 待测
设计流程: 上位机工具会根据链路一键生成 effect_info[]
          根据收到的数据比对 effect_info[] 中的EFFECT_MODE_IDX，匹配则进入callback中进行相应的处理，否则返回

 */

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#define TRACE_R32(...)            print_r32(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#define TRACE_R32(...)
#endif // TRACE_EN


#if EFFECT_DBG_ADJUST_EN

void music_dac_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    TRACE("%s\n", __func__);

    TRACE("channel %d\n",tool_eq_coef->channel);
    TRACE("enable %d\n",tool_eq_coef->enable);
    TRACE("sample_rate %d\n",tool_eq_coef->sample_rate);
    TRACE("max_bandnum %d\n",tool_eq_coef->max_bandnum);
    TRACE("param_size %d\n",tool_eq_coef->param_size);
    TRACE("param_cnt %d\n",tool_eq_coef->param_cnt);
    TRACE("band_value_start %d\n",tool_eq_coef->band_value_start);
    TRACE("band_value_cnt %d\n",tool_eq_coef->band_value_cnt);
    TRACE("reverse %d\n",tool_eq_coef->reverse);
    TRACE("RSVD %d\n",tool_eq_coef->RSVD);
    TRACE("gain 0x%x\n",tool_eq_coef->gain);
    for(uint i=0; i<tool_eq_coef->max_bandnum; i++) {
        TRACE_R32(&tool_eq_coef->coef[i][0], 5);
        TRACE("\n");
    }

    music_eq_set_by_param(EQ_CHL|EQ_CHR, tool_eq_coef->max_bandnum, (u32 *)&tool_eq_coef->gain);
}

void music_dac_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
    TRACE("%s\n", __func__);
//    print_r(buf,len);
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);

    TRACE("channel %d\n",tool_drc_coef->channel);
    TRACE("enable %d\n",tool_drc_coef->enable);
    TRACE("delay %d\n",tool_drc_coef->delay);
    TRACE("sample_rate %d\n",tool_drc_coef->sample_rate);
    TRACE("coeff_cnt 0x%x\n",tool_drc_coef->coeff_cnt);
    TRACE("RSVD 0x%x\n",tool_drc_coef->RSVD);
    TRACE("in_attack_coeff 0x%x\n",tool_drc_coef->in_attack_coeff);
    TRACE("in_release_coeff 0x%x\n",tool_drc_coef->in_release_coeff);
    TRACE("out_attack_coeff 0x%x\n",tool_drc_coef->out_attack_coeff);
    TRACE("out_release_coeff 0x%x\n",tool_drc_coef->out_release_coeff);
    TRACE("tav_coeff 0x%x\n",tool_drc_coef->tav_coeff);

    music_drc_set_by_param(tool_drc_coef->coeff_cnt,(u32 *)&tool_drc_coef->in_attack_coeff);
}

void local_mic_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if WIRELESS_MIC_EQ_DRC_EN
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if(params == 1) {//update
        if (wireless_role_is_adapter()) {
            toolkit_ack_fail();
            return;
        }
        loc_mic_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
#else
    toolkit_ack_fail();
#endif
}

void local_mic_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if WIRELESS_MIC_EQ_DRC_EN
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
    if(params == 1) {//update
        if (wireless_role_is_adapter()) {
            toolkit_ack_fail();
            return;
        }
        loc_mic_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
#else
    toolkit_ack_fail();
#endif
}

void mix_mic_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ADAPTER_MIX_DRC_EN
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if(params == 1 && wireless_get_status()) {//update
        if (!wireless_role_is_adapter()) {
            toolkit_ack_fail();
            return;
        }
        mix_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
#else
    toolkit_ack_fail();
#endif //
}

void mix_mic_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ADAPTER_MIX_DRC_EN
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);

    if(params == 1 && wireless_get_status()) {//update
        if (!wireless_role_is_adapter()) {
            toolkit_ack_fail();
            return;
        }
        mix_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
#else
    toolkit_ack_fail();
#endif
}

void mic_magic_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if MAGIC_EN
    tool_magic_pitch_coef_t *tool_magic_pitch_coef = (tool_magic_pitch_coef_t *)(buf + 14);//去掉包头
    if(params == 1) {//update
        TRACE("channel %d\n", tool_magic_pitch_coef->channel);
        TRACE("enable %d\n", tool_magic_pitch_coef->enable);
        TRACE("shfit_hz %d\n", tool_magic_pitch_coef->shfit_hz);

        magic_audio_mute_set(!tool_magic_pitch_coef->enable);
        magic_audio_param_set(tool_magic_pitch_coef->shfit_hz);
    }
#else
    toolkit_ack_fail();
#endif
}

void mic_echo_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ECHO_EN
    tool_echo_pitch_coef_t *tool_echo_pitch_coef = (tool_echo_pitch_coef_t *)(buf + 14);//去掉包头
    if(params == 1){//update
        TRACE("channel %d\n", tool_echo_pitch_coef->channel);
        TRACE("enable %d\n", tool_echo_pitch_coef->enable);
        TRACE("lp_filter_en %d\n", tool_echo_pitch_coef->lp_filter_en);
        TRACE("attention %d\n", tool_echo_pitch_coef->attention);
        TRACE("delay %d\n", tool_echo_pitch_coef->delay);
        TRACE("cutoff_freq %d\n", tool_echo_pitch_coef->cutoff_freq);
        TRACE("dry %d\n", tool_echo_pitch_coef->dry);
        TRACE("wet %d\n", tool_echo_pitch_coef->wet);
        echo_audio_param_set(tool_echo_pitch_coef->attention, tool_echo_pitch_coef->delay, tool_echo_pitch_coef->cutoff_freq, tool_echo_pitch_coef->lp_filter_en, tool_echo_pitch_coef->dry, tool_echo_pitch_coef->wet);
    }
#else
    toolkit_ack_fail();
#endif
}

#if BT_SCO_EQ_DRC_EN
void msbc_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if(bt_sco_is_msbc()) {

    } else {
        TRACE("ERROR !! NO MATCH\n");
        toolkit_ack_fail();
        return;
    }
    if(params == 1) {//update
        sco_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
}
void msbc_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
    if(bt_sco_is_msbc()) {

    } else {
        TRACE("ERROR !! NO MATCH\n");
        toolkit_ack_fail();
        return;
    }
    if(params == 1) {//update
        sco_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
}
void csvd_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if(bt_sco_is_msbc()) {
        TRACE("ERROR !! NO MATCH\n");
        toolkit_ack_fail();
        return;
    } else {

    }
    if(params == 1) {//update
        sco_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
}
void csvd_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
    if(bt_sco_is_msbc()) {
        TRACE("ERROR !! NO MATCH\n");
        toolkit_ack_fail();
        return;
    } else {

    }
    if(params == 1) {//update
        sco_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
}
#else
void msbc_eq_effect_update_callback(u8 *buf, u32 len, u8 params){}
void msbc_drc_effect_update_callback(u8 *buf, u32 len, u8 params){}
void csvd_eq_effect_update_callback(u8 *buf, u32 len, u8 params){}
void csvd_drc_effect_update_callback(u8 *buf, u32 len, u8 params){}
#endif
#endif

void usb_mic_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ADAPTER_USB_MIC_EQ_DRC_EN
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if(params == 1) {//update
        usb_mic_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ADAPTER_USB_MIC_EQ_DRC_EN
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
    if(params == 1) {//update
        usb_mic_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic1_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ADAPTER_USB_MIC_EQ_DRC_EN
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if(params == 1) {//update
        usb_mic1_stereo_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic1_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ADAPTER_USB_MIC_EQ_DRC_EN
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
    if(params == 1) {//update
        usb_mic1_stereo_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic0_eq_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ADAPTER_USB_MIC_EQ_DRC_EN
    tool_eq_coef_t *tool_eq_coef = (tool_eq_coef_t *)(buf + 14);
    if(params == 1) {//update
        usb_mic0_stereo_pacc_set_eq_by_param(tool_eq_coef->max_bandnum, (const u32 *)&tool_eq_coef->gain);
    }
#else
    toolkit_ack_fail();
#endif
}

void usb_mic0_drc_effect_update_callback(u8 *buf, u32 len, u8 params)
{
#if ADAPTER_USB_MIC_EQ_DRC_EN
    tool_drc_coef_t *tool_drc_coef = (tool_drc_coef_t *)(buf + 14);
    if(params == 1) {//update
        usb_mic0_stereo_pacc_set_drc_by_param(tool_drc_coef->coeff_cnt, (const u32 *)&tool_drc_coef->in_attack_coeff);
    }
#else
    toolkit_ack_fail();
#endif
}
