#ifndef _API_NR_H
#define _API_NR_H

//数字音量
#define _MAX_GAIN               (0x8000)

//10^(n/20), n为DB数                                                  n
#define AEC_DIG_P0DB            (int)(_MAX_GAIN * 1.000000)           //0db
#define AEC_DIG_P0_5DB          (int)(_MAX_GAIN * 1.059254)           //0.5db
#define AEC_DIG_P1DB            (int)(_MAX_GAIN * 1.122018)           //1db
#define AEC_DIG_P1_5DB          (int)(_MAX_GAIN * 1.188502)           //1.5db
#define AEC_DIG_P2DB            (int)(_MAX_GAIN * 1.258925)           //2db
#define AEC_DIG_P2_5DB          (int)(_MAX_GAIN * 1.333521)           //2.5db
#define AEC_DIG_P3DB            (int)(_MAX_GAIN * 1.412538)           //3db
#define AEC_DIG_P3_5DB          (int)(_MAX_GAIN * 1.496236)           //3.5db
#define AEC_DIG_P4DB            (int)(_MAX_GAIN * 1.584893)           //4db
#define AEC_DIG_P4_5DB          (int)(_MAX_GAIN * 1.678804)           //4.5db
#define AEC_DIG_P5DB            (int)(_MAX_GAIN * 1.778279)           //5db
#define AEC_DIG_P5_5DB          (int)(_MAX_GAIN * 1.883649)           //5.5db
#define AEC_DIG_P6DB            (int)(_MAX_GAIN * 1.995262)           //6db
#define AEC_DIG_P6_5DB          (int)(_MAX_GAIN * 2.113489)           //6.5db
#define AEC_DIG_P7DB            (int)(_MAX_GAIN * 2.238721)           //7db
#define AEC_DIG_P7_5DB          (int)(_MAX_GAIN * 2.371374)           //7.5db
#define AEC_DIG_P8DB            (int)(_MAX_GAIN * 2.511886)           //8db
#define AEC_DIG_P8_5DB          (int)(_MAX_GAIN * 2.660725)           //8.5db
#define AEC_DIG_P9DB            (int)(_MAX_GAIN * 2.818383)           //9db
#define AEC_DIG_P9_5DB          (int)(_MAX_GAIN * 2.985383)           //9.5db
#define AEC_DIG_P10DB           (int)(_MAX_GAIN * 3.162278)           //10db
#define AEC_DIG_P10_5DB         (int)(_MAX_GAIN * 3.349654)           //10.5db
#define AEC_DIG_P11DB           (int)(_MAX_GAIN * 3.548134)           //11db
#define AEC_DIG_P11_5DB         (int)(_MAX_GAIN * 3.758374)           //11.5db
#define AEC_DIG_P12DB           (int)(_MAX_GAIN * 3.981072)           //12db
#define AEC_DIG_P12_5DB         (int)(_MAX_GAIN * 4.216965)           //12.5db
#define AEC_DIG_P13DB           (int)(_MAX_GAIN * 4.466836)           //13db
#define AEC_DIG_P13_5DB         (int)(_MAX_GAIN * 4.731513)           //13.5db
#define AEC_DIG_P14DB           (int)(_MAX_GAIN * 5.011872)           //14db
#define AEC_DIG_P14_5DB         (int)(_MAX_GAIN * 5.308844)           //14.5db
#define AEC_DIG_P15DB           (int)(_MAX_GAIN * 5.623413)           //15db
#define AEC_DIG_P15_5DB         (int)(_MAX_GAIN * 5.956621)           //15.5db
#define AEC_DIG_P16DB           (int)(_MAX_GAIN * 6.309573)           //16db
#define AEC_DIG_P16_5DB         (int)(_MAX_GAIN * 6.683439)           //16.5db
#define AEC_DIG_P17DB           (int)(_MAX_GAIN * 7.079458)           //17db
#define AEC_DIG_P17_5DB         (int)(_MAX_GAIN * 7.498942)           //17.5db
#define AEC_DIG_P18DB           (int)(_MAX_GAIN * 7.943282)           //18db
#define AEC_DIG_P18_5DB         (int)(_MAX_GAIN * 8.413951)           //18.5db
#define AEC_DIG_P19DB           (int)(_MAX_GAIN * 8.912509)           //19db
#define AEC_DIG_P19_5DB         (int)(_MAX_GAIN * 9.440609)           //19.5db
#define AEC_DIG_P20DB           (int)(_MAX_GAIN * 10.000000)          //20db
#define AEC_DIG_P20_5DB         (int)(_MAX_GAIN * 10.592537)          //20.5db
#define AEC_DIG_P21DB           (int)(_MAX_GAIN * 11.220185)          //21db
#define AEC_DIG_P21_5DB         (int)(_MAX_GAIN * 11.885022)          //21.5db
#define AEC_DIG_P22DB           (int)(_MAX_GAIN * 12.589254)          //22db
#define AEC_DIG_P22_5DB         (int)(_MAX_GAIN * 13.335214)          //22.5db
#define AEC_DIG_P23DB           (int)(_MAX_GAIN * 14.125375)          //23db
#define AEC_DIG_P23_5DB         (int)(_MAX_GAIN * 14.962357)          //23.5db
#define AEC_DIG_P24DB           (int)(_MAX_GAIN * 15.848932)          //24db
#define AEC_DIG_P24_5DB         (int)(_MAX_GAIN * 16.788040)          //24.5db
#define AEC_DIG_P25DB           (int)(_MAX_GAIN * 17.782794)          //25db
#define AEC_DIG_P25_5DB         (int)(_MAX_GAIN * 18.836491)          //25.5db
#define AEC_DIG_P26DB           (int)(_MAX_GAIN * 19.952623)          //26db
#define AEC_DIG_P26_5DB         (int)(_MAX_GAIN * 21.134890)          //26.5db
#define AEC_DIG_P27DB           (int)(_MAX_GAIN * 22.387211)          //27db
#define AEC_DIG_P27_5DB         (int)(_MAX_GAIN * 23.713737)          //27.5db
#define AEC_DIG_P28DB           (int)(_MAX_GAIN * 25.118864)          //28db
#define AEC_DIG_P28_5DB         (int)(_MAX_GAIN * 26.607251)          //28.5db
#define AEC_DIG_P29DB           (int)(_MAX_GAIN * 28.183829)          //29db
#define AEC_DIG_P29_5DB         (int)(_MAX_GAIN * 29.853826)          //29.5db
#define AEC_DIG_P30DB           (int)(_MAX_GAIN * 31.622777)          //30db
#define AEC_DIG_P30_5DB         (int)(_MAX_GAIN * 33.496544)          //30.5db
#define AEC_DIG_P31DB           (int)(_MAX_GAIN * 35.481339)          //31db
#define AEC_DIG_P31_5DB         (int)(_MAX_GAIN * 37.583740)          //31.5db

enum DUMP_ENC_TYPE {
    DUMP_MIC_TALK               = 0,    //主MIC数据
    DUMP_MIC_FF,                        //副MIC数据
    DUMP_MIC_FB,                        //FB MIC数据，三麦算法的FB MIC
    DUMP_MIC_NR,                        //降噪后数据
    DUMP_AEC_INPUT,                     //AEC输入数据
    DUMP_AEC_FAR,                       //AEC远端数据
    DUMP_AEC_OUTPUT,                    //AEC输出数据
    DUMP_FAR_NR_INPUT,                  //远端降噪输入数据
    DUMP_FAR_NR_OUTPUT,                 //远端降噪输出数据
    DUMP_EQ_OUTPUT,                     //MIC EQ输出数据

    DUMP_PLC_IN                 = 32,   //PLC 输入数据
    DUMP_PLC_OUT,                       //PLC 输出数据
    DUMP_PLC_STA,                       //PLC 状态数据

    DUMP_SCO_MAX_NB             = 5,    //talk + ff + fb + far/eq_out + nr 目前最多5条通路够用了
};

#define DUMP_SNDP_MASK                      0x000f
#define DUMP_AEC_MASK                       0x00f0

#define NR_CFG_TYPE_MASK                    0x1F

//call param
enum NR_TYPE {
    NR_TYPE_NONE            = 0,
    NR_TYPE_AINS,
    NR_TYPE_SNDP_SM,
    NR_TYPE_SNDP_DM,
    NR_TYPE_SNDP_FBDM,
    NR_TYPE_SMIC_AI,                            //自研单MIC AI降噪算法
    NR_TYPE_SMIC_PRO_AI,                        //自研单MIC 大模型AI降噪算法
    NR_TYPE_DMIC_AI,                            //NR_TYPE_DMDNN
    NR_TYPE_USER,
};

enum AEC_TYPE {
    AEC_TYPE_NONE           = 0,
    AEC_TYPE_DEFAULT,                           //默认硬件频域AEC
};

enum AEC_NR_MODE {
    AEC_NR_MODE0            = 0,                //AEC(线性+非线性) + NR
    AEC_NR_MODE1,                               //AEC(线性) + NR + AEC(非线性)
    AEC_NR_MODE2,                               //NR + AEC(线性) + AEC(非线性)
};

typedef struct {
    u8 level;                                   //稳态最大降噪量，范围0~29dB
    u8 dnn_level;                               //dnn最大降噪量，范围0~29dB
    u8 param_printf;

    const u32 *coef;                            //声加配置资源，非0：使用声加工具配置（从资源读取）
} sndp_sm_cb_t;

typedef struct {
    u8 distance;
    //小于保护角的语音被保留，大于抑制角的语音被抑制，中间平滑过渡
    u8 degree;                                  //语音保护角，范围0~90度
    u8 degree1;                                 //语音抑制角 = degree + degree1，范围0~90度

    u8 level;                                   //稳态最大降噪量，范围0~29dB
    u8 dnn_level;                               //dnn最大降噪量，范围0~29dB
    u8 param_printf;
    int bf_upper;                               //固定波束频率上限, 默认128
    const u32 *coef;                            //声加配置资源，非0：使用声加工具配置（从资源读取）
} sndp_dm_cb_t;

typedef struct {
    u8 level;                                   //稳态最大降噪量，范围0~29dB
    u8 dnn_level;                               //dnn最大降噪量，范围0~29dB
    u8 param_printf;
    u8 noiserms_th0;                            //混音阈值，范围0~100

    const u32 *coef;                            //声加配置资源，非0：使用声加工具配置（从资源读取）
} sndp_fbdm_cb_t;

enum NR_CFG_EN {
    NR_CFG_FAR_EN           = BIT(0),           //使能远端降噪
    NR_CFG_NEAR_AINS2_EN    = BIT(1),           //使能声加单麦前AINS2
    NR_CFG_TRUMPET_EN       = BIT(2),           //使能汽车喇叭声降噪
    NR_CFG_SCO_FADE_EN      = BIT(3),           //使能通话前500ms淡入
    NR_CFG_DAC_DNR_EN       = BIT(4),           //使能通话下行————动态降噪
    NR_CFG_DAC_DRC_EN       = BIT(5),           //使能通话下行————DRC
};

typedef struct {
    u32 nt;
    u8  prior_opt_idx;
	u8  ns_ps_rate;
	u8  trumpet_en;
	u8  nt_post;
	u8  exp_range;
	u8  print_en;

} ains3_cb_t;

typedef struct {
    u8 mic_cfg;
    u8 distance;                                //双mic间距：mm（步进1mm）
    u8 degree;                                  //语音角度（小于90度）:10+5*N (5度步进，N<17)
    u8 max_degree;                              //最大保护角度（小于90度）:10+5*N (5度步进，N<17)
    u8 level;                                   //降噪强度：2*N（2dB步进，N<30）
    u8 wind_level;
    u8 param_printf;                            //使能参数打印
    u8 rfu[1];
    const int *filter_coef;                     //int coef[512]
    int maxIR;                                  //Q18
    int maxIR2;                                 //Q18
    u16 windnoise_conditioned_eq_x0;            //风噪抑制起始频段(Hz)
    u16 windnoise_conditioned_eq_x1;            //风噪抑制终止频段(Hz)
    int windnoise_conditioned_eq_y1;            //风噪抑制的终止大小(db), 最小为0db
    u8 snr_x0_level;                            //如果在一般信噪比下，降噪不足，就适当调大（0~0.4 步进0.05），默认0.1
    u8 snr_x1_level;                            //如果低信噪比下，语音断断续续，就适当调小（0.5~1 步进0.05），默认0.8
    int gc_NoiseRMSdB_TH0;                      //降噪量控制参数配置，在噪声水平高时，降低双麦的实际降噪量
    u8 windEQ_en;                               //风噪EQ的控制接口
} dmic_cb_t;

typedef struct {
	u16 nt;
	u8  nt_post;
	s16 exp_range_H;
	s16 exp_range_L;
	u8  model_select;
	u16 min_value;
	u16 nostation_floor;
	u8  wind_thr;
	u8  wind_en;
	u8  noise_ps_rate;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	u8  wind_level;
	u16 wind_range;
	u16 low_fre_range;
	u16 low_fre_range0;
	u8  pitch_filter_en;
    u8  param_printf;                           //使能参数打印
	u16 mask_floor;
	u8  mask_floor_r;
	u8  music_lev;
	u8  comforN_level;
	u16 gain_expand;
	u8  nn_only;
	u16 nn_only_len;
	u16 gain_assign;
	u8  sin_gain_post_en;
	u16 sin_gain_post_len;
	u16 sin_gain_post_len_f;
} dnn_cb_t;

typedef struct {
	u16 gain_floor;
	//u8  nt_post;
	//s16 exp_range_H;
	//s16 exp_range_L;
	u8  noise_ps_rate;
	u8  prior_opt_idx;
	u8  prior_opt_ada_en;
	//u8  wind_level;
	//u16 wind_range;
	u16 low_fre_range;
	u8  param_printf;                           //使能参数打印
	u8  music_lev;
	u8  nn_only;
	u16 nn_only_len;
	u16 gain_assign;
	u8  sin_gain_post_en;
	u16 sin_gain_post_len;
	u16 sin_gain_post_len_f;
} dnn_pro_cb_t;

typedef struct {
    u8  param_printf;                           //使能参数打印
    u8  mic_cfg;                                //主副麦选择
    u8  bf_type;                                //beamforming类型
	s16 distance;				                //双麦间距
	u16 degree;					                //拾音角度方向
	s16 nt;						                //普通降噪量
	u8  nt_post;						             //后滤波降噪量
	s16 exp_range;
	//u8  fast_convergence_en;                    //快速收敛功能，对人声有影响，默认不开
	//u16 lowside_corr;                           //下支路相关系数

	//	u8  trumpet_en;                             //喇叭声抑制
	u8  cmp_tbl_dis;
	u8  dm_dnn_en;                              //是否使能双麦AI算法
	u8  noise_ps_rate;
	u8  comp_mic_fre;                           //双麦+AI的参数
	u16 comp_mic_fre_L;                         //传统双麦的参数
	u16 comp_mic_fre_H;                         //传统双麦的参数
	u8  prior_opt_idx;
	u16 low_fre_ns0_range;
	u8  wind_sig_choose;
	u8  bf_choose;
	u8  tf_en;
	u8  post_filter_en;
	u8  phase_comp_en;
	u8  phase_comp_en_B;
	u8  low_bf_lim;
	s32 NSC;
	u16 tf_len;
	u8  tf_norm_en;
	u8  tf_norm_only_en;
	s32 opt_limit;
	s16 exp_range_H;
	s16 exp_range_L;
	u8	music_lev;
	u16 gain_expand;
	u8  nn_only;
	u16 nn_only_len;
	u8 hi_fre_en;
	u16 gain_assign;
	u8  sin_gain_post_en;
	u16 b_frein2_floor;
	u16 Gcoh_thr;
	u16 Gcoh_sum_thr;
	u8 wind_or_en;
	u8  v_white_en;
	//u8  wind_type;
	s16 sin_gain_post_len_f;
	u8  enlarge_v;
	s16 mask_floor;
    u16 mask_floor_hi;
	s16 wind_state_lim;
	s16 wind_sup_floor;
	u8  wind_sup_open;
	u8  wind_a_pow_en;
	u8  bfrein_en;
	u8  bfrein3_ns;
	u16 wind_len_used;
	u8  bfrein3_en;
	s32 gu_value;
	u8 TCtrigger_sta;
	u8  white_approach_shape;
	u8 low_grad_prot;
	s32 grad_gain_floor;
} dmns_cb_t;

typedef struct {
    u8 mode;                                    //auto or manual mode
    u8 level;                                   // 降噪量, range: 0~5
    u8 thr;                                     // range: 0~20
} peri_nr_cfg_t;

typedef struct {
    u8 nr_type;                                 //近端降噪类型
    u8 nr_cfg_en;                               //远端降噪、喇叭声降噪、500ms淡入等降噪配置
    u32 dump_en;
    u8 rfu[2];
    u8 calling_voice_pow;                       //用于呼出电话，响铃之前动态降噪
    u8 calling_voice_cnt;
    u8 calling_voice_temp_cnt;
    void *far_nr;                               //远端降噪算法配置
} nr_cb_t;

typedef struct {
    u32 type            :4;
    u32 mode            :1;                     //auto or manual mode
    u32 nlp_bypass      :1;                     //aec nlp bypass
    u32 nlp_level       :4;
    u32 nlp_part        :2;
    u32 comforn_level   :4;
    u32 comforn_floor   :10;
    u32 comforn_en      :1;
    u32 xe_add_corr     :16;
    u32 upbin           :8;
    u32 lowbin          :8;
    u32 diverge_th      :5;
    u32 gamma           :16;
    u32 aggrfact        :16;
    u32 mic_ch          :1;
    u32 dig_gain        :24;
    u32 echo_th         ;
    u16 far_offset;
    u8  ff_mic_ref_en;
    u32 qidx;
    u8  bandrange;
    u8  mu_step;
} aec_cfg_t;

typedef struct {
    u8 alc_en;
    u8 rfu[1];
    u8 fade_in_step;
    u8 fade_out_step;
    u8 fade_in_delay;
    u8 fade_out_delay;
    s32 far_voice_thr;
} alc_cb_t;

typedef struct {
    alc_cb_t alc;
    nr_cb_t nr;

    u8 rfu[3];
    u8 mic_eq_en        : 1;
    u8 mic_drc_en       : 1;
    u8 audio_init_flag  : 1;
} call_cfg_t;

void unlock_dcode(void);
void load_code_bt_voice(void);
void unlock_code_bt_voice(void);
void load_code_sco_comm(void);

void bt_call_init(call_cfg_t *p);
void bt_call_exit(void);
bool bt_sco_dnn_en(void);

void bt_dnn_init(void *alg_cb);
void bt_dnn_exit(void);

void bt_dnn_pro_init(void *alg_cb);
void bt_dnn_pro_exit(void);

void bt_agc_init(s32 sampleHzIn, s32 bit, s32 agcDb, s32 agcDbfs);
void bt_sco_agc_proc_do(s16 *ptr, int samples);

void nr_init(u16 nt);
void nr_process(s16 *data);    //data:64个点
u32 nr_gain_radio_convert(u8 dB);



void sco_pcm_kick_start(void);
void bt_sco_adc_to_alg_input(u8 *ptr, u32 samples, u32 pcm_mode);
bool bt_sco_enc_mic_test(u8 *ptr, u32 samples, u32 pcm_mode);

void alc_init(void);
bool alc_isr(void);
void alc_exit(void);

/*
 * NPU初始化
 */
void npu_init(void);

/*
 * NPU ram初始化
 * code_addr:   指令buffer起始地址
 * code_len:    指令buffer长度
 * weight_addr: 权值buffer起始地址
 * weight_len:  权值buffer长度
 * ram0_addr:   RAM0 buffer起始地址
 * ram0_len:    RAM0 buffer长度
 * ram1_addr:   RAM1 buffer起始地址
 * ram1_len:    RAM1 buffer长度
 */
void npu_ram_init(void *code_addr, int code_len, void *weight_addr, int weight_len, void *ram0_addr, int ram0_len, void *ram1_addr, int ram1_len);

#endif
