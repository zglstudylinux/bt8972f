#include "include.h"
#include "dac0_out.h"

static dac0_out_cfg_t dac0_out_cfg;
u32 aufifo0_cnt = 0;

AT(.text.dac0_proc)
void dac0_out_audio_input(u8 *ptr, u32 samples, u32 params)
{
    u32 pcm_mode = params & 0xffff;
    bool is_24bit = pcm_mode & PCM_24BIT;
    uint nch = pcm_mode & PCM_CHMASK;

    if (!dac0_out_cfg.mute) {
        aubuf0_dma_w4_done();
        aubuf0_dma_kick(ptr, samples, nch, is_24bit);
    }

    if (dac0_out_cfg.callback) {
        dac0_out_cfg.callback(ptr, samples, params);
    }
}

AT(.text.dac0_proc)
void dac0_get_fifocnt(u32 tick_cnt)
{
    aufifo0_cnt = AUBUF0FIFOCNT >> 18;
}

AT(.text.dac0_proc.fifocnt)
void dac0_play_sync_fifocnt(u16 high_thr, u16 low_thr)
{
    u16 fifo_cnt = aufifo0_cnt;

    u32 phase = 0;
    if (fifo_cnt <= low_thr) {
        phase = 0xffffff - ((low_thr - fifo_cnt)*0xf);
    } else if (fifo_cnt >= high_thr) {
        phase = 0x0 + ((fifo_cnt - high_thr)*0xf);
    } else {
        phase = 0x0000;
    }

    dac_phase_set(phase);
}

AT(.text.dac0_out)
void dac0_out_audio_output_callback_set(audio_callback_t callback)
{
    dac0_out_cfg.callback = callback;
}

AT(.text.dac0_out)
void dac0_out_audio_mute_set(uint8_t mute)
{
//    if(mute == 0) {
//        dac0_aubuf_init();
//        dac0_spr_set(dac0_out_cfg.sample_rate);
//        dac0_vol_set(DIG_N0DB);
//        dac0_fade_in();
//    }
    dac0_out_cfg.mute = mute;
}

AT(.text.dac0_out)
void dac0_out_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&dac0_out_cfg, 0, sizeof(dac0_out_cfg));
//    dac0_out_param_cfg_t *dac0_out_param_temp_ptr = dac0_out_param_get_by_sam_rate(sample_rate);
//    dac0_out_cfg.low_thr = dac0_out_param_temp_ptr->low_thr;
//    dac0_out_cfg.high_thr = dac0_out_param_temp_ptr->high_thr;
    dac0_out_cfg.sample_rate = sample_rate;

    dac_aubuf_init();
    aubuf0_dma_init();
    dac_spr_set(sample_rate);
    dac_phase_set(0);
    dac0_out_audio_mute_set(0);

    dac_vol_set(DIG_N0DB);
    dac_fade_in();
}
