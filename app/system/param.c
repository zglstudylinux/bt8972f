#include "include.h"


#define param_read8(a, b)       b = cm_read8(PAGE0(a))
#define param_write8(a, b)      cm_write8(PAGE0(a), b)
#define param_read16(a, b)      b = cm_read16(PAGE0(a))
#define param_write16(a, b)     cm_write16(PAGE0(a), b)
#define param_read32(a, b)      b = cm_read32(PAGE0(a))
#define param_write32(a, b)     cm_write32(PAGE0(a), b)
#define param_read(a, b, c)     cm_read(a, PAGE0(b), c)
#define param_write(a, b, c)    cm_write(a, PAGE0(b), c)
#define param_sync_do()         cm_sync()


AT(.text.bsp.param)
void param_init(bool reset)
{
    //LANG ID初值
    if ((LANG_SELECT < LANG_EN_ZH) && xcfg_cb.lang_id >= LANG_EN_ZH) {
        printf("Language Select Error\n");
        xcfg_cb.lang_id = LANG_SELECT;
    }
    cm_init(MAX_CM_PAGE, CM_START, CM_SIZE);
    //printf("CM: %x\n", cm_read8(PAGE0(0)));
    //printf("CM: %x\n", cm_read8(PAGE1(0)));

    sys_cb.rand_seed = sys_get_rand_key();

    param_lang_id_read();
    if(sys_cb.lang_id >= LANG_EN_ZH) {
        if (xcfg_cb.lang_id == 2) {
            sys_cb.lang_id = 0;             //出厂默认英文
        } else if (xcfg_cb.lang_id == 3) {
            sys_cb.lang_id = 1;             //出厂默认中文
        } else {
            sys_cb.lang_id = xcfg_cb.lang_id;
        }
    }

    param_sys_vol_read();
    if ((SYS_LIMIT_VOLUME == 0) || (sys_cb.vol > VOL_MAX)) {
        sys_cb.vol = SYS_INIT_VOLUME;                   //上电还原到默认音量
    } else {
        if (sys_cb.vol < SYS_LIMIT_VOLUME) {
            sys_cb.vol = SYS_LIMIT_VOLUME;
        }
    }
    sys_cb.a2dp_vol = sys_cb.vol;

    param_hfp_vol_read();
    if (sys_cb.hfp_vol > 15) {
        sys_cb.hfp_vol = 15;
    }

    sys_cb.sw_rst_flag = sw_reset_source_get();

#if (CHARGE_BOX_TYPE == CBOX_SSW)
    param_vuart_popup_flag_read();
#endif

#if QTEST_EN
    update_set_qcheck_code();
#endif
}

AT(.text.bsp.param)
void bsp_param_write(u8 *buf, u32 addr, uint len)
{
    param_write(buf, addr, len);
}

AT(.text.bsp.param)
void bsp_param_read(u8 *buf, u32 addr, uint len)
{
    param_read(buf, addr, len);
}

AT(.text.bsp.param)
void bsp_param_sync(void)
{
    param_sync_do();
}

#if BT_LOCAL_ADDR
AT(.text.bsp.param)
void param_random_key_read(u8 *key)
{
    if (key == NULL) {
        return;
    }
    param_read(key, PARAM_RANDOM_KEY, 4);
}

AT(.text.bsp.param)
void param_random_key_write(void)
{
    u32 key;
    param_random_key_read((u8*)&key);
    if (key == 0 || key == UINT_MAX) {
        key = sys_cb.rand_seed;
        param_write((u8 *)&key, PARAM_RANDOM_KEY, 4);
    }
}
#endif // BT_LOCAL_ADDR

AT(.text.bsp.param)
void param_sys_vol_write(void)
{
    param_write((u8 *)&sys_cb.vol, PARAM_SYS_VOL, 1);
}

AT(.text.bsp.param)
void param_sys_vol_read(void)
{
    param_read((u8 *)&sys_cb.vol, PARAM_SYS_VOL, 1);
}

AT(.text.bsp.param)
void param_hfp_vol_write(void)
{
    param_write((u8 *)&sys_cb.hfp_vol, PARAM_HSF_VOL, 1);
}

AT(.text.bsp.param)
void param_hfp_vol_read(void)
{
    param_read((u8 *)&sys_cb.hfp_vol, PARAM_HSF_VOL, 1);
}

AT(.text.bsp.param)
void param_lang_id_write(void)
{
    param_write((u8 *)&sys_cb.lang_id, PARAM_LANG_ID, 1);
}

AT(.text.bsp.param)
void param_lang_id_read(void)
{
    param_read((u8 *)&sys_cb.lang_id, PARAM_LANG_ID, 1);
}

AT(.text.bsp.param)
void param_sync(void)
{
    param_sync_do();
}

AT(.text.bsp.param)
u32 param_get_xosc_addr(void)
{
    return PARAM_BT_XOSC_CAP;
}

#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
AT(.text.bsp.param)
void param_bond_nb_read(void)
{
    param_read((u8 *)&sys_cb.wl_bond_nb, PARAM_BONDING_NB, 1);
}

AT(.text.bsp.param)
void param_bond_nb_write(void)
{
    param_write((u8 *)&sys_cb.wl_bond_nb, PARAM_BONDING_NB, 1);
    param_sync_do();
}

AT(.text.bsp.param)
void param_bond_key_read(void)
{
    param_read((u8 *)&sys_cb.wl_bd_key, PARAM_BONDING_KEY, 2);
}

AT(.text.bsp.param)
void param_bond_key_write(void)
{
    param_write((u8 *)&sys_cb.wl_bd_key, PARAM_BONDING_KEY, 2);
    param_sync_do();
}

AT(.text.bsp.param)
void param_bond_mode_read(void)
{
    param_read((u8 *)&sys_cb.bond_mode, PARAM_BONDING_MODE, 1);
}

AT(.text.bsp.param)
void param_bond_mode_write(void)
{
    param_write((u8 *)&sys_cb.bond_mode, PARAM_BONDING_MODE, 1);
    param_sync_do();
}
#endif

#if (CHARGE_BOX_TYPE == CBOX_SSW)
//充电仓命令开关广播功能
void param_vuart_popup_flag_read(void)
{
    u8 temp;

    param_read(&temp, PARAM_VUART_POPUP_FLAG, 1);
    if ((temp == 1) || (temp == 2)) {
        sys_cb.popup_en = temp - 1;
    }else{
        sys_cb.popup_en = 1;
    }
}

void param_vuart_popup_flag_write(u8 data)
{
    u8 temp;

    param_read(&temp, PARAM_VUART_POPUP_FLAG, 1);
    if (temp != data) {
        temp = data;
        param_write((u8 *)&temp, PARAM_VUART_POPUP_FLAG, 1);
        param_sync();
    }
}

#endif

AT(.text.bsp.param.fota)
void param_fot_addr_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_ADDR, 4);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_addr_read(u8 *param)
{
    param_read(param, PARAM_FOT_ADDR, 4);
}

AT(.text.bsp.param.fota)
void param_fot_remote_ver_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_REMOTE_VER, 2);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_remote_ver_read(u8 *param)
{
    param_read(param, PARAM_FOT_REMOTE_VER, 2);
}

AT(.text.bsp.param.fota)
void param_fot_head_info_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_HEAD_INFO, 8);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_head_info_read(u8 *param)
{
    param_read(param, PARAM_FOT_HEAD_INFO, 8);
}

AT(.text.bsp.param.fota)
void param_fot_hash_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_HASH, 4);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_hash_read(u8 *param)
{
    param_read(param, PARAM_FOT_HASH, 4);
}

AT(.text.bsp.param.fota)
void param_fot_type_write(u8 *param)
{
    param_write((u8 *)param, PARAM_FOT_TYPE, 1);
    param_sync();
}

AT(.text.bsp.param.fota)
void param_fot_type_read(u8 *param)
{
    param_read(param, PARAM_FOT_TYPE, 1);
}
