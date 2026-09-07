#include "include.h"
#include "dac1_out.h"

static dac1_out_cfg_t dac1_out_cfg;
u32 aufifo1_cnt = 0;

AT(.text.dac1_proc)
void dac1_out_audio_input(u8 *ptr, u32 samples, u32 params)
{
    u32 pcm_mode = params & 0xffff;
    bool is_24bit = pcm_mode & PCM_24BIT;
    uint nch = pcm_mode & PCM_CHMASK;

    if (!dac1_out_cfg.mute) {
        aubuf1_dma_w4_done();
        aubuf1_dma_kick(ptr, samples, nch, is_24bit);
    }

    if (dac1_out_cfg.callback) {
        dac1_out_cfg.callback(ptr, samples, params);
    }
}

AT(.text.dac1_proc)
void dac1_get_fifocnt(u32 tick_cnt)
{
    aufifo1_cnt = AUBUF1FIFOCNT >> 18;
}

AT(.text.dac1_proc.fifocnt)
void dac1_play_sync_fifocnt(u16 high_thr, u16 low_thr)
{
    u16 fifo_cnt = aufifo1_cnt;

    u32 phase = 0;
    if (fifo_cnt <= low_thr) {
        phase = 0xffff00;
    } else if (fifo_cnt >= high_thr) {
        phase = 0xff;
    } else {
        phase = 0x0000;
    }

    dac1_phase_set(phase);
}

AT(.text.dac1_out)
void dac1_out_audio_output_callback_set(audio_callback_t callback)
{
    dac1_out_cfg.callback = callback;
}

AT(.text.dac1_out)
void dac1_out_audio_mute_set(uint8_t mute)
{
//    if(mute == 0) {
//        dac1_aubuf_init();
//        dac1_spr_set(dac1_out_cfg.sample_rate);
//        dac1_vol_set(DIG_N0DB);
//        dac1_fade_in();
//    }
    dac1_out_cfg.mute = mute;
}

AT(.text.dac1_out)
void dac1_out_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&dac1_out_cfg, 0, sizeof(dac1_out_cfg));
//    dac1_out_param_cfg_t *dac1_out_param_temp_ptr = dac1_out_param_get_by_sam_rate(sample_rate);
//    dac1_out_cfg.low_thr = dac1_out_param_temp_ptr->low_thr;
//    dac1_out_cfg.high_thr = dac1_out_param_temp_ptr->high_thr;
    dac1_out_cfg.sample_rate = sample_rate;

    dac1_aubuf_init();
    dac1_spr_set(sample_rate);
    dac1_phase_set(0);
    dac1_out_audio_mute_set(0);

    dac1_vol_set(DIG_N0DB);
    dac1_fade_in();
}
