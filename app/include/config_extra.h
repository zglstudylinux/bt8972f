#ifndef __CONFIG_EXTRA_H__
#define __CONFIG_EXTRA_H__

/*****************************************************************************
 * Module    : SDK版本配置
 *****************************************************************************/
#ifndef SDK_VERSION
    #define SDK_VERSION                 0x0120      //V0120
#endif

/*****************************************************************************
 * Module    : 系统功能配置
 *****************************************************************************/
#if VDDIO_LIMIT_SEL > 7
    #error "VDDIO_LIMIT_SEL > 7  is not allowed!"
#endif

#if VDDBT_LIMIT_SEL > 7
    #error "VDDBT_LIMIT_SEL > 7  is not allowed!"
#endif

#if VDDCORE_LIMIT_SEL > 7
    #error "VDDCORE_LIMIT_SEL > 7  is not allowed!"
#endif


#undef  SYS_ADJ_DIGVOL_EN
#define SYS_ADJ_DIGVOL_EN               1

#ifndef SYS_MODE_BREAKPOINT_EN
#define SYS_MODE_BREAKPOINT_EN          0
#endif // SYS_MODE_BREAKPOINT_EN

#ifndef ENERGY_LED_EN
#define ENERGY_LED_EN                   0
#endif

#ifndef PLUGIN_SYS_INIT_FINISH_CALLBACK
#define PLUGIN_SYS_INIT_FINISH_CALLBACK 0
#endif

#ifndef PLUGIN_FUNC_IDLE_ENTER_CHECK
#define PLUGIN_FUNC_IDLE_ENTER_CHECK    0
#endif


#ifndef FLASH_SPEED_UP_EN
#define FLASH_SPEED_UP_EN               1
#endif

#ifndef MUSIC_AAC_SUPPORT
#define MUSIC_AAC_SUPPORT               0
#endif // MUSIC_AAC_SUPPORT

#ifndef USER_NTC
#define USER_NTC                        0
#endif


#if !EQ_DBG_IN_UART
#undef EQ_DBG_IN_UART_VUSB_EN
#define EQ_DBG_IN_UART_VUSB_EN          0
#endif

/*****************************************************************************
 * Module    :  充电仓功能选择
 *****************************************************************************/
#if !CHARGE_EN
    #undef CHARGE_BOX_EN
    #define CHARGE_BOX_EN         0
#endif

#if !CHARGE_BOX_EN
    #undef CHARGE_BOX_TYPE
    #define CHARGE_BOX_TYPE         CBOX_NONE
#endif

#if (CHARGE_BOX_TYPE != CBOX_SSW)
    #undef CHARGE_BOX_INTF_SEL
    #define CHARGE_BOX_INTF_SEL     INTF_NONE
#endif

#if ((CHARGE_BOX_TYPE == CBOX_SSW) && !BT_TWS_EN)
    #undef CHARGE_BOX_TYPE
    #define CHARGE_BOX_TYPE         CBOX_NONE
#endif

/*****************************************************************************
 * Module    :  产测功能选择
 *****************************************************************************/
#if !IODM_TEST_EN && !QTEST_EN
    #undef TEST_INTF_SEL
    #define TEST_INTF_SEL               INTF_NONE
#endif

#if IODM_TEST_EN
#undef FUNC_BT_DUT_EN

#define FUNC_BT_DUT_EN            1
#endif

#if IODM_TEST_EN || FUNC_BT_FCC_EN || FUNC_BT_DUT_EN || QTEST_EN
#define TEST_MODE_BT_INFO         1
#endif

#if !DAC_PT_EN
#undef DAC_PT_NUM
#define DAC_PT_NUM                      0
#endif

/*****************************************************************************
 * Module    :  UART功能选择（待整理）
 *****************************************************************************/
#if   BT_EQ_DUMP_EN || BT_PLC_DUMP_EN
    #define BT_SCO_DUMP_TX_EN           1
#else
    #define BT_SCO_DUMP_TX_EN           0
#endif

#if (CHARGE_BOX_INTF_SEL == INTF_UART1) || (TEST_INTF_SEL == INTF_UART1)
    #define UART1_EN                    1
#endif

#if (CHARGE_BOX_INTF_SEL == INTF_HUART) ||  \
    (TEST_INTF_SEL == INTF_HUART) ||        \
    EQ_DBG_IN_UART ||                       \
    BT_SCO_DUMP_TX_EN ||                    \
    QTEST_EN ||                             \
    ANC_TOOL_EN
    /*(强制打开HUART)*/
    #define HUART_EN                    1
#endif

/*****************************************************************************
 * Module    : 音乐功能配置
 *****************************************************************************/
#if !FUNC_MUSIC_EN && ！DEVICE_RECORD_EN
#undef  MUSIC_UDISK_EN
#undef  MUSIC_SDCARD_EN

#undef  MUSIC_WAV_SUPPORT
#undef  MUSIC_WMA_SUPPORT
#undef  MUSIC_APE_SUPPORT
#undef  MUSIC_FLAC_SUPPORT
#undef  MUSIC_SBC_SUPPORT
#undef  MUSIC_M4A_SUPPORT
#undef  MUSIC_AAC_SUPPORT
#undef  MUSIC_ID3_TAG_EN
#undef  MUSIC_FOLDER_SELECT_EN
#undef  MUSIC_AUTO_SWITCH_DEVICE
#undef  MUSIC_BREAKPOINT_EN
#undef  MUSIC_MODE_RETURN
#undef  MUSIC_PLAYDEV_BOX_EN
#undef  MUSIC_ID3_TAG_EN
#undef  MUSIC_REC_FILE_FILTER
#undef  MUSIC_LRC_EN
#undef  MUSIC_NAVIGATION_EN
#undef  MUSIC_ENCRYPT_EN

#define MUSIC_UDISK_EN              0
#define MUSIC_SDCARD_EN             0
#define MUSIC_WAV_SUPPORT           0
#define MUSIC_WMA_SUPPORT           0
#define MUSIC_APE_SUPPORT           0
#define MUSIC_FLAC_SUPPORT          0
#define MUSIC_M4A_SUPPORT           0
#define MUSIC_SBC_SUPPORT           0
#define MUSIC_ID3_TAG_EN            0
#define MUSIC_AAC_SUPPORT           0
#define MUSIC_FOLDER_SELECT_EN      0
#define MUSIC_AUTO_SWITCH_DEVICE    0
#define MUSIC_BREAKPOINT_EN         0
#define MUSIC_QSKIP_EN              0
#define MUSIC_MODE_RETURN           0
#define MUSIC_PLAYDEV_BOX_EN        0
#define MUSIC_ID3_TAG_EN            0
#define MUSIC_REC_FILE_FILTER       0
#define MUSIC_LRC_EN                0
#define MUSIC_NAVIGATION_EN         0
#define MUSIC_ENCRYPT_EN            0
#endif // FUNC_MUSIC_EN

#if !CHARGE_EN
#undef  CHARGE_TRICK_EN
#undef  CHARGE_DC_IN
#undef  CHARGE_DC_NOT_PWRON
#define CHARGE_TRICK_EN             0
#define CHARGE_DC_NOT_PWRON         0
#define CHARGE_DC_IN()              0
#endif

#if !AUX_DETECT_EN
#undef AUX_DETECT_INIT
#undef AUX_IS_ONLINE
#undef AUX_DETECT_IS_BUSY

#define AUX_DETECT_INIT()
#define AUX_IS_ONLINE()          0
#define AUX_DETECT_IS_BUSY()        0
#endif

#ifndef PWROFF_PRESS_TIME
#define PWROFF_PRESS_TIME           3               //1:大约1秒    3: 1.5s,  6: 2s,  9: 2.5s, 12: 3s
#endif // PWROFF_PRESS_TIME

#ifndef PWRON_PRESS_TIME
#define PWRON_PRESS_TIME            1500           //1.5s
#endif // PWRON_PRESS_TIME

#ifndef DOUBLE_KEY_TIME
#define DOUBLE_KEY_TIME             1
#endif // DOUBLE_KEY_TIME

#ifndef AUX_MODE_2_PWROFF_TONE_EN
#define AUX_MODE_2_PWROFF_TONE_EN       0           //插入AUX直接关机时，是否播报关机提示音。
#endif // AUX_MODE_2_PWROFF_TONE_EN

#ifndef LPWR_WARING_TIMES
#define LPWR_WARING_TIMES           0xff
#endif // LPWR_WARING_TIMES

#if !BUZZER_EN
#undef BUZZER_INIT
#define BUZZER_INIT()
#undef BUZZER_ON
#define BUZZER_ON()
#undef BUZZER_OFF
#define BUZZER_OFF()
#endif

#ifndef LED_LOWBAT_EN
#define LED_LOWBAT_EN              0
#endif // RLED_LOWBAT_EN

#ifndef RLED_LOWBAT_FOLLOW_EN
#define RLED_LOWBAT_FOLLOW_EN       0
#endif // RLED_LOWBAT_FOLLOW_EN

#ifndef SLEEP_DAC_OFF_EN
#define SLEEP_DAC_OFF_EN            1
#endif // SLEEP_DAC_OFF_EN

#ifndef PWRON_FRIST_BAT_EN
#define PWRON_FRIST_BAT_EN          1
#endif // PWRON_FRIST_BAT_EN

#if !LED_DISP_EN
#undef LED_LOWBAT_EN
#undef RLED_LOWBAT_FOLLOW_EN
#undef BLED_FFT_EN
#undef BLED_LOW2ON_EN
#undef BLED_CHARGE_FULL_EN
#undef BT_RECONN_LED_EN
#define LED_LOWBAT_EN              0
#define RLED_LOWBAT_FOLLOW_EN       0
#define BLED_FFT_EN                 0
#define BLED_LOW2ON_EN              0
#define BLED_CHARGE_FULL_EN         0
#define BT_RECONN_LED_EN            0
#endif

#if !LED_DISP_EN
#undef LED_INIT
#define LED_INIT()
#undef LED_SET_ON
#define LED_SET_ON()
#undef LED_SET_OFF
#define LED_SET_OFF()
#endif

#if !LED_PWR_EN
#undef LED_PWR_INIT
#define LED_PWR_INIT()
#undef LED_PWR_SET_ON
#define LED_PWR_SET_ON()
#undef LED_PWR_SET_OFF
#define LED_PWR_SET_OFF()
#endif

#undef GUI_LCD_EN
#define GUI_LCD_EN                      0
#undef  MUSIC_NAVIGATION_EN
#define MUSIC_NAVIGATION_EN             0

#ifndef LOUDSPEAKER_MUTE_EN
#define LOUDSPEAKER_MUTE_EN             0
#endif

#if !LOUDSPEAKER_MUTE_EN
#undef LOUDSPEAKER_MUTE_INIT
#define LOUDSPEAKER_MUTE_INIT()
#undef LOUDSPEAKER_MUTE_DIS
#define LOUDSPEAKER_MUTE_DIS()
#undef LOUDSPEAKER_MUTE
#define LOUDSPEAKER_MUTE()
#undef LOUDSPEAKER_UNMUTE
#define LOUDSPEAKER_UNMUTE()
#undef LOUDSPEAKER_UNMUTE_DELAY
#define LOUDSPEAKER_UNMUTE_DELAY        0
#endif

#ifndef IR_INPUT_NUM_MAX
#define IR_INPUT_NUM_MAX                999         //最大输入数字9999
#endif // IR_INPUT_NUM_MAX

#ifndef FMRX_THRESHOLD_VAL
#define FMRX_THRESHOLD_VAL              128
#endif // FMRX_THRESHOLD_VAL

#ifndef FMRX_SEEK_DISP_CH_EN
#define FMRX_SEEK_DISP_CH_EN            0
#endif // FMRX_SEEK_DISP_CH_EN

#ifndef SD_SOFT_DETECT_EN
#define SD_SOFT_DETECT_EN               0
#undef SD_IS_SOFT_DETECT
#define SD_IS_SOFT_DETECT()             0
#endif // SD_SOFT_DETECT_EN

#if ADKEY_MUX_LED_EN
#undef  ADKEY_EN
#undef  ADKEY_MUX_SDCLK_EN
#undef  ADKEY_PU10K_EN

#define ADKEY_EN                      1
#define ADKEY_MUX_SDCLK_EN            0
#define ADKEY_PU10K_EN                  0
#endif // ADKEY_MUX_LED_EN

#if ((SD0_MAPPING == SD0MAP_G5) || (SD0_MAPPING == SD0MAP_G4)) && MUSIC_UDISK_EN && MUSIC_SDCARD_EN
#define SD_USB_MUX_IO_EN				0
#else
#define SD_USB_MUX_IO_EN				0
#endif

#ifndef DAC_DRC_EN
#define DAC_DRC_EN                      0
#endif

#if !PWRKEY_EN
#undef PWRKEY_IS_PRESS
#define PWRKEY_IS_PRESS()               0
#endif

#if !TKEY_EN
#undef  TKEY_SOFT_PWR_EN
#undef  TKEY_INEAR_EN
#undef  TKEY_DEBUG_EN
#undef  TKEY_IS_PRESS
#undef  TKEY_TEMP_EN
#define TKEY_SOFT_PWR_EN                0
#define TKEY_INEAR_EN                   0
#define TKEY_DEBUG_EN                   0
#define TKEY_IS_PRESS()                 0
#define TKEY_TEMP_EN                    0
#else
#undef  PWRKEY_EN
#define PWRKEY_EN                       0
#endif

#if TKEY_INEAR_EN
#undef  TKEY_TEMP_EN
#define TKEY_TEMP_EN                    1
#endif // TKEY_INEAR_EN

#if !TKEY_SOFT_PWR_EN
#undef TKEY_LOWPWR_WAKEUP_DIS
#define TKEY_LOWPWR_WAKEUP_DIS          0
#endif

#if !USER_INEAR_DETECT_EN
#undef TKEY_INEAR_EN
#undef USER_INEAR_DET_OPT
#undef INEAR_IS_ONLINE
#define TKEY_INEAR_EN                   0
#define USER_INEAR_DET_OPT              0
#define INEAR_IS_ONLINE()               0
#endif

#if !USER_INEAR_DET_OPT
#undef INEAR_OPT_PORT_INIT
#define INEAR_OPT_PORT_INIT()
#endif

/*****************************************************************************
 * Module    : 录音功能配置
 *****************************************************************************/
#if !FUNC_REC_EN
#undef  FMRX_REC_EN
#undef  AUX_REC_EN
#undef  MIC_REC_EN
#undef  REC_TYPE_SEL
#undef  REC_AUTO_PLAY
#undef  REC_FAST_PLAY
#undef  BT_REC_EN
#undef  BT_HFP_REC_EN
#undef  KARAOK_REC_EN
#undef  REC_STOP_MUTE_1S

#define FMRX_REC_EN                 0
#define AUX_REC_EN                  0
#define MIC_REC_EN                  0
#define REC_AUTO_PLAY               0
#define REC_FAST_PLAY               0
#define BT_REC_EN                   0
#define BT_HFP_REC_EN               0
#define KARAOK_REC_EN               0
//#define REC_TYPE_SEL                REC_NO
#define REC_STOP_MUTE_1S            0
#endif //FUNC_REC_EN


#if FMRX_INSIDE_EN
#undef  FMRX_QN8035_EN
#define FMRX_QN8035_EN              0
//#else
//#undef  I2C_SW_EN
//#define I2C_SW_EN                   1
#endif

#if !I2C_SW_EN
#undef  I2C_MUX_SD_EN
#define I2C_MUX_SD_EN               0
#endif


/*****************************************************************************
 * Module    : karaok相关配置
 *****************************************************************************/
#if SYS_KARAOK_EN
#undef FUNC_SPEAKER_EN
#undef BT_HFP_REC_EN

#define FUNC_SPEAKER_EN             0
#define BT_HFP_REC_EN               0   //Karaok不支持通话录音

#else
#undef SYS_ECHO_EN
#undef HIFI4_ECHO_EN
#undef HIFI4_REVERB_EN
#undef SYS_MAGIC_VOICE_EN
#undef HIFI4_PITCH_SHIFT_EN
#undef HIFI4_HOWLING_EN
#undef SYS_HOWLING_EN
#undef KARAOK_REC_EN

#define SYS_ECHO_EN                 0
#define HIFI4_ECHO_EN               0
#define HIFI4_REVERB_EN             0
#define SYS_MAGIC_VOICE_EN          0
#define HIFI4_PITCH_SHIFT_EN        0
#define HIFI4_HOWLING_EN            0
#define SYS_HOWLING_EN              0
#define KARAOK_REC_EN               0
#endif // SYS_KARAOK_EN

#if SYS_HOWLING_EN
#undef SYS_MAGIC_VOICE_EN
#define SYS_MAGIC_VOICE_EN          1
#endif


/*****************************************************************************
 * Module    : 蓝牙相关配置
 *****************************************************************************/
#if (LE_AB_LINK_APP_EN && AB_MATE_APP_EN)
#error "APP: please don't open LE_AB_LINK_APP_EN & AB_MATE_APP_EN at the same time\n"
#endif

#if (LE_AB_LINK_APP_EN && LE_USER_APP_EN)
#error "APP: please don't open LE_AB_LINK_APP_EN & LE_USER_APP_EN at the same time\n"
#endif

#if (AB_MATE_APP_EN && LE_USER_APP_EN)
#error "APP: please don't open AB_MATE_APP_EN & LE_USER_APP_EN at the same time\n"
#endif


#if LE_AB_LINK_APP_EN || AB_MATE_APP_EN || GFPS_EN || LE_USER_APP_EN || LE_PRIV_EN
    #define LE_EN                   1
    #define BT_DUAL_MODE_EN         1
#else
    #define LE_EN                   0
    #define BT_DUAL_MODE_EN         0
#endif

#if GFPS_EN
    #define LE_ADDRESS_TYPE         3       //GAP_RANDOM_ADDRESS_RESOLVABLE
#else
    #define LE_ADDRESS_TYPE         0       //GAP_RANDOM_ADDRESS_TYPE_OFF
#endif

#if !LE_EN
#undef LE_AB_FOT_EN
#undef LE_PAIR_EN
#undef LE_SM_SC_EN
#define LE_AB_FOT_EN                0
#define LE_PAIR_EN                  0
#define LE_SM_SC_EN                 0
#endif

#if GFPS_EN
#undef LE_ADV_POWERON_EN
#define LE_ADV_POWERON_EN          0
#endif

#ifndef BT_A2DP_RECON_EN
#define BT_A2DP_RECON_EN            0
#endif

#if BT_BACKSTAGE_EN
#undef MUSIC_WMA_SUPPORT
#undef MUSIC_FLAC_SUPPORT
#undef FMRX_THRESHOLD_FZ

#define MUSIC_WMA_SUPPORT           0
#define MUSIC_FLAC_SUPPORT          0
#define FMRX_THRESHOLD_FZ           0
#endif

#ifndef BT_HFP_RINGS_BEFORE_NUMBER
#define BT_HFP_RINGS_BEFORE_NUMBER  0
#endif

#ifndef BT_A2DP_AAC_AUDIO_EN
#define BT_A2DP_AAC_AUDIO_EN        0
#endif // BT_A2DP_AAC_AUDIO_EN

#if !BT_A2DP_EN
#undef  BT_A2DP_AAC_AUDIO_EN
#define BT_A2DP_AAC_AUDIO_EN        0
#endif

#if BT_FCC_TEST_EN || LE_BQB_RF_EN    //FCC 默认PB3 (USB_DP) 波特率1500000通信, 关闭用到PB3的程序
#undef FUNC_USBDEV_EN
#undef MUSIC_UDISK_EN
#undef ADAPTER_USB_MIC_RX_EN
#define FUNC_USBDEV_EN             0
#define MUSIC_UDISK_EN             0
#define ADAPTER_USB_MIC_RX_EN      0
//#if (UART0_PRINTF_SEL == PRINTF_PB3)
//#undef UART0_PRINTF_SEL
//#define UART0_PRINTF_SEL  PRINTF_NONE
//#endif
#endif

#ifndef BT_TWS_EN
#define BT_TWS_EN                   0
#endif

#if !BT_TWS_EN
#undef BT_TWS_SCO_EN
#undef BT_TWS_PAIR_MODE
#undef BT_TWS_PAIR_ID
#undef BT_TWS_PAIR_BONDING_EN
#undef BT_TWS_MS_SWITCH_EN
#undef BLE_TWS_MS_SWITCH_EN
#undef BT_TWS_DBG_EN
#define BT_TWS_SCO_EN               0
#define BT_TWS_PAIR_MODE            0
#define BT_TWS_PAIR_ID              0
#define BT_TWS_PAIR_BONDING_EN      0
#define BT_TWS_MS_SWITCH_EN         0
#define BLE_TWS_MS_SWITCH_EN        0
#define BT_TWS_DBG_EN               0

#undef FOT_SUPPORT_TWS
#define FOT_SUPPORT_TWS             0
#undef BT_TWS_DBG_EN
#define BT_TWS_DBG_EN               0
#undef WARNING_TWS_CHANNEL
#define WARNING_TWS_CHANNEL         0
#endif

#if BT_TWS_DBG_EN
    #if !BT_SPP_EN
    #error "TWS_DBG: please open BT_TWS_DBG_EN & BT_SPP_EN at the same time\n"
    #endif
#endif // BT_TWS_DBG_EN

#if AB_MATE_APP_EN
#undef EQ_APP_EN
#undef BT_AVRCP_PLAY_STATUS_EN
#define EQ_APP_EN                   1
#define BT_AVRCP_PLAY_STATUS_EN    1

#undef LE_AB_FOT_EN
#undef BT_AB_FOT_EN
#define LE_AB_FOT_EN                0
#define BT_AB_FOT_EN                0
#endif

#if BT_AB_FOT_EN || LE_AB_FOT_EN
#undef FOT_EN
#define FOT_EN                      1   //是否打开FOTA升级功能
#endif

#if BT_AB_FOT_EN
#if !BT_SPP_EN
#error "FOTA: please open BT_AB_FOT_EN & BT_SPP_EN at the same time\n"
#endif
#endif // BT_AB_FOT_EN

#if BT_HID_VOL_CTRL_EN
#undef  BT_HID_EN
#undef  BT_A2DP_VOL_CTRL_EN

#define BT_HID_EN                    1
#define BT_A2DP_VOL_CTRL_EN          1
#endif // BT_HID_VOL_CTRL_EN

#if !EQ_APP_EN
#undef EQ_APP_NUM
#define EQ_APP_NUM                      0
#endif

#if BT_A2DP_LHDC_AUDIO_EN || BT_A2DP_LDAC_AUDIO_EN
#undef DAC_OUT_SPR
#define DAC_OUT_SPR                     DAC_OUT_96K
#define BT_A2DP_VENDOR_AUDIO_EN         1
#endif


/*****************************************************************************
 * Module    : 通话算法相关配置
 *****************************************************************************/
#define BT_SCO_SMIC_EN                  0
#define BT_SCO_DMIC_EN                  0

#if (BT_SCO_SMIC_AI_EN || BT_SCO_SMIC_AI_PRO_EN)
#undef BT_SCO_SMIC_EN
#define BT_SCO_SMIC_EN                  1
#undef BT_AEC_FF_MIC_REF_EN
#define BT_AEC_FF_MIC_REF_EN            0
#endif

#if !BT_SCO_DMIC_EN && !BT_SCO_SMIC_EN
#undef ENC_DBG_EN
#define ENC_DBG_EN                      0
#endif

#if BT_SCO_SMIC_EN && BT_SCO_DMIC_EN
#error "NR cfg err: please choose only one of the NR algorithms at the same time!\n"
#endif

#if (BT_SCO_DUMP_EN || BT_EQ_DUMP_EN) && ((CHARGE_BOX_TYPE == CBOX_SSW) || QTEST_EN)
#warning "DUMP warning: please select only one nr algorithm in BT_XXX_DUMP_EN, VUSB_XXX_EN\n"
#endif

#if (BT_SCO_DUMP_EN + BT_EQ_DUMP_EN) > 1
#error "DUMP warning: please select only one dump in BT_SCO_DUMP_EN, , BT_EQ_DUMP_EN\n"
#endif

#if !WIRELESS_CON_AND_BT_PAIRING_EN
//关闭通话相关算法
#undef BT_SCO_EQ_DRC_EN
#undef BT_SCO_AGC_EN
#undef BT_SCO_SMIC_AI_EN
#undef BT_SCO_SMIC_AI_PRO_EN
#define BT_SCO_EQ_DRC_EN                        0
#define BT_SCO_AGC_EN                           0
#define BT_SCO_SMIC_AI_EN                       0
#define BT_SCO_SMIC_AI_PRO_EN                   0
#endif

/*****************************************************************************
 * Module    : 无线mic功能选择配置
 *****************************************************************************/
#define WIRELESS_EN                             (FUNC_ADAPTER_EN || FUNC_DEVICE_EN)

#if !WIRELESS_EN
    #undef WIRELESS_MIC_EN
    #undef WIRELESS_SPK_EN
    #undef WIRELESS_MIC_CMD_EN
    #define WIRELESS_MIC_EN                     0
    #define WIRELESS_SPK_EN                     0
    #define WIRELESS_MIC_CMD_EN                 0
#else
  #if !WIRELESS_MIC_EN && !WIRELESS_SPK_EN
    #error "please enable at least one of WIRELESS_MIC_EN and WIRELESS_SPK_EN."
  #endif
#endif

#if FUNC_ADAPTER_EN && FUNC_DEVICE_EN
    #define WIRELESS_MIC_ROLE                   2       //0=无线麦TX, 1=无线麦RX, 2=同时支持TX&RX（通过配置选择）
#elif FUNC_ADAPTER_EN
    #define WIRELESS_MIC_ROLE                   1       //0=无线麦TX, 1=无线麦RX, 2=同时支持TX&RX（通过配置选择）
#elif FUNC_DEVICE_EN
    #define WIRELESS_MIC_ROLE                   0       //0=无线麦TX, 1=无线麦RX, 2=同时支持TX&RX（通过配置选择）
#else
    #define WIRELESS_MIC_ROLE                   0xff    //0=无线麦TX, 1=无线麦RX, 2=同时支持TX&RX（通过配置选择）
#endif

#if !FUNC_ADAPTER_EN
    #undef ADAPTER_DAC_OUTPUT_EN
    #undef ADAPTER_I2S_OUTPUT_EN
    #undef ADAPTER_USB_MIC_RX_EN
    #define ADAPTER_DAC_OUTPUT_EN               0
    #define ADAPTER_I2S_OUTPUT_EN               0
    #define ADAPTER_USB_MIC_RX_EN               0
#endif

#if WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_16K
    #define FRAME_SIZE_MIN                      20      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_24K
    #define FRAME_SIZE_MIN                      30      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_32K
    #define FRAME_SIZE_MIN                      40      //1.25ms
#elif WIRELESS_MIC_SAMPLE_RATE_SELECT == SAMPLE_RATE_48K
    #define FRAME_SIZE_MIN                      60      //1.25ms
#else
    #error "WIRELESS_MIC_SAMPLE_RATE_SELECT is not allowed!"
#endif

#if WIRELESS_MIC_SAMPLES_SELECT < FRAME_SIZE_MIN
    #error "WIRELESS_MIC_SAMPLES_SELECT is not allowed!"
#endif
#if (WIRELESS_MIC_SAMPLES_SELECT % FRAME_SIZE_MIN) != 0
    #error "WIRELESS_MIC_SAMPLES_SELECT is not allowed!"
#endif
#define WIRELESS_MIC_PCM_MODE                   (WIRELESS_MIC_CHANNEL_SELECT | (WIRELESS_MIC_24B_PCM_EN * PCM_24BIT))

#define WIRELESS_MIC_DFU_TX_INTERVAL            (WIRELESS_MIC_SAMPLES_SELECT/FRAME_SIZE_MIN)
#ifndef WIRELESS_MIC_TX_INTERVAL
    #define WIRELESS_MIC_TX_INTERVAL            (WIRELESS_MIC_SAMPLES_SELECT/FRAME_SIZE_MIN)
#endif

#if WIRELESS_CON_VERS==2 || WIRELESS_CON_VERS==3 || WIRELESS_CON_VERS==5 || WIRELESS_CON_VERS==6 || WIRELESS_CON_VERS == 9
#if ADAPTER_SAVE_PARAM_EN
    #define WIRELESS_CON_INTERVAL               12
#else
    #define WIRELESS_CON_INTERVAL               60
#endif
    #define WIRELESS_CON_COMB_BUF_EN            0       //单包
    #define WIRELESS_CON_CRC_EN                 1

  #if WIRELESS_MIC_TX_INTERVAL == 0
    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
  #elif WIRELESS_MIC_TX_INTERVAL == 1                   //1.25ms传不了几byte，组合成2.5ms
    #undef WIRELESS_MIC_TX_INTERVAL
    #define WIRELESS_MIC_TX_INTERVAL            2
    #define WIRELESS_MIC_COMB_NB                WIRELESS_MIC_TX_INTERVAL
    #define WIRELESS_SPK_COMB_NB                WIRELESS_MIC_TX_INTERVAL
  #else
    #define WIRELESS_MIC_COMB_NB                (WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_DFU_TX_INTERVAL)
    #define WIRELESS_SPK_COMB_NB                (WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_DFU_TX_INTERVAL)
  #endif
  #if (WIRELESS_MIC_SAMPLES_SELECT*WIRELESS_MIC_COMB_NB)%(WIRELESS_MIC_TX_INTERVAL*FRAME_SIZE_MIN) != 0
    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
  #endif
  #if WIRELESS_MIC_TX_INTERVAL == 0
    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
  #endif
//  #if WIRELESS_MIC_SAMPLES_SELECT%(WIRELESS_MIC_TX_INTERVAL*FRAME_SIZE_MIN) != 0
//    #error "WIRELESS_MIC_TX_INTERVAL is not allowed!"
//  #endif
  #if (WIRELESS_CON_INTERVAL*WIRELESS_CON_LINK_NB)%WIRELESS_MIC_TX_INTERVAL != 0
    #error "WIRELESS_CON_INTERVAL is not allowed!"
  #endif
#else
    #error "WIRELESS_CON_VERS is not allowed!"
#endif

#if WIRELESS_MIC_COMB_NB == 0
    #undef WIRELESS_MIC_COMB_NB
    #define WIRELESS_MIC_COMB_NB                1
#endif


#if !WIRELESS_SPK_EN
    #undef WIRELESS_SPK_SAMPLE_RATE_SELECT
    #undef WIRELESS_SPK_SAMPLES_SELECT
    #undef WIRELESS_SPK_CHANNEL_SELECT
    #undef WIRELESS_SPK_24B_PCM_EN
    #undef WIRELESS_SPK_FRAME_SIZE
    #undef WIRELESS_SPK_RETRY_NB
    #undef WIRELESS_SPK_ENC_MAX_US
    #undef WIRELESS_SPK_DEC_MAX_US
    #undef WIRELESS_SPK_SINGLE_PLC_EN
    #define WIRELESS_SPK_SAMPLE_RATE_SELECT         WIRELESS_MIC_SAMPLE_RATE_SELECT
    #define WIRELESS_SPK_SAMPLES_SELECT             WIRELESS_MIC_SAMPLES_SELECT
    #define WIRELESS_SPK_CHANNEL_SELECT             1
    #define WIRELESS_SPK_24B_PCM_EN                 0               //无线mic 24bit PCM使能
    #define WIRELESS_SPK_FRAME_SIZE                 25              //每帧压缩后大小
    #define WIRELESS_SPK_RETRY_NB                   1               //重传次数（LINK_NB=1时支持1或2， LINK_NB=2时只支持1）
    #define WIRELESS_SPK_ENC_MAX_US                 650
    #define WIRELESS_SPK_DEC_MAX_US                 700
    #define WIRELESS_SPK_SINGLE_PLC_EN              0
#endif


#if WIRELESS_CON_AND_BT_PAIRING_EN
    #undef WIRELESS_CON_ADV_EXT_EN
    #undef WIRELESS_CON_BONDING_EN
    #define WIRELESS_CON_ADV_EXT_EN             1
    #define WIRELESS_CON_BONDING_EN             0

    #undef BT_DISCOVER_CTRL_EN
    #undef BT_HFP_EN
    #define BT_DISCOVER_CTRL_EN                 1
    #define BT_HFP_EN                           1
#elif WIRELESS_CON_BONDING_EN
    #undef WIRELESS_CON_ADV_EXT_EN
    #define WIRELESS_CON_ADV_EXT_EN             1
#endif

#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    #undef WIRELESS_CON_INTERVAL
    #define WIRELESS_CON_INTERVAL               WIRELESS_MIC_TX_INTERVAL*2
#endif

#if WIRELESS_CON_VERS == 9
#define WIRELESS_MIC_2TNR_EN                    1
#define WIRELESS_CON_2TNR_NB                    2           ///两发N收的rx数量，最大不超过4
#endif

/*****************************************************************************
 * Module    : 对讲机相关配置
 *****************************************************************************/
///暂时先放在extra中配置给客户测试，后续独立成单独config.h文件
#if DEVICE_INTERPHONE_EN
    #undef WIRELESS_CON_LINK_NB
    #undef WIRELESS_SPK_EN
    #undef DEVICE_WL_CON_CLK
    #define WIRELESS_CON_LINK_NB                    1               //连接设备数
    #define WIRELESS_SPK_EN                         1
    #define DEVICE_WL_CON_CLK                       SYS_160M
#endif

/*****************************************************************************
 * Module    : 算法相关配置
 *****************************************************************************/
#if YLCRN_L3_EN && (!WIRELESS_MIC_24B_PCM_EN)
    #error "YLCRN_L3_EN is not allowed 16bit!"
#endif

#if !YLCRN_L3_EN
    #undef YLCRN_L3_DUMP_EN
    #define YLCRN_L3_DUMP_EN               0
#endif

#define AUDIO_DUMP_EN                           (YLCRN_L3_DUMP_EN || AINS5_DUMP_EN || YLCRN_L2_DUMP_EN || AINS4_DUMP_EN || DNR_FRE_DUMP_EN)

#define NR_ALG_EN                               (YLCRN_L3_EN)


/*****************************************************************************
 * Module    : ANC相关配置
 *****************************************************************************/
#if ANC_EN
#undef SYS_ADJ_DIGVOL_EN
#undef TINY_TRANSPARENCY_EN

#define SYS_ADJ_DIGVOL_EN           1       //DAC固定模拟增益调数字增益
#define TINY_TRANSPARENCY_EN        0       //开ANC不能打开小通透

#if !(BT_A2DP_LHDC_AUDIO_EN || BT_A2DP_LDAC_AUDIO_EN)
#undef DAC_OUT_SPR
#define DAC_OUT_SPR                 DAC_OUT_48K
#endif
#endif

#if TINY_TRANSPARENCY_EN
#undef SYS_ADJ_DIGVOL_EN

#define SYS_ADJ_DIGVOL_EN           1       //DAC固定模拟增益调数字增益
#endif

#if !ANC_EN
#undef ANC_EQ_RES2_EN
#define ANC_EQ_RES2_EN              0        //如果ANC关闭的情况下，不能开启优先使用资源2
#endif

#if ANC_EQ_RES2_EN
#undef FLASH_RESERVE_SIZE
#define FLASH_RESERVE_SIZE          0x6000
#endif

#if ANC_ALG_EN
#if ANC_ALG_HOWLING_FB_EN ||            \
    ANC_ALG_FIT_DETECT_FF_FB_EN ||      \
    ANC_ALG_HOWLING_FF_EN ||            \
    ANC_ALG_WIND_NOISE_FF_FB_EN ||      \
    ANC_ALG_LIMITER_FF_EN ||            \
    ANC_ALG_DUMP_EN ||                  \
    ANC_ALG_MSC_ADP_FB_EN ||            \
    ANC_ALG_ADP_EQ_FF_FB_EN ||          \
    ANC_ALG_AEM_RT_FF_FB_EN
#undef ANC_DS_DMA_EN
#define ANC_DS_DMA_EN                   1
#endif
#if ANC_ALG_STEREO_EN
#undef SDADC_5CH_EN
#define SDADC_5CH_EN                    1
#endif // ANC_ALG_STEREO_EN
#else
#undef ANC_ALG_WIND_NOISE_FF_TALK_EN
#undef ANC_ALG_WIND_NOISE_FF_FB_EN
#undef ANC_ALG_ASM_SIM_FF_EN
#undef ANC_ALG_HOWLING_FB_EN
#undef ANC_ALG_FIT_DETECT_FF_FB_EN
#undef ANC_ALG_HOWLING_FF_EN
#undef ANC_ALG_AI_WN_FF_EN
#undef ANC_ALG_AI_WN_MINI_FF_EN
#undef ANC_ALG_LIMITER_FF_EN
#undef ANC_ALG_DYVOL_FF_EN
#undef ANC_ALG_MSC_ADP_FB_EN
#undef ANC_ALG_ADP_EQ_FF_FB_EN
#undef ANC_ALG_ASM_FF_EN
#undef ANC_ALG_AEM_RT_FF_FB_EN
#undef ANC_SNDP_SAE_SHIELD_ADAPTER_EN
#undef ANC_SNDP_SAE_WIND_DETECT_EN
#undef ANC_ALG_USER_EN
#undef ANC_ALG_DUMP_EN
#undef ANC_ALG_DBG_EN
#define ANC_ALG_WIND_NOISE_FF_TALK_EN   0
#define ANC_ALG_WIND_NOISE_FF_FB_EN     0
#define ANC_ALG_ASM_SIM_FF_EN           0
#define ANC_ALG_HOWLING_FB_EN           0
#define ANC_ALG_FIT_DETECT_FF_FB_EN     0
#define ANC_ALG_HOWLING_FF_EN           0
#define ANC_ALG_AI_WN_FF_EN				0
#define ANC_ALG_AI_WN_MINI_FF_EN        0
#define ANC_ALG_LIMITER_FF_EN           0
#define ANC_ALG_DYVOL_FF_EN             0
#define ANC_ALG_MSC_ADP_FB_EN           0
#define ANC_ALG_ADP_EQ_FF_FB_EN         0
#define ANC_ALG_ASM_FF_EN               0
#define ANC_ALG_AEM_RT_FF_FB_EN         0
#define ANC_SNDP_SAE_SHIELD_ADAPTER_EN  0
#define ANC_SNDP_SAE_WIND_DETECT_EN     0
#define ANC_ALG_USER_EN                 0
#define ANC_ALG_DUMP_EN                 0
#define ANC_ALG_DBG_EN                  0
#endif // ANC_ALG_EN

#if ANC_ALG_ADP_EQ_FF_FB_EN
#undef DAC_EQ_AFTER_VOL_EN
#define DAC_EQ_AFTER_VOL_EN             1
#define ANC_ALG_ADP_EQ_NUM              1
#else
#define ANC_ALG_ADP_EQ_NUM              0
#endif

#if ANC_ALG_HOWLING_FB_EN && ANC_ALG_HOWLING_FF_EN
#error "ANC ALG: please select only one howling algorithm in ANC_ALG_HOWLING_FB_EN and ANC_ALG_HOWLING_FF_EN"
#endif

#if (ANC_ALG_WIND_NOISE_FF_TALK_EN + ANC_ALG_AI_WN_FF_EN + ANC_ALG_WIND_NOISE_FF_FB_EN + ANC_ALG_AI_WN_MINI_FF_EN) > 1
#error "ANC ALG: please select only one wind noise detect algorithm"
#endif

#if !ANC_ALG_DUMP_EN && ANC_ALG_DUMP_FOR_ANC_MODE
#error "ANC ALG: please set the macro ANC_ALG_DUMP_EN"
#endif

#if ANC_TOOL_EN

#undef ANC_DS_DMA_EN
#define ANC_DS_DMA_EN                   1

#if !ANC_EN
#error "ANC TOOL: please set the macro ANC_EN"
#endif

#if ANC_TOOL_SPP_EN && (!BT_SPP_EN)
#error "ANC TOOL: please set the macro BT_SPP_EN"
#endif

#else
#undef ANC_TOOL_SPP_EN
#undef ANC_TOOL_LINK_FB_MSC_SIMU_EN
#define ANC_TOOL_SPP_EN                 1
#define ANC_TOOL_LINK_FB_MSC_SIMU_EN    1
#endif // ANC_TOOL_EN

/*****************************************************************************
 * Module    : uart0 printf 功能自动配置(自动关闭SD卡，USB)
 *****************************************************************************/
#if (UART0_PRINTF_SEL == PRINTF_PB2)
    #if (SD0_MAPPING == SD0MAP_G2)
    #undef  MUSIC_SDCARD_EN
    #define MUSIC_SDCARD_EN         0
    #endif
#elif (UART0_PRINTF_SEL == PRINTF_PA7)
    #if (SD0_MAPPING == SD0MAP_G1)
    #undef  MUSIC_SDCARD_EN
    #define MUSIC_SDCARD_EN         0
    #endif
#elif (UART0_PRINTF_SEL == PRINTF_PB3)
    #undef  FUNC_USBDEV_EN
    #undef  MUSIC_UDISK_EN
    #define FUNC_USBDEV_EN          0
    #define MUSIC_UDISK_EN          0
#elif (UART0_PRINTF_SEL == PRINTF_PE13)
    #if (SD0_MAPPING == SD0MAP_G4)
    #undef  MUSIC_SDCARD_EN
    #define MUSIC_SDCARD_EN         0
    #endif
#elif (UART0_PRINTF_SEL == PRINTF_NONE)
    //关闭所以打印信息
    #undef printf
    #undef vprintf
    #undef print_r
    #undef print_r16
    #undef print_r32
    #undef printk
    #undef vprintk
    #undef print_kr
    #undef print_kr16
    #undef print_kr32

    #define printf(...)
    #define vprintf(...)
    #define print_r(...)
    #define print_r16(...)
    #define print_r32(...)
    #define printk(...)
    #define vprintk(...)
    #define print_kr(...)
    #define print_kr16(...)
    #define print_kr32(...)
#endif

#if !MUSIC_SDCARD_EN
#undef SD_DETECT_INIT
#undef SD_IS_ONLINE
#undef SD_DETECT_IS_BUSY

#define SD_DETECT_INIT()
#define SD_IS_ONLINE()              0
#define SD_DETECT_IS_BUSY()            0
#endif


/*****************************************************************************
 * Module    : usb device 功能配置
 *****************************************************************************/
#if !FUNC_USBDEV_EN && !ADAPTER_USB_MIC_RX_EN && !DEVICE_USB_EN
    #undef  UDE_STORAGE_EN
    #undef  UDE_SPEAKER_EN
    #undef  UDE_HID_EN
    #undef  UDE_MIC_EN
    #undef  UDE_ENUM_TYPE

    #define UDE_STORAGE_EN              0
    #define UDE_SPEAKER_EN              0
    #define UDE_HID_EN                  0
    #define UDE_MIC_EN                  0
    #define UDE_ENUM_TYPE               0
#elif (ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN)
    #undef  UDE_ENUM_TYPE

    #define UDE_ENUM_TYPE               (UDE_STORAGE_EN*0x01 + UDE_SPEAKER_EN*0x02 + UDE_HID_EN*0x04 + UDE_MIC_EN*0x08 + UDE_IAP_EN*0x10)
#else
    #define UDE_ENUM_TYPE               (UDE_STORAGE_EN*0x01 + UDE_SPEAKER_EN*0x02 + UDE_HID_EN*0x04 + UDE_MIC_EN*0x08)
#endif

/*****************************************************************************
 * Module    : iis 功能配置
 *****************************************************************************/
#if I2S_EN

#if I2S_MODE_SEL
#undef  I2S_DMA_EN
#define I2S_DMA_EN                      1       //slave模式必须打开DMA功能
#endif

#endif // I2S_EN

/*****************************************************************************
 * Module    : 提示音配置
 *****************************************************************************/
#if (!WARNING_TONE_EN)
#undef WARNING_WSBC_RES_EN
#undef WARNING_MP3_RES_EN
#undef WARNING_WAV_RES_EN
#undef WARNING_PIANO_RES_EN
#define WARNING_WSBC_RES_EN             0
#define WARNING_MP3_RES_EN              0
#define WARNING_WAV_RES_EN              0
#define WARNING_PIANO_RES_EN            0

#undef WARNING_POWER_ON
#undef WARNING_POWER_OFF
#undef WARNING_FUNC_MUSIC
#undef WARNING_FUNC_BT
#undef WARNING_FUNC_CLOCK
#undef WARNING_FUNC_FMRX
#undef WARNING_FUNC_AUX
#undef WARNING_FUNC_USBDEV
#undef WARNING_FUNC_SPEAKER
#undef WARNING_LOW_BATTERY
#undef WARNING_BT_WAIT_CONNECT
#undef WARNING_BT_PAIR
#undef WARNING_BT_CONNECT
#undef WARNING_BT_DISCONNECT
#undef WARNING_BT_LOW_LATENCY
#undef WARNING_BT_CALL_CTRL
#undef WARNING_BT_RING_NUMBER
#undef WARNING_LEFT_RIGHT_CH
#undef WARNING_USB_SD
#undef WARNING_BT_HID_MENU
#undef WARNING_BTHID_CONN
#undef WARNING_MAX_VOLUME

#define WARNING_POWER_ON               0
#define WARNING_POWER_OFF              0
#define WARNING_FUNC_MUSIC             0
#define WARNING_FUNC_BT                0
#define WARNING_FUNC_CLOCK             0
#define WARNING_FUNC_FMRX              0
#define WARNING_FUNC_AUX               0
#define WARNING_FUNC_USBDEV            0
#define WARNING_FUNC_SPEAKER           0
#define WARNING_LOW_BATTERY            0
#define WARNING_BT_WAIT_CONNECT        0
#define WARNING_BT_PAIR                0
#define WARNING_BT_CONNECT             0
#define WARNING_BT_DISCONNECT          0
#define WARNING_BT_LOW_LATENCY         0
#define WARNING_BT_CALL_CTRL           0
#define WARNING_BT_RING_NUMBER         0
#define WARNING_LEFT_RIGHT_CH          0
#define WARNING_USB_SD                 0
#define WARNING_BT_HID_MENU            0
#define WARNING_BTHID_CONN             0
#define WARNING_MAX_VOLUME             0
#endif

#if !FUNC_MUSIC_EN
#undef WARNING_USB_SD
#define WARNING_USB_SD                 0
#endif

#if !FUNC_AUX_EN
#undef  WARNING_FUNC_AUX
#define WARNING_FUNC_AUX               0
#endif

#if !FUNC_FMRX_EN
#undef  WARNING_FUNC_FMRX
#define WARNING_FUNC_FMRX              0
#endif

#if !FUNC_USBDEV_EN
#undef  WARNING_FUNC_USBDEV
#define WARNING_FUNC_USBDEV            0
#endif

#if !FUNC_SPEAKER_EN
#undef  WARNING_FUNC_SPEAKER
#define WARNING_FUNC_SPEAKER           0
#endif

#if !BT_HFP_RING_NUMBER_EN
    #undef WARNING_BT_RING_NUMBER
    #define WARNING_BT_RING_NUMBER         0
#endif

#if !BT_A2DP_LOW_LATENCY_EN
    #undef WARNING_BT_LOW_LATENCY
    #define WARNING_BT_LOW_LATENCY         0
#endif

#if ((!MUSIC_UDISK_EN) && (!MUSIC_SDCARD_EN))
#undef  USB_SD_UPDATE_EN
#define USB_SD_UPDATE_EN               0
#endif

#if SD_USB_MUX_IO_EN == 1
	#undef FUNC_USBDEV_EN
	#define FUNC_USBDEV_EN 			   0
#endif

#ifndef UPD_FILENAME
#define UPD_FILENAME                   "fw5000.upd"
#endif

#if ((SD0_MAPPING == SD0MAP_G1) || (SD0_MAPPING == SD0MAP_G4) || (SD0_MAPPING == SD0MAP_G5) || (SD0_MAPPING == SD0MAP_G6))
#define SDCLK_AD_CH                    ADCCH_PA6
#elif (SD0_MAPPING == SD0MAP_G2)
#define SDCLK_AD_CH                    ADCCH_PB1
#else
#define SDCLK_AD_CH                    ADCCH_PE6
#endif
#define USB_SUPPORT_EN                 (FUNC_USBDEV_EN || ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN)
#define SD_SUPPORT_EN                  (MUSIC_SDCARD_EN || UDE_STORAGE_EN)

#if SD_SUPPORT_EN
#define SDCMD_MUX_DETECT_EN            1
#define SDCLK_MUX_DETECT_EN            1
#else
#undef  ADKEY_MUX_SDCLK_EN
#undef  I2C_MUX_SD_EN

#define I2C_MUX_SD_EN                  0
#define SDCMD_MUX_DETECT_EN            0
#define SDCLK_MUX_DETECT_EN            0
#define ADKEY_MUX_SDCLK_EN           0
#endif // SD_SUPPORT_EN

#if !SD_SUPPORT_EN
#undef  SD_SOFT_DETECT_EN
#define SD_SOFT_DETECT_EN              0
#endif

#if (FUNC_MUSIC_EN+FUNC_CLOCK_EN+FUNC_BT_EN+FUNC_BTHID_EN+FUNC_AUX_EN+FUNC_SPEAKER_EN+FUNC_DEVICE_EN) > 1
    #define CHANGE_MODE_MSG_EN          1
#else
    #define CHANGE_MODE_MSG_EN          0
#endif

/*****************************************************************************
* Module    : 有冲突或功能上不能同时打开的宏
*****************************************************************************/
#if (SYS_KARAOK_EN && BT_BACKSTAGE_EN)
#error "SYS_KARAOK_EN and BT_BACKSTAGE_EN (include BT_APP_EN) can't open at sametime"
#endif

#define DAC_EQ_NUM_TOTAL               (DAC_PT_NUM + EQ_APP_NUM)

#if (DAC_EQ_NUM_TOTAL > 20)
#error "error: (DAC_PT_NUM + EQ_APP_NUM) > 20"
#endif
#endif // __CONFIG_EXTRA_H__

