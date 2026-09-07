#include "include.h"
#include "bsp_dac.h"

#define PCM_IS_24BITS()             (bool)(cfg_pcm_out_format & PCM_OUT_24BITS)

uint16_t cfg_pcm_out_format = 0;
uint8_t cfg_dac_out_spr = 0;
const u8 *dac_dvol_table;

//数字音量DB音量表
AT(.rodata.dac)
const u16 dac_dvol_tbl_db[MAX_DB_LEVEL+1] = {
    DIG_N0DB,   DIG_N1DB,   DIG_N2DB,   DIG_N3DB,   DIG_N4DB,   DIG_N5DB,   DIG_N6DB,   DIG_N7DB,
    DIG_N8DB,   DIG_N9DB,   DIG_N10DB,  DIG_N11DB,  DIG_N12DB,  DIG_N13DB,  DIG_N14DB,  DIG_N15DB,
    DIG_N16DB,  DIG_N17DB,  DIG_N18DB,  DIG_N19DB,  DIG_N20DB,  DIG_N21DB,  DIG_N22DB,  DIG_N23DB,
    DIG_N24DB,  DIG_N25DB,  DIG_N26DB,  DIG_N27DB,  DIG_N28DB,  DIG_N29DB,  DIG_N30DB,  DIG_N31DB,
    DIG_N32DB,  DIG_N33DB,  DIG_N34DB,  DIG_N35DB,  DIG_N36DB,  DIG_N37DB,  DIG_N38DB,  DIG_N39DB,
    DIG_N40DB,  DIG_N41DB,  DIG_N42DB,  DIG_N43DB,  DIG_N44DB,  DIG_N45DB,  DIG_N46DB,  DIG_N47DB,
    DIG_N48DB,  DIG_N49DB,  DIG_N50DB,  DIG_N51DB,  DIG_N52DB,  DIG_N53DB,  DIG_N54DB,  DIG_N55DB,
    DIG_N56DB,  DIG_N57DB,  DIG_N58DB,  DIG_N59DB,  DIG_N60DB,
};

AT(.rodata.dac)
const u8 dac_dvol_tbl_16[16 + 1] = {
    60,  43,  32,  26,  24,  22,  20,  18, 16,
    14,  12,  10,  8,   6,   4,   2,   0,
};

AT(.rodata.dac)
const u8 dac_dvol_tbl_32[32 + 1] = {
    60,  50,  43,  38,  34,  30,  28,  26,
    24,  23,  22,  21,  20,  19,  18,  17,
    16,  15,  14,  13,  12,  11,  10,  9,
    8,   7,   6,   5,   4,   3,   2,   1,   0,
};

//后续删掉
#if !DAC_PERF_EN
AT(.com_text.dac.bit16)
void dac_put_sample_16bit(s16 left, s16 right)
{
    if(PCM_IS_24BITS()) {
        AUBUF0DATA = ((s32)left)<<8;
        AUBUF0DATA24R = ((s32)right)<<8;
    } else {
        AUBUF0DATA = (right << 16) | (left & 0xffff);
    }
}
AT(.com_text.dac.bit32)
void dac_put_sample_24bit(s32 left, s32 right)
{
    if(PCM_IS_24BITS()) {
        AUBUF0DATA = left;
        AUBUF0DATA24R = right;
    } else {
        AUBUF0DATA = ((right>>8) << 16) | ((left>>8) & 0xffff);
    }
}
AT(.com_text.dac.bit24)
void aubuf_dma_pre_proc(s32 *pcm, uint samples, uint nch) {}
#endif

//将音量级数vol转换成db_level
AT(.text.bsp.dac)
u8 bsp_volume_vol2lev(u8 vol)
{
    u8 tbl_idx;

    if (vol > VOL_MAX) {
        vol = VOL_MAX;
    }
    tbl_idx = dac_dvol_table[vol] + sys_cb.gain_offset;
    if (tbl_idx > MAX_DB_LEVEL) {
        tbl_idx = MAX_DB_LEVEL;
    }

    return MAX_DB_LEVEL - tbl_idx;                //level=MAX_DB_LEVEL-tbl_idx, 值越大音量越大
}

//将db_level转换成数字音量dvol
u16 bsp_volume_lev2dvol(u8 level)
{
    if (level > MAX_DB_LEVEL) {
        level = MAX_DB_LEVEL;
    }
    return dac_dvol_tbl_db[MAX_DB_LEVEL - level]; //tbl_idx=MAX_DB_LEVEL-level
}

//将音量级数vol转换成数字音量dvol
AT(.text.vol.convert)
u16 bsp_volume_vol2dvol(u8 vol)
{
    u8 level = bsp_volume_vol2lev(vol);
    return bsp_volume_lev2dvol(level);
}

AT(.text.bsp.dac.vol_dblr)
void bsp_change_volume_db_l_r(u8 l_level, u8 r_level)
{
    u16 dvol_l = bsp_volume_lev2dvol(l_level);
    u16 dvol_r = bsp_volume_lev2dvol(r_level);
    dac_balance_set(dvol_l, dvol_r);
}

AT(.text.bsp.dac)
void bsp_change_volume_db(u8 level)
{
    u16 dvol;

    dvol = bsp_volume_lev2dvol(level);
    dac_vol_set(dvol);
}

AT(.text.bsp.dac)
void bsp_change_volume(u8 vol)
{
    if (vol <= VOL_MAX) {
        u8 level = bsp_volume_vol2lev(vol);
        bsp_change_volume_db(level);
    }
}

AT(.text.bsp.dac)
bool bsp_set_volume(u8 vol)
{
    if (!bsp_res_is_vol_busy()) {       //未播放提示音，修改音量
        bsp_change_volume(vol);
    }
    if (vol == sys_cb.vol) {
        gui_box_show_vol();
        return false;
    }

    if (vol <= VOL_MAX) {
        sys_cb.vol = vol;
        gui_box_show_vol();

        sys_cb.cm_times = 0;
        sys_cb.cm_vol_change = 1;
    }
    printf("vol: %d\n", sys_cb.vol);
    return true;
}

AT(.text.bsp.dac)
u8 bsp_volume_inc(u8 vol)
{
    vol++;
    if(vol > VOL_MAX)
        vol = VOL_MAX;
    return vol;
}

AT(.text.bsp.dac)
u8 bsp_volume_dec(u8 vol)
{
    if(vol > 0)
        vol--;
    return vol;
}

//vcmbuf及差分
AT(.text.bsp.dac)
bool bsp_dac_off_for_bt_conn(void)
{
    if ((DAC_OFF_FOR_BT_CONN_EN) && (DAC_CH_SEL >= DAC_VCMBUF_MONO)) {
        return true;
    }
    return false;
}

AT(.text.bsp.dac)
void dac_set_anl_offset(u8 bt_call_flag)
{
    sys_cb.gain_offset = DAC_MAX_GAIN;
}

AT(.text.bsp.dac)
void dac_set_vol_table(u8 vol_max)
{
    if (vol_max == 16) {
        dac_dvol_table = dac_dvol_tbl_16;
    } else {
        dac_dvol_table = dac_dvol_tbl_32;
    }
    dac_set_anl_offset(0);
}

//开机控制DAC电容放电等待时间
AT(.text.dac.pull)
void dac_pull_down_delay(void)
{
    delay_5ms(DAC_PULL_DOWN_DELAY);
}

#if LOUDSPEAKER_MUTE_EN
AT(.com_text.dac.mute)
void dac_set_mute_callback(u8 mute_flag)
{
    if (mute_flag) {
        bsp_loudspeaker_mute();
    } else {
        if (!sys_cb.mute) {
            bsp_loudspeaker_unmute();
        }
    }
}
#endif

AT(.text.bsp.dac)
void dac_init(void)
{
    sys_cb.dac_disable = 0;

    cfg_dac_out_spr = DAC_OUT_SPR;
//#if FUNC_USBDEV_EN
//    cfg_pcm_out_format = PCM_OUT_24BITS;                //打开usb device默认用24bit dac
//#else
    cfg_pcm_out_format = (DAC_24BITS_EN * PCM_OUT_24BITS) /*| PCM_OUT_MONO*/;
//#endif

    dac_set_vol_table(xcfg_cb.vol_max);
    printf("[%s] vol_max:%d, offset: %d\n", __func__, xcfg_cb.vol_max, sys_cb.gain_offset);

    dac_aubuf_init();

    dac_power_on();

    dac_ang_gain_set(ANL_N0DB);

    plugin_music_eq();

#if DAC_DRC_EN
    u32 res_addr = (u32)effect_res_addr_get(EFFECT_IDX_MSC_DAC_DRC);
    u32 res_len  = effect_res_len_get(EFFECT_IDX_MSC_DAC_DRC);
    music_drc_set_by_res(res_addr, res_len);
#endif
}

AT(.text.bsp.dac)
bool dac_is_disable(void)
{
    return (bool)(sys_cb.dac_disable);
}
