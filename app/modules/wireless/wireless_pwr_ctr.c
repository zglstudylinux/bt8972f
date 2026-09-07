#include "include.h"
#include "wireless_pwr_ctr.h"

#if WIRELESS_CON_PWR_CTR

static ws_rx_info_str_t ws_rx_info_str[WIRELESS_CON_LINK_NB];

void ws_pwr_ctr_init(void)
{
    memset((u8 *)&ws_rx_info_str, 0 , WIRELESS_CON_LINK_NB*sizeof(ws_rx_info_str_t));
}

void ws_pwr_ctr_reset(uint8_t idx)
{
    memset((u8 *)&ws_rx_info_str[idx], 0 , sizeof(ws_rx_info_str_t));
}

AT(.com_text.mic_dec)
static uint8_t ws_pwr_ctr_pwr_change(uint8_t change_flag, uint8_t pwr_level)
{
    uint8_t change_pwr_level = pwr_level;

    if (change_flag == INC_FLAG) {
        if (change_pwr_level && (change_pwr_level != MAX_PWR_LEVEL)) {
            change_pwr_level -=1;
        }
    } else {
        if (change_pwr_level != MIN_PWR_LEVEL) {
            change_pwr_level +=1;
        }
    }

    return change_pwr_level;
}

AT(.com_text.mic_dec)
void ws_pwr_ctr_rx_process(uint8_t idx, uint8_t bfi)
{
    ws_rx_info_str_t *ws_rx_info_ptr = &ws_rx_info_str[idx];
    uint8_t pwr_level = ws_rx_info_ptr->pwr_level;

    ws_rx_info_ptr->rx_cnt++;
    if (bfi) {
        ws_rx_info_ptr->rx_fail_cnt++;
    }

    if (ws_rx_info_ptr->rx_cnt >= RX_CYCLE_NUM) {
        if (ws_rx_info_ptr->rx_fail_cnt > RX_FAIL_THR) {
            ///增加功率
            pwr_level = ws_pwr_ctr_pwr_change(INC_FLAG, pwr_level);
            if (ws_rx_info_ptr->rx_fail_cnt > RX_FAIL_SP_THR) {
                ///继续增加功率
                pwr_level = ws_pwr_ctr_pwr_change(INC_FLAG, pwr_level);
            }
            ws_rx_info_ptr->rx_ok_cycle = 0;
        } else {
            ws_rx_info_ptr->rx_ok_cycle++;
        }
        ws_rx_info_ptr->rx_cnt = 0;
        ws_rx_info_ptr->rx_fail_cnt = 0;
    }

    if (ws_rx_info_ptr->rx_ok_cycle >= RX_OK_CYCLE_NUM) {
        ws_rx_info_ptr->rx_ok_cycle = 0;
        ///降低功耗
        pwr_level = ws_pwr_ctr_pwr_change(DEC_FLAG, pwr_level);
    }

    if (pwr_level != ws_rx_info_ptr->pwr_level) {
        ws_rx_info_ptr->pwr_level = pwr_level;
        ws_rx_info_ptr->tx_cmd_flag = 1;
    }
}

void ws_pwr_ctr_tx_cmd_process(void)
{
    for (uint8_t i = 0; i < WIRELESS_CON_LINK_NB; i++) {
        ws_rx_info_str_t *ws_rx_info_ptr = &ws_rx_info_str[i];
        if (ws_rx_info_ptr->tx_cmd_flag) {
            ws_rx_info_ptr->tx_cmd_flag = 0;
            wireless_tx_pwr_ctr_cmd(i, ws_rx_info_ptr->pwr_level);
        }
    }
}

#if WIRELESS_MIC_2TNR_EN
uint8_t pwr_level_2tnr[WIRELESS_CON_2TNR_NB];

void ws_pwr_ctr_2tnr_init(void)
{
    for (u8 i = 0; i < WIRELESS_CON_2TNR_NB; i++) {
        pwr_level_2tnr[i] = 0;
    }
}

void wl_audio_2tnr_pwr_ctr_set(uint8_t index, uint8_t pwr_level)
{
    uint8_t min_pwr_level = MIN_PWR_LEVEL;

    pwr_level_2tnr[index] = pwr_level;

    for (u8 i = 0; i < WIRELESS_CON_2TNR_NB; i++) {
        if ((wireless_cb.connected_sta&BIT(i)) && (pwr_level_2tnr[i] < min_pwr_level)) {
            min_pwr_level = pwr_level_2tnr[i];
        }
    }

    printf("2tnr_pwr_ctr_str, index:%d, pwr_level:%d, min_pwr_level:%d, con_star:%d\n", index, pwr_level, min_pwr_level, wireless_cb.connected_sta);

    wl_audio_pwr_ctr_set(cfg_le_conn_vers(), index, min_pwr_level);
}
#endif

bool ws_pwr_level_ctr_en(void)
{
    return true;
}

void ws_pwr_level_delta_get(uint8_t pwr_level, u8 *pa_gain_detal, u8 *mix_gain_detal, u8 *dig_gain_detal)
{
    switch(pwr_level) {
        case 1:
            *pa_gain_detal = 0;         //6 2 56  6.9dBm
            *mix_gain_detal = 1;
            *dig_gain_detal = 0;
            break;

        case 2:
            *pa_gain_detal = 2;         //4 2 56  4.6dBm
            *mix_gain_detal = 1;
            *dig_gain_detal = 0;
            break;

        case 3:
            *pa_gain_detal = 2;         //4 1 56  2.7dBm
            *mix_gain_detal = 2;
            *dig_gain_detal = 0;
            break;

        default:
            break;
    }
}
#endif
