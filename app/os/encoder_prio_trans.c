#include "include.h"
#include "encoder_prio_trans.h"


#define INCACHE_SIZE        2048
#define OUTCACHE_SIZE       2048

static encoder_prio_trans_cfg_t encoder_prio_trans_cfg AT(.bss.encoder_prio_trans);

#if ENCODER_BUF_EN
static uint8_t encoder_incache[INCACHE_SIZE] AT(.bss.encoder_prio_trans);
static uint8_t encoder_outcache[OUTCACHE_SIZE] AT(.bss.encoder_prio_trans);

static const rbuf_tbl_t enc_buf_tbl[1] = {
    {
        .buf = encoder_incache,
        .size = INCACHE_SIZE,
    },
};
#endif

AT(.com_text.encoder_prio_trans)
void encoder_prio_trans_kick(uint samples, u32 params, audio_callback_t handle)
{
    encoder_prio_trans_cfg.samples     = samples;
    encoder_prio_trans_cfg.params      = params;
    encoder_prio_trans_cfg.handle      = handle;

    ///kick低优先级线程，去获取encoder_prio_trans输出模块
    kick_encoder_prio_trans();
}

AT(.com_text.encoder_prio_trans)
void encoder_prio_trans_audio_input(u8 *ptr, u32 samples, u32 params)
{
#if ENCODER_BUF_EN
    uint nbyte = params & PCM_24BIT? 4 : 2;
    uint nch = params & PCM_CHMASK;
    uint frame_size = samples*nbyte*nch;

    ring_buf_t *ring_buf = &(encoder_prio_trans_cfg.ring_buf);
    ring_buf_put(ring_buf, ptr, frame_size);
#else
    encoder_prio_trans_cfg.ptr = ptr;
#endif

    if(encoder_prio_trans_cfg.callback != NULL) {
        encoder_prio_trans_kick(samples, params, encoder_prio_trans_cfg.callback);
    }
}

AT(.com_text.encoder_prio_trans)
void encoder_prio_trans_process(void)
{
#if ENCODER_BUF_EN
    uint nch = params & PCM_CHMASK;
    uint nbyte = params & PCM_24BIT? 4 : 2;
    uint frame_size = encoder_prio_trans_cfg.samples * nbyte * nch;

    ring_buf_t *ring_buf = &(alg_prio_trans_cfg.ring_buf);
    uint total_size = ring_buf_get_count(ring_buf);
    u8 *cache = encoder_outcache;

    if(encoder_prio_trans_cfg.handle) {
        if (!ring_buf_get(ring_buf, cache, frame_size)) {
            memset(cache, 0x00, frame_size*2);
        }
        encoder_prio_trans_cfg.handle(cache, encoder_prio_trans_cfg.samples, encoder_prio_trans_cfg.params);
    }
#else
    u8 *cache = encoder_prio_trans_cfg.ptr;

    if(encoder_prio_trans_cfg.handle) {
        encoder_prio_trans_cfg.handle(cache, encoder_prio_trans_cfg.samples, encoder_prio_trans_cfg.params);
    }
#endif
}

AT(.text.encoder_prio_trans)
void encoder_prio_trans_audio_output_callback_set(audio_callback_t callback)
{
    encoder_prio_trans_cfg.callback = callback;
}

AT(.text.encoder_prio_trans)
void encoder_prio_trans_audio_mute_set(uint8_t mute)
{
//    encoder_prio_trans_cfg.mute = mute;
}

AT(.text.encoder_prio_trans)
void encoder_prio_trans_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&encoder_prio_trans_cfg, 0, sizeof(encoder_prio_trans_cfg_t));
    encoder_prio_trans_cfg.sample_rate = sample_rate;
    encoder_prio_trans_cfg.samples     = samples;

#if ENCODER_BUF_EN
    memset(&encoder_incache, 0, INCACHE_SIZE);
    memset(&encoder_outcache, 0, OUTCACHE_SIZE);
    ring_buf_init(&(encoder_prio_trans_cfg.ring_buf), enc_buf_tbl, 1, 0);
#endif
}
