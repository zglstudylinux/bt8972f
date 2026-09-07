/*****************************************************************************
 * Module    : thread_enc
 * File      : thread_enc.c
 * Function  : 高优先级线程回调函数，处理快速的算法
 * Attention : 要求在1~2ms内完成，不允许flash缺页、信号量等阻塞
 *****************************************************************************/

#include "include.h"
#include "os_thread.h"

void encoder_prio_trans_process(void);
void src_buf_process(void);

#if !ADAPTER_USB_SPK_TX_EN
AT(.text.src_proc.src_buf.proc) WEAK
void src_buf_process(void)
{}
#endif

AT(.com_text.thread.coder)
void kick_src_buf_proc(void)
{
    os_send_enc_proc_msg(KICK_ENC_BUF_PROC);
}

AT(.com_text.thread.coder)
void thread_enc_proc_msg_cb(u32 msg)
{
    switch (msg) {
        case KICK_ENC_PRIO_TRANS:
            encoder_prio_trans_process();
            break;
        case KICK_ENC_BUF_PROC:
            src_buf_process();
        default:
            break;
    }
}
