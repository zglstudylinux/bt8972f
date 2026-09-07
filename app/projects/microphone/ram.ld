//-----------------------------------------------------------------------------------
//  Created by zoro on 2024-5-22.
//
//  代码、常量、变量分段命名规则
//
//  格式: .seg.module.xxx
//      [seg    ] 声明段功能，例如.text, .bss, .data等
//      [module ] 声明所属模块，例如.text.dac, .text.adc等
//      [xxx    ] 声明模块功能，例如.text.dac.init, .text.dac.exit, .text.dac.proc.v1
//
//  [seg] 默认规则
//  .com_text.module.xxx        公共区代码段
//  .com_rodata.module.xxx      公共区常量
//  .text.module.xxx            flash代码段（默认放flash，也可以在ram.ld指定到RAM）
//  .rodata.module.xx           flash常量（默认放flash，也可以在ram.ld指定到RAM）
//  .data.module.xx             有初值的变量（公共区）
//  .bss.module.xx              无初值的变量（公共区）
//  .buf.module.xx              无初值的buffer（默认放公共区，也可以在ram.ld指定RAM空间）
//-----------------------------------------------------------------------------------


#include "config.h"

ENTRY(_start)

//Define the flash max size
__max_flash_size = FLASH_CODE_SIZE;

__base = 0x10000000;

// retention RAM
__cache_stack_vma   = 0x10600;  //for loader
__cache_stack_size  = 512;      //for loader
__stack_vma         = 0x10800;
__stack_ram_size    = 2k;
__comm_vma          = __stack_vma + __stack_ram_size;
__comm_ram_size     = 125k;


// non retention RAM
__cram_vma          = 0x30000;
__cram_size         = 128k;

// retention RAM for npu or ains4/5
__nram_vma          = 0x54000;      //npu
__nram_size         = 100k;
__ains4_ram_vma     = 0x54000;
__ains4_ram_size    = 100k;
__ains5_ram_vma     = 0x54000;
__ains5_ram_size    = 100k;

// non retention RAM
__aram_vma          = 0x100000;
__aram_size         = 20k;

__dram_vma          = 0x108000;
__dram_size         = 0x4600;
__eram_vma          = 0x107000;
__eram_size         = 0xa00;
__nrram_vma         = 0x105000;
__nrram_size        = 2400;


//-----------------------------------------------------------------------------------
//cram/nram复用段

//1.1 FM/AUX/SPEAKER
__func_overlap_vma      = __cram_vma;
__func_overlap_size     = __cram_size;


//1.2和1.3公共部分
__comm_overlap_vma      = __cram_vma;
__comm_overlap_size     = __cram_size;
__comm_overlap1_vma     = __nram_vma;
__comm_overlap1_size    = __nram_size;


//1.2 MUSIC_IO/MUSIC_A2DP公共部分

//1.2.1 MUSIC_IO(MP3/WMA/FLAC)
__msc_overlap_vma       = __comm_overlap_vma;
__msc_overlap_size      = 15k;
__msc_comm_vma          = __msc_overlap_vma + __msc_overlap_size;
__msc_comm_size         = __comm_overlap_size - __msc_overlap_size;
__msc_comm1_vma         = __comm_overlap1_vma;
__msc_comm1_size        = __comm_overlap1_size;

//1.2.2 MUSIC_A2DP(SBC/AAC/LHDC/LDAC)
__a2dp_overlap_vma      = __comm_overlap_vma;
__a2dp_overlap_size     = 38k;
__a2dp_comm_vma         = __a2dp_overlap_vma + __a2dp_overlap_size;
__a2dp_comm_size        = __comm_overlap_size - __a2dp_overlap_size;

__a2dp_overlap1_vma     = __comm_overlap1_vma;
__a2dp_overlap1_size    = __comm_overlap1_size - 32k;
__a2dp_comm1_vma        = __comm_overlap1_vma + __a2dp_overlap1_size;
__a2dp_comm1_size       = 32k;


//1.3 SCO
__sco_comm_vma          = __comm_overlap_vma;
__sco_comm_size         = __comm_overlap_size;
__sco_comm1_vma         = __comm_overlap1_vma;      //npu, sco_alg
__sco_comm1_size        = __comm_overlap1_size;


//1.4 USBDEV
__usbdev_overlap_vma    = __cram_vma;
__usbdev_overlap_size   = __cram_size;

//1.5 WIRELESS
__ws_com_ram_size       = 30k;
__ws_com_ram_vma        = __cram_vma + __cram_size - __ws_com_ram_size;

#if DEVICE_INTERPHONE_EN
__adapter_comm_size     = 75k;             //对讲机模式，接收端部分功能不用，省一些空间
__adapter_comm_vma      = __cram_vma;

__emit_comm_size        = __cram_size - __adapter_comm_size;
__emit_comm_vma         = __cram_vma + __adapter_comm_size;
#else
__adapter_comm_size     = __cram_size - __ws_com_ram_size;
__adapter_comm_vma      = __cram_vma;

__emit_comm_size        = __adapter_comm_size;
__emit_comm_vma         = __adapter_comm_vma;
#endif


MEMORY
{
    init                : org = __base,                 len = 512
    flash(rx)           : org = __base + 512,           len = __max_flash_size
    comm(rx)            : org = __comm_vma,             len = __comm_ram_size
    stack               : org = __stack_vma,            len = __stack_ram_size
    boot                : org = __cache_stack_vma,      len = __cache_stack_size

    aram                : org = __aram_vma,             len = __aram_size
    dram                : org = __dram_vma,             len = __dram_size
    eram                : org = __eram_vma,             len = __eram_size
    nram                : org = __nram_vma,             len = __nram_size
    nrram               : org = __nrram_vma,            len = __nrram_size

    //MUSIC
    msc_mp3_comm(rx)    : org = __msc_overlap_vma,      len = __msc_overlap_size
    msc_wma_comm(rx)    : org = __msc_overlap_vma,      len = __msc_overlap_size
    msc_aac_comm(rx)    : org = __msc_overlap_vma,      len = __msc_overlap_size
    msc_flac_comm(rx)   : org = __msc_overlap_vma,      len = __msc_overlap_size
    msc_wav_comm(rx)    : org = __msc_overlap_vma,      len = __msc_overlap_size
    msc_ape_comm(rx)    : org = __msc_overlap_vma,      len = __msc_overlap_size
    msc_comm            : org = __msc_comm_vma,         len = __msc_comm_size
    msc_comm1           : org = __msc_comm1_vma,        len = __msc_comm1_size

    //A2DP
    a2dp_comm(rx)       : org = __a2dp_comm_vma,        len = __a2dp_comm_size
    a2dp_comm1          : org = __a2dp_comm1_vma,       len = __a2dp_comm1_size
    a2dp_anc_tool_comm  : org = __comm_overlap1_vma,    len = __comm_overlap1_size

    //SCO
    sco_comm(rx)        : org = __sco_comm_vma,         len = __sco_comm_size
    sco_comm1(rx)       : org = __sco_comm1_vma,        len = __sco_comm1_size

    //FM/AUX/SPEAKER/I2S/SPDIF
    func_comm(rx)       : org = __func_overlap_vma,     len = __func_overlap_size

    //USBDEV
    usbdev_comm(rx)     : org = __usbdev_overlap_vma,   len = __usbdev_overlap_size

    //WIRELESS
    com_ws_com(rx)      : org = __ws_com_ram_vma,        len = __ws_com_ram_size
    adapter_comm(rx)    : org = __adapter_comm_vma,      len = __adapter_comm_size
    device_comm(rx)     : org = __emit_comm_vma,         len = __emit_comm_size
    com_ains4(rx)       : org = __ains4_ram_vma,         len = __ains4_ram_size
    com_ains5(rx)       : org = __ains5_ram_vma,         len = __ains5_ram_size

}

SECTIONS
{
    .init : {
        *(.reset)
    } > init

    .data_cpu (NOLOAD) : {
        *(.buf.cpu_regs)
    } > boot

    .code_comm : {
        __vertor_vma = .;
        *(.vector)
        *(.text.sram0*)
        *(.com_text*)
        *(.piano_com*)
        *debug.o(.rodata*)
        *(.com_rodata*)
        *(.text.npu*)
        *(.data*)
        *(.sdata*)
        *(.plt)
        *libvoices.a:math_float.o(.srodata*)
        . = ALIGN(512);
    } > comm AT > flash

//-----------------------------------------------------------------------------------
//EFFECT

//-----------------------------------------------------------------------------------
//A2DP和SCO公共部分
    .dram_sbc __dram_vma (NOLOAD) : {
        *(.sbc_decram_buf)
        . = 0xe00;
        *(.sbcbuf*)

        *(.tsco_cache)
        *(.sco_cache*)
        *(.sco_data*)
        *(.sco_hci)
//        *(.plc_buf)
//        *(.plc_data)
        *(.sbcenc.buf)
        *(.nr_buf*)
        *(.sco_dump*)
    } > dram

//-----------------------------------------------------------------------------------
//A2DP
    __effect_comm_vma = __a2dp_comm_vma;
    .code_effect : {
        *(.effect_rodata*)
        *(.effect_text*)
        . = ALIGN(4);
    } > a2dp_comm AT > flash

    __anc_comm_vma = __effect_comm_vma + SIZEOF(.code_effect);
    .code_anc : {
        *(.anc_rodata*)
        *(.anc_text*)
        *(.sndp_sae_rodata*)
        *(.sndp_sae_text*)
        . = ALIGN(4);
    } > a2dp_comm AT > flash

    .data_a2dp (NOLOAD) : {
        *(.a2dp_play)
        *(.a2dp_cache1)
        *(.codecs_buf.*)

        *(.anc_dbg_buf)
        *(.effect_buf*)
        *(.effect_exbuf*)
        *(.vad*)
    } > a2dp_comm

    .data_a2dp1 __aram_vma (NOLOAD) : {
        *(.a2dp_cache)
        *(.a2dp_play.bb.acl)
    } > aram

    .data_a2dp2 (NOLOAD) : {
        *(.a2dp_cache2)
    } > a2dp_comm1

//ANC TOOL
    .code_a2dp_anc_tool : {
        *(.rodata.anc_tool.*)
        *(.text.anc_tool.*)
        . = ALIGN(4);
    } > a2dp_anc_tool_comm AT > flash

    .buf_a2dp_anc_tool (NOLOAD) : {
        *(.buf.anc_tool.*)
        *(.eqtest.rxbuf)
    } > a2dp_anc_tool_comm

//-----------------------------------------------------------------------------------
//SCO
    .code_sco_alg : {

    } > sco_comm AT > flash

    .data_sco_alg (NOLOAD) : {
        *(.buf.aec.hnl)
        *(.agc_buf*)
        *(.buf.sco_pacc*)
        *(.bss1.sco_dump*)
        *(.sco_alg_buf*)
        . = ALIGN(4);
        __sco_overlap_vma = .;      //overlap start addr
    } > sco_comm

    .code_sco_dnn __sco_overlap_vma : {
        *(.rodata.dnn_npu.*)
        *(.text.dnn_npu.*)
        . = ALIGN(4);
    } > sco_comm AT > flash

    .data_dnn (NOLOAD) : {
        *(.buf.dnn_npu*)
    } > sco_comm

    .code_sco_dnn_pro __sco_overlap_vma : {
        *(.rodata.dnn_pro_npu.*)
        *(.text.dnn_pro_npu.*)
        . = ALIGN(4);
    } > sco_comm AT > flash

    .data_dnn_pro (NOLOAD) : {
        *(.buf.dnn_pro_npu*)
    } > sco_comm

    .code_sco_dmdnn __sco_overlap_vma : {
        *(.rodata.dmdnn_npu.*)
        *(.text.dmdnn_npu.*)
        . = ALIGN(4);
    } > sco_comm AT > flash

    .data_sco_dmdnn (NOLOAD) : {
        *(.dmdnn_npu_buf*)
    } > sco_comm

    .data_aec __aram_vma (NOLOAD) : {
        *(.aec_buf)
    } > aram
//-----------------------------------------------------------------------------------
//MUSIC
#if !DEVICE_RECORD_EN
    .data_music (NOLOAD) : {
        *(.avio_buf)
        *(.diskio_buf)
        *(.pff.buf)
        *(.fname.buf)
        *(.pff_buf*)
    } > msc_comm
#endif
    .data_music1 (NOLOAD) : {
        *(.mp3_dbg_buf*)
        *(.aac_dbg_buf*)
    } > msc_comm1

    .aram_music __aram_vma (NOLOAD) : {
        *(.mp3buf.dec)
        . = 0x1000;
        *(.stm_buf)
        . = 0x2000;
        *(.usb_buf*)
        . = 0x4800;
    } > aram

    .dram_mp3 __dram_vma (NOLOAD) : {
        . = 0x1000;
        *(.mp3_decram_buf)
        . = 0x4500;
    } > dram

    .dram_wav __dram_vma (NOLOAD) : {
        *(.wavbuf.dec)
        . = 0x3000;
    } > dram

//-----------------------------------------------------------------------------------
//USBDEV
#if !ADAPTER_USB_MIC_RX_EN && !DEVICE_USB_EN
    .data_usbdev (NOLOAD) : {
        __buf_start_usbdev = .;
        . = ALIGN(4);
        *(.udev_buf*)
        *(.usb.device.desc.buf)
        *(.ude.aubuf)
        __buf_end_usbdev = .;
    } > usbdev_comm

//    .dram_usb __dram_vma (NOLOAD) : {
//        *(.usb.device.desc.buf)
//        *(.ude.aubuf)
//        . = 0x3000;
//    } > dram
#endif

//-----------------------------------------------------------------------------------
//WIRELESS_MIC

    .code_ws_com : {
       __code_start_lc3com = .;
        *(.text.lc3_com*)
        *(.rodata.lc3_com*)
        *(.rodata.lc3)
        *(.text.lc3f_com*)
        *(.rodata.lc3f_com*)
        . = ALIGN(4);
        __code_end_lc3com = .;

        *(.text.dac0_proc*)
        *(.text.dac1_proc*)

#if DEVICE_RECORD_EN
        *(.avio_buf)
        *(.diskio_buf)
        *(.pff.buf)
        *(.fname.buf)
        *(.pff_buf*)
#endif

        . = ALIGN(512);
    } > com_ws_com AT > flash

    .data_ws_com (NOLOAD) : {
        __buf_start_lc3com = .;
        *(.buf.lc3f_com*)
        __buf_end_lc3com = .;

    } > com_ws_com

    .code_adapter : {
//        *(.text.adapter.wl_rx*)
//        *(.text.adapter.wl_tx*)
//        *(.text.adapter.mic_dec*)
//        *(.com_text.ble.wl_pscan*)
//        *(.com_text.ble.wl_page*)
//        *(.com_text.ble.t2r*)

        __code_start_pacc_voice = .;
        . = ALIGN(4);
        __code_end_pacc_voice = .;

        __code_start_plc_soft = .;
        *(.text.plc_proc*)
        *(.rodata.plc*)
        __code_end_plc_soft = .;

        __code_start_lc3dec = .;
        *(.text.lc3_dec*)
        *(.rodata.lc3_dec*)
        *(.text.lc3f_dec*)
        *(.rodata.lc3f_dec*)
        . = ALIGN(4);
        __code_end_lc3dec = .;

        __code_start_i2s = .;
        *(.text.i2s_audio_in_proc*)
        *(.text.i2s_audio_out_proc*)
        *(.text.i2s_in_out_proc*)
        . = ALIGN(4);
        __code_end_i2s = .;

#if ADAPTER_HUART_OUTPUT_EN || ADAPTER_HUART_INPUT_EN
        __code_huart_output = .;
        *(.text.huart*)
        *(.rodata.huart*)
        __code_huart_output = .;
#endif // ADAPTER_HUART_OUTPUT_EN

#if ADAPTER_YLCRN_L3_EN
        __code_start_ylcrn_L3 = .;
        *(.text.ylcrn_L3_proc*)
        *(.rodata.ylcrn_L3*)
        __code_end_ylcrn_L3 = .;
#endif

#if ADAPTER_YLCRN_L2_EN
        __code_start_ylcrn_L2 = .;
        *(.text.ylcrn_L2_proc*)
        *(.rodata.ylcrn_L2*)
        __code_end_ylcrn_L2 = .;
#endif

#if ADAPTER_ECHO_EN
        __code_start_echo = .;
        *(.text.echo_proc*)
        *(.rodata.echo*)
        __code_end_echo = .;
#endif
        __code_start_drc = .;
       *(.rodata.drc_v3_proc*)
       *(.text.drc_v3_proc*)
        . = ALIGN(512);
        __code_end_drc = .;

        __code_start_src = .;
        *(.text.src_proc*)
        *(.rodata.src*)
        __code_end_src = .;


//        __code_start_plc = .;
//        *(.text.plc_proc*)
//        *(.rodata.plc*)
//        . = ALIGN(4);
//        __code_end_plc = .;

        *(.text.mic_mix_proc*)

        . = ALIGN(512);
    } > adapter_comm AT > flash

    .data_adapter (NOLOAD) : {
        __buf_start_lc3dec = .;
        *(.buf.lc3_dec*)
        *(.buf.lc3f_dec*)
        __buf_end_lc3dec = .;

        __buf_start_plc = .;
        *(.buf.plc_buf_adapter)
        *(.buf.plc_buf)
        __buf_end_plc = .;

        __buf_start_src = .;
        *(.buf.src*)
        *(.src_buf*)
        __buf_end_src = .;

#if ADAPTER_HUART_OUTPUT_EN || ADAPTER_HUART_INPUT_EN
        __buf_huart = .;
        *(.buf.huart*)
        __buf_huart = .;
#endif // ADAPTER_HUART_OUTPUT_EN

#if ADAPTER_YLCRN_L3_EN
        __buf_start_ylcrn_L3 = .;
        *(.buf.ylcrn_L3*)
        __buf_end_ylcrn_L3 = .;
#endif

#if ADAPTER_YLCRN_L2_EN
        __buf_start_ylcrn_L2 = .;
        *(.buf.ylcrn_L2*)
        __buf_end_ylcrn_L2 = .;
#endif

#if ADAPTER_ECHO_EN
        __buf_start_echo = .;
        *(.buf.echo*)
        __buf_end_echo = .;
#endif

#if ADAPTER_USB_MIC_RX_EN
        __buf_start_usbdev = .;
        . = ALIGN(4);
        *(.usb.device.desc.buf)
        *(.udev_buf*)
        *(.ude.aubuf)
        __buf_end_usbdev = .;
#endif

#if ADAPTER_MFI_EN
        __buf_start_usb_iap2 = .;
        *(.iap2_buf*)
        *(.iap2.authen_info.buf)
        __buf_end_usb_iap2 = .;
#endif

        *(.buf.adapter*)
        *(.buf.mic_mix*)
#if I2S_EN
        __buf_start_i2s = .;
        *(.i2s_buf*)
        __buf_end_i2s = .;
#endif
    } > adapter_comm

    .code_device : {
        __code_start_lc3enc = .;
        *(.text.lc3_enc*)
        *(.rodata.lc3_enc*)
        *(.text.lc3f_enc*)
        *(.rodata.lc3f_enc*)
        __code_end_lc3enc = .;

#if WIRELESS_MIC_ECHO_EN
        __code_start_echo = .;
        *(.text.echo_proc*)
        *(.rodata.echo*)
        __code_end_echo = .;
#endif

#if WIRELESS_MIC_YLCRN_L3_EN
        __code_start_ylcrn_L3 = .;
        *(.text.ylcrn_L3_proc*)
        *(.rodata.ylcrn_L3*)
        __code_end_ylcrn_L3 = .;
#endif

#if DG_ADC_EN
        __code_start_dg_adc = .;
        *(.text.dg_adc_proc*)
        *(.rodata.dg_adc_*)
        __code_end_dg_adc = .;
#endif

#if WIRELESS_MIC_YLCRN_L2_EN
        __code_start_ylcrn_L2 = .;
        *(.text.ylcrn_L2_proc*)
        *(.rodata.ylcrn_L2*)
        __code_end_ylcrn_L2 = .;
#endif

#if WIRELESS_MIC_AGC_EN
        __code_start_agc = .;
        *(.text.agc_proc*)
        *(.rodata.agc*)
        __code_end_agc = .;
#endif

#if WIRELESS_MIC_DNR_FRE_EN
        __code_start_dnr_fre = .;
        *(.text.dnr_fre_proc*)
        *(.rodata.dnr_fre*)
        __code_end_dnr_fre = .;
#endif
        . = ALIGN(512);
    } > device_comm AT > flash

    .data_device (NOLOAD) : {
        __buf_start_lc3enc = .;
        *(.buf.lc3_enc*)
        __buf_end_lc3enc = .;

#if DEVICE_RECORD_EN
        __buf_start_record = .;
        *(.buf.record*)
        __buf_end_record = .;
#endif

#if WIRELESS_MIC_ECHO_EN
        __buf_start_echo = .;
        *(.buf.echo*)
        __buf_end_echo = .;
#endif

#if WIRELESS_MIC_YLCRN_L3_EN
        __buf_start_ylcrn_L3 = .;
        *(.buf.ylcrn_L3*)
        __buf_end_ylcrn_L3 = .;
#endif

#if DG_ADC_EN
        __buf_start_dg_adc = .;
        *(.buf.dg_adc*)
        __buf_end_dg_adc = .;
#endif

#if WIRELESS_MIC_YLCRN_L2_EN
        __buf_start_ylcrn_L2 = .;
        *(.buf.ylcrn_L2*)
        __buf_end_ylcrn_L2 = .;
#endif

#if WIRELESS_MIC_AGC_EN
        __buf_start_agc = .;
        *(.buf.agc*)
        __buf_end_agc = .;
#endif

#if WIRELESS_MIC_DNR_FRE_EN
        __buf_start_dnr_fre = .;
        *(.buf.dnr_fre*)
        __buf_end_dnr_fre = .;
#endif
        *(.buf.device_mic*)
    } > device_comm

    //AINS4
    .code_ains4 : {
        __code_start_ains4 = .;
        *(.text.ains4_proc*)
        *(.rodata.ains4*)
        *(.rodata1.ains4*)
        __code_end_ains4 = .;
        . = ALIGN(512);
    } > com_ains4 AT > flash

    .data_ains4 (NOLOAD) : {
        __buf_start_ains4 = .;
        *(.buf.ains4*)
        __buf_end_ains4 = .;
    } > com_ains4

    //AINS5
    .code_ains5 : {
        __code_start_ains5 = .;
        *(.text.ains5_proc*)
        *(.rodata.ains5*)
        __code_end_ains5 = .;
        . = ALIGN(512);
    } > com_ains5 AT > flash

    .data_ains5 (NOLOAD) : {
        __buf_start_ains5 = .;
        *(.buf.ains5*)
        __buf_end_ains5 = .;
    } > com_ains5
//-----------------------------------------------------------------------------------
//TEST
    .dram_upd __dram_vma (NOLOAD) : {
        . = 0x3800;
        *(.upd_ota*)
        *(.upd_buf*)
        . = 0x4500;
    } > dram

    .fcc_buf __aram_vma (NOLOAD) : {
        . = 0x1000;
        *(.fcc_buf.huart)
        . = 0x4c00;
        *(.test_buf)
    } > aram

    .eram_fot_qt __eram_vma (NOLOAD) : {
        *(.fot_data.buf)
        *(.qtest_buf)
        *(.epc.cb)
        . = 0xa00;
    } > eram

//-----------------------------------------------------------------------------------
//FM/AUX/SPEAKER/I2S/SPDIF
    .dram_spf __dram_vma (NOLOAD) : {
        *(.spf_buf*)
        . = 0x3000;
    } > dram

    .dram_i2s __dram_vma (NOLOAD) : {
        *(.i2s_buf*)
        . = 0x3000;
    } > dram

//-----------------------------------------------------------------------------------
    .flash : {
        __code_start_stream = .;
        *(.com_stream.text*)
        . = ALIGN(512);
        __code_end_stream = .;

        __code_start_sniff = .;
        *(.sniff*)
        . = ALIGN(512);
        __code_end_sniff = .;

        __code_start_btdec_com = .;
        *(.rodata.btdec*)
        *(.btdec*)
        . = ALIGN(512);
        __code_end_btdec_com = .;

        __code_start_mp3dec = .;
        *(.mp3dec*)
        . = ALIGN(512);
        __code_end_mp3dec = .;

        __code_start_sbcdec = .;
        *(.sbcdec*)
        *(.sbc_com*)
        . = ALIGN(512);
        __code_end_sbcdec = .;

        __code_start_aacdec = .;
        *(.aacdec*)
        . = ALIGN(512);
        __code_end_aacdec = .;

        __code_start_wsbcdec = .;
        *(.text.wsbc.asm)
        *(.text.wsbc.*)
        . = ALIGN(512);
        __code_end_wsbcdec = .;

        __code_start_wavdec = .;
        *(.wavdec*)
        . = ALIGN(512);
        __code_end_wavdec = .;

        __code_start_usbdev = .;
        *(.usbdev*)
        . = ALIGN(4);
        __code_end_usbdev = .;

        __code_start_sleep = .;
        *(.saradc_text*)
        *(.sleep_text*)
        . = ALIGN(512);
        __code_end_sleep = .;

        __code_start_charge = .;
        *(.text.charge*)
        *(.text.charge_box)
        *(.text.bsp.tkey)
        . = ALIGN(512);
        __code_end_charge = .;

        __code_start_bt_voice = .;
        *(.bt_voice*)
        *(.sco_mic_dnr*)
        . = ALIGN(512);
        __code_end_bt_voice = .;

        __code_start_pwrsave = .;
        *(.text.pwroff*)
        . = ALIGN(512);
        __code_end_pwrsave = .;

        __code_start_bb_test = .;
        *(.bb_test*)
        *(.bb_test_rodata*)
        . = ALIGN(512);
        __code_end_bb_test = .;

        __code_start_record = .;
        *(.com_rec*)
        . = ALIGN(512);
        __code_end_record = .;

        __code_start_update = .;
        *(.text.update*)
        . = ALIGN(512);
        __code_end_update = .;

        __code_start_huart = .;
        *(.com_huart*)
        . = ALIGN(512);
        __code_end_huart = .;

        __code_start_spdif = .;
        *(.spdif_text*)
        . = ALIGN(512);
        __code_end_spdif = .;

        __code_start_fota = .;
        *(.text.fot.cache*)
        . = ALIGN(512);
        __code_end_fota = .;

        __code_start_spiflash_id = .;
        *(.text.spiflash.*)
        . = ALIGN(512);
        __code_end_spiflash_id = .;

        __code_start_opus = .;
        *(.text.opus.asm)
        *(.text.opus.proc)
        *(.text.opus.ec_tell)
        *(.text.opus.entenc)
        *(.text.opus.mathops)
        *(.rodata.opus*)
        . = ALIGN(512);
        __code_end_opus = .;

        __code_start_vad = .;
        *(.vad_text*)
        . = ALIGN(512);
        __code_end_vad = .;

        __code_start_ws_com = .;
        *(.text.adapter.wl_rx*)
        *(.text.adapter.wl_tx*)
        *(.text.adapter.mic_dec*)
        *(.text.device.wl_rx*)
        *(.text.device.wl_tx*)
        *(.text.ble.t2r*)
        *(.text.ble.wl_page*)
        *(.text.ble.wl_pscan*)
        . = ALIGN(512);
        __code_end_ws_com = .;

        __code_start_bts = .;
        *(.text.stk.tws.txreq*)
        *(.text.stk.l2cap.txreq*)
        . = ALIGN(512);
        *(.text.stk.tws.send*)
        . = ALIGN(512);
        *(.text.stk.tws.recv*)
        . = ALIGN(512);
        *(.text.stk.hci.tx*)
        . = ALIGN(512);
        *(.text.bts.hci.rx*)
        . = ALIGN(512);
        *(.text.stk.bt.acl.tx*)
        . = ALIGN(512);
        __code_end_bts = .;

        *(.irq_init.aligned)
        *(.text.qtest.process)
        *(.text.func.process)
        *(.text.charge_box.process)
        *(.text.ab_mate.process)
        *(.text.func.bt.process)
        *(.text.warning.proc*)
        . = ALIGN(512);
        *(.text.ble.chmap*)
        . = ALIGN(512);

        *(.text*)
        *(.rodata*)
        *(.srodata*)
        *(.rela*)
        LONG(0)
        . = ALIGN(512);

    } > flash

    .data_comm (NOLOAD) : {
        . = ALIGN(4);
        __heap_start = .;
        *(.mem_heap)
        __heap_end = .;

        __bss_start = .;
        *(COMMON)
        *(.bss*)
        *(.sbss*)
        *(.buf*)

        *(.dac_obuf*)
        *(.adc_buf*)
        *(.tkey_buf.debug)
        *(.wav_res_buf)
        *(.au0out_buf*)
        *(.sco_enc.obuf)
        __bss_end = .;

        ///bram
        __btmem_start = .;
        *(.btmem.bthw)
        *(.btmem*)
        *(.btmem.stack.sdp)
        *(.btmem.stack.spp)
        *(.btmem.stack.l2cap)
        *(.btmem.stack.hid*)

        *(.ble_cache*)
        *(.ble_buf*)
        *(.gfps*)
        *(.app.buf*)
        *(.fot_data.com*)
        __btmem_end = .;
    } > comm
    __btmem_size = __btmem_end - __btmem_start;
    __bss_size  = __bss_end - __bss_start;
    __heap_size = __heap_end - __heap_start;

//-----------------------------------------------------------------------------------
//待整理
    .dram_opus __dram_vma (NOLOAD) : {
        *(.opus_buf*)
        //. = 0x4500;
    } > dram

    //若是叠加播放，需要独立的ram
    .dram_tone __dram_vma (NOLOAD) : {
        *(.wsbc_buf*)
        *(.tone_buf*)
    } > dram

    .dram_eq_rec __dram_vma (NOLOAD) : {
        *(.eq_rec_buf)
        . = 0x1000;
    }  > dram

    .nrram_peri_nr __nrram_vma (NOLOAD) : {
        *(.peri_nr*)
    } > nrram

    .eram_mp3enc __eram_vma (NOLOAD) : {
        *(.mp3_enc_hwbuf)
        . = 0xa00;
    } > eram

    .eram_mp2enc __eram_vma (NOLOAD) : {
        *(.l2enc_buf.cb)
        . = 0xa00;
    } > eram

    .nram_npu __nram_vma (NOLOAD) : {
        *(.npu_weight*)
        . = 0x10000;
        *(.npu_ram0*)
        . = 0x14000;
        *(.npu_ram1*)
        . = 0x19000;
    } > nram
}

//stack
__irq_stack_start = __stack_vma;
__irq_stack_end = __stack_vma + __stack_ram_size;
__irq_stack_size = __stack_ram_size;

//Calc the lma
__bank_size = SIZEOF(.flash);
__comm_lma  = LOADADDR(.code_comm);
__comm_size = SIZEOF(.code_comm);
__data_size = SIZEOF(.data_comm);

//MUSIC & A2DP
__code_effect_comm_vma    = __effect_comm_vma;
__code_effect_comm_lma    = LOADADDR(.code_effect);
__code_effect_comm_size   = SIZEOF(.code_effect);

//SCO
__code_sco_comm_vma     = __sco_comm_vma;
__code_sco_comm_lma     = LOADADDR(.code_sco_alg);
__code_sco_comm_size    = SIZEOF(.code_sco_alg);
__code_sco_dnn_vma      = __sco_overlap_vma;
__code_sco_dnn_lma      = LOADADDR(.code_sco_dnn);
__code_sco_dnn_size     = SIZEOF(.code_sco_dnn);
__code_sco_dnn_pro_vma  = __sco_overlap_vma;
__code_sco_dnn_pro_lma  = LOADADDR(.code_sco_dnn_pro);
__code_sco_dnn_pro_size = SIZEOF(.code_sco_dnn_pro);

__code_sco_dmdnn_vma    = __sco_overlap_vma;
__code_sco_dmdnn_lma    = LOADADDR(.code_sco_dmdnn);
__code_sco_dmdnn_size   = SIZEOF(.code_sco_dmdnn);

//WIRELESS_MIC
__code_ws_com_vma       = __ws_com_ram_vma;
__code_ws_com_lma       = LOADADDR(.code_ws_com);
__code_ws_com_size      = SIZEOF(.code_ws_com);
__code_adapter_vma      = __adapter_comm_vma;
__code_adapter_lma      = LOADADDR(.code_adapter);
__code_adapter_size     = SIZEOF(.code_adapter);
__data_adapter_size     = SIZEOF(.data_adapter);
__code_emit_vma         = __emit_comm_vma;
__code_emit_lma         = LOADADDR(.code_device);
__code_emit_size        = SIZEOF(.code_device);
__data_emit_size        = SIZEOF(.data_device);

//AINS4
__code_ains4_vma         = __ains4_ram_vma;
__code_ains4_lma         = LOADADDR(.code_ains4);
__code_ains4_size        = SIZEOF(.code_ains4);
__data_ains4_size        = SIZEOF(.data_ains4);
//AINS5
__code_ains5_vma         = __ains5_ram_vma;
__code_ains5_lma         = LOADADDR(.code_ains5);
__code_ains5_size        = SIZEOF(.code_ains5);
__data_ains5_size        = SIZEOF(.data_ains5);
