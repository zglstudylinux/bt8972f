#include "include.h"

#define RES_BUF(n)              RES_BUF_MP3(n)          //主要格式，个别可单独修改tbl
#define RES_LEN(n)              RES_LEN_MP3(n)          //主要格式，个别可单独修改tbl


#if (LANG_SELECT == LANG_EN || LANG_SELECT == LANG_EN_ZH)
const res_addr_t en_lang_tbl[RES_IDX_MAX] = {
#if WARNING_POWER_ON
    [RES_IDX_POWERON        ] = {&RES_BUF(EN_POWERON),          &RES_LEN(EN_POWERON)},
#endif
#if WARNING_POWER_OFF
    [RES_IDX_POWEROFF       ] = {&RES_BUF(EN_POWEROFF),         &RES_LEN(EN_POWEROFF)},
#endif
#if (LANG_SELECT == LANG_EN_ZH)
    [RES_IDX_LANGUAGE_EN    ] = {&RES_BUF(EN_LANGUAGE),         &RES_LEN(EN_LANGUAGE)},
    [RES_IDX_LANGUAGE_ZH    ] = {&RES_BUF(ZH_LANGUAGE),         &RES_LEN(ZH_LANGUAGE)},
#endif
#if WARNING_LOW_BATTERY
    [RES_IDX_LOW_BATTERY    ] = {&RES_BUF(EN_LOW_BATTERY),      &RES_LEN(EN_LOW_BATTERY)},
#endif

#if WARNING_MAX_VOLUME
#if WARING_MAXVOL_TYPE == RES_TYPE_PIANO
    [RES_IDX_MAX_VOL        ] = {(u32 *)&RES_BUF_PIANO(PIANO_MAX_VOL),  (u32 *)&RES_LEN_PIANO(PIANO_MAX_VOL)},
#elif WARING_MAXVOL_TYPE == RES_TYPE_TONE
    [RES_IDX_MAX_VOL        ] = {(u32 *)&RES_BUF_TONE(TONE_MAX_VOL),    (u32 *)&RES_LEN_TONE(TONE_MAX_VOL)},
//#elif WARING_MAXVOL_TYPE == RES_TYPE_WAV
#elif WARING_MAXVOL_TYPE == RES_TYPE_MP3
    [RES_IDX_MAX_VOL        ] = {&RES_BUF_MP3(EN_MAX_VOL),      &RES_LEN_MP3(EN_MAX_VOL)},
#elif WARING_MAXVOL_TYPE == RES_TYPE_WSBC
    [RES_IDX_MAX_VOL        ] = {&RES_BUF_SBC(EN_MAX_VOL),      &RES_LEN_SBC(EN_MAX_VOL)},
#else
    [RES_IDX_MAX_VOL        ] = {&RES_BUF(EN_MAX_VOL),          &RES_LEN(EN_MAX_VOL)},
#endif
#endif

#if WARNING_USB_SD
    [RES_IDX_SDCARD_MODE    ] = {&RES_BUF(EN_SDCARD_MODE),      &RES_LEN(EN_SDCARD_MODE)},
    [RES_IDX_USB_MODE       ] = {&RES_BUF(EN_USB_MODE),         &RES_LEN(EN_USB_MODE)},
#endif
#if WARNING_FUNC_AUX
    [RES_IDX_AUX_MODE       ] = {&RES_BUF(EN_AUX_MODE),         &RES_LEN(EN_AUX_MODE)},
#endif
#if WARNING_FUNC_CLOCK
    [RES_IDX_CLOCK_MODE     ] = {&RES_BUF(EN_CLOCK_MODE),       &RES_LEN(EN_CLOCK_MODE)},
#endif
#if WARNING_FUNC_FMRX
    [RES_IDX_FM_MODE        ] = {&RES_BUF(EN_FM_MODE),          &RES_LEN(EN_FM_MODE)},
#endif
#if WARNING_FUNC_SPEAKER
    [RES_IDX_SPK_MODE       ] = {&RES_BUF(EN_SPK_MODE),         &RES_LEN(EN_SPK_MODE)},
#endif
#if WARNING_FUNC_USBDEV
    [RES_IDX_PC_MODE        ] = {&RES_BUF(EN_PC_MODE),          &RES_LEN(EN_PC_MODE)},
#endif
#if WARNING_FUNC_BT
    [RES_IDX_BT_MODE        ] = {&RES_BUF(EN_BT_MODE),          &RES_LEN(EN_BT_MODE)},
#endif
#if WARNING_LEFT_RIGHT_CH
    [RES_IDX_LEFT_CH        ] = {&RES_BUF(EN_LEFT_CH),          &RES_LEN(EN_LEFT_CH)},
    [RES_IDX_RIGHT_CH       ] = {&RES_BUF(EN_RIGHT_CH),         &RES_LEN(EN_RIGHT_CH)},
#endif
#if WARNING_BT_PAIR
    [RES_IDX_PAIRING        ] = {&RES_BUF(EN_PAIRING),          &RES_LEN(EN_PAIRING)},
#endif
#if WARNING_BT_WAIT_CONNECT
    [RES_IDX_WAIT4CONN      ] = {&RES_BUF(EN_WAIT4CONN),        &RES_LEN(EN_WAIT4CONN)},
#endif
#if WARNING_BT_CONNECT
    [RES_IDX_CONNECTED      ] = {&RES_BUF(EN_CONNECTED),        &RES_LEN(EN_CONNECTED)},
#endif
#if WARNING_BT_DISCONNECT
    [RES_IDX_DISCONNECT     ] = {&RES_BUF(EN_DISCONNECT),       &RES_LEN(EN_DISCONNECT)},
#endif
#if WARNING_BT_RING_NUMBER
    [RES_IDX_NUM_0          ] = {&RES_BUF(EN_NUM_0),            &RES_LEN(EN_NUM_0)},
    [RES_IDX_NUM_1          ] = {&RES_BUF(EN_NUM_1),            &RES_LEN(EN_NUM_1)},
    [RES_IDX_NUM_2          ] = {&RES_BUF(EN_NUM_2),            &RES_LEN(EN_NUM_2)},
    [RES_IDX_NUM_3          ] = {&RES_BUF(EN_NUM_3),            &RES_LEN(EN_NUM_3)},
    [RES_IDX_NUM_4          ] = {&RES_BUF(EN_NUM_4),            &RES_LEN(EN_NUM_4)},
    [RES_IDX_NUM_5          ] = {&RES_BUF(EN_NUM_5),            &RES_LEN(EN_NUM_5)},
    [RES_IDX_NUM_6          ] = {&RES_BUF(EN_NUM_6),            &RES_LEN(EN_NUM_6)},
    [RES_IDX_NUM_7          ] = {&RES_BUF(EN_NUM_7),            &RES_LEN(EN_NUM_7)},
    [RES_IDX_NUM_8          ] = {&RES_BUF(EN_NUM_8),            &RES_LEN(EN_NUM_8)},
    [RES_IDX_NUM_9          ] = {&RES_BUF(EN_NUM_9),            &RES_LEN(EN_NUM_9)},
#endif
#if WARNING_BT_LOW_LATENCY
    [RES_IDX_MUSIC_MODE     ] = {&RES_BUF(EN_MUSIC_MODE),       &RES_LEN(EN_MUSIC_MODE)},
    [RES_IDX_GAME_MODE      ] = {&RES_BUF(EN_GAME_MODE),        &RES_LEN(EN_GAME_MODE)},
#endif
#if WARNING_BT_CALL_CTRL
    [RES_IDX_CALL_HANGUP    ] = {&RES_BUF(EN_CALL_HANGUP),      &RES_LEN(EN_CALL_HANGUP)},
    [RES_IDX_CALL_REJECT    ] = {&RES_BUF(EN_CALL_REJECT),      &RES_LEN(EN_CALL_REJECT)},
    [RES_IDX_REDIALING      ] = {&RES_BUF(EN_REDIALING),        &RES_LEN(EN_REDIALING)},
#endif
#if WARNING_BT_HID_MENU
    [RES_IDX_CAMERA_ON      ] = {&RES_BUF(EN_CAMERA_ON),        &RES_LEN(EN_CAMERA_ON)},
    [RES_IDX_CAMERA_OFF     ] = {&RES_BUF(EN_CAMERA_OFF),       &RES_LEN(EN_CAMERA_OFF)},
#endif
    [RES_IDX_FINDMY_SOUND   ] = {(u32 *)&RES_BUF_PIANO(PIANO_FINEMY_SOUND), (u32 *)&RES_LEN_PIANO(PIANO_FINEMY_SOUND)},
    [RES_IDX_RING           ] = {&RES_BUF_MP3(RING),            &RES_LEN_MP3(RING)},
#if WARNING_TAKE_PHOTO
    [RES_IDX_TAKE_PHOTO     ] = {&RES_BUF_MP3(TAKE_PHOTO),      &RES_LEN_MP3(TAKE_PHOTO)},
#endif
    [RES_IDX_UPDATE         ] = {&RES_BUF_MP3(UPDATE),          &RES_LEN_MP3(UPDATE)},
    [RES_IDX_UPDATE_DONE    ] = {&RES_BUF_MP3(UPDATE_DONE),     &RES_LEN_MP3(UPDATE_DONE)},
#if WARNING_BT_CONNECT
    [RES_IDX_TWS_CONNECTED  ] = {&RES_BUF(EN_CONNECTED),        &RES_LEN(EN_CONNECTED)},
#endif
};
#endif

#if (LANG_SELECT == LANG_ZH || LANG_SELECT == LANG_EN_ZH)
const res_addr_t zh_lang_tbl[RES_IDX_MAX] = {
#if WARNING_POWER_ON
    [RES_IDX_POWERON        ] = {&RES_BUF(ZH_POWERON),          &RES_LEN(ZH_POWERON)},
#endif
#if WARNING_POWER_OFF
    [RES_IDX_POWEROFF       ] = {&RES_BUF(ZH_POWEROFF),         &RES_LEN(ZH_POWEROFF)},
#endif
#if (LANG_SELECT == LANG_EN_ZH)
    [RES_IDX_LANGUAGE_EN    ] = {&RES_BUF(EN_LANGUAGE),         &RES_LEN(EN_LANGUAGE)},
    [RES_IDX_LANGUAGE_ZH    ] = {&RES_BUF(ZH_LANGUAGE),         &RES_LEN(ZH_LANGUAGE)},
#endif
#if WARNING_LOW_BATTERY
    [RES_IDX_LOW_BATTERY    ] = {&RES_BUF(ZH_LOW_BATTERY),      &RES_LEN(ZH_LOW_BATTERY)},
#endif

#if WARNING_MAX_VOLUME
#if WARING_MAXVOL_TYPE == RES_TYPE_PIANO
    [RES_IDX_MAX_VOL        ] = {(u32 *)&RES_BUF_PIANO(PIANO_MAX_VOL),  (u32 *)&RES_LEN_PIANO(PIANO_MAX_VOL)},
#elif WARING_MAXVOL_TYPE == RES_TYPE_TONE
    [RES_IDX_MAX_VOL        ] = {(u32 *)&RES_BUF_TONE(TONE_MAX_VOL),    (u32 *)&RES_LEN_TONE(TONE_MAX_VOL)},
//#elif WARING_MAXVOL_TYPE == RES_TYPE_WAV
#elif WARING_MAXVOL_TYPE == RES_TYPE_MP3
    [RES_IDX_MAX_VOL        ] = {&RES_BUF_MP3(EN_MAX_VOL),      &RES_LEN_MP3(EN_MAX_VOL)},
#elif WARING_MAXVOL_TYPE == RES_TYPE_WSBC
    [RES_IDX_MAX_VOL        ] = {&RES_BUF_SBC(EN_MAX_VOL),      &RES_LEN_SBC(EN_MAX_VOL)},
#else
    [RES_IDX_MAX_VOL        ] = {&RES_BUF(ZH_MAX_VOL),          &RES_LEN(ZH_MAX_VOL)},
#endif
#endif

#if WARNING_USB_SD
    [RES_IDX_SDCARD_MODE    ] = {&RES_BUF(ZH_SDCARD_MODE),      &RES_LEN(ZH_SDCARD_MODE)},
    [RES_IDX_USB_MODE       ] = {&RES_BUF(ZH_USB_MODE),         &RES_LEN(ZH_USB_MODE)},
#endif
#if WARNING_FUNC_AUX
    [RES_IDX_AUX_MODE       ] = {&RES_BUF(ZH_AUX_MODE),         &RES_LEN(ZH_AUX_MODE)},
#endif
#if WARNING_FUNC_CLOCK
    [RES_IDX_CLOCK_MODE     ] = {&RES_BUF(ZH_CLOCK_MODE),       &RES_LEN(ZH_CLOCK_MODE)},
#endif
#if WARNING_FUNC_FMRX
    [RES_IDX_FM_MODE        ] = {&RES_BUF(ZH_FM_MODE),          &RES_LEN(ZH_FM_MODE)},
#endif
#if WARNING_FUNC_SPEAKER
    [RES_IDX_SPK_MODE       ] = {&RES_BUF(ZH_SPK_MODE),         &RES_LEN(ZH_SPK_MODE)},
#endif
#if WARNING_FUNC_USBDEV
    [RES_IDX_PC_MODE        ] = {&RES_BUF(ZH_PC_MODE),          &RES_LEN(ZH_PC_MODE)},
#endif
#if WARNING_FUNC_BT
    [RES_IDX_BT_MODE        ] = {&RES_BUF(ZH_BT_MODE),          &RES_LEN(ZH_BT_MODE)},
#endif
#if WARNING_LEFT_RIGHT_CH
    [RES_IDX_LEFT_CH        ] = {&RES_BUF(ZH_LEFT_CH),          &RES_LEN(ZH_LEFT_CH)},
    [RES_IDX_RIGHT_CH       ] = {&RES_BUF(ZH_RIGHT_CH),         &RES_LEN(ZH_RIGHT_CH)},
#endif
#if WARNING_BT_PAIR
    [RES_IDX_PAIRING        ] = {&RES_BUF(ZH_PAIRING),          &RES_LEN(ZH_PAIRING)},
#endif
#if WARNING_BT_WAIT_CONNECT
    [RES_IDX_WAIT4CONN      ] = {&RES_BUF(ZH_WAIT4CONN),        &RES_LEN(ZH_WAIT4CONN)},
#endif
#if WARNING_BT_CONNECT
    [RES_IDX_CONNECTED      ] = {&RES_BUF(ZH_CONNECTED),        &RES_LEN(ZH_CONNECTED)},
#endif
#if WARNING_BT_DISCONNECT
    [RES_IDX_DISCONNECT     ] = {&RES_BUF(ZH_DISCONNECT),       &RES_LEN(ZH_DISCONNECT)},
#endif
#if WARNING_BT_RING_NUMBER
    [RES_IDX_NUM_0          ] = {&RES_BUF(ZH_NUM_0),            &RES_LEN(ZH_NUM_0)},
    [RES_IDX_NUM_1          ] = {&RES_BUF(ZH_NUM_1),            &RES_LEN(ZH_NUM_1)},
    [RES_IDX_NUM_2          ] = {&RES_BUF(ZH_NUM_2),            &RES_LEN(ZH_NUM_2)},
    [RES_IDX_NUM_3          ] = {&RES_BUF(ZH_NUM_3),            &RES_LEN(ZH_NUM_3)},
    [RES_IDX_NUM_4          ] = {&RES_BUF(ZH_NUM_4),            &RES_LEN(ZH_NUM_4)},
    [RES_IDX_NUM_5          ] = {&RES_BUF(ZH_NUM_5),            &RES_LEN(ZH_NUM_5)},
    [RES_IDX_NUM_6          ] = {&RES_BUF(ZH_NUM_6),            &RES_LEN(ZH_NUM_6)},
    [RES_IDX_NUM_7          ] = {&RES_BUF(ZH_NUM_7),            &RES_LEN(ZH_NUM_7)},
    [RES_IDX_NUM_8          ] = {&RES_BUF(ZH_NUM_8),            &RES_LEN(ZH_NUM_8)},
    [RES_IDX_NUM_9          ] = {&RES_BUF(ZH_NUM_9),            &RES_LEN(ZH_NUM_9)},
#endif
#if WARNING_BT_LOW_LATENCY
    [RES_IDX_MUSIC_MODE     ] = {&RES_BUF(ZH_MUSIC_MODE),       &RES_LEN(ZH_MUSIC_MODE)},
    [RES_IDX_GAME_MODE      ] = {&RES_BUF(ZH_GAME_MODE),        &RES_LEN(ZH_GAME_MODE)},
#endif
#if WARNING_BT_CALL_CTRL
    [RES_IDX_CALL_HANGUP    ] = {&RES_BUF(ZH_CALL_HANGUP),      &RES_LEN(ZH_CALL_HANGUP)},
    [RES_IDX_CALL_REJECT    ] = {&RES_BUF(ZH_CALL_REJECT),      &RES_LEN(ZH_CALL_REJECT)},
    [RES_IDX_REDIALING      ] = {&RES_BUF(ZH_REDIALING),        &RES_LEN(ZH_REDIALING)},
#endif
#if WARNING_BT_HID_MENU
    [RES_IDX_CAMERA_ON      ] = {&RES_BUF(ZH_CAMERA_ON),        &RES_LEN(ZH_CAMERA_ON)},
    [RES_IDX_CAMERA_OFF     ] = {&RES_BUF(ZH_CAMERA_OFF),       &RES_LEN(ZH_CAMERA_OFF)},
#endif
    [RES_IDX_FINDMY_SOUND   ] = {(u32 *)&RES_BUF_PIANO(PIANO_FINEMY_SOUND), (u32 *)&RES_LEN_PIANO(PIANO_FINEMY_SOUND)},
    [RES_IDX_RING           ] = {&RES_BUF_MP3(RING),            &RES_LEN_MP3(RING)},
#if WARNING_TAKE_PHOTO
    [RES_IDX_TAKE_PHOTO     ] = {&RES_BUF_MP3(TAKE_PHOTO),      &RES_LEN_MP3(TAKE_PHOTO)},
#endif
    [RES_IDX_UPDATE         ] = {&RES_BUF_MP3(UPDATE),          &RES_LEN_MP3(UPDATE)},
    [RES_IDX_UPDATE_DONE    ] = {&RES_BUF_MP3(UPDATE_DONE),     &RES_LEN_MP3(UPDATE_DONE)},
#if WARNING_BT_CONNECT
    [RES_IDX_TWS_CONNECTED  ] = {&RES_BUF(ZH_CONNECTED),        &RES_LEN(ZH_CONNECTED)},
#endif
};
#endif

#if LANG_SELECT == LANG_EN_ZH
void multi_lang_init(uint lang_id)
{
}
#endif
