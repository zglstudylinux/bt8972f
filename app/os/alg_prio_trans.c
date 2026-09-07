#include "include.h"
#include "alg_prio_trans.h"


#define INCACHE_SIZE        768
#define OUTCACHE_SIZE       256

static uint8_t alg_outcache[OUTCACHE_SIZE] AT(.buf.adapter.alg_prio_trans);
static uint8_t alg_incache[INCACHE_SIZE] AT(.buf.adapter.alg_prio_trans);
static alg_prio_trans_cfg_t alg_prio_trans_cfg AT(.buf.adapter.alg_prio_trans);

static const rbuf_tbl_t alg_buf_tbl[1] = {
    {
        .buf = alg_incache,
        .size = INCACHE_SIZE,
    },
};

AT(.com_text.alg_prio_trans)
void alg_prio_trans_audio_input(u8 *ptr, u32 samples, u32 params)
{
    uint nbyte = params & PCM_24BIT? 4 : 2;
    uint input_nch = params & PCM_CHMASK;
    uint input_size = samples*nbyte*input_nch;

    uint output_nch = alg_prio_trans_cfg.params & PCM_CHMASK;
    uint output_size = alg_prio_trans_cfg.samples*nbyte*output_nch;

    ring_buf_t *ring_buf = &(alg_prio_trans_cfg.ring_buf);
    ring_buf_put(ring_buf, ptr, input_size);

    if((alg_prio_trans_cfg.callback != NULL) && (ring_buf_get_count(ring_buf) >= output_size)) {
        ///kick低优先级线程，去获取alg_prio_trans输出模块
        kick_alg_prio_trans();
    }

}

AT(.com_text.alg_prio_trans)
void alg_prio_trans_process(void)
{
    uint output_nch = alg_prio_trans_cfg.params & PCM_CHMASK;
    uint output_size = alg_prio_trans_cfg.samples*2*output_nch;

    u8 *cache = alg_outcache;
    ring_buf_t *ring_buf = &(alg_prio_trans_cfg.ring_buf);

    if(alg_prio_trans_cfg.callback) {
        while (ring_buf_get(ring_buf, cache, output_size)) {
            alg_prio_trans_cfg.callback(cache, alg_prio_trans_cfg.samples, alg_prio_trans_cfg.params);
        }
    }
}

AT(.text.alg_prio_trans)
void alg_prio_trans_audio_output_callback_set(audio_callback_t callback)
{
    alg_prio_trans_cfg.callback = callback;
}

AT(.text.alg_prio_trans)
void alg_prio_trans_audio_mute_set(uint8_t mute)
{
//    alg_prio_trans_cfg.mute = mute;
}

AT(.text.alg_prio_trans)
void alg_prio_trans_init(u8 sample_rate, u16 samples, u8 channel)
{
    memset(&alg_prio_trans_cfg, 0, sizeof(alg_prio_trans_cfg_t));
    alg_prio_trans_cfg.sample_rate = sample_rate;
    alg_prio_trans_cfg.samples     = samples;
    alg_prio_trans_cfg.params      = channel;

    memset(&alg_incache, 0, INCACHE_SIZE);
    memset(&alg_outcache, 0, OUTCACHE_SIZE);
    ring_buf_init(&(alg_prio_trans_cfg.ring_buf), alg_buf_tbl, 1, 0);
}


