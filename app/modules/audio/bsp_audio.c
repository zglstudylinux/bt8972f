#include "include.h"

void bt_adc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void bt_sco_tx_process(u8 *ptr, u32 samples, u32 pcm_mode);
void bt_alc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void aux_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void speaker_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void uda_mic_in_process(u8 *ptr, u32 samples, u32 pcm_mode);
void karaok_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void voice_assistant_sdadc_process(u8 *ptr,u32 samples,u32 pcm_mode);
void ttp_sdadc_process(u8 *ptr, u32 samples, u32 pcm_mode);
void anc_alg_sdadc_process(u8* ptr, u32 samples, u32 pcm_mode);
void anc_alg_audio_path_cfg_set(sdadc_cfg_t* cfg);

#if FUNC_AUX_EN
    #define aux_sdadc_callback      aux_sdadc_process
#else
    #define aux_sdadc_callback      sdadc_dummy
#endif // FUNC_AUX_EN

#if FUNC_SPEAKER_EN
    #define speaker_sdadc_callback  speaker_sdadc_process
#else
    #define speaker_sdadc_callback  sdadc_dummy
#endif // FUNC_SPEAKER_EN

#if UDE_MIC_EN
    #define usbmic_sdadc_callback   uda_mic_in_process
#else
    #define usbmic_sdadc_callback   sdadc_dummy
#endif // UDE_MIC_EN

    #define bt_sdadc_callback       bt_adc_process

    #define karaok_sdadc_callback   sdadc_dummy

#if TINY_TRANSPARENCY_EN
    #define ttp_sdadc_callback      ttp_sdadc_process
#else
    #define ttp_sdadc_callback      sdadc_dummy
#endif

#define opus_sdadc_callback   sdadc_dummy

#define anc_alg_sdadc_callback      sdadc_dummy

#if IODM_TEST_EN
    #define iodm_test_sdadc_callback     iodm_test_sdadc_process
#else
    #define iodm_test_sdadc_callback     sdadc_dummy
#endif
#if VAD_EN
    #define vad_sdadc_callback     vad_sdadc_process
#else
    #define vad_sdadc_callback     sdadc_dummy
#endif

//MIC analog gain: 0~14(共15级), step 3DB (0db ~ +42db)
//adadc digital gain: 0~63, step 0.5 DB, 保存在gain的低6bit
const sdadc_cfg_t rec_cfg_tbl[] = {
/*   通道,             采样率,    模拟增益, 数字增益,    BITS,    通路控制,    样点数,   回调函数*/
    {AUX_CHANNEL_CFG,  SPR_44100,   2,         0,       0xff,     ADC2DAC_EN,    256,    aux_sdadc_callback},            /* AUX     */
    {MIC_CHANNEL_CFG,  SPR_48000,   10,        0,       0xff,     ADC2DAC_EN,    256,    speaker_sdadc_callback},        /* SPEAKER */
    {MIC_CHANNEL_CFG,  SPR_8000,    12,        0,       1,        ADC2DAC_EN,    480,    bt_sdadc_callback},             /* BTMIC   */
    {MIC_CHANNEL_CFG,  SPR_48000,   12,        0,       0,        ADC2DAC_EN,    48*3,   usbmic_sdadc_callback},         /* USBMIC  */
    {MIC_CHANNEL_CFG,  SPR_44100,   12,        0,       0xff,     ADC2SRC_EN,    256,    karaok_sdadc_callback},         /* KARAOK  */
    {MIC_CHANNEL_CFG,  SPR_16000,   12,        0,       0xff,     ADC2DAC_EN,    256,    opus_sdadc_callback},           /* opus  */
    {MIC_CHANNEL_CFG,  SPR_44100,   6,         0,       1,        ADC2SRC_EN,    128,    ttp_sdadc_callback},            /* TRANSPARENCY  */
    {MIC_CHANNEL_CFG,  SPR_16000,   12,        0,       1,        ADC2SANC_EN,   256,    anc_alg_sdadc_callback},        /* ANC ALG */
    {MIC_CHANNEL_CFG,  SPR_48000,   10,        0,       0xff,     ADC2DAC_EN,    256,    iodm_test_sdadc_callback},      /* IODM TEST */
    {MIC_CHANNEL_CFG,  SPR_16000,   10,        0,       0xff,     ADC2ASR_EN,    256,    vad_sdadc_callback},            /* ASR */
};

AT(.rodata.mic)
const u8 mic_mapping_tbl[5] = {CH_MIC0, CH_MIC1, CH_MIC2, CH_MIC3, CH_MIC4};

#if FUNC_AUX_EN
AT(.rodata.aux)
static const u16 auxl_adc_tbl[3] = {
    0, CH_AUXL0, CH_AUXL1,
};

AT(.rodata.aux)
static const u16 auxr_adc_tbl[3] = {
    0, CH_AUXR0, CH_AUXR1,
};

u16 bsp_aux_ch_getcfg(void);
#endif

/*****************************************************************************
 * 功能   : 根据Settings，获取mic通路
 * 输入   : audio_path_idx,用于判断是否为BT_MIC_PATH
 * 注意   : 若mic通路选取异常，会报err;BT_MIC_PATH可选单、双麦,其他模式只采用单麦即主麦
 * 返回   : 无
 *****************************************************************************/
static u32 bsp_mic_ch_getcfg(u8 audio_path)
{
    u16 ch_sel = mic_mapping_tbl[xcfg_cb.call_talk_mic];

    return ch_sel;
}

/*****************************************************************************
 * 功能   : 初始化对应AUDIO_PATH
 * 输入   : audio_path_idx 对应不同功能使用的audio通路
 * 注意   : 区分bt_call和其他状态
 * 返回   : 无
 *****************************************************************************/
void audio_path_init(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));

#if FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_AUX) {
        cfg.channel = bsp_aux_ch_getcfg();
        cfg.anl_gain = xcfg_cb.aux_anl_gain | xcfg_cb.aux_anl_gain<<6;      //双声道模拟增益保持一致
        cfg.dig_gain = xcfg_cb.aux_dig_gain | xcfg_cb.aux_dig_gain<<6;      //双声道数字增益保持一致
    }
#endif // FUNC_AUX_EN

    if (path_idx == AUDIO_PATH_BTMIC || path_idx == AUDIO_PATH_KARAOK || path_idx == AUDIO_PATH_OPUS || path_idx == AUDIO_PATH_TTP || path_idx == AUDIO_PATH_SPEAKER || path_idx == AUDIO_PATH_USBMIC || path_idx == AUDIO_PATH_ASR) {
        if (path_idx == AUDIO_PATH_BTMIC) {
            if (sys_cb.hfp_karaok_en) {
                memcpy(&cfg, &rec_cfg_tbl[AUDIO_PATH_KARAOK], sizeof(sdadc_cfg_t));
                cfg.sample_rate = SPR_48000;
            } else {
                if (bt_sco_is_msbc() || bt_sco_dnn_en()) {     //如果开了msbc或dnn，则采样率设为16k
                    cfg.sample_rate = SPR_16000;
                }
            }
        }
        cfg.channel = bsp_mic_ch_getcfg(path_idx);

        cfg.anl_gain = ((xcfg_cb.mic_anl_gain) |
                        (xcfg_cb.mic_anl_gain<<6) |
                        (xcfg_cb.mic_anl_gain<<12)|
                        (xcfg_cb.mic_anl_gain<<18)|
                        (xcfg_cb.mic_anl_gain<<24));
        cfg.dig_gain = ((xcfg_cb.bt_mic_dig_gain) |
                        (xcfg_cb.bt_mic_dig_gain<<6) |
                        (xcfg_cb.bt_mic_dig_gain<<12)|
                        (xcfg_cb.bt_mic_dig_gain<<18)|
                        (xcfg_cb.bt_mic_dig_gain<<24));
    }

#if IODM_TEST_EN
    if (path_idx == AUDIO_PATH_IODM_MIC_TEST){
        iodm_test_loopback_set_mic_ch(&cfg.channel, &cfg.anl_gain, &cfg.dig_gain);
    }
#endif

    int ret = sdadc_init(&cfg);
    if(ret < 0) {
        printf("sdadc_init fail: %d\n", ret);
    }
}

/*****************************************************************************
 * 功能   : 启动AUDIO采集和DAC数据处理
 * 输入   : audio_path_idx 对应不同功能使用的audio通路
 * 注意   : channel需要和init时保持一致，否则通路会启动失败
 * 返回   : 无
 *****************************************************************************/
void audio_path_start(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));
#if FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_AUX) {
        cfg.channel = bsp_aux_ch_getcfg();
    }
#endif // FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_BTMIC || path_idx == AUDIO_PATH_KARAOK || path_idx == AUDIO_PATH_OPUS || path_idx == AUDIO_PATH_TTP || path_idx == AUDIO_PATH_SPEAKER || path_idx == AUDIO_PATH_USBMIC || path_idx == AUDIO_PATH_ASR) {
        cfg.channel = bsp_mic_ch_getcfg(path_idx);
    }

#if IODM_TEST_EN
    if (path_idx == AUDIO_PATH_IODM_MIC_TEST){
        iodm_test_loopback_set_mic_ch(&cfg.channel, &cfg.anl_gain, &cfg.dig_gain);
    }
#endif

    sdadc_start(cfg.channel);

    if(path_idx != AUDIO_PATH_BTMIC) {
        sdadc_dma_start(cfg.channel);
    }
}

/*****************************************************************************
 * 功能   : 关闭对应AUDIO_PATH
 * 输入   : audio_path_idx 对应不同功能使用的audio通路
 * 注意   : 关闭audio后，功耗要和打开audio之前保持一致
 * 返回   : 无
 *****************************************************************************/
void audio_path_exit(u8 path_idx)
{
    sdadc_cfg_t cfg;
    memcpy(&cfg, &rec_cfg_tbl[path_idx], sizeof(sdadc_cfg_t));

#if FUNC_AUX_EN
    if (path_idx == AUDIO_PATH_AUX) {
        cfg.channel = bsp_aux_ch_getcfg();
    }
#endif // FUNC_AUX_EN

    if (path_idx == AUDIO_PATH_BTMIC || path_idx == AUDIO_PATH_KARAOK || path_idx == AUDIO_PATH_OPUS || path_idx == AUDIO_PATH_TTP || path_idx == AUDIO_PATH_SPEAKER || path_idx == AUDIO_PATH_ASR || path_idx == AUDIO_PATH_USBMIC) {
        cfg.channel = bsp_mic_ch_getcfg(path_idx);
    }

#if IODM_TEST_EN
    if (path_idx == AUDIO_PATH_IODM_MIC_TEST){
        iodm_test_loopback_set_mic_exit(&cfg.channel);
    }
#endif

    sdadc_exit(cfg.channel);

    {
        adpll_spr_set(DAC_OUT_SPR);
    }
}

u16 bsp_aux_ch_getcfg(void)
{
#if FUNC_AUX_EN
    u16 aux_sel = auxl_adc_tbl[xcfg_cb.auxl_sel] | (auxr_adc_tbl[xcfg_cb.auxr_sel] << 8);
    return aux_sel;
#else
    return 0;
#endif // FUNC_AUX_EN
}
