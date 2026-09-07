#include "include.h"
#include "api_alg.h"
#include "ains5.h"

/*
 * 文件名称: ains5.c
 * 功能描述: 本文件为软件 ains5 处理模块
    AT(.buf.ains5);
    AT(.rodata.ains5)
    AT(.text.ains5_proc)
    AT(.text.ains5_init)

    注意mic_pcm_t 实际配置类型

    本算法资源采用复用npuram的方式，所以默认使能后需要按照以下流程才能够体验
    1、调用 gtcrn_L2_mic_mute_set(1) mute住停掉 AI降噪(GTRCRN_L1/GTCRN_L2) 数据流,
    2、调用 npu_exit() 关闭npu, 如果没有关闭npu对npuram操作会直接复位
    2、调用 ains5_mic_param_set() ，一条龙load ram + init
    3、调用 ains5_mic_mute_set(0)，解除ains5 mute，即开启体验ains5算法

        mic_enc_mute_set(1);
        gtcrn_L2_mic_mute_set(1);
        ains5_mic_param_set(1000);
        ains5_mic_mute_set(0);
        delay_5ms(10);
        mic_enc_mute_set(0);

    与之相对的退出ains5切到 AI降噪(GTRCRN_L1/GTCRN_L2)需要按照以下流程才能够正常切换
    1、调用 ains5_mic_mute_set(1)，mute住停掉ains5数据流
    2、相关算法的初始化
    3、解除对应的mute

        mic_enc_mute_set(1);
        ains5_mic_mute_set(1);
        gtcrn_L2_mic_param_set(1000);
        gtcrn_L2_mic_mute_set(0);
        delay_5ms(10);
        mic_enc_mute_set(0);

 ****************************************************************************************
    code + rodata : 14.5 k
    buf           : 34 k
    npu           ：
    time          :  2.986ms/5ms  hw_fft+float 已加速
 */

#if AINS5_EN

#define AINS5_INFO_PRINT             1
#define FRAME_LEN                    240                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          240                         //每次存取帧长

static struct tog_bug_tag ains5_tbuf AT(.buf.ains5);                    //乒乓buf控制
static mic_pcm_t ains5_cache_buf[FRAME_LEN*2] AT(.buf.ains5);           //乒乓buf缓存
static mic_pcm_t ains5_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.ains5);     //输出buf中转缓存
static void *ains5_proc_ptr = ains5_cache_buf;

static ains5_cb_t ains5_cb AT(.buf.ains5);
static ains5_mic_cfg_t ains5_mic_cfg AT(.bss.ains5);

#if AINS5_INFO_PRINT
AT(.com_text.ains5)
const char ains5_info[] = "AINS5_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.ains5_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(ains5_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.com_text.ains5_proc)
void ains5_mic_audio_input(u8 *ptr, u32 samples, u32 params)
{
    if (!ains5_mic_cfg.mute && wireless_cb.alg_en) {

        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&ains5_tbuf, (u8 *)ains5_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&ains5_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&ains5_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                ains5_proc_ptr = tog_bug_get_w_block(&ains5_tbuf);
                tog_buf_wr_toggle(&ains5_tbuf);
                ains5_mic_cfg.kick_proc_done++;
                ains5_mic_proc_kick_start();

            }

            memcpy(ptr, ains5_tmp_buf, rlen*sizeof(mic_pcm_t));
            if(ains5_mic_cfg.callback) {
                ains5_mic_cfg.callback((void *)ptr, rlen, params);
            }

            samples -= rlen;
        }
    } else {
        if(ains5_mic_cfg.callback) {
            ains5_mic_cfg.callback((void *)ptr, samples, params);
        }
    }

}

//ains5算法启动计算 放在低优先级现场处理
AT(.text.ains5_proc)
void ains5_mic_proc_cb(void)
{
    mic_pcm_t *rptr = ains5_proc_ptr;

#if AINS5_INFO_PRINT
    info_printf();
#endif
#if AINS5_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 0, 1);
#endif

    ains5_process(rptr);

#if AINS5_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 1, 1);
#endif
    ains5_mic_cfg.kick_proc_done--;
}

AT(.text.ains5_set)
void ains5_mic_output_callback_set(audio_callback_t callback)
{
    ains5_mic_cfg.callback = callback;
}

AT(.text.ains5_init)
void ains5_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&ains5_mic_cfg, 0, sizeof(ains5_mic_cfg));
    ains5_mic_cfg.mute = 1;

    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
    ains5_mic_mute_set(1);
#if AINS5_DUMP_EN
    audio_dump_init();
#endif

//    ains5_mic_param_set(1000);
//    ains5_mic_mute_set(0);
}

AT(.text.ains5_exit)
void ains5_mic_exit(void)
{

}

static const s32 fft_in_test[512] = {
19872,  36082,  48792,  58502,  66900,  72345,  78724,  82660,
89167,  95213,  96763,  105186,  108642,  112668,  122541,  133142,
153949,  177762,  212053,  258414,  310590,  374667,  440402,  516346,
591297,  649763,  705853,  758469,  812928,  860784,  894782,  937678,
990647,  1047705,  1107328,  1165855,  1206933,  1217172,  1218875,  1210154,
1172075,  1115180,  1049667,  978578,  928905,  900504,  866768,  845388,
830197,  797434,  774290,  731235,  645191,  574705,  513698,  443418,
398745,  395839,  425803,  486130,  584676,  714359,  854757,  1008562,
1178140,  1367809,  1560331,  1734418,  1906828,  2065557,  2203513,  2343237,
2469992,  2576744,  2688580,  2836422,  2986329,  3113673,  3251688,  3361819,
3427759,  3447970,  3388168,  3281718,  3137341,  2959511,  2760134,  2534181,
2333327,  2153607,  1986047,  1850718,  1719169,  1573764,  1405532,  1230464,
1034512,  817854,  611539,  374915,  151399,  -30692,  -212799,  -361854,
-498198,  -584859,  -600150,  -587211,  -500890,  -364305,  -200823,  3762,
246312,  544902,  817978,  1075434,  1355804,  1617542,  1896071,  2173732,
2455682,  2781506,  3102454,  3458793,  3828502,  4167976,  4473153,  4709884,
4901900,  5016647,  5036513,  4995136,  4882321,  4778074,  4681391,  4510600,
4294616,  4067639,  3853745,  3626686,  3390690,  3159414,  2838316,  2485266,
2128423,  1718350,  1315849,  883818,  460928,  114361,  -222032,  -493143,
-682996,  -809739,  -888412,  -966795,  -999767,  -1033594,  -1080171,  -1074560,
-1057725,  -984238,  -833798,  -632764,  -342985,  7694,  374880,  742277,
1119633,  1478299,  1811144,  2160332,  2479153,  2746358,  3021797,  3343062,
3669558,  3958688,  4198366,  4366569,  4507926,  4600657,  4619755,  4645902,
4640536,  4618858,  4618922,  4565907,  4451961,  4261495,  4008095,  3711444,
3355595,  2964645,  2560861,  2220109,  1922656,  1631316,  1411345,  1200153,
970555,  730954,  480824,  218910,  -91687,  -352582,  -579514,  -800062,
-881867,  -897603,  -870873,  -778886,  -696960,  -605726,  -510973,  -430147,
-381259,  -321111,  -189603,  -77634,  54086,  241172,  418115,  619746,
785966,  913643,  1078412,  1215272,  1335101,  1467264,  1578783,  1677539,
1766316,  1838216,  1845089,  1816568,  1787440,  1743121,  1736625,  1742565,
1736678,  1786798,  1841411,  1920526,  1998506,  2042850,  2056387,  2024053,
2019794,  2000882,  1972953,  1985037,  1966939,  2021416,  2111190,  2144469,
2211995,  2292259,  2381733,  2436418,  2459943,  2471688,  2369334,  2271364,
2193448,  2043640,  1881988,  1665449,  1466197,  1301284,  1077275,  859313,
637146,  403336,  173671,  -58550,  -247767,  -458223,  -703184,  -916099,
-1136649,  -1374100,  -1582076,  -1753834,  -1900043,  -2007664,  -2081798,  -2104286,
-2032695,  -1923458,  -1797603,  -1611821,  -1417895,  -1225988,  -1029399,  -865538,
-699822,  -503462,  -271032,  4738,  310700,  658336,  1037370,  1420776,
1788573,  2106383,  2357371,  2545600,  2665102,  2733052,  2777488,  2772566,
2756540,  2749954,  2691662,  2624595,  2529034,  2367726,  2191112,  1964759,
1700299,  1407068,  1079297,  770618,  444050,  118325,  -193948,  -510094,
-789029,  -1074506,  -1340231,  -1559070,  -1757523,  -1885687,  -1977846,  -2032215,
-2036276,  -2043217,  -2006868,  -1947262,  -1899838,  -1824471,  -1700086,  -1535118,
-1396327,  -1221722,  -978294,  -751307,  -472108,  -160610,  130677,  407345,
633848,  891421,  1134759,  1310678,  1466687,  1580061,  1708119,  1805926,
1861263,  1933235,  1954783,  1961966,  1944508,  1882818,  1804197,  1656663,
1518891,  1381998,  1202107,  1028539,  847290,  704402,  565249,  397359,
247026,  90926,  -41608,  -168971,  -292776,  -398079,  -529029,  -612887,
-653684,  -699119,  -727964,  -751461,  -753053,  -741865,  -719052,  -668693,
-602475,  -514004,  -422826,  -321007,  -213218,  -134988,  -46901,  45092,
112810,  172538,  216992,  269477,  324668,  356759,  404851,  451727,
472684,  487672,  472938,  430740,  372887,  291389,  206129,  111983,
6651,  -91844,  -173572,  -245411,  -316199,  -362967,  -392904,  -431430,
-467132,  -498848,  -530048,  -555031,  -573969,  -589549,  -598445,  -601305,
-588330,  -562606,  -547151,  -523376,  -479348,  -425955,  -363382,  -303806,
-240959,  -177203,  -118919,  -57528,  -13717,  15199,  42513,  60389,
68149,  65329,  66237,  66033,  58845,  56183,  45732,  35375,
25697,  4751,  -14334,  -37839,  -65654,  -93003,  -121010,  -139255,
-159578,  -180607,  -191839,  -200739,  -202374,  -199405,  -193985,  -184785,
-174613,  -157846,  -138893,  -118937,  -96099,  -73253,  -48575,  -23995,
0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,
0,  0,  0,  0,  0,  0,  0,  0,
};

AT(.text.ains5_set.param)
void ains5_mic_param_set(s16 ains5_nt)
{
    load_code_wl_ains5();

    tog_buf_init(&ains5_tbuf, ains5_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
//    memset((uint8_t *)&ains5_mic_cfg, 0, sizeof(ains5_mic_cfg));
    memset(&ains5_cb, 0, sizeof(ains5_cb));

    ains5_cb.param_printf           = 0;
	//ains5_cb.nt                     = 6;
	//ains5_cb.nt_post                = 0; //0-6 >0才起效 0为不开gain指数化 开的话默认为3

	ains5_cb.noise_ps_rate          = 1;
	ains5_cb.prior_opt_idx	       = 15;
	ains5_cb.prior_opt_ada_en	   = 1;

	ains5_cb.low_fre_range          = 6; //
	ains5_cb.low_fre_range0         = 0;
	//ains5_cb.pitch_filter_en		   = 1;
	ains5_cb.ps_lowlimt             = 0;
	ains5_cb.mask_floor			   = 0.244f;//8000;
	ains5_cb.noise_ceil			   = 0;
	ains5_cb.music_lev			   = 6.0f;
	//ains5_cb.comforN_level		   = 1;
	ains5_cb.spp_vad_len			   = 1;
	ains5_cb.sin_gain_post_low_en   = 0;
	ains5_cb.sin_gain_post_len	   = 0;

	ains5_cb.spp_thr				   = 0.48828125f;//16000;
	ains5_cb.lquantile_sm		   = 0.75f;//24576;
	ains5_cb.factor				   = 30.0f;
	ains5_cb.quan_gap			   = 0.1f;
	ains5_cb.quan_gap_low_len	   = 6;
	ains5_cb.quan_gap_low		   = 0.1f;
	ains5_cb.qhat_assign			   = 1.0f;//32768;//31129;
	ains5_cb.qhat_assign_low		   = 1.0f;//32768;//22937;
	ains5_cb.spp_max_en			   = 3;
	ains5_cb.spp_en				   = 1;
	ains5_cb.ai_vad_hard_thres	   = 0.38653564453125f;//12666;//19666;
	ains5_cb.enr_thres			   = 0;
	ains5_cb.prev_noise_ada_en	   = 1;
	if(	ains5_cb.mask_floor>0.4883f){
		ains5_cb.prev_noise_len	   = 6;
		ains5_cb.pri_to_post_sm	   = 0.98f;//32112;//;//0.98f
	}
	else{
		ains5_cb.prev_noise_len	   = 6;
		ains5_cb.pri_to_post_sm	   = 0.99f;//32440;//32112;//0.98f
	}
	if(	ains5_cb.mask_floor>0.3052f){
		ains5_cb.overdrive			   = 1.0f;//32768;
	}
	else{
		ains5_cb.overdrive			   = 1.25f;//40960;
	}
	ains5_cb.low_gain_l			   = 1;//若要扫频这里设1
	ains5_cb.low_floor_add_l		   = 4;
	ains5_cb.low_mask_floor_add	   = 0.03052f;//1000;
	ains5_cb.dd_od_ctl_en		   = 1;
	ains5_cb.ai_vad_spp_hard_en	   = 1;
	ains5_cb.spp_thr_noiseprev	   = 1.0f;
	ains5_cb.msc_len				   = 8;
	ains5_cb.msc_thr				   = 0.1525879f;//5000;
    ains5_init(&ains5_cb);


    //提前送几帧数据进去 让算法起效 规避前面的白噪

    memset(&ains5_cache_buf[0], 0, 240*8);
    memcpy(&ains5_cache_buf[0], fft_in_test, 480);
//    print_r(ains5_cache_buf, 480);

    ains5_process(ains5_cache_buf);
    ains5_process(ains5_cache_buf);
    ains5_process(ains5_cache_buf);
    ains5_process(ains5_cache_buf);

    ains5_process(ains5_cache_buf);
    ains5_process(ains5_cache_buf);
    ains5_process(ains5_cache_buf);
    ains5_process(ains5_cache_buf);
    ains5_process(ains5_cache_buf);

//    print_r(ains5_cache_buf, 480);
    memset(&ains5_cache_buf[0], 0, 240*8);

}

AT(.text.ains5_set.mute)
void ains5_mic_mute_set(uint8_t mute)
{
    ains5_mic_cfg.mute = mute;
    if(mute) {
        while(ains5_mic_cfg.kick_proc_done){
            printf("#");
        }
        //tog_buf_init(&ains5_tbuf, ains5_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    }
}

AT(.text.ains5_get.mute)
uint8_t ains5_mic_mute_get(void)
{
    return ains5_mic_cfg.mute;
}
#else
void ains5_mic_proc_cb(void){}
#endif
