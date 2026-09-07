#include "include.h"
#include "api_alg.h"
#include "ylcrn_L3.h"

/*
 * 文件名称: ylcrn_L3.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.ylcrn_L3);
    AT(.rodata.ylcrn_L3)
    AT(.text.ylcrn_L3_proc)
    AT(.text.ylcrn_L3_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata : 21402
    buf           : 41736
    npu           ：100k
    time          : 12.8ms/20ms   no hw_fft
 */

#if YLCRN_L3_EN

#define YLCRN_L3_INFO_PRINT          0
#define FRAME_LEN                    960                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          240                         //每次存取帧长

static struct tog_bug_tag ylcrn_L3_tbuf AT(.buf.ylcrn_L3);                    //乒乓buf控制
static mic_pcm_t ylcrn_L3_cache_buf[FRAME_LEN*2] AT(.buf.ylcrn_L3);           //乒乓buf缓存
static mic_pcm_t ylcrn_L3_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.ylcrn_L3);     //输出buf中转缓存
static void *ylcrn_L3_proc_ptr = ylcrn_L3_cache_buf;

static ylcrn_L3_cb_t ylcrn_L3_cb AT(.buf.ylcrn_L3);
static ylcrn_L3_mic_cfg_t ylcrn_L3_mic_cfg AT(.buf.ylcrn_L3);

#if YLCRN_L3_INFO_PRINT
AT(.com_text.ylcrn_L3)
const char ylcrn_L3_info[] = "YLCRN_L3_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.ylcrn_L3_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(ylcrn_L3_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.ylcrn_L3_proc)
void ylcrn_L3_mic_audio_input(u8 *ptr, u32 samples, u32 params)
{
    if (!ylcrn_L3_mic_cfg.mute && wireless_cb.alg_en) {

        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&ylcrn_L3_tbuf, (u8 *)ylcrn_L3_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&ylcrn_L3_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&ylcrn_L3_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                ylcrn_L3_proc_ptr = tog_bug_get_w_block(&ylcrn_L3_tbuf);
                tog_buf_wr_toggle(&ylcrn_L3_tbuf);
                ylcrn_L3_mic_proc_kick_start();
            }

            memcpy(ptr, ylcrn_L3_tmp_buf, rlen*sizeof(mic_pcm_t));
            if(ylcrn_L3_mic_cfg.callback) {
                ylcrn_L3_mic_cfg.callback((void *)ptr, rlen, params);
            }

            samples -= rlen;
        }
    } else {
        if(ylcrn_L3_mic_cfg.callback) {
            ylcrn_L3_mic_cfg.callback((void *)ptr, samples, params);
        }
    }

}

//ylcrn_L3算法启动计算 放在低优先级现场处理
AT(.text.ylcrn_L3_proc)
void ylcrn_L3_mic_proc_cb(void)
{
    mic_pcm_t *rptr = ylcrn_L3_proc_ptr;

#if YLCRN_L3_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 0, 1);
#endif

#if YLCRN_L3_INFO_PRINT
    info_printf();
#endif

    ylcrn_L3_ns_process(rptr);

#if YLCRN_L3_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 1, 1);
#endif
}

AT(.text.ylcrn_L3_set)
void ylcrn_L3_mic_output_callback_set(audio_callback_t callback)
{
    ylcrn_L3_mic_cfg.callback = callback;
}

AT(.text.ylcrn_L3_init)
void ylcrn_L3_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    tog_buf_init(&ylcrn_L3_tbuf, ylcrn_L3_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    memset((uint8_t *)&ylcrn_L3_mic_cfg, 0, sizeof(ylcrn_L3_mic_cfg));
    ylcrn_L3_mic_param_set(1800);

    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
    ylcrn_L3_mic_mute_set(1);


#if YLCRN_L3_DUMP_EN
    audio_dump_init();
#endif
}

AT(.text.ylcrn_L3_exit)
void ylcrn_L3_mic_exit(void)
{

}

AT(.text.ylcrn_L3_set.param)
void ylcrn_L3_mic_param_set(s16 ylcrn_L3_nt)
{
    memset((uint8_t *)&ylcrn_L3_cb, 0, sizeof(ylcrn_L3_cb));
	ylcrn_L3_cb.overdrive			= 32768;
	ylcrn_L3_cb.adaptive_floor		= 0;
	ylcrn_L3_cb.denoiseBound		= 1800;
	ylcrn_L3_cb.denoiseBoundLow		= 1800;
	ylcrn_L3_cb.denoiseBoundHigh	= 1800;
	ylcrn_L3_cb.denoiseBoundProb	= 8000;
	ylcrn_L3_cb.smooth_en			= 1;
	ylcrn_L3_cb.modelUpdatePars0	= 2;
	ylcrn_L3_cb.prior_opt_idx		= 3;
	ylcrn_L3_cb.prior_opt_ada_en	= 1;
	ylcrn_L3_cb.quan_gap			= 3277;
	ylcrn_L3_cb.quan_gap_low_len	= 6;
	ylcrn_L3_cb.quan_gap_low		= 3277;
	ylcrn_L3_cb.lquantile_sm		= 26214;
	ylcrn_L3_cb.factor				= 30*32768;

	ylcrn_L3_cb.delta_k_up		    = 1;
	ylcrn_L3_cb.sin_dnn_en			= 0;
	ylcrn_L3_cb.sin_all_en			= 0;
	ylcrn_L3_cb.sin_all_len			= 512;

	ylcrn_L3_cb.low_noise_range		= 16;
	ylcrn_L3_cb.spp_fre_p			= 6000;
	ylcrn_L3_cb.spp_fre_len			= 64;

	ylcrn_L3_cb.high_gain_len        = 64;
	ylcrn_L3_cb.gain_assign_len		= 512;
	ylcrn_L3_cb.gain_assign			= 26666;
	ylcrn_L3_cb.nn_only_len			= 0;
	ylcrn_L3_cb.music_lev			= 9;
	ylcrn_L3_cb.intensity			= 0;
	ylcrn_L3_cb.mask_vad_fre_thr	= 0;
	ylcrn_L3_cb.spp_max_en			= 1;

    ylcrn_L3_ns_init(&ylcrn_L3_cb);

}

AT(.text.ylcrn_L3_set.mute)
void ylcrn_L3_mic_mute_set(uint8_t mute)
{
    ylcrn_L3_mic_cfg.mute = mute;
    if(mute) {
        tog_buf_init(&ylcrn_L3_tbuf, ylcrn_L3_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    }
}

AT(.text.ylcrn_L3_get.mute)
uint8_t ylcrn_L3_mic_mute_get(void)
{
    return ylcrn_L3_mic_cfg.mute;
}
#else
void ylcrn_L3_mic_proc_cb(void){}
#endif
