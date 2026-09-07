/*
 * 本地MIC混合无线MIC后，一起输出
 *
 ****************************************************************************************
 */
#include "include.h"
#include "mic_mix.h"

#if ADAPTER_LOCAL_MIC_MIX_EN

#if WIRELESS_MIC_24B_PCM_EN
    #define EXP_MILTIPLE_24BIT        2           //开启24bit时，数据长度*2
#else
    #define EXP_MILTIPLE_24BIT        1
#endif // WIRELESS_MIC_24B_PCM_EN

#define MIC_INCACHE_SIZE            (WIRELESS_MIC_SAMPLES_SELECT*2) * 2 * EXP_MILTIPLE_24BIT
#define DATA_CACHE_LEN              WIRELESS_MIC_SAMPLES_SELECT * EXP_MILTIPLE_24BIT

static mic_mix_t mic_mix;

u8 mic_incache      [MIC_INCACHE_SIZE]          AT(.buf.mic_mix.cache);
s16 mic_outcache    [DATA_CACHE_LEN]            AT(.buf.mic_mix.cache);

AT(.buf.mic_mix.cache);
ring_buf_t mic_ring_buf;

AT(.rodata.mic_mix.tbl)
const rbuf_tbl_t mic_buf_tbl[1] = {
    {
        .buf = mic_incache,
        .size = MIC_INCACHE_SIZE,
    },
};

AT(.text.mic_mix.init)
void mic_ring_buf_init(void)
{
    ring_buf_init(&mic_ring_buf, mic_buf_tbl, 1, 0);
}

AT(.text.mic_mix)
bool mic_ring_buf_write(uint8_t *ptr, uint len)
{
    return ring_buf_put((void *)&mic_ring_buf, ptr, len);
}

AT(.text.mic_mix)
bool mic_ring_buf_read(uint8_t *ptr, uint len)
{
    return ring_buf_get((void *)&mic_ring_buf, ptr, len);
}

AT(.text.mic_mix)
uint32_t mic_buf_pcm_get_data_len(void)
{
    return mic_ring_buf.count;
}

///本地麦adc采样回调
AT(.com_text.bsp.wireless_mic)
void mic_mix_process_cb(u8 *ptr, u32 samples, u32 params)
{
    if(mic_mix.mute) {
        memset(ptr, 0, samples * 2 * EXP_MILTIPLE_24BIT);
    }
//#if ADAPTER_LOCAL_MIC_EQ_DRC_EN
//    //不在adc会调中处理硬件eq/drc要注意传参地址是否合法
//    mic_eq_drc_proc((u16 *)ptr,(u16 *)ptr,samples);
//#endif

    if(!wireless_cb.connected_sta && mic_mix.kick_flag) {
        mic_mix.kick_flag = 0;
        memset(&mic_incache[0], 0, MIC_INCACHE_SIZE);
    }

    if(mic_mix.kick_flag){
        mic_ring_buf_write(ptr, samples * 2 * EXP_MILTIPLE_24BIT);
    } else  {
        mic_ring_buf_write(ptr, samples * 2 * EXP_MILTIPLE_24BIT);
        local_mic_proc_start();
    }
}

///无线麦未连接时，alg线程kick到这里，拿数据后推DAC
AT(.com_text.bsp.wireless_mic)
void local_mic_output(void)
{
    mic_ring_buf_read((void *)mic_outcache, WIRELESS_MIC_SAMPLES_SELECT * 2 * EXP_MILTIPLE_24BIT);

    mic_mix_audio_input((void *)mic_outcache, WIRELESS_MIC_SAMPLES_SELECT, BIT(0) | (WIRELESS_MIC_24B_PCM_EN << 7));
}

AT(.text.mic_mix)
void mic_mix_set_mute(u8 mute)
{
    mic_mix.mute = mute;

    if(mic_mix.mute) {
        memset(&mic_incache[0], 0, MIC_INCACHE_SIZE);
    }
}

//无线麦克风连接时走这里输出
AT(.com_text.mic_mix)
void mic_mix_audio_input(u8 *ptr, u32 samples, u32 params)
{
    if (samples && wireless_cb.connected_sta){
        if(!mic_mix.kick_flag) {
            mic_mix.kick_flag = 1;
        }
    }

    if (mic_mix.callback) {
        mic_mix.callback((void *)ptr, samples, params);
    }
}

AT(.text.mic_mix)
void mic_mix_output_callback_set(audio_callback_t callback)
{
    mic_mix.callback = callback;
}

AT(.text.dual_mic.proc)
s16 *mic_mix_buf_read(u16 samples)
{
    if (mic_ring_buf_read((void *)mic_outcache, samples * 2 * EXP_MILTIPLE_24BIT)) {
        return (void*)mic_outcache;
    }
}

AT(.text.mic_mix)
void mic_mix_init(u8 sample_rate, u16 samples, u8 channel)
{
    printf("%s\n", __func__);
    memset(mic_incache, 0, sizeof(mic_incache));
    memset(mic_outcache, 0, sizeof(mic_outcache));
    mic_ring_buf_init();
//#if ADAPTER_LOCAL_MIC_EQ_DRC_EN
//    mic_eq_drc_cfg(0);
//    if (mic_eq_set_by_res(RES_BUF_WS_MIC_48K_EQ, RES_LEN_WS_MIC_48K_EQ)) {
//    }
//    if(mic_drc_set_by_res(RES_BUF_WS_MIC_48K_DRC, RES_LEN_WS_MIC_48K_DRC)) {
//    }
//#endif

    local_mic_init();                   //本地麦sdadc初始化
}

AT(.text.mic_mix)
void mic_mix_pacc_init(void)
{
//    printf("%s\n", __func__);
//
//    //先初始化PACC链路
//    mix_pacc_init();
//
//    //然后设置参数
//    mix_pacc_set_param();
//
//    //最后使能PACC
//    mix_pacc_enable();
}

AT(.text.karaok)
void local_mic_pacc_init(void)
{
//    //先初始化PACC链路
//    loc_mic_pacc_init();
//
//    //然后设置参数
//    loc_mic_pacc_set_param();
//
//    //最后使能PACC
//    loc_mic_pacc_enable();
//
//    mic_mix_pacc_init();
}

#endif
