#ifndef _BT_SCO_H
#define _BT_SCO_H


extern bool bt_sco_is_msbc(void);                      //判断当前通话是否是宽带通话

void bt_sco_pcm_buf_init(void);

void bt_sco_dump_cb(uint type, void *ptr, uint size);

//void mic_post_gain_process_s(s16 *ptr, int gain, int samples);

u16 dnr_buf_maxpow(void *ptr, u16 len);

void bt_sco_tick_init(void);
void bt_sco_sync_proc(void);

bool sco_audio_is_init(void);
#endif
