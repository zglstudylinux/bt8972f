#ifndef  __WSBC_H_
#define  __WSBC_H_

void *wsbc_dec_init(u32 sample_rate, u8 channels, int *error, u8 *mem, u32 size);
int wsbc_decode(void *handle, const u8 *data, u32 len, s16 *pcm, s32 max_size);

bool wsbc_play_init(u16 *sample_rate, u8 *frame_size);
uint wsbc_play_proc(u8 *input, s16 *obuf, uint frame_size);

void wsbc_play_test(void);

#endif // __WSBC_H_
