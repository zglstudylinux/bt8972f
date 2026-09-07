#ifndef __OS_THREAD_
#define __OS_THREAD_

#include "decoder_prio_trans.h"
#include "encoder_prio_trans.h"
#include "alg_prio_trans.h"

//------------------------------------------------------------------------------------------
//高优先级（要求在一帧时间内完成）
enum {
    KICK_DEC_PRIO_TRANS     = 0,
    KICK_DEC_PRIO_TRANS1,

    KICK_DEC_WARNING        = 0x20,
};

void os_send_dec_proc_msg(u32 msg);
#define kick_decoder_prio_trans(idx)            os_send_dec_proc_msg(KICK_DEC_PRIO_TRANS + (idx))


//------------------------------------------------------------------------------------------
//中优先级（要求在一帧时间内完成）
enum {
    KICK_ENC_PRIO_TRANS     = 0,
    KICK_ENC_BUF_PROC,
};

void os_send_enc_proc_msg(u32 msg);
#define kick_encoder_prio_trans()               os_send_enc_proc_msg(KICK_ENC_PRIO_TRANS)


//------------------------------------------------------------------------------------------
//低优先级
enum {
    KICK_ALG_PRIO_TRANS = 0,
    YLCRN_L3_PROCESS,
    YLCRN_L2_PROCESS,
    AINS4_PROCESS,
    AINS5_PROCESS,
    DNR_FRE_PROCESS,
    AGC_PROCESS,
    DG_ADC_PROCESS,
    LOCAL_MIC_KICK,
};

void os_alg_sem_pend(uint timeout);             //alg线程等待信号量，timeout时间>=2（单位5ms）
void os_alg_sem_post(void);
void os_send_alg_proc_msg(u32 msg);
#define kick_alg_prio_trans()                   os_send_alg_proc_msg(KICK_ALG_PRIO_TRANS)
#define ylcrn_L3_mic_proc_kick_start()          os_send_alg_proc_msg(YLCRN_L3_PROCESS)
#define ylcrn_L2_mic_proc_kick_start()          os_send_alg_proc_msg(YLCRN_L2_PROCESS)
#define ains4_mic_proc_kick_start()             os_send_alg_proc_msg(AINS4_PROCESS)
#define ains5_mic_proc_kick_start()             os_send_alg_proc_msg(AINS5_PROCESS)
#define dnr_fre_mic_proc_kick_start()           os_send_alg_proc_msg(DNR_FRE_PROCESS)
#define agc_mic_proc_kick_start()               os_send_alg_proc_msg(AGC_PROCESS)
#define local_mic_proc_start()                  os_send_alg_proc_msg(LOCAL_MIC_KICK)
#define two_adc_agc_proc_kick_start()           os_send_alg_proc_msg(DG_ADC_PROCESS)

#endif // __THREAD_ALG_
