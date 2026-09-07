#include "include.h"
#include "api_alg.h"
#include "agc.h"

/*
 * 文件名称: agc.c
 * 功能描述: 本文件为软件agc处理模块
    AT(.buf.agc);
    AT(.rodata.agc)
    AT(.text.agc_proc)
    AT(.text.agc_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata : 3k
    buf           : 2.5k
    time          : 0.269ms/20ms
 */

#if AGC_EN

#define TOG_BUF_BYPASS               1                           //是否bypass togbuf流程 减少链路演示和还缓存buf
#define AGC_INFO_PRINT               0
#define FRAME_LEN                    240                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          240                         //每次存取帧长

#if !TOG_BUF_BYPASS
static struct tog_bug_tag agc_tbuf AT(.buf.agc);                    //乒乓buf控制
static mic_pcm_t agc_cache_buf[FRAME_LEN*2] AT(.buf.agc);           //乒乓buf缓存
static mic_pcm_t agc_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.agc);     //输出buf中转缓存
static void *agc_proc_ptr = agc_cache_buf;
#endif

static agc_cb_t agc_cb AT(.buf.agc);
static agc_mic_cfg_t agc_mic_cfg AT(.buf.agc);

#if AGC_INFO_PRINT
AT(.com_text.agc)
const char agc_info[] = "AGC_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.agc_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(agc_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.agc_proc)
void agc_mic_audio_input(u8 *ptr, u32 samples, u32 params)
{
    if (!agc_mic_cfg.mute && wireless_cb.alg_en) {
#if !TOG_BUF_BYPASS
        while (samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples;
            if (tog_buf_get(&agc_tbuf, (u8 *)agc_tmp_buf, rlen*sizeof(mic_pcm_t))) {
                tog_buf_rd_toggle(&agc_tbuf);
            }

            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if (tog_buf_put(&agc_tbuf, ptr, rlen*sizeof(mic_pcm_t))) {
                agc_proc_ptr = tog_bug_get_w_block(&agc_tbuf);
                tog_buf_wr_toggle(&agc_tbuf);
                agc_mic_proc_kick_start();
            }

            memcpy(ptr, agc_tmp_buf, rlen*sizeof(mic_pcm_t));
            if (agc_mic_cfg.callback) {
                agc_mic_cfg.callback((void *)ptr, rlen, params);
            }

            samples -= rlen;
        }
#else
        agc_proc_24bit( (mic_pcm_t*)ptr, (mic_pcm_t*)ptr, FRAME_LEN);
        if (agc_mic_cfg.callback) {
            agc_mic_cfg.callback((void *)ptr, samples, params);
        }
#endif
    } else {
        if (agc_mic_cfg.callback) {
            agc_mic_cfg.callback((void *)ptr, samples, params);
        }
    }

}

//agc算法启动计算 放在低优先级现场处理
AT(.text.agc_proc)
void agc_mic_proc_cb(void)
{
#if !TOG_BUF_BYPASS

    mic_pcm_t *rptr = agc_proc_ptr;

#if AGC_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 0, 1);
#endif

#if AGC_INFO_PRINT
    info_printf();
#endif

    agc_proc_24bit(rptr, rptr, FRAME_LEN);

#if AGC_DUMP_EN
    audio_dump_input(rptr, FRAME_LEN, 1, 1);
#endif

#endif
}

AT(.text.agc_set)
void agc_mic_output_callback_set(audio_callback_t callback)
{
    agc_mic_cfg.callback = callback;
}

AT(.text.agc_init)
void agc_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
#if !TOG_BUF_BYPASS
    tog_buf_init(&agc_tbuf, agc_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
#endif
    memset((uint8_t *)&agc_mic_cfg, 0, sizeof(agc_mic_cfg));

	// parameter init outside
	agc_cb.agc_en					= 1;
	agc_cb.sampleHzIn				= 48000;
	agc_cb.bit						= 24;

	agc_cb.compress_agcDb			= 9; // 最大增益 单位dB
	agc_cb.target_agcDbfs			= 3; // 目标电平 设置3会将信号目标设置为-3dB
	agc_cb.low_signal_en			= 0;
	agc_cb.max_gain					= 10;
	agc_cb.capacitorSlow_default	= 10737418; // 根据最小输入电平调整，Q30。例如最小幅度为0.1, 则设为0.1f * 32768 * 32768.

	AgcInit_24bit(&agc_cb);

	///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
    agc_mic_mute_set(1);

#if AGC_DUMP_EN
    audio_dump_init();
#endif
}

AT(.text.agc_exit)
void agc_mic_exit(void)
{

}

AT(.text.agc_set.param)
void agc_mic_param_set(s16 agc_nt)
{

}

AT(.text.agc_set.mute)
void agc_mic_mute_set(uint8_t mute)
{
    agc_mic_cfg.mute = mute;
    if(mute) {
#if !TOG_BUF_BYPASS
        tog_buf_init(&agc_tbuf, agc_cache_buf, FRAME_LEN*sizeof(mic_pcm_t));
#endif
    }
}

AT(.text.agc_get.mute)
uint8_t agc_mic_mute_get(void)
{
    return agc_mic_cfg.mute;
}
#else
void agc_mic_proc_cb(void){}
#endif
