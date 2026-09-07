#include "include.h"
#include "wsbc.h"

#define WSBC_BUF_SIZE           2500
#define WSBC_PKT_LEN            40
#define WSBC_MAX_FRAME_SIZE     20*16

AT(.wsbc_buf.dec.cb)
static u8 wsbc_mem[WSBC_BUF_SIZE];

static void *handle = NULL;
static u32 enc_code = 0;


bool wsbc_play_init(u16 *sample_rate, u8 *frame_size)
{
    u8 buf[8], sum;
    int err;

    ring_buf_peek(&warning_cb.res_ring_buf, buf, 8, 0);
    if(buf[0] != 0x7a) {
        return false;
    }
    *sample_rate = GET_LE16(&buf[2]);
    *frame_size = buf[1];

    handle = wsbc_dec_init(*sample_rate, 1, &err, (void *)wsbc_mem, WSBC_BUF_SIZE);

    printf("wsbc_init: %d, %x, spr=%d\n", err, handle, *sample_rate);
	if(err != 0 || handle == NULL) {
        return false;
    }

    ring_buf_get_old(&warning_cb.res_ring_buf, (rbuf_callback_t)dummy_func, 8);
    sum = 0;
    for(uint i=0; i<8; i++) {
        sum += buf[i];
    }

    enc_code = sum | (sum<<8) | (sum<<16) | (sum<<24);
    return true;
}

AT(.com_text.wsbc)
uint wsbc_play_proc(u8 *input, s16 *obuf, uint frame_size)
{
    u32 *ptr = (u32 *)input;
    for(uint i=0; i<(frame_size+3)/4; i++) {
        ptr[i] ^= enc_code;
    }

//    print_r(input, frame_size);
    int ret = wsbc_decode(handle, (const u8 *)input, frame_size, obuf, WSBC_MAX_FRAME_SIZE);
//    if(ret > 0) {
//        print_r(obuf, samples*2);
//    }
    return (ret > 0)? ret : 0;
}

//测试全部wsbc提示音
void wsbc_play_test(void)
{
//    bt_audio_bypass();
//    warning_play((u8 *)RES_BUF_EN_POWERON_SBC, RES_LEN_EN_POWERON_SBC);
//    warning_play((u8 *)RES_BUF_EN_POWEROFF_SBC, RES_LEN_EN_POWEROFF_SBC);
//    warning_play((u8 *)RES_BUF_EN_CONNECTED_SBC, RES_LEN_EN_CONNECTED_SBC);
//    warning_play((u8 *)RES_BUF_EN_DISCONNECT_SBC, RES_LEN_EN_DISCONNECT_SBC);
//    bt_audio_enable();
}

