/*****************************************************************************
 * Module    : thread_dec
 * File      : thread_dec.c
 * Function  : 高优先级线程回调函数，处理快速的算法
 * Attention : 要求在1~2ms内完成，不允许flash缺页、信号量等阻塞
 *****************************************************************************/

#include "include.h"
#include "os_thread.h"

AT(.com_text.thread.decoder)
void thread_dec_proc_msg_cb(u32 msg)
{
    switch (msg) {
        case KICK_DEC_PRIO_TRANS:
            decoder_prio_trans_process(0);
            break;
        case KICK_DEC_PRIO_TRANS1:
            decoder_prio_trans_process(1);
            break;

        case KICK_DEC_WARNING:
#if WARNING_WSBC_RES_EN
            warning_dec_proc();
#endif
            break;

        default:
            break;
    }
}
