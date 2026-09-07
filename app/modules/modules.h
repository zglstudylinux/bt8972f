#ifndef __MODULES_H
#define __MODULES_H

#include "key/key_scan.h"
#include "audio/audio.h"
#include "audio/mic.h"
#include "audio/spk.h"
#include "bluetooth/bluetooth.h"
#include "bluetooth/app/app_fota/app_fota.h"
#include "wireless/wireless.h"
#include "wireless/wireless_data.h"

#include "gui/gui.h"
//#include "fs/fs.h"

#include "charge/charge.h"
#include "charge/charge_box.h"

//#include "effect/music_effect.h"

#include "warning/warning_play.h"
#include "warning/res_play.h"

#include "record/record.h"

#include "test/iodm.h"
#include "test/qtest.h"
#include "test/vusb_test.h"

#include "usb_device/usb_device.h"
#include "usb_device/usb_audio.h"
#include "usb_device/usb_enum.h"
#include "usb_device/usb_table.h"
#include "usb_device/usb_device_storage.h"
#include "usb_device/usb_device_update.h"
#include "mfi_iap2/usb_device_iap2.h"

#include "tool/toolkit.h"
#include "tool/toolkit_effect.h"

#include "huart_audio/huart_audio_in_mix.h"
#include "huart_audio/huart_audio_out.h"

#include "debug/offline_log.h"
#include "debug/debug.h"
#include "debug/audio_dump.h"
#include "voice/agc.h"
#include "voice/echo.h"
#include "voice/ains4.h"
#include "voice/ains5.h"
#include "voice/ylcrn_L2.h"
#include "voice/ylcrn_L3.h"
#include "voice/dnr_fre.h"
#include "voice/dg_adc.h"
#include "voice/denoise_api.h"
///音乐处理
#include "effect/effect_idx.h"
#include "effect/mic_effect.h"

#include "i2s_audio/i2s_audio.h"
#endif // __MODULES_H
