#ifndef _WIRELESS_H
#define _WIRELESS_H

#if WIRELESS_EN
#include "wireless_link_list.h"
#include "wireless_txrx.h"
#include "wireless_proc.h"
#include "wireless_cmd.h"
#include "wireless_pwr_ctr.h"
//#include "../../os/os_thread.h"

#if WIRELESS_MIC_24B_PCM_EN
    typedef s32                     mic_pcm_t;
    #define PCM_MAX_V               8388607
    #define PCM_MIN_V               -8388608
#else
    typedef s16                     mic_pcm_t;
    #define PCM_MAX_V               32767
    #define PCM_MIN_V               -32768
#endif

#if WIRELESS_CON_COMB_BUF_EN
    #define WIRELESS_MIC_RX_BUFFER_SIZE     (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)*WIRELESS_MIC_RETRY_NB
    #define WIRELESS_MIC_TX_BUFFER_SIZE     (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)*WIRELESS_MIC_RETRY_NB
    #define WIRELESS_SPK_RX_BUFFER_SIZE     (WIRELESS_SPK_FRAME_SIZE*WIRELESS_SPK_COMB_NB)*WIRELESS_MIC_RETRY_NB
    #define WIRELESS_SPK_TX_BUFFER_SIZE     (WIRELESS_SPK_FRAME_SIZE*WIRELESS_SPK_COMB_NB)*WIRELESS_MIC_RETRY_NB
#else
    #define WIRELESS_MIC_RX_BUFFER_SIZE     (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)
    #define WIRELESS_MIC_TX_BUFFER_SIZE     (WIRELESS_MIC_FRAME_SIZE*WIRELESS_MIC_COMB_NB)
    #define WIRELESS_SPK_RX_BUFFER_SIZE     (WIRELESS_SPK_FRAME_SIZE*WIRELESS_MIC_COMB_NB)
    #define WIRELESS_SPK_TX_BUFFER_SIZE     (WIRELESS_SPK_FRAME_SIZE*WIRELESS_MIC_COMB_NB)
#endif

#define wireless_is_sleep()             (wireless_con_get_status() == 0)

void mic_enc_audio_input(u8 *ptr, u32 samples, u32 params);
void mic_enc_audio_output_callback_set(audio_callback_t callback);
void mic_enc_init(u8 sample_rate, u16 samples, u8 channel);
void mic_enc_reset(void);
void mic_mute_set(u8 mute);

void mic_dec_audio_input(u8 *ptr, u32 samples, u32 params);
void mic_dec_audio_output_callback_set(audio_callback_t callback);
void mic_dec_init(u8 sample_rate, u16 samples, u8 channel);
void mic_dec_reset(u8 idx);

void spk_enc_audio_input(u8 *ptr, u32 samples, u32 params);
void spk_enc_audio_output_callback_set(audio_callback_t callback);
void spk_enc_init(u8 sample_rate, u16 samples, u8 channel);
void spk_enc_reset(void);
void spk_mute_set(u8 mute);

void spk_dec_audio_input(u8 *ptr, u32 samples, u32 params);
void spk_dec_audio_output_callback_set(audio_callback_t callback);
void spk_dec_init(u8 sample_rate, u16 samples, u8 channel);
void spk_dec_reset(void);

void wireless_interphone_init(void);
void wireless_interphone_exit(u8 idx, u8 con_sta);

void wireless_init(void);
void wireless_adapter_init(void);
void wireless_device_init(void);
void wireless_device_exit(void);
void wireless_adapter_exit(u8 idx, u8 con_sta);
void wireless_channel_status(u8 idx, u8 chidx, s8 rssi, u8 bfi, u8 chstatus);
void wireless_enter_sleep(void);
void wireless_exit_sleep(void);

bool wireless_mic_is_active(void);
void wireless_status_change(void);
bool wireless_discon_all(void);

bool wireless_get_link_info_addr(uint8_t *wl_addr);

#if WIRELESS_DUMP_EN
void wireless_dump_init(void);
void wireless_dump_proc(void);
void wireless_dump_reset(u8 idx);
void wireless_dump_set_rx_status(u8 idx, u8 chidx, s8 rssi, u8 bfi);
void wireless_dump_set_chmap_cb(u8 idx, const u8 *chmap);
#else
    #define wireless_dump_init()
    #define wireless_dump_proc()
    #define wireless_dump_reset(idx)
    #define wireless_dump_set_rx_status(idx, chidx, rssi, bfi)
    #define wireless_dump_set_chmap_cb(idx, chmap)
#endif

#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
#define WL_PAIR_HEADER1                   0xAA
#define WL_PAIR_MODE                      0xB0

enum{
    BONDING_MODE_NONE = 0,
    BONDING_MODE_WL,
    BONDING_MODE_BT,
};

bool wireless_bonding_check(uint8_t *param);
void wireless_bonding_init(void);
void wireless_bonding_clr(void);
void wireless_device_bond_update(void);
void wireless_device_pairing_enable(void);
void wireless_adapter_bond_update(void);
void wireless_adapter_pairing_enable(uint8_t en);
#endif


#endif // WIRELESS_EN
#endif //_WIRELESS_H
