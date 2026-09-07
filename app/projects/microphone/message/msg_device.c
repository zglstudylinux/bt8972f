#include "include.h"
#include "func.h"
#include "func_device.h"

#if (WIRELESS_MIC_YLCRN_L2_EN && WIRELESS_MIC_AINS5_EN)
void ylcrn_L2_switch_ains5(void)
{
    static u8 ylcrn_L2_switch_ains5_flag = 0;
    ylcrn_L2_switch_ains5_flag ^= 1;
    if (ylcrn_L2_switch_ains5_flag) {
        mic_mute_set(1);
        ylcrn_L2_mic_mute_set(1);
        npu_exit();
        ains5_mic_param_set(1000);
        ains5_mic_mute_set(0);
        delay_5ms(10);
        mic_mute_set(0);
    } else {
        mic_mute_set(1);
        ains5_mic_mute_set(1);
        ylcrn_L2_mic_param_set(1000);
        ylcrn_L2_mic_mute_set(0);
        delay_5ms(10);
        mic_mute_set(0);
    }
}
#endif

AT(.text.func.device)
void func_device_message(u16 msg)
{
    switch (msg) {

    case MSG_MUSIC_PLAY_PAUSE:
#if (WIRELESS_MIC_YLCRN_L2_EN && WIRELESS_MIC_AINS5_EN)
        ylcrn_L2_switch_ains5();
#else
        denoise_mute_set(!denoise_mute_get(), DENOISE_ALL);
#endif
        break;

#if FUNC_BT_EN
    case MSG_FUNC_BT:
        func_cb.sta = FUNC_BT;
        break;
#endif
#if WIRELESS_CON_BONDING_EN
    case MSG_PAIRING:
        //进入配对状态
        printf("MSG_PAIRING\n");
        wireless_device_pairing_enable();
        break;
#endif

    case MSG_SYS_1S:
        break;

    default:
        func_message(msg);
        break;
    }
}
