#include "config.h"

@echo off

cd /d %~dp0

set tone_dir=..\bin\res_tone
set res_dir=..\bin\res

set tone_zh_dir=%tone_dir%\zh
set tone_en_dir=%tone_dir%\en
set res_zh_dir=%res_dir%\zh
set res_en_dir=%res_dir%\en


//先删掉res_dir目录旧的提示音文件
if exist "%res_dir%" (
echo del /s %res_dir%\*.mp3
echo del /s %res_dir%\*.sbc
echo del /s %res_dir%\*.wav
del /s %res_dir%\*.mp3 > NUL
del /s %res_dir%\*.sbc > NUL
) else (
mkdir %res_dir%
mkdir %res_zh_dir%
mkdir %res_en_dir%
)

//copy命令的宏定义
#define COPY_RES(res_file)          copy /y %tone_dir%res_file %res_dir%res_file > NUL

#if LANG_SELECT == LANG_EN_ZH
    #define COPY_ZH_RES(res_file)       copy /y %tone_zh_dir%res_file %res_zh_dir%res_file > NUL
    #define COPY_EN_RES(res_file)       copy /y %tone_en_dir%res_file %res_en_dir%res_file > NUL
#elif   LANG_SELECT == LANG_ZH
    #define COPY_ZH_RES(res_file)       copy /y %tone_zh_dir%\res_file %res_zh_dir%res_file > NUL
    #define COPY_EN_RES(res_file)
#elif   LANG_SELECT == LANG_EN
    #define COPY_ZH_RES(res_file)
    #define COPY_EN_RES(res_file)       copy /y %tone_en_dir%res_file %res_en_dir%res_file > NUL
#else
    #define COPY_ZH_RES(res_file)
    #define COPY_EN_RES(res_file)
#endif


//下面将用到的提示音文件从tone_dir拷贝到res_dir目录
echo copy %tone_dir% file to %res_dir%

COPY_RES(\ring.mp3)
COPY_RES(\update.mp3)
COPY_RES(\update_done.mp3)

#if (LANG_SELECT == LANG_EN_ZH)
COPY_EN_RES(\language.mp3)
COPY_ZH_RES(\language.mp3)
#endif

#if WARNING_BT_RING_NUMBER
COPY_EN_RES(\num_0.mp3)
COPY_ZH_RES(\num_0.mp3)
COPY_EN_RES(\num_1.mp3)
COPY_ZH_RES(\num_1.mp3)
COPY_EN_RES(\num_2.mp3)
COPY_ZH_RES(\num_2.mp3)
COPY_EN_RES(\num_3.mp3)
COPY_ZH_RES(\num_3.mp3)
COPY_EN_RES(\num_4.mp3)
COPY_ZH_RES(\num_4.mp3)
COPY_EN_RES(\num_5.mp3)
COPY_ZH_RES(\num_5.mp3)
COPY_EN_RES(\num_6.mp3)
COPY_ZH_RES(\num_6.mp3)
COPY_EN_RES(\num_7.mp3)
COPY_ZH_RES(\num_7.mp3)
COPY_EN_RES(\num_8.mp3)
COPY_ZH_RES(\num_8.mp3)
COPY_EN_RES(\num_9.mp3)
COPY_ZH_RES(\num_9.mp3)
#endif

#if WARNING_POWER_ON
COPY_EN_RES(\poweron.mp3)
COPY_ZH_RES(\poweron.mp3)
#endif

#if WARNING_POWER_OFF
COPY_EN_RES(\poweroff.mp3)
COPY_ZH_RES(\poweroff.mp3)
#endif

#if WARNING_LOW_BATTERY
COPY_EN_RES(\low_battery.mp3)
COPY_ZH_RES(\low_battery.mp3)
#endif

#if WARNING_MAX_VOLUME
#if WARING_MAXVOL_TYPE == RES_TYPE_MP3
COPY_EN_RES(\max_vol.mp3)
COPY_ZH_RES(\max_vol.mp3)
#elif WARING_MAXVOL_TYPE == RES_TYPE_WSBC
COPY_EN_RES(\max_vol.sbc)
COPY_ZH_RES(\max_vol.sbc)
#endif
#endif

#if WARNING_MIN_VOLUME
COPY_EN_RES(\min_vol.mp3)
COPY_ZH_RES(\min_vol.mp3)
#endif

#if WARNING_FUNC_MUSIC || WARNING_BT_LOW_LATENCY
COPY_EN_RES(\music_mode.mp3)
COPY_ZH_RES(\music_mode.mp3)
#endif

#if WARNING_BT_LOW_LATENCY
COPY_EN_RES(\game_mode.mp3)
COPY_ZH_RES(\game_mode.mp3)
#endif

#if WARNING_USB_SD
COPY_EN_RES(\sdcard_mode.mp3)
COPY_ZH_RES(\sdcard_mode.mp3)
COPY_EN_RES(\usb_mode.mp3)
COPY_ZH_RES(\usb_mode.mp3)
#endif

#if WARNING_FUNC_CLOCK
COPY_EN_RES(\clock_mode.mp3)
COPY_ZH_RES(\clock_mode.mp3)
#endif

#if WARNING_FUNC_FMRX
COPY_EN_RES(\fm_mode.mp3)
COPY_ZH_RES(\fm_mode.mp3)
#endif

#if WARNING_FUNC_AUX
COPY_EN_RES(\aux_mode.mp3)
COPY_ZH_RES(\aux_mode.mp3)
#endif

#if WARNING_FUNC_USBDEV
COPY_EN_RES(\pc_mode.mp3)
COPY_ZH_RES(\pc_mode.mp3)
#endif

#if WARNING_FUNC_SPEAKER
COPY_EN_RES(\spk_mode.mp3)
COPY_ZH_RES(\spk_mode.mp3)
#endif

#if WARNING_FUNC_BT
COPY_EN_RES(\bt_mode.mp3)
COPY_ZH_RES(\bt_mode.mp3)
#endif

#if WARNING_BT_WAIT_CONNECT
COPY_EN_RES(\wait4conn.mp3)
COPY_ZH_RES(\wait4conn.mp3)
#endif

#if WARNING_BT_CONNECT
COPY_EN_RES(\connected.mp3)
COPY_ZH_RES(\connected.mp3)
#endif

#if WARNING_BT_DISCONNECT
COPY_EN_RES(\disconnect.mp3)
COPY_ZH_RES(\disconnect.mp3)
#endif

#if WARNING_BT_CALL_CTRL
#endif

#if WARNING_BT_PAIR
COPY_EN_RES(\pairing.mp3)
COPY_ZH_RES(\pairing.mp3)
#endif

#if WARNING_LEFT_RIGHT_CH
COPY_EN_RES(\left_ch.mp3)
COPY_ZH_RES(\left_ch.mp3)
COPY_EN_RES(\right_ch.mp3)
COPY_ZH_RES(\right_ch.mp3)
#endif

#if WARNING_BT_HID_MENU
COPY_EN_RES(\camera_on.mp3)
COPY_ZH_RES(\camera_on.mp3)
COPY_EN_RES(\camera_off.mp3)
COPY_ZH_RES(\camera_off.mp3)
#endif

#if WARNING_BTHID_CONN
COPY_EN_RES(\hid_connected.mp3)
COPY_ZH_RES(\hid_connected.mp3)
COPY_EN_RES(\hid_disconnect.mp3)
COPY_ZH_RES(\hid_disconnect.mp3)
#endif

#if WARNING_TAKE_PHOTO
COPY_RES(\take_photo.mp3)
#endif

