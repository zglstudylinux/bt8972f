#include "include.h"
#include "warning_play.h"
#include "res_play.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

#define TRACE_ERR(...)          printf(__VA_ARGS__)


u32 waring_get_digvol_cb(void);
void warning_dec_kick_start(void);


#if WARNING_WSBC_RES_EN

enum {
    PLAY_STA_IDLE,
    PLAY_STA_STOP,
    PLAY_STA_STOPING,
    PLAY_STA_DEC_1ST,
    PLAY_STA_W4_KICK,
    PLAY_STA_PROC,
};

warning_cb_t warning_cb AT(.tone_buf.warning);
volatile u8 warning_play_state = PLAY_STA_IDLE;

const rbuf_tbl_t res_buf_tbl[1] = {
    {
        .buf = warning_cb.ibuf,
        .size = CACHE_BUF_SIZE,
    },
};

AT(.text.warning)
static u8 *warning_ptr_get(void)
{
    return warning_cb.res_ptr+warning_cb.rd_pos;
}

static ALWAYS_INLINE u32 warning_remain_len_get(void)
{
    return (warning_cb.res_size - warning_cb.rd_pos);
}

AT(.text.warning.proc)
void warning_fill_stream(void)
{
    u32 cbuf_reamin_len = CACHE_BUF_SIZE - warning_cb.res_ring_buf.count;
    u8 *warning_ptr = warning_ptr_get();
    u32 warning_remain_len = warning_remain_len_get();

    while (cbuf_reamin_len >= (CACHE_BUF_SIZE/2) && warning_remain_len > 0) {
        if (warning_remain_len >= cbuf_reamin_len) {
            ring_buf_put((void *)&warning_cb.res_ring_buf, warning_ptr, cbuf_reamin_len);
            warning_cb.rd_pos += cbuf_reamin_len;
        } else {
            ring_buf_put((void *)&warning_cb.res_ring_buf, warning_ptr, warning_remain_len);
            warning_cb.rd_pos += warning_remain_len;
        }
        cbuf_reamin_len = CACHE_BUF_SIZE - warning_cb.res_ring_buf.count;
        warning_ptr = warning_ptr_get();
        warning_remain_len = warning_remain_len_get();

        if (!warning_remain_len) {
            return;
        }
    }
}

AT(.text.warning)
static bool warning_init(u8 *ptr, u32 size, u8 *spr_idx)
{
    u16 sample_rate;
    memset((u8 *)&warning_cb, 0, sizeof(warning_cb));
    warning_cb.res_ptr  = ptr;
    warning_cb.res_size = size;

    ring_buf_init((void *)&warning_cb.res_ring_buf, &res_buf_tbl[0], 1, 0);
    warning_fill_stream();

    if(!wsbc_play_init(&sample_rate, &warning_cb.frame_size)) {
        return false;
    }

    if(sample_rate == 8000) {
        *spr_idx = SPR_8000;
    } else if(sample_rate == 16000) {
        *spr_idx = SPR_16000;
    } else {
        return false;
    }
    return true;
}

AT(.text.warning)
static void warning_start(u8 spr_idx, uint8_t res_flag)
{
    dac1_aubuf_init();
    if(res_flag & RES_FLAG_LOCAL) {
        dac1_vol_set(waring_get_digvol_cb());
    } else {
        dac1_vol_set(0);
    }
    dac1_spr_set(spr_idx);

    dac1_fade_in();
    dac1_fade_wait();

    warning_play_state = PLAY_STA_DEC_1ST;
    TRACE("res_play_start\n");
}

AT(.text.warning) NO_INLINE
void warning_stop(void)
{
    aubuf1_dma_w4_done();
    dac1_fade_out();
    dac1_put_zero(128);                 //使OBUF播放完
    dac1_aubuf_w4_empty();
    dac1_fade_wait();
    dac1_vol_set(0);
    AUBUF1CON |= BIT(0);                //Reset audio Buffer
    DACDIGCON0 &= ~BIT(8);              //SRC1 disable

    TRACE("res_play_stop\n");
    warning_play_state = PLAY_STA_IDLE;
}

AT(.com_text.warning)
void warning_dec_check_kick(void)
{
    u8 play_state = warning_play_state;
    if(play_state >= PLAY_STA_DEC_1ST) {        //解码状态
        if(!aubuf1_dma_is_busy()) {             //上一帧DMA是否完成
            warning_dec_kick_start();
        }
    }
}

AT(.com_text.warning)
void warning_kick_dac_dma(void)
{
    uint samples = warning_cb.samples;

    if(samples > 0) {
        ///注意obuf放eram时不支持dma方式推dac
        aubuf1_dma_kick(warning_cb.obuf, samples, 1, false);
//        print_r((u8 *)serial, frame_len);

//        ///目前放到eram不支持dma方式推dac，后面再看看怎么改
//        for (uint i=0; i<samples; i++) {
//            dac1_put_sample_16bit_w(warning_cb.obuf[i], warning_cb.obuf[i]);
//        }
    }
}

AT(.com_text.warning)
void warning_dec_proc(void)
{
    u8 serial[MAX_SERIAL_SIZE];
    u16 frame_len = warning_cb.frame_size;
    u8 play_state = warning_play_state;

    if(play_state == PLAY_STA_PROC || play_state == PLAY_STA_DEC_1ST) {
        if (ring_buf_get((void *)&warning_cb.res_ring_buf, serial, frame_len)) {
            aubuf1_dma_w4_done();
            warning_cb.samples = wsbc_play_proc((u8 *)serial, (s16 *)warning_cb.obuf, frame_len);
            if(play_state == PLAY_STA_DEC_1ST) {
                warning_play_state = PLAY_STA_W4_KICK;
            } else {
                warning_kick_dac_dma();
            }
        } else {
            if(warning_remain_len_get() < frame_len) {
                warning_play_state = PLAY_STA_STOP;
            }
        }
    } else if(play_state == PLAY_STA_STOPING) {
        warning_play_state = PLAY_STA_STOP;
    }
}

AT(.com_text.warning)
void warning_play_kick(void)
{
    GLOBAL_INT_DISABLE();
    if(warning_play_state == PLAY_STA_W4_KICK) {
        warning_play_state = PLAY_STA_PROC;
        warning_kick_dac_dma();
        GLOBAL_INT_RESTORE();
    } else {
        GLOBAL_INT_RESTORE();
    }
}

AT(.text.warning)
int warning_play_init(u8 *ptr, u32 size, bool kick_start, uint8_t res_flag)
{
    u8 spr_idx = SPR_16000;
    TRACE("res_play_init: %x, %x\n", ptr, size);
    if ((size == 0) || (ptr == 0)) {
        return -1;
    }

    if(!warning_init(ptr, size, &spr_idx)) {
        return -2;
    }

    warning_start(spr_idx, res_flag);
    return 0;
}

AT(.text.warning.proc)
int warning_play_proc(void)
{
    //避免错过kick时间
    warning_play_kick();

    //循环填充位流
    u8 play_state = warning_play_state;
    if(play_state == PLAY_STA_STOPING) {
        return 0;
    } else if(play_state > PLAY_STA_STOP) {
        u32 res_len = warning_remain_len_get();
        u32 cbuf_len = warning_cb.res_ring_buf.count;
        if(cbuf_len < warning_cb.frame_size && res_len > 0) {
            printf("\n====res_less: (%d,%d)\n", cbuf_len, res_len);
        }
        warning_fill_stream();
        return 0;
    }
    return 1;
}

GLOBAL_ALIGNED(128)
void warning_play_exit(void)
{
    u8 tout = 8;

    GLOBAL_INT_DISABLE();
    if(warning_play_state > PLAY_STA_STOPING) {
        warning_play_state = PLAY_STA_STOPING;
    }
    GLOBAL_INT_RESTORE();

    TRACE("res_play_exit\n");
    while(warning_play_state == PLAY_STA_STOPING && tout--) {
        delay_5ms(1);
    }

    warning_stop();
}
#endif // WARNING_WSBC_RES_EN

