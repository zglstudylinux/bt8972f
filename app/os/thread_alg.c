/*****************************************************************************
 * Module    : thread_alg
 * File      : thread_alg.c
 * Function  : 低优先级线程回调函数，处理耗时的算法
 * Attention : 允许少量flash缺页
 *****************************************************************************/

#include "include.h"
#include "os_thread.h"


AT(.com_text.thread_alg)
void thread_alg_proc_msg_cb(u32 msg)
{
    switch (msg) {
        case YLCRN_L3_PROCESS:
            ylcrn_L3_mic_proc_cb();
            break;

        case YLCRN_L2_PROCESS:
            ylcrn_L2_mic_proc_cb();
            break;

        case AINS4_PROCESS:
            ains4_mic_proc_cb();
            break;

        case AINS5_PROCESS:
            ains5_mic_proc_cb();
            break;

        case DNR_FRE_PROCESS:
            dnr_fre_mic_proc_cb();
            break;

        case AGC_PROCESS:
            agc_mic_proc_cb();
            break;

        case DG_ADC_PROCESS:
            dg_adc_mic_proc_cb();
            break;

        case LOCAL_MIC_KICK:
#if ADAPTER_LOCAL_MIC_MIX_EN
            local_mic_output();
#endif

        default:
            break;
    }
}
