#ifndef __API_ALG_H
#define __API_ALG_H

#if WIRELESS_MIC_24B_PCM_EN
typedef s32			mic_pcm_t;
#else
typedef s16			mic_pcm_t;
#endif


///------------------------------------------------------------------------------------------
//lc3s, 16bit 2.5ms
void lc3s_dec_init(u8 sample_rate, u16 samples);
void lc3s_dec(u8 *ibuf, void *obuf, uint samples, uint bfi, uint index);
void lc3s_dec_exit(uint index);
void lc3s_enc_init(u8 sample_rate, u16 samples);
void lc3s_enc(void *ibuf, u8 *obuf, uint samples);
void lc3s_enc_exit(void);

//lc3f, 24bit 5ms
void lc3f_dec_init(u8 sample_rate, u16 samples);
void lc3f_dec(u8 *ibuf, void *obuf, uint samples, uint bfi, uint index);
void lc3f_dec_exit(uint index);
void lc3f_enc_init(u8 sample_rate, u16 samples);
void lc3f_enc(void *ibuf, u8 *obuf, uint samples);
void lc3f_enc_exit(void);

///------------------------------------------------------------------------------------------
//plc_soft, 16bit
/* type : 0 代表48k音频，用于adapter， type : 1代表16k或8k，用于通话*/
void plc_soft_v2_init(u8 idx, u8 sample_rate, u8 pkt_len, u8 type);
void plc_soft_v2_exit(u8 idx);
void plc_soft_v2_process(s16 *int_data, u8 bfi, u8 idx, u8 type);

//plc_soft, 24bit
void plc_soft_init(u8 idx, uint pkt_len);
void plc_soft_process(s32 *int_data, u32 samples, u8 bfi, u8 idx);
void plc_soft_exit(u8 idx);

///------------------------------------------------------------------------------------------
///软件src算法结构体以及相关API声明
typedef struct{
	u8  src_en;
	u8  resv_en;
	int samplerate_in;
	int samplerate_out;
	int phase_comp;
	int phase_i;
	int phase_o;
	int phase_sum;
} srccon_t;

void src_init(u32 ch_index, u32 spr_in, u32 spr_out);
int src_frame_resample(u32 ch_index, short *src_in, short *src_out, int in_cnt);
int src_frame_resample_24bit(u32 ch_index, int *src_in, int *src_out, int in_cnt);
void src_phase_comp_set(u32 ch_index, int phase);

///------------------------------------------------------------------------------------------
///echo算法结构体以及相关API声明
typedef struct {
    u8   skip_flag;			/*开启后相同buf最大delay可多一倍*/
	u8   lp_filter_en;       /*滤波器使能*/
	u8   attenuation_set;    /*1-9 ->10percent-90percent*/
	u16  delay_set;          /*1-500ms*/
    u16  dry;
    u16  wet;
    mic_pcm_t *delay_lbuf_set;
    u32  cutoffFreq_set;     /*截至频率选择*/
} echo_init_t;

#if WIRELESS_MIC_24B_PCM_EN
void echo_init_24bit(echo_init_t *p);
void echo_process_24bit(mic_pcm_t *ldata);
void echo_update_param_24bit(echo_init_t *p, u8 fade_en);
#define echo_init           echo_init_24bit
#define echo_process        echo_process_24bit
#define echo_update_param   echo_update_param_24bit
#else
void echo_init_16bit(echo_init_t *p);
void echo_process_16bit(mic_pcm_t *ldata);
void echo_update_param_16bit(echo_init_t *p, u8 fade_en);
#define echo_init       echo_init_16bit
#define echo_process    echo_process_16bit
#define echo_update_param   echo_update_param_16bit
#endif

///------------------------------------------------------------------------------------------
///YLCRN_L2 AI大模型降噪算法结构体以及相关API声明
typedef struct {
#if 1
	//s32 overdrive;
	float overdrive;
	u8  adaptive_floor;
	//u16 denoiseBound;
	float denoiseBound;
	float denoiseBoundLow;
	float denoiseBoundHigh;
	float denoiseBoundProb;
	float denoiseBoundRatio;
	u8  smooth_en;
	u8  modelUpdatePars0;
	//s32 factor;
	//u8  delta_k_up;
	//s16 lquantile_sm;
	//s32 quan_gap;
	//s16 quan_gap_low_len;
	//s32 quan_gap_low;
	u8  sin_dnn_en;
	u8  sin_all_en;
	s16 sin_all_len;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u16 low_noise_range;
	//u16 spp_fre_p;
	float spp_fre_p;
	u16 spp_fre_len;
	u16 high_gain_len;
	//s16 music_lev;
	float music_lev;
	s16 prev_noise_len;
	//s16 gain_assign;
	float gain_assign;
	u8  hi_gain_mode;
#else
	s32 overdrive;
	u8  adaptive_floor;
	u16 denoiseBound;
	u16 denoiseBoundLow;
	u16 denoiseBoundHigh;
	u16 denoiseBoundProb;
	u16 denoiseBoundRatio;
	u8  smooth_en;
	u8  modelUpdatePars0;
	//s32 factor;
	//u8  delta_k_up;
	//s16 lquantile_sm;
	//s32 quan_gap;
	//s16 quan_gap_low_len;
	//s32 quan_gap_low;
	u8  sin_dnn_en;
	u8  sin_all_en;
	s16 sin_all_len;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u16 low_noise_range;
	u16 spp_fre_p;

	u16 spp_fre_len;
	u16 high_gain_len;
	s16 music_lev;

	s16 prev_noise_len;
	s16 gain_assign;
	u8  hi_gain_mode;
#endif

} ylcrn_L2_cb_t;
void ylcrn_L2_ns_init (ylcrn_L2_cb_t *p);
void ylcrn_L2_ns_process(s32* data);
///------------------------------------------------------------------------------------------
///YLCRN_L3 AI大模型降噪算法结构体以及相关API声明

typedef struct {
	s32 overdrive;
	u8  adaptive_floor;
	u16 denoiseBound;
	u16 denoiseBoundLow;
	u16 denoiseBoundHigh;
	u16 denoiseBoundProb;

	u8  smooth_en;
	u8  modelUpdatePars0;
	s32 factor;
	u8  delta_k_up;
	s16 lquantile_sm;
	s32 quan_gap;
	s16 quan_gap_low_len;
	s32 quan_gap_low;
	u8  sin_dnn_en;
	u8  sin_all_en;
	s16 sin_all_len;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u16 low_noise_range;
	u16 spp_fre_p;
	u16 spp_fre_len;
	u16 high_gain_len;
	s16 gain_assign_len;
	s16 gain_assign;
	s16 nn_only_len;
	s16 music_lev;
	s16 intensity;
	s16 mask_vad_fre_thr;
	u8  spp_max_en;
} ylcrn_L3_cb_t;
void ylcrn_L3_ns_init (ylcrn_L3_cb_t *p) ;
int ylcrn_L3_ns_process(s32* data);

///------------------------------------------------------------------------------------------
///AINS4降噪算法结构体以及相关API声明
typedef struct {

	//s32 noise_db2;
	//s32 noise_db3;
	s32 denoiseBound;
	s32 snr_thr;
	u8  overdrive_adapt_en;
	s32 overdrive;
	u8  smooth_en;
	u8  modelUpdatePars0;
	//u8  enlarge_v;
	s16 gainHB_rd;
	u8 delta_k_up;
	//s16 denoiseBound_fix;
	u8 	yuan_en;
	s16 enr_thres;
	s16  low_fre_range;
	s16 prior_opt_idx;
	s16 prior_opt_freh;
	u8  prior_opt_ada_en;
	s16 lquantile_sm;
	s16 hi_gain_len;
	s32 factor;
	s32 quan_gap;
	s16 quan_gap_low_len;
	s32 quan_gap_low;
	s16 speech_update;
	//s16 smooth_v;
	//u8  enr_mean_max_en;
	//s32 enr_nr_thr;
	u8	spp_en;
	s16 ymin_floor;
	s16 ymin_idx;
	s16 gain_ceil;
	s32 gain_ceil_ratio;
	s16 noise_min_floor;
	//s16 smooth_logLrt;
	s16 music_lev;
	u8  noise_min_en;
	s32 ai_vad_hard_thres;
	u16 spp_vad_len;
	s32 qhat_assign;
	s32 qhat_assign_low;
	u8  spp_max_en;
	u8  low_floor_add_l;
	s16 low_mask_floor_add;
	u8  dd_od_ctl_en;
	u8  ai_vad_spp_hard_en;
	s32 spp_thr_noiseprev;
	s16 spp_thr;
	s16 prev_noise_len;
	u8  vad_low_noise_en;
	s16 vad_low_noise_p;
	u8  vad_low_max_en;
	u8  sin_keep_en;
	s32 sin_keep_thr;
	s16 sin_keep_gap;
	u8  sin_coswin_en;
	u8  sin_keep_idx_lim;
	s32 priorModelPars0;
	s16 alp1_dd;

	//u16 pitch_filter_range;
} ains4_cb_t;

void ains4_init(ains4_cb_t *ains4_cb);
int ains4_process(s32 *data);
///------------------------------------------------------------------------------------------
///AINS5降噪算法结构体以及相关API声明
typedef struct {
	//u16 nt;
	//s16 exp_range_H;
	//s16 exp_range_L;
	//u8  model_select;
	u16 min_value;
	u16 nostation_floor;
	u8  wind_thr;
	u8  wind_en;
	u8  noise_ps_rate;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
    u8  param_printf;                           //使能参数打印
   // u8  wind_level;
	//u16 wind_range;
	u16 low_fre_range;
	u16 low_fre_range0;
	u8  pitch_filter_en;
	float mask_floor;
	u16 noise_ceil;
	u16 ps_lowlimt;
	u8  mask_floor_r;
	float  music_lev;
	u16 gain_expand;
	u8  nn_only;
	u16 nn_only_len;
	u16 gain_assign;
	u8  sin_gain_post_low_en;
	u16 sin_gain_post_len;
	u16 sin_gain_post_len_f;
	float spp_thr;
	u16 spp_vad_len;

	float factor;
	float lquantile_sm;
	float quan_gap;
	s16 quan_gap_low_len;
	float quan_gap_low;

	float qhat_assign;
	float qhat_assign_low;

	u8  spp_max_en;
	u8	spp_en;
	float ai_vad_hard_thres;
	u16 enr_thres;
	u8  prev_noise_ada_en;
	s16  prev_noise_len;
	float pri_to_post_sm;
	float overdrive;
	u8  low_gain_l;
	u8  low_floor_add_l;
	float low_mask_floor_add;
	u8  dd_od_ctl_en;
	u8  ai_vad_spp_hard_en;
	float spp_thr_noiseprev;
	s16 msc_len;
	float msc_thr;
	//u16 pitch_filter_range;
} ains5_cb_t;
void ains5_init(ains5_cb_t *cb);
void ains5_process(s32 *data);
///------------------------------------------------------------------------------------------
///AGC算法结构体以及相关API声明
typedef struct {
	u8  agc_en;
	u16 sampleHzIn;           //语音采样率
	u8  bit;
	u8  compress_agcDb;
	u8  target_agcDbfs;
	u8  low_signal_en;      //针对低能量信号的gaintable,开启后仍由TARGETLEVELRDBFS控制最终电平，COMPRESSIONGAINDB失效
	u8  max_gain;           //开启LOW_SIGNAL_EN后有效，最大增益
	s32 capacitorSlow_default;
} agc_cb_t;
void AgcInit_24bit(agc_cb_t* agc_cb);
void agc_proc_24bit(s32 *input, s32 *output, s32 pcmLen);

///------------------------------------------------------------------------------------------
///DNR_FRE算法结构体以及相关API声明
typedef struct {
	//s32 noise_db2;
	//s32 noise_db3;
	s16 denoiseBound;
	s32 overdrive;
	u8  smooth_en;
	s64 enr_thres;
	s16 smooth_v;
	u8  enr_mean_max_en;
	s16 music_lev;
	s32 enr_nr_thr;
	s16 low_fre_range;
	s16 prior_opt_idx;
	s32 in_attack;
	s32 in_release;
	s32 fs;
	s32 noise_init;
	u8  noise_init_en;
	//u16 pitch_filter_range;
} dnr_fre_cb_t;

void dnr_fre_init(dnr_fre_cb_t *dnr_fre_cb);
int dnr_fre_process(s32 *data);
#endif // __API_CODEC_H

