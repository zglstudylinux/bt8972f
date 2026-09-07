/**********************************************************************
*
*   strong_sco.c
*   定义库里面通话算法部分WEAK函数的Strong函数，动态关闭库代码
***********************************************************************/
#include "include.h"

/*****************************************************************************
 * Module    : AEC算法强定义
 *****************************************************************************/
#if !BT_AEC_EN
AT(.com_text.weak.isr.aec)
bool aec_isr(void) { return false; }
AT(.bt_voice.aec)
void aec_nlms_process(u8 mic_sel) {}
AT(.bt_voice.aec)
void aec_nlp_process(void) {}
void aec_process(void) {};
void bt_aec_process(u8 *ptr, u32 samples, u32 pcm_mode) {}
void aec_init(void) {}
void aec_exit(void) {}
#endif

#if !BT_ALC_EN
void alc_init(void) {}
void alc_exit(void) {}
void alc_process(void) {}
void alc_fade_in(s16 *buf) {}
void alc_fade_out(s16 *buf) {}
AT(.bt_voice.alc)
void bt_alc_process(u8 *ptr, u32 samples, u32 pcm_mode) {};
#endif


/*****************************************************************************
 * Module    : 通话其他部分强定义
 *****************************************************************************/

#if !BT_SCO_DUMP_EN && !BT_EQ_DUMP_EN && !BT_PLC_DUMP_EN
AT(.bt_voice.sco.dump)
void bt_sco_dump_cb(uint type, void *ptr, uint size) {}
#endif

#if !SDADC_DRC_EN
bool sdadc_drc_v3_init(u8 *drc_addr, int drc_len){return false;}
AT(.com_text.sdadc.drc_v3)
bool sdadc_drc_v3_calc(s16 *ptr, u32 samples, u32 pcm_mode){return false;}
#endif

/*****************************************************************************
 * Module    : 通话上行降噪算法强定义
 *****************************************************************************/
#if !BT_SCO_AGC_EN
void bt_agc_init(s32 sampleHzIn, s32 bit, s32 agcDb, s32 agcDbfs) {}
void bt_sco_agc_proc_do(s16 *ptr, int samples) {}
#endif


//#if !BT_SCO_SMIC_AI_EN && !BT_SCO_DMIC_AI_EN
//AT(.bt_voice.sco)
//bool bt_sco_dnn_en(void) {return 0;}
//void dnn_far_upsample(s16 *out, s16 *in, u32 samples, u8 step) {}
//u32 dnn_near_downsample(s16 *ptr, u32 samples) {return 0;}
//#endif

#if !BT_SCO_SMIC_AI_EN
void bt_dnn_init(void *alg_cb) {}
void dnn_sm_process(void) {}
void bt_dnn_exit(void) {}
#endif

#if !BT_SCO_SMIC_AI_PRO_EN
void bt_dnn_pro_init(void *alg_cb) {}
void dnn_pro_sm_process(void) {}
void bt_dnn_pro_exit(void) {}
#endif

#if !BT_SCO_DMIC_AI_EN
void bt_dmns_init(void *alg_cb) {}
void dnn_dm_process(void) {}
void bt_dmns_exit(void) {}
#else
AT(.text.init.npu.ram_dft)
void npu_ram_set_default(int ram_idx, uint16_t size_byte) {}
#endif

#if !BT_HFP_EN
void bt_sco_pcm_process(void (*out_func) (s16 *buf)){}
void sco_init_var(void){}
#endif

