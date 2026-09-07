#include "include.h"
#include "decoder_prio_trans.h"


static decoder_prio_trans_cfg_t decoder_prio_trans_cfg AT(.bss.decoder_prio_trans);

AT(.com_text.decoder_prio_trans)
void decoder_prio_trans_audio_input(u8 *ptr, u32 samples, u32 params)
{
    uint idx = (params >> 16);

    //由于decoder线程处理比较及时，这里只是传递ptr，没有做缓存处理
    decoder_prio_trans_cfg.ptr      = ptr;
    decoder_prio_trans_cfg.samples  = samples;
    decoder_prio_trans_cfg.params   = params;

    //触发低一级的decoder线程，在decoder_prio_trans_process中处理
    kick_decoder_prio_trans(idx);
}

AT(.com_text.decoder_prio_trans)
void decoder_prio_trans_process(u8 idx)
{
    //decoder线程处理，触发一次处理一帧
    if (decoder_prio_trans_cfg.callback) {
        decoder_prio_trans_cfg.callback(decoder_prio_trans_cfg.ptr, decoder_prio_trans_cfg.samples, decoder_prio_trans_cfg.params);
    }
}

AT(.text.decoder_prio_trans)
void decoder_prio_trans_audio_output_callback_set(audio_callback_t callback)
{
    decoder_prio_trans_cfg.callback = callback;
}

AT(.text.decoder_prio_trans)
void decoder_prio_trans_audio_mute_set(uint8_t mute)
{
}

AT(.text.decoder_prio_trans)
void decoder_prio_trans_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&decoder_prio_trans_cfg, 0, sizeof(decoder_prio_trans_cfg_t));
    decoder_prio_trans_cfg.sample_rate = sample_rate;
    decoder_prio_trans_cfg.samples     = samples;
}

