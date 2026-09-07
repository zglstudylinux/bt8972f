#include "include.h"
#include "api_alg.h"
#include "ylcrn_L2.h"

/*
 * 文件名称: ylcrn_L2.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.ylcrn_L2);
    AT(.rodata.ylcrn_L2)
    AT(.text.ylcrn_L2_proc)
    AT(.text.ylcrn_L2_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata : 14k
    buf           : 18k
    npu           ：100k
    time          : 6.82ms /10ms npu+float+hwfft
 */

#if YLCRN_L2_EN

#define YLCRN_L2_INFO_PRINT          0
#define FRAME_LEN                    480                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          240                         //每次存取帧长

static struct tog_bug_tag ylcrn_L2_tbuf AT(.buf.ylcrn_L2);                    //乒乓buf控制
static mic_pcm_t ylcrn_L2_cache_buf[FRAME_LEN*2] AT(.buf.ylcrn_L2);           //乒乓buf缓存
static mic_pcm_t ylcrn_L2_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.ylcrn_L2);     //输出buf中转缓存
static void *ylcrn_L2_proc_ptr = ylcrn_L2_cache_buf;

static ylcrn_L2_cb_t ylcrn_L2_cb AT(.buf.ylcrn_L2);
static ylcrn_L2_mic_cfg_t ylcrn_L2_mic_cfg AT(.buf.ylcrn_L2);

#if YLCRN_L2_INFO_PRINT
AT(.com_text.ylcrn_L2)
const char ylcrn_L2_info[] = "YLCRN_L2_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.ylcrn_L2_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(ylcrn_L2_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.ylcrn_L2_proc)
void ylcrn_L2_mic_audio_input(u8 *ptr, u32 samples, u32 params)
{
    if (!ylcrn_L2_mic_cfg.mute && wireless_cb.alg_en) {

        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if(tog_buf_get(&ylcrn_L2_tbuf, (u8 *)ylcrn_L2_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&ylcrn_L2_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(tog_buf_put(&ylcrn_L2_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                ylcrn_L2_proc_ptr = tog_bug_get_w_block(&ylcrn_L2_tbuf);
                tog_buf_wr_toggle(&ylcrn_L2_tbuf);
                ylcrn_L2_mic_cfg.kick_proc_done++;
                ylcrn_L2_mic_proc_kick_start();
            }

            memcpy(ptr, ylcrn_L2_tmp_buf, rlen*sizeof(mic_pcm_t));
            if(ylcrn_L2_mic_cfg.callback) {
                ylcrn_L2_mic_cfg.callback((void *)ptr, rlen, params);
            }

            samples -= rlen;
        }
    } else {
        if(ylcrn_L2_mic_cfg.callback) {
            ylcrn_L2_mic_cfg.callback((void *)ptr, samples, params);
        }
    }

}

//ylcrn_L2算法启动计算 放在低优先级现场处理
AT(.text.ylcrn_L2_proc)
void ylcrn_L2_mic_proc_cb(void)
{
    mic_pcm_t *rptr = ylcrn_L2_proc_ptr;
#if YLCRN_L2_INFO_PRINT
    info_printf();
#endif
#if YLCRN_L2_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 0, 1);
#endif

    ylcrn_L2_ns_process(rptr);

#if YLCRN_L2_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 1, 1);
#endif

    ylcrn_L2_mic_cfg.kick_proc_done--;
}

AT(.text.ylcrn_L2_set)
void ylcrn_L2_mic_output_callback_set(audio_callback_t callback)
{
    ylcrn_L2_mic_cfg.callback = callback;
}

AT(.text.ylcrn_L2_init)
void ylcrn_L2_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&ylcrn_L2_mic_cfg, 0, sizeof(ylcrn_L2_mic_cfg));
    tog_buf_init(&ylcrn_L2_tbuf, ylcrn_L2_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));

    ylcrn_L2_mic_param_set(0);

    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
    ylcrn_L2_mic_mute_set(1);
#if YLCRN_L2_DUMP_EN
    audio_dump_init();
#endif
}

AT(.text.ylcrn_L2_exit)
void ylcrn_L2_mic_exit(void)
{

}

AT(.text.ylcrn_L2_set.param)
void ylcrn_L2_mic_param_set(s16 ylcrn_L2_nt)
{
    memset((uint8_t *)&ylcrn_L2_cb, 0, sizeof(ylcrn_L2_cb));

	ylcrn_L2_cb.overdrive			= 1.0f; //32768;
	ylcrn_L2_cb.adaptive_floor		= 0;
	ylcrn_L2_cb.denoiseBound			= 0.055f;//1800.0f / 32768.0f;
	ylcrn_L2_cb.denoiseBoundLow		= 0.03357f;//1100.0f / 32768.0f;
	ylcrn_L2_cb.denoiseBoundHigh		= 0.0793457f;//2600.0f / 32768.0f;
	ylcrn_L2_cb.denoiseBoundProb		= 0.3357f;//11000.0f / 32768.0f;
	ylcrn_L2_cb.smooth_en			= 0;
	ylcrn_L2_cb.modelUpdatePars0		= 2;
	ylcrn_L2_cb.prior_opt_idx		= 10;
	ylcrn_L2_cb.prior_opt_ada_en		= 1;

	ylcrn_L2_cb.sin_dnn_en			= 1;
	ylcrn_L2_cb.sin_all_en			= 0;
	ylcrn_L2_cb.sin_all_len			= 0;

	ylcrn_L2_cb.low_noise_range		= 11;
	ylcrn_L2_cb.spp_fre_p			= 0.366211f;//12000.0f / 32768.0f;
	ylcrn_L2_cb.spp_fre_len			= 16;
	ylcrn_L2_cb.high_gain_len        = 32;
	ylcrn_L2_cb.music_lev			= 16.0f;
	ylcrn_L2_cb.prev_noise_len		= 11;
	ylcrn_L2_cb.gain_assign			= 0.81378174f;//26666.0f / 32768.0f;
	ylcrn_L2_cb.hi_gain_mode			= 0;

    ylcrn_L2_ns_init(&ylcrn_L2_cb);

}

AT(.text.ylcrn_L2_set.mute)
void ylcrn_L2_mic_mute_set(uint8_t mute)
{
    ylcrn_L2_mic_cfg.mute = mute;
    if (mute) {
        while(ylcrn_L2_mic_cfg.kick_proc_done){
            printf("#");
        }
//        npu_exit();
        tog_buf_init(&ylcrn_L2_tbuf, ylcrn_L2_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
    }
}

AT(.text.ylcrn_L2_get.mute)
uint8_t ylcrn_L2_mic_mute_get(void)
{
    return ylcrn_L2_mic_cfg.mute;
}
#else
void ylcrn_L2_mic_proc_cb(void){}
#endif
