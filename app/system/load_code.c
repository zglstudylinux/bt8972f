#include "include.h"
#include "load_code.h"

//lock到cache中
extern u32 __code_start_mp3dec, __code_end_mp3dec;
extern u32 __code_start_wavdec, __code_end_wavdec;
extern u32 __code_start_mp3enc, __code_end_mp3enc;
extern u32 __code_start_mp2enc, __code_end_mp2enc;
extern u32 __code_start_usbdev, __code_end_usbdev;
extern u32 __code_start_sniff, __code_end_sniff;
extern u32 __code_start_sleep, __code_end_sleep;
extern u32 __code_start_pwrsave, __code_end_pwrsave;
extern u32 __code_start_charge, __code_end_charge;

extern u32 __code_start_piano, __code_end_piano;
extern u32 __code_start_record, __code_end_record;
extern u32 __code_start_huart, __code_end_huart;
extern u32 __code_start_stream, __code_end_stream;
extern u32 __code_start_bb_test, __code_end_bb_test;
extern u32 __code_start_spdif, __code_end_spdif;
extern u32 __code_start_aacdec, __code_end_aacdec;

extern u32 __code_start_sbcdec, __code_end_sbcdec;
extern u32 __code_start_bt_voice, __code_end_bt_voice;
extern u32 __code_start_btdec_com, __code_end_btdec_com;
extern u32 __code_start_opus, __code_end_opus;
extern u32 __code_start_fota, __code_end_fota;
extern u32 __code_start_spiflash_id, __code_end_spiflash_id;
extern u32 __code_start_vad, __code_end_vad;
extern u32 __code_start_ws_com, __code_end_ws_com;

//load到固定ram中
extern u32 __code_effect_comm_vma, __code_effect_comm_lma, __code_effect_comm_size;
extern u32 __code_anc_comm_vma, __code_anc_comm_lma, __code_anc_comm_size;
extern u32 __code_anc_tool_vma, __code_anc_tool_lma, __code_anc_tool_size;
extern u32 __code_sco_comm_vma, __code_sco_comm_lma, __code_sco_comm_size;
extern u32 __code_sco_ains_vma, __code_sco_ains_lma, __code_sco_ains_size;
extern u32 __code_sco_dmdnn_vma, __code_sco_dmdnn_lma, __code_sco_dmdnn_size;
extern u32 __code_sco_dnn_vma, __code_sco_dnn_lma, __code_sco_dnn_size;
extern u32 __code_sco_dnn_pro_vma, __code_sco_dnn_pro_lma, __code_sco_dnn_pro_size;
extern u32 __code_sco_sndp_vma, __code_sco_sndp_lma, __code_sco_sndp_size;
extern u32 __code_ws_com_vma, __code_ws_com_lma, __code_ws_com_size;
extern u32 __code_adapter_vma, __code_adapter_lma, __code_adapter_size;
extern u32 __code_emit_vma, __code_emit_lma, __code_emit_size;
extern u32 __code_ains4_vma, __code_ains4_lma, __code_ains4_size;
extern u32 __code_ains5_vma, __code_ains5_lma, __code_ains5_size;


#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif


static struct {
    lock_t record[LOCK_IDX_MAX];
} lock_cb;

AT(.rodata.lock.str)
const struct {
    const char name[8];
    u32 start_addr;
    u32 end_addr;
} lock_record[LOCK_IDX_MAX] = {
    [LOCK_IDX_CHARGE]       = { "charge",   (u32)&__code_start_charge,      (u32)&__code_end_charge,    },
    [LOCK_IDX_SLEEP]        = { "sleep",    (u32)&__code_start_sleep,       (u32)&__code_end_sleep,     },
    [LOCK_IDX_PWRSAVE]      = { "pwrsave",  (u32)&__code_start_pwrsave,     (u32)&__code_end_pwrsave,   },
    [LOCK_IDX_SPIFLASH]     = { "flash",    (u32)&__code_start_spiflash_id, (u32)&__code_end_spiflash_id,  },
    [LOCK_IDX_HUART]        = { "huart",    (u32)&__code_start_huart,       (u32)&__code_end_huart,     },

    [LOCK_IDX_STREAM]       = { "stream",   (u32)&__code_start_stream,      (u32)&__code_end_stream,    },
    [LOCK_IDX_A2DP]         = { "btdec",    (u32)&__code_start_btdec_com,   (u32)&__code_end_btdec_com, },
    [LOCK_IDX_SCO]          = { "btvoice",  (u32)&__code_start_bt_voice,    (u32)&__code_end_bt_voice,  },
    [LOCK_IDX_FOTA]         = { "fota",     (u32)&__code_start_fota,        (u32)&__code_end_fota,      },
    [LOCK_IDX_BT_SNIFF]     = { "sniff",    (u32)&__code_start_sniff,       (u32)&__code_end_sniff, },
    [LOCK_IDX_BB_TEST]      = { "bbtest",   (u32)&__code_start_bb_test,     (u32)&__code_end_bb_test,   },

    [LOCK_IDX_MP3DEC]       = { "mp3dec",   (u32)&__code_start_mp3dec,      (u32)&__code_end_mp3dec,    },
    [LOCK_IDX_WAVDEC]       = { "wavdec",   (u32)&__code_start_wavdec,      (u32)&__code_end_wavdec,    },
    [LOCK_IDX_SBCDEC]       = { "sbcdec",   (u32)&__code_start_sbcdec,      (u32)&__code_end_sbcdec,    },
    [LOCK_IDX_AACDEC]       = { "aacdec",   (u32)&__code_start_aacdec,      (u32)&__code_end_aacdec,    },

    [LOCK_IDX_OPUS]         = { "opus",     (u32)&__code_start_opus,        (u32)&__code_end_opus,      },
#if DEVICE_RECORD_EN
    [LOCK_IDX_RECORD]       = { "record",   (u32)&__code_start_record,      (u32)&__code_end_record,    },
#endif
#if FUNC_USBDEV_EN || ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN
    [LOCK_IDX_USBDEV]       = { "usbdev",   (u32)&__code_start_usbdev,      (u32)&__code_end_usbdev,    },
#endif
    [LOCK_IDX_WL]           = { "wireless", (u32)&__code_start_ws_com,       (u32)&__code_end_ws_com,   },
};

void lock_init(void)
{
    memset(&lock_cb, 0, sizeof(lock_cb));
}

//-----------------------------------------------------------------------------------
//lock到cache中，最多只能24k
bool lock_code(u8 idx)
{
    bool ret = false;
    const char *str;

    if(idx >= LOCK_IDX_MAX) {
        return false;
    }

    u32 start_addr = lock_record[idx].start_addr;
    u32 end_addr = lock_record[idx].end_addr;

    if(lock_cb.record[idx] != 0) {
        str = "repeat";
    } else {
        lock_cb.record[idx] = lock_cache(start_addr, end_addr);
        if (lock_cb.record[idx] == 0) {
            str = "fail";
        } else {
            str = "ok";
            ret = true;
        }
    }

    TRACE("lock [%s] %s, addr=%x, size=%x\n", lock_record[idx].name, str, start_addr, end_addr-start_addr);
    return ret;
}

bool unlock_code(u8 idx)
{
    if(idx >= LOCK_IDX_MAX) {
        return false;
    }

    if (lock_cb.record[idx] == 0) {
        return true;
    }

    unlock_cache(lock_cb.record[idx]);
    lock_cb.record[idx] = 0;

    TRACE("unlock [%s] ok\n", lock_record[idx].name);
    return true;
}

//load到固定ram中，不同模式load不同模式的代码，有一些模式是公用
void load_code(const char* str, void *vma, void *lma, u32 size)
{
    TRACE("load %s: vma=%08x, lma=%08x, size=%08x\n", str, (u32)vma, (u32)lma, size);
    memcpy(vma, lma, size);
}


//-----------------------------------------------------------------------------------
//解码相关
void unlock_code_mutil(u8 from, u8 to)
{
    for(u8 i=from; i<=to; i++) {
        unlock_code(i);
    }
}

void unlock_dcode(void)
{
    unlock_code_mutil(LOCK_IDX_MP3DEC, LOCK_IDX_AACDEC);
}

WEAK void load_code_mp3dec(void)
{
    unlock_code_btdec();

    unlock_dcode();
    lock_code(LOCK_IDX_MP3DEC);
}

WEAK void load_code_wavdec(void)
{
    unlock_dcode();
    lock_code(LOCK_IDX_WAVDEC);
}

WEAK void load_code_sbcdec(void)
{
    unlock_dcode();
    lock_code(LOCK_IDX_SBCDEC);
}

void load_code_aacdec(void)
{
    unlock_dcode();
    lock_code(LOCK_IDX_AACDEC);
}

void lock_code_stream(void)
{
    lock_code(LOCK_IDX_STREAM);
}

void unlock_code_stream(void)
{
    unlock_code(LOCK_IDX_STREAM);
}

//-----------------------------------------------------------------------------------
//蓝牙相关
void lock_code_bt_sleep(void)
{
    lock_code(LOCK_IDX_SLEEP);
}

void unlock_code_bt_sleep(void)
{
    unlock_code(LOCK_IDX_SLEEP);
}

void unlock_code_decode_bt_sleep(void)
{
    unlock_dcode();
}

void load_code_fota(void)
{
    lock_code(LOCK_IDX_FOTA);
}

void unlock_code_fota(void)
{
    unlock_code(LOCK_IDX_FOTA);
}

void load_code_bt_voice(void)
{
    lock_code(LOCK_IDX_SCO);
}

void unlock_code_bt_voice(void)
{
    unlock_code(LOCK_IDX_SCO);
}

void load_code_btdec(void)
{
    lock_code(LOCK_IDX_A2DP);
}

void unlock_code_btdec(void)
{
    unlock_code(LOCK_IDX_A2DP);
}

void lock_code_sniff(void)
{
    lock_code(LOCK_IDX_BT_SNIFF);
}

void unlock_code_sniff(void)
{
    unlock_code(LOCK_IDX_BT_SNIFF);
}

void lock_code_bb_test(void)
{
    lock_code(LOCK_IDX_BB_TEST);
}

void unlock_code_bb_test(void)
{
    unlock_code(LOCK_IDX_BB_TEST);
}

//-----------------------------------------------------------------------------------
//KARAOK、录音相关
WEAK void lock_code_record(void)
{
#if FUNC_RECORD_EN
    lock_code(LOCK_IDX_RECORD);
#endif
}

WEAK void unlock_code_record(void)
{
#if FUNC_RECORD_EN
    unlock_code(LOCK_IDX_RECORD);
#endif
}

//WEAK
//void lock_code_mp3enc(void)
//{
//    lock_code(LOCK_IDX_MP3ENC);
//}
//
//WEAK
//void unlock_code_mp3enc(void)
//{
//    unlock_code(LOCK_IDX_MP3ENC);
//}
//
//WEAK
//void lock_code_mp2enc(void)
//{
//    lock_code(LOCK_IDX_MP2ENC);
//}
//
//WEAK
//void unlock_code_mp2enc(void)
//{
//    unlock_code(LOCK_IDX_MP2ENC);
//}

WEAK void lock_code_opus(void)
{
    lock_code(LOCK_IDX_OPUS);
}

WEAK void unlock_code_opus(void)
{
    unlock_code(LOCK_IDX_OPUS);
}


//-----------------------------------------------------------------------------------
//系统相关
WEAK void lock_code_spdif(void)
{
}

WEAK void lock_code_usbdev(void)
{
#if FUNC_USBDEV_EN || ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN
    lock_code(LOCK_IDX_USBDEV);
#endif
}

void unlock_code_usbdev(void)
{
#if FUNC_USBDEV_EN || ADAPTER_USB_MIC_RX_EN || DEVICE_USB_EN
    unlock_code(LOCK_IDX_USBDEV);
#endif
}

void lock_code_pwrsave(void)
{
    lock_code(LOCK_IDX_PWRSAVE);
}

void unlock_code_pwrsave(void)
{
    unlock_code(LOCK_IDX_PWRSAVE);
}

void lock_code_charge(void)
{
    lock_code(LOCK_IDX_CHARGE);
}

void unlock_code_charge(void)
{
    unlock_code(LOCK_IDX_CHARGE);
}

void lock_code_huart(void)
{
    lock_code(LOCK_IDX_HUART);
}

void unlock_code_huart(void)
{
    unlock_code(LOCK_IDX_HUART);
}

WEAK void load_code_spiflash_id(void)
{
    lock_code(LOCK_IDX_SPIFLASH);
}

WEAK void unlock_spiflash_id(void)
{
    unlock_code(LOCK_IDX_SPIFLASH);
}

//ws_com，无线麦一直存在
void lock_code_wl_com(void)
{
    lock_code(LOCK_IDX_WL);
}

void unlock_code_wl_com(void)
{
    unlock_code(LOCK_IDX_WL);
}

//-----------------------------------------------------------------------------------
void load_bt_sndp_code(u8 type)
{
}

void load_bt_dnn_code(void)
{
}

void load_code_sco_comm(void)
{
    load_code("sco_comm", &__code_sco_comm_vma, &__code_sco_comm_lma, (u32)&__code_sco_comm_size);
}

void load_code_ains(void)
{
    load_code("sco_ains", &__code_sco_ains_vma, &__code_sco_ains_lma, (u32)&__code_sco_ains_size);
}

void load_code_dmdnn(void)
{
    load_code("sco_dmdnn", &__code_sco_dmdnn_vma, &__code_sco_dmdnn_lma, (u32)&__code_sco_dmdnn_size);
}

void load_code_dnn_pro(void)
{
    load_code("sco_dnn_pro", &__code_sco_dnn_pro_vma, &__code_sco_dnn_pro_lma, (u32)&__code_sco_dnn_pro_size);
}

void load_code_dnn(void)
{
    load_code("sco_dnn", &__code_sco_dnn_vma, &__code_sco_dnn_lma, (u32)&__code_sco_dnn_size);
}

void load_code_sndp(void)
{
    load_code("sco_sndp", &__code_sco_sndp_vma, &__code_sco_sndp_lma, (u32)&__code_sco_sndp_size);
}

//WIRELESS_MIC
void load_code_wl_adapter(void)
{
    load_code("wl_com", &__code_ws_com_vma, &__code_ws_com_lma, (u32)&__code_ws_com_size);
    load_code("wl_adapter", &__code_adapter_vma, &__code_adapter_lma, (u32)&__code_adapter_size);
}

void load_code_wl_device(void)
{
    load_code("wl_com", &__code_ws_com_vma, &__code_ws_com_lma, (u32)&__code_ws_com_size);
    load_code("wl_device", &__code_emit_vma, &__code_emit_lma, (u32)&__code_emit_size);
}

void load_code_wl_interphone(void)
{
    load_code("wl_com", &__code_ws_com_vma, &__code_ws_com_lma, (u32)&__code_ws_com_size);
    load_code("wl_adapter", &__code_adapter_vma, &__code_adapter_lma, (u32)&__code_adapter_size);
    load_code("wl_device", &__code_emit_vma, &__code_emit_lma, (u32)&__code_emit_size);
}

void load_code_wl_ains4(void)
{
    load_code("wl_ains4", &__code_ains4_vma, &__code_ains4_lma, (u32)&__code_ains4_size);
}

void load_code_wl_ains5(void)
{
    load_code("wl_ains5", &__code_ains5_vma, &__code_ains5_lma, (u32)&__code_ains5_size);
}
