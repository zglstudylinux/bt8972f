#include "include.h"
#include "wireless_cmd.h"
#include "func.h"


void echo_audio_mute_set(u8 mute);
void plc_adpcm_audio_mute_set(u8 mute);
void ains3_audio_mute_set(u8 mute);
void src_audio_mute_set(u8 mute);
void dac_out_audio_mute_set(u8 mute);
void magic_mic_mute_set(u8 mute);
void usb_mic_in_audio_mute_set(u8 mute);
void howling_audio_mute_set(u8 mute);
void magic_mic_mute_set(u8 mute);
void mic_mute_set(u8 mute);
bool bsp_set_volume(u8 vol);
bool usb_device_hid_send(u16 hid_val, int auto_release);

#if WIRELESS_EN

/*****************************************************************************
 * Module    : 作用于发射端代码
 *****************************************************************************/
AT(.text.wireless_cmd.usb)
void wireless_tx_usb_cmd(u8 msg, u8 param)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_USB_CMD;
    pdu.buf[0] = msg;
    pdu.buf[1] = param;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
}

AT(.text.wireless_cmd.user)
void wireless_tx_user_cmd(u8 *ptr, u8 len)
{
    wireless_cmd_t pdu;
    len = (len>TX_MAX_BUF_SIZE)? TX_MAX_BUF_SIZE : len;

    pdu.cmd = PRIVATE_USER_DATA;
    memcpy(&pdu.buf[0], ptr, len);

    wireless_send_cmd(0, (u8 *)&pdu, len+1);
}

AT(.text.wireless_cmd.user)
void wireless_tx_pwr_ctr_cmd(u8 index, u8 pwr_level)
{
    wireless_cmd_t pdu;

    pdu.cmd = PRIVATE_PWR_CTR_CMD;
    pdu.buf[0] = pwr_level;

    wireless_send_cmd(index, (u8 *)&pdu, 2);
}

AT(.text.wireless_cmd.mic_cmd0)
void wireless_tx_mic_cmd0(u8 index, u8 opcode, u8 param)
{
    wireless_cmd_t pdu;
    pdu.cmd = PRIVATE_WS_MIC_CMD;
    pdu.buf[0] = opcode;
    pdu.buf[1] = param;

    wireless_send_cmd(index, (u8 *)&pdu, 3);
}

AT(.text.wireless_cmd.mic_cmd1)
bool wireless_tx_mic_cmd(u8 index, u8 opcode, u8 *buf, u8 size)
{
    if(size <= CMD_MAX_SIZE-2) {
        wireless_cmd_t pdu;
        pdu.cmd = PRIVATE_WS_MIC_CMD;
        pdu.buf[0] = opcode;
        memcpy(&pdu.buf[1], buf, size);

        if(wireless_send_cmd(index, (u8 *)&pdu, 2+size)) {
            return true;
        }
    }

    return false;
}

AT(.text.wireless_cmd.mute)
void wireless_tx_ws_audio_ctr_cmd(u8 mute_enable)
{
    wireless_cmd_t pdu;
    pdu.cmd = PRIVATE_WS_MIC_CMD;
    pdu.buf[0] = AUDIO_CRT_MUTE;
    pdu.buf[1] = mute_enable;

    wireless_send_cmd(0, (u8 *)&pdu, 3);
    wireless_send_cmd(1, (u8 *)&pdu, 3);
}

/*****************************************************************************
 * Module    : 作用于接收端代码
 *****************************************************************************/
static const u16 wl_msc_msg[] = {
    MSG_VOL_UP,             //MUSIC_CTL_VOL_UP
    MSG_VOL_DOWN,           //MUSIC_CTL_VOL_DOWN
    MSG_MUSIC_PREV,         //MUSIC_CTL_PREV
    MSG_MUSIC_NEXT,         //MUSIC_CTL_NEXT
    MSG_MUSIC_PLAY_PAUSE,   //MUSIC_CTL_PLAY_PAUSE
    MSG_VOICE_RM,           //MUSIC_CTL_VOICE_REMOVE
};

AT(.text.wireless_cmd)
void wireless_rx_mic_cmd(u8 index, wireless_cmd_t *pdu)
{
    u8 opcode = pdu->buf[0];

    switch(opcode) {
#if ECHO_EN
        case ECHO_DELAY_LEVEL_UP:
            echo_delay_level_up();
            break;

        case ECHO_DELAY_LEVEL_DOWN:
            echo_delay_level_down();
            break;
#endif
#if MAGIC_EN
        case MAGIC_LEVEL_CHANGE:
            magic_effect_level_change();
            break;
#endif

        case WL_DISCONNECT:
            printf("WL_DISCONNECT:%d\n", index);
            if(index) {
                wireless_disconnect_req(1);
            } else {
                wireless_disconnect_req(0);
            }
            break;

        case MUSIC_CTL_VOL_UP:
        case MUSIC_CTL_VOL_DOWN:
        case MUSIC_CTL_PREV:
        case MUSIC_CTL_NEXT:
        case MUSIC_CTL_PLAY_PAUSE:
        case MUSIC_CTL_VOICE_REMOVE:
            msg_enqueue(wl_msc_msg[(opcode-MUSIC_CTL_1ST)]);
            break;
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
        case AUDIO_CRT_MUTE:
            wl_audio_ctr_cb(cfg_le_conn_vers(), pdu->buf[1]);
            break;
#endif
        default:
#if WIRELESS_MIC_CMD_EN
            if(wireless_role_is_adapter()) {
                func_adapter_cmd_rx_cb(index, opcode, pdu->buf+1);
            } else {
                func_device_cmd_rx_cb(opcode, pdu->buf+1);
            }
#endif
            break;
    }
}

#if ADAPTER_USB_SPK_TX_EN || ADAPTER_USB_MIC_RX_EN
AT(.text.wireless_cmd.usb)
void wireless_rx_usb_cmd(wireless_cmd_t *pdu)
{
    u8 opcode = pdu->buf[0];
    u8 param = pdu->buf[1];

    switch(opcode) {
        case USB_SET_SPK_VOLUME:
            bsp_set_volume(param);
            break;

        case USB_CTL_PLAY_PAUSE:
//            printf("UDE_HID_PLAYPAUSE\n");
            usb_device_hid_send(UDE_HID_PLAYPAUSE, 1);
            break;

        case USB_CTL_VOLUME_UP:
//            printf("UDE_HID_VOLUP\n");
            usb_device_hid_send(UDE_HID_VOLUP, 1);
            break;

        case USB_CTL_VOLUME_DOWN:
//            printf("UDE_HID_VOLDOWN\n");
            usb_device_hid_send(UDE_HID_VOLDOWN, 1);
            break;

        case USB_CTL_PREVFILE:
//            printf("UDE_HID_PREVFILE\n");
            usb_device_hid_send(UDE_HID_PREVFILE, 1);
            break;

        case USB_CTL_NEXTFILE:
//            printf("UDE_HID_NEXTFILE\n");
            usb_device_hid_send(UDE_HID_NEXTFILE, 1);
            break;

        case USB_CTL_WDT_RESET:
            WDT_RST();
            break;
        case USB_CTL_MIC_STA:
//           printf("wireless_usb_mic_sta %d\n",wireless_usb_mic_sta);
//            wireless_usb_mic_sta = param;
        default:
            break;
    }
}
#endif

/*****************************************************************************
 * Module    : 公共部分代码
 *****************************************************************************/
AT(.text.wireless_cmd)
void wireless_rx_cmd(u8 index, u8 *ptr, u8 len)
{
    wireless_cmd_t *pdu = (void *)ptr;
//    printf("rx_cmd(%d,%d): ", pdu->buf[0], pdu->buf[1]);
//    print_r(ptr, len);

    if (pdu->cmd == PRIVATE_WS_MIC_CMD) {
        wireless_rx_mic_cmd(index, pdu);
#if ADAPTER_USB_SPK_TX_EN || ADAPTER_USB_MIC_RX_EN
    } else if(pdu->cmd == PRIVATE_USB_CMD) {
        wireless_rx_usb_cmd(pdu);
#endif
    } else if(pdu->cmd == PRIVATE_PWR_CTR_CMD) {
#if WIRELESS_MIC_2TNR_EN && WIRELESS_CON_PWR_CTR
        wl_audio_2tnr_pwr_ctr_set(index, pdu->buf[0]);
#else
        wl_audio_pwr_ctr_set(cfg_le_conn_vers(), index, pdu->buf[0]);
#endif
    } else {
        wireless_rx_user_cmd(index, ptr+1, len-1);
    }
}

AT(.text.wireless_cmd)
void wireless_cmd_init(void)
{
    wireless_cmd_buf_init();
}

#endif // WIRELESS_EN
