#include "include.h"

AT(.rodata.res.tbl)
const u32 res_tone_len = 1;
AT(.rodata.res.tbl)
const u32 res_piano_len = 2;

AT(.text.tws.res.addr) WEAK
bool res_tbl_get_addr(u32 index, u32 *addr, u32 *len)
{
    if(index < RES_IDX_MAX) {
        const res_addr_t *p = NULL;
#if WARNING_LEFT_RIGHT_CH == 2
        //区分TWS左右声道
        if(index == RES_IDX_TWS_CONNECTED && xcfg_cb.bt_tws_lr_mode > 0) {
            if(sys_cb.tws_left_channel) {
                index = RES_IDX_LEFT_CH;
            } else {
                index = RES_IDX_RIGHT_CH;
            }
        }
#endif

#if (LANG_SELECT == LANG_EN_ZH)
        if(sys_cb.lang_id == 0) {
            p = &en_lang_tbl[index];
        } else {
            p = &zh_lang_tbl[index];
        }
#elif (LANG_SELECT == LANG_EN)
        p = &en_lang_tbl[index];
#elif (LANG_SELECT == LANG_ZH)
        p = &zh_lang_tbl[index];
#else
    #error "请选择提示音语言\n"
#endif
        if(p->ptr == NULL || p->len == NULL) {
            return false;
        }

        *len  = *p->len;
        if(*len==1 || *len == 2) {
            *addr = (u32)p->ptr;    //tone和piano在tbl中存的是数组地址
        } else {
            *addr = *p->ptr;        //资源在tbl中存的是flash地址指针
        }
    } else {
        *addr = 0;
        *len = 0;
    }

    return (*addr != 0) && (*len != 0);
}

AT(.text.tws.res.type) WEAK
u8 res_tbl_get_type(u32 index)
{
    u8 type = RES_TYPE_INVALID;
    if(index >= RES_IDX_MAX) {
    } else {
        u32 addr, len;
        if (res_tbl_get_addr(index, &addr, &len)) {
            u8 *ptr = (u8 *)addr;

            if(len == 1) {
                type = RES_TYPE_TONE;
            } else if(len == 2) {
                type = RES_TYPE_PIANO;
            } else if(len >= 8) {
                if ((GET_LE32(ptr) == 0x46464952) && (GET_LE32(ptr + 8) == 0x45564157)) {   //Check "RIFF", "WAVE" tag
                    type = RES_TYPE_WAV;
                } else if(ptr[0] == 0x7a) {
                    type = RES_TYPE_WSBC;
                } else {
                    type = RES_TYPE_MP3;
                }
            }
        }
    }

//    printf("res_type(%d): %d\n", index, type);
    return type;
}
