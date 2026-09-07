#ifndef __WIRELESS_LINK_LIST_H
#define __WIRELESS_LINK_LIST_H


typedef enum{
    MIC_TYPE  = 1,
    ENCODER_PRIO_TRANS_TYPE,
    MIC_EQ_DRC_TYPE,
    MIC_ENC_TYPE,
    DAC0_OUT_TYPE,

    DECODER_PRIO_TRANS_TYPE,
    ALG_PRIO_TRANS_TYPE,
    MIC_DEC_TYPE,
    MIC_MIX_TYPE,
    SRC_TYPE,
    SRC_BUF_TYPE,

    DAC_OUT_TYPE,
    USB_MIC_IN_TYPE,
    USB_SPK_OUT_TYPE,
    USBMIC_EQ_DRC_TYPE,
    ECHO_TYPE,
    MAGCI_PITCH_SHIFT_TYPE,
    HOWLING_AIMASK_TYPE,
    DNN_L1_TYPE,
    I2S_OUT_TYPE,
    I2S_IN_TYPE,
    I2S_IN_AND_OUT_TYPE,
    HOWLING_DNN_L2_TYPE,
    YLCRN_L2_TYPE,
    YLCRN_L3_TYPE,
    AINS4_TYPE,
    AINS5_TYPE,
    DNR_FRE_TYPE,
    AGC_MIC_TYPE,
    MIC_REC_TYPE,
    MIC_STEREO_TYPE,
    HUART_TX_TYPE,
    HUART_RX_TYPE,

    SPK_TYPE,
    SPK_ENC_TYPE,
    SPK_DEC_TYPE,
} MODULE_TYPE;

typedef void (*module_input_t)(u8 *ptr, u32 samples, u32 params);
typedef void (*module_init_t)(u8 sample_rate, u16 samples, u8 channel);
typedef void (*module_set_callback_t)(module_input_t callback);

typedef struct {
    u8 module_type;
    u8 module_enable;
    module_init_t  module_init;
    module_input_t module_input;
    module_set_callback_t module_set_callback;
} wl_link_list_t;

void wl_link_list_init(wl_link_list_t *link_list, u8 module_count, u8 sample_rate, u16 samples, u8 channel);

#endif // __WIRELESS_LINK_LIST_H
