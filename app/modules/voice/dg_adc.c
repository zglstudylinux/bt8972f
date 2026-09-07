#include "include.h"
#include "api_alg.h"
#include "dg_adc.h"

/*
 * 文件名称: dg_adc.c
 * 功能描述: 本文件为软件dnn_L1处理模块
    AT(.buf.dg_adc);
    AT(.rodata.dg_adc)
    AT(.text.dg_adc_pro)
    AT(.text.dg_adc_init)

    注意mic_pcm_t 实际配置类型

 ****************************************************************************************
    code + rodata : 1030
    buf           : 5852
    npu           ：0
    time          :380us(主频100MHz)
 */

#if DG_ADC_EN

#define DG_ADC_INFO_PRINT             0
#define FRAME_LEN                    240                         //算法处理帧长
#define PROCESS_OUT_SAMPLES          480                         //每次存取帧长

static mic_pcm_t dg_adc_cache_buf[FRAME_LEN*2] AT(.buf.dg_adc);
static mic_pcm_t dg_adc_tmp_buf[PROCESS_OUT_SAMPLES] AT(.buf.dg_adc);
static mic_pcm_t dg_adc_input[2] AT(.buf.dg_adc);

static dg_adc_mic_cfg_t dg_adc_mic_cfg AT(.buf.dg_adc);

typedef struct {
    u8 dg_adc_cache[PROCESS_OUT_SAMPLES*4];

    ring_buf_t dg_adc_ring_buf;
} dg_adc_buf_t;

static dg_adc_buf_t dg_adc_buf AT(.buf.dg_adc);

const rbuf_tbl_t dg_adc_buf_tbl[1] = {
    {
        .buf = dg_adc_buf.dg_adc_cache,
        .size = PROCESS_OUT_SAMPLES*4,
    },
};


#if DG_ADC_INFO_PRINT
AT(.com_text.dg_adc)
const char dg_adc_info[] = "DG_ADC_EN samples = %d, isrcnt = %d (SR_%d) %d\n";

AT(.text.dg_adc_proc)
static void info_printf(void)
{
    static u32 ticks = 0;
    static u32 isr_cnt = 0;
    isr_cnt++;
    if(tick_check_expire(ticks,1000)) {
        my_printf(dg_adc_info, FRAME_LEN, isr_cnt, FRAME_LEN*isr_cnt, sizeof(mic_pcm_t));
        isr_cnt = 0;
        ticks = tick_get();
    }
}
#endif

AT(.text.dg_adc_proc)
void dg_adc_mic_audio_input(u8 *ptr, u32 samples, u32 params)
{
    if (!dg_adc_mic_cfg.mute && wireless_cb.alg_en) {
        while(samples > 0) {
            uint rlen = (samples > PROCESS_OUT_SAMPLES)? PROCESS_OUT_SAMPLES : samples*2;
            ring_buf_t *ring_buf = &(dg_adc_buf.dg_adc_ring_buf);
            ring_buf_put(ring_buf, (u8 *)ptr, rlen * 4);
            // 填充block, true表示算法攒帧完毕, kick低优先级线程处理算法
            if(ring_buf_get_total(ring_buf) >= (WIRELESS_MIC_SAMPLES_SELECT*2)) {
                two_adc_agc_proc_kick_start();
            }

            memcpy(ptr, dg_adc_tmp_buf, samples*sizeof(mic_pcm_t));

            if(dg_adc_mic_cfg.callback) {
                dg_adc_mic_cfg.callback((void *)ptr, samples, params);
            }
            samples -= (rlen/2);
        }
    } else {

        mic_pcm_t *rptr = (mic_pcm_t *)ptr;

        for(int i=0; i<WIRELESS_MIC_SAMPLES_SELECT; i++) {
            dg_adc_tmp_buf[i] = rptr[2*i+1];
        }

        if(dg_adc_mic_cfg.callback) {
            dg_adc_mic_cfg.callback((void *)dg_adc_tmp_buf, samples, params);
        }
    }

}

//two_adc_agc算法启动计算 放在低优先级现场处理
AT(.text.dg_adc_proc)
void dg_adc_mic_proc_cb(void)
{
    ring_buf_t *ring_buf = &(dg_adc_buf.dg_adc_ring_buf);
    ring_buf = &(dg_adc_buf.dg_adc_ring_buf);
    ring_buf_get(ring_buf,(u8 *)dg_adc_cache_buf, 480 * 4);

    mic_pcm_t *rptr = dg_adc_cache_buf;

    for(int i=0; i<WIRELESS_MIC_SAMPLES_SELECT; i++) {
        dg_adc_input[1] = rptr[2*i];          //低增益部分adc数据
        dg_adc_input[0] = rptr[2*i+1];        //高增益部分adc数据
//        dg_adc_tmp_buf[i] = rptr[2*i+1];
        dg_adc_pro(dg_adc_input, (s32 *)(&dg_adc_tmp_buf[i]));
    }

}

AT(.text.dg_adc_set)
void dg_adc_mic_output_callback_set(audio_callback_t callback)
{
    dg_adc_mic_cfg.callback = callback;
}

AT(.text.dg_adc_init)
void dg_adc_mic_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset((uint8_t *)&dg_adc_mic_cfg, 0, sizeof(dg_adc_mic_cfg));
    memset(dg_adc_cache_buf, 0, FRAME_LEN*sizeof(mic_pcm_t));
    memset(dg_adc_buf.dg_adc_cache, 0, PROCESS_OUT_SAMPLES*4);
    ring_buf_init(&(dg_adc_buf.dg_adc_ring_buf), dg_adc_buf_tbl, 1, 0);
    ///算法初始化处先mute，等连上无线麦再打开/接收端发送使能再打开
    dg_adc_mic_mute_set(0);
    dg_adc_init();

}

AT(.text.dg_adc_exit)
void dg_adc_mic_exit(void)
{
}

AT(.text.dg_adc_set.mute)
void dg_adc_mic_mute_set(uint8_t mute)
{
    dg_adc_mic_cfg.mute = mute;
}

AT(.text.dg_adc_get.mute)
uint8_t dg_adc_mic_mute_get(void)
{
    return dg_adc_mic_cfg.mute;
}
#else
void dg_adc_mic_proc_cb(void){}
#endif
