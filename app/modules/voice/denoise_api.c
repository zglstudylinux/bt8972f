#include "include.h"

bool denoise_mute = 1;

AT(.com_text.set_mute)
void denoise_mute_set(uint8_t mute, uint16_t denoise_type)
{
#if YLCRN_L3_EN || YLCRN_L2_EN || AINS4_EN || AINS5_EN || AGC_EN
    if (!mute) {
        if (wireless_role_is_adapter()) {
            sys_clk_req(INDEX_WL_ALG_EN, ADAPTER_WL_ALG_EN_CLK);
        } else {
            sys_clk_req(INDEX_WL_ALG_EN, DEVICE_WL_ALG_EN_CLK);
        }
    }
    denoise_mute = mute;
#endif // YLCRN_L3_EN

#if YLCRN_L3_EN
    if (denoise_type&DENOISE_GTCRN_L3) {
        ylcrn_L3_mic_mute_set(mute);
    }
#endif
#if YLCRN_L2_EN
    if (denoise_type&DENOISE_YLCRN_L2) {
        ylcrn_L2_mic_mute_set(mute);
    }
#endif
#if AINS4_EN
    if (denoise_type&DENOISE_AINS4) {
        ains4_mic_mute_set(mute);
    }
#endif
#if AINS5_EN
    if (denoise_type&DENOISE_AINS5) {
        ains5_mic_mute_set(mute);
    }
#endif
#if AGC_EN
    if (denoise_type&DENOISE_AGC) {
        agc_mic_mute_set(mute);
    }
#endif

    if (mute) {
        sys_clk_free(INDEX_WL_ALG_EN);
    }
}

AT(.com_text.get_mute)
bool denoise_mute_get(void)
{
    return denoise_mute;
}

