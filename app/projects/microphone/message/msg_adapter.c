#include "include.h"
#include "func.h"


AT(.text.func.wireless_mic)
void func_adapter_message(u16 msg)
{
    switch (msg) {
	case MSG_MUSIC_PLAY_PAUSE:
		//usb_device_hid_send(UDE_HID_VOLUP, 1);//RX通过usb hid指令控制手机拍照
		break;
	
    case MSG_CHANGE_NR:
        func_adapter_set_nr_sta();
        break;

#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
    case MSG_PAIRING:
        if(wireless_cb.connected_sta & BIT(0)) {
            wireless_disconnect_req(0);
        }
        if(wireless_cb.connected_sta & BIT(1)) {
            wireless_disconnect_req(1);
        }
        printf("MSG_PAIRING\n");
        wireless_bonding_clr();
        wireless_adapter_pairing_enable(1);
        break;
#endif

    case MSG_SYS_1S:
#if WIRELESS_CON_PWR_CTR
        ws_pwr_ctr_tx_cmd_process();
#endif
        break;

#if ADAPTER_USB_MIC_RX_EN
        case EVT_UDE_SET_VOL:
            printf("ude set db: %d\n", f_ude.db_level);
            bsp_change_volume_db(f_ude.db_level);
            break;

        case EVT_UDE_SET_VOL_L_R:
            printf("ude set l r: %d, %d\n", f_ude.db_level_l, f_ude.db_level_r);
            bsp_change_volume_db_l_r(f_ude.db_level_l, f_ude.db_level_r);
            break;
#endif
    default:
        func_message(msg);
        break;
    }
}
