#include "include.h"
#include "api_alg.h"
#include "ains4.h"

/*
 * 文件名称: ains4.c
 * 功能描述: 本文件为软件 ains4 处理模块
    AT(.buf.ains4);
    AT(.rodata.ains4)
    AT(.rodata1.ains4)
    AT(.text.ains4_proc)
    AT(.text.ains4_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata :
    buf           :
    npu           ：
    time          :   6.42ms / 10ms no hwfft
 */

#if AINS4_EN

#define AINS4_INFO_PRINT             1
#define FRAME_LEN                    480                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          240                         //每次存取帧长

static struct tog_bug_tag ains4_tbuf AT(.buf.ains4);                    //乒乓buf控制
static mic_pcm_t ains4_cache_buf[FRAME_LEN*2] AT(.buf.ains4);           //乒乓buf缓存
static mic_pcm_t ains4_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.ains4);     //输出buf中转缓存
static void *ains4_proc_ptr = ains4_cache_buf;

static ains4_cb_t ains4_cb AT(.buf.ains4);
static ains4_mic_cfg_t ains4_mic_cfg AT(.bss.ains4);

#if AINS4_INFO_PRINT
AT(.com_text.ains4)
const char ains4_info[] = "AINS4_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.ains4_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(ains4_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.com_text.ains4_proc)
void ains4_mic_audio_input(u8 *ptr, u32 samples, u32 params)
{
    if (!ains4_mic_cfg.mute && wireless_cb.alg_en) {

        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&ains4_tbuf, (u8 *)ains4_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&ains4_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&ains4_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                ains4_proc_ptr = tog_bug_get_w_block(&ains4_tbuf);
                tog_buf_wr_toggle(&ains4_tbuf);
                ains4_mic_cfg.kick_proc_done++;
                ains4_mic_proc_kick_start();

            }

            memcpy(ptr, ains4_tmp_buf, rlen*sizeof(mic_pcm_t));
            if(ains4_mic_cfg.callback) {
                ains4_mic_cfg.callback((void *)ptr, rlen, params);
            }

            samples -= rlen;
        }
    } else {
        if(ains4_mic_cfg.callback) {
            ains4_mic_cfg.callback((void *)ptr, samples, params);
        }
    }

}

//ains4算法启动计算 放在低优先级现场处理
AT(.text.ains4_proc)
void ains4_mic_proc_cb(void)
{
    mic_pcm_t *rptr = ains4_proc_ptr;

#if AINS4_INFO_PRINT
    info_printf();
#endif
#if AINS4_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 0, 1);
#endif

    ains4_process(rptr);

#if AINS4_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 1, 1);
#endif
    ains4_mic_cfg.kick_proc_done--;
}

AT(.text.ains4_set)
void ains4_mic_output_callback_set(audio_callback_t callback)
{
    ains4_mic_cfg.callback = callback;
}

AT(.text.ains4_init)
void ains4_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&ains4_mic_cfg, 0, sizeof(ains4_mic_cfg));

    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
    ains4_mic_mute_set(0);

#if AINS4_DUMP_EN
    audio_dump_init();
#endif

    ains4_mic_param_set(1000);
//    ains4_mic_mute_set(0);
}

AT(.text.ains4_exit)
void ains4_mic_exit(void)
{

}


AT(.text.ains4_set.param)
void ains4_mic_param_set(s16 ains4_nt)
{
    load_code_wl_ains4();
    tog_buf_init(&ains4_tbuf, ains4_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    memset(&ains4_cb, 0, sizeof(ains4_cb));

#if 1
	u8 mode, nr_level;
	mode = 0;
	nr_level = 2;
	//ains4_cb.noise_db2			   = -15;
	ains4_cb.yuan_en				= 0;

	ains4_cb.snr_thr               = (32768*5)>>1;
	if(mode==1){
		if(ains4_cb.yuan_en==1){
			ains4_cb.denoiseBound		    = 30000;  //if yuan_en=1,21000, else 12000
		}
		else{
			ains4_cb.denoiseBound		    = 30000;
		}
		ains4_cb.music_lev				= 6;
		ains4_cb.overdrive_adapt_en    = 1;
		ains4_cb.overdrive			    = 32768/4;
		ains4_cb.gain_ceil				= 30000;
		ains4_cb.noise_min_en			= 1;
		//ains4_cb.smooth_logLrt  		= 0;
	}
	else{                                             //enc
		if(ains4_cb.yuan_en==1){
			ains4_cb.denoiseBound		    = 6000;  //
		}
		else{
			ains4_cb.denoiseBound		    = 6000;
		}
		ains4_cb.music_lev				 = 11;
		ains4_cb.overdrive_adapt_en     = 0;
		ains4_cb.gain_ceil				= 32767;
		ains4_cb.noise_min_en			= 0;
	}
	ains4_cb.overdrive			    = 32768;
	ains4_cb.speech_update			= 32440;
	ains4_cb.alp1_dd				= 0x7d71;//QCONST16(0.98f, 15);
	ains4_cb.smooth_en			    = 1;
	ains4_cb.modelUpdatePars0	    = 1;//0:no use HIST 1:only update one time first 2：always update
	ains4_cb.gainHB_rd			    = 32767;//0-32767
	ains4_cb.delta_k_up		    = 0;
	//ains4_cb.denoiseBound_fix		= 40;//

	ains4_cb.enr_thres				= 0;
	ains4_cb.prior_opt_idx			= 10;
	ains4_cb.prior_opt_ada_en	    = 1;

	ains4_cb.prior_opt_freh        = 166;

	ains4_cb.low_fre_range			= 11;

	ains4_cb.hi_gain_len			= 128;
	if(mode==1){
		ains4_cb.lquantile_sm		= 26216;
		ains4_cb.factor			= 10*32768;
	}
	else{
		ains4_cb.lquantile_sm		= 24578;
		ains4_cb.factor			= 30*32768;
	}

	ains4_cb.quan_gap				= 3277;
	ains4_cb.quan_gap_low_len		= 6;
	ains4_cb.quan_gap_low			= 3277;
	//ains4_cb.speech_update			= 32440;
	//ains4_cb.smooth_v				= 27853; // 0.85f
	//ains4_cb.enr_mean_max_en		= 1;//1:mean   0:max
	//ains4_cb.enr_nr_thr			= -60;//dB
	ains4_cb.spp_en				= 1;
	ains4_cb.ymin_floor			= 200;
	ains4_cb.ymin_idx   			= 6;
	ains4_cb.noise_min_floor		= 0;

    ains4_cb.gain_ceil_ratio		= 0;//DIV32_16_Q15_Dynamic(32767, ains4_cb.gain_ceil);
	ains4_cb.spp_vad_len			= 1;
	ains4_cb.ai_vad_hard_thres	    = 12666;//19666;
	ains4_cb.qhat_assign		    = 32768;//31129;
	ains4_cb.qhat_assign_low		= 32768;//22937;
	ains4_cb.spp_max_en			= 1;
	ains4_cb.low_floor_add_l		= 15;
	ains4_cb.low_mask_floor_add	= 1000;
	ains4_cb.dd_od_ctl_en		    = 1;
	ains4_cb.ai_vad_spp_hard_en	= 0;
	ains4_cb.spp_thr_noiseprev	    = 32768;
	ains4_cb.spp_thr				= 16000;
	ains4_cb.prev_noise_len		= 6;
	ains4_cb.vad_low_noise_en		= 0;
	ains4_cb.vad_low_noise_p		= 0x2666;//QCONST16(0.3f, 15);
	ains4_cb.vad_low_max_en		= 1;
	ains4_cb.priorModelPars0		= 49152;
	ains4_cb.sin_keep_en			= 1;
	ains4_cb.sin_keep_thr			= 100000;
	ains4_cb.sin_keep_idx_lim		= 2;
	ains4_cb.sin_keep_gap			= 5;
	ains4_cb.sin_coswin_en			= 1;
#endif

    ains4_init(&ains4_cb);

}

AT(.text.ains4_set.mute)
void ains4_mic_mute_set(uint8_t mute)
{
    ains4_mic_cfg.mute = mute;
    if(mute) {
        while(ains4_mic_cfg.kick_proc_done){
            printf("#");
        }
        //tog_buf_init(&ains4_tbuf, ains4_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    }
}

AT(.text.ains4_get.mute)
uint8_t ains4_mic_mute_get(void)
{
    return ains4_mic_cfg.mute;
}
#else
void ains4_mic_proc_cb(void){}
#endif
