#include "include.h"
#include "api.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

#if WIRELESS_MIC_EN && WIRELESS_SPK_EN && !DEVICE_INTERPHONE_EN
    #define D2A_CODEC               (WIRELESS_CON_CODEC_SEL | WS_CODEC_MONO)
    #define A2D_CODEC               (WIRELESS_CON_CODEC_SEL | CODEC_STEREO)
#elif WIRELESS_MIC_EN
    #define D2A_CODEC               (WIRELESS_CON_CODEC_SEL | WS_CODEC_MONO)
    #define A2D_CODEC               0
#else
    #define D2A_CODEC               0
    #define A2D_CODEC               (WIRELESS_CON_CODEC_SEL | WS_CODEC_MONO)
#endif

#if WIRELESS_EN
//link
//uint8_t cfg_wireless_cntl_bit       = WIRELESS_CON_CNTL_BIT;
uint8_t cfg_wireless_rssi_thr       = WIRELESS_CON_RSSI_THR;
uint8_t cfg_wireless_link_nb        = WIRELESS_CON_LINK_NB;
uint8_t cfg_wireless_d2a_tx_size    = WIRELESS_MIC_TX_BUFFER_SIZE;                      //传输封包大小
uint8_t cfg_wireless_con_interval   = WIRELESS_CON_INTERVAL*WIRELESS_CON_LINK_NB;       //连接interval（V1的传输interval），单位1.25ms
uint8_t cfg_wireless_tx_comb_nb     = WIRELESS_MIC_COMB_NB;                             //传输封包组合数量
uint8_t cfg_wireless_tx_interval    = WIRELESS_MIC_TX_INTERVAL;                         //V2、V3的传输interval，单位1.25ms
uint8_t cfg_wireless_tx_retry       = WIRELESS_MIC_RETRY_NB;                            //V2、V3的传输次数
uint16_t cfg_wireless_feat          = ((WIRELESS_CON_VERS<<8)&FEAT_VERS) | (FEAT_D2A*WIRELESS_MIC_EN) | (FEAT_A2D*WIRELESS_SPK_EN) | (FEAT_HOP_V1) | (FEAT_BONDING*WIRELESS_CON_BONDING_EN) | (WIRELESS_CON_LINK_NB&0xf);
uint16_t cfg_wireless_ext_feat      = ((EXT_FEAT_ADV_EXT*WIRELESS_CON_ADV_EXT_EN) | (((WIRELESS_CON_VERS>>3)<<1)&EXT_FEAT_VERS));
uint16_t cfg_wireless_d2a_enc_us    = (WIRELESS_MIC_ENC_MAX_US + WIRELESS_MIC_EQ_DRC_DELAY + WIRELESS_MIC_ECHO_DELAY + WIRELESS_MIC_AGC_DELAY);
uint16_t cfg_wireless_d2a_dec_us    = WIRELESS_MIC_DEC_MAX_US;
#if DEVICE_INTERPHONE_EN
uint16_t cfg_wireless_a2d_enc_us    = (WIRELESS_MIC_ENC_MAX_US + WIRELESS_MIC_EQ_DRC_DELAY + WIRELESS_MIC_ECHO_DELAY + WIRELESS_MIC_AGC_DELAY);
uint16_t cfg_wireless_a2d_dec_us    = WIRELESS_MIC_DEC_MAX_US;
uint8_t cfg_wireless_a2d_tx_retry   = WIRELESS_MIC_RETRY_NB;                            //V5、V9的SPK的传输次数
uint8_t cfg_wireless_a2d_tx_comb_nb = WIRELESS_MIC_COMB_NB;                             //SPK传输封包组合数量
uint8_t cfg_wireless_a2d_tx_size    = WIRELESS_MIC_TX_BUFFER_SIZE;                      //传输封包大小
#else
uint8_t cfg_wireless_a2d_tx_size    = WIRELESS_SPK_TX_BUFFER_SIZE;                      //传输封包大小
uint8_t cfg_wireless_a2d_tx_retry   = WIRELESS_SPK_RETRY_NB;                            //V5、V9的SPK的传输次数
uint8_t cfg_wireless_a2d_tx_comb_nb = WIRELESS_SPK_COMB_NB;                             //SPK传输封包组合数量
uint16_t cfg_wireless_a2d_enc_us    = WIRELESS_SPK_ENC_MAX_US;
uint16_t cfg_wireless_a2d_dec_us    = WIRELESS_SPK_DEC_MAX_US;
#endif
uint8_t cfg_wireless_ota_en         = ADAPTER_AB_FOT_DEVICE_SUPPORT;
uint8_t cfg_wireless_codec[2]       = {
    D2A_CODEC,      //D2A Codec
    A2D_CODEC,      //A2D Codec
};

#if WIRELESS_MIC_2TNR_EN
uint8_t cfg_wireless_2tnr_nb        = WIRELESS_CON_2TNR_NB;
#endif

//设备组队时，首先用设备名称le_name匹配，在setting中可以设置
//同时也检查设备的单/双工、编解码特性，特性不同不允许组队，参见cfg_wireless_feat和cfg_wireless_codec

uint8_t cfg_discon_auto_pwroff      = 0;

//rf
uint8_t cfg_bb_rf_freq_bands        = WIRELESS_CON_FREQ_BAND; //0:2402M~2480M  1:2200M~2278M  2:2320M~2398M  3:2500M~2578M  4: 2482M~2560M


AT(.com_text.mic_dec)
void wireless_channel_status(u8 idx, u8 chidx, s8 rssi, u8 bfi, u8 chstatus)
{
    if(bfi || chstatus != 0) {
        if(rssi > -80 || (rssi > -85 && chstatus == 250)) {
            wireless_con_channel_assess(chidx, false);
        }
    } else {
        wireless_con_channel_assess(chidx, true);
    }

    wireless_dump_set_rx_status(idx, chidx, rssi, (bfi || chstatus != 0));
}

//------------------------------------------------------------------------------------------
AT(.text.wireless.cb)
int wireless_get_local_name(uint8_t *wl_name)
{
#if TBOX_TEST_EN
    if (vusb_test_is_sucess()) {
        return vusb_test_name_get(wl_name);
    }
#endif

    int len;
    len = strlen(xcfg_cb.wl_name);

    memcpy(wl_name, xcfg_cb.wl_name, len);

    return len;
}

AT(.text.wireless.cb)
void wireless_get_local_bd_addr(u8 *addr)
{
#if BT_LOCAL_ADDR
    param_random_key_read(&addr[2]);
    addr[0] = 0x41;
    addr[1] = 0x42;
#else
    memcpy(addr, xcfg_cb.bt_addr, 6);
#endif
}

AT(.text.wireless.cb)
void wireless_con_set_chmap_cb(u8 idx, const u8 *chmap)
{
    wireless_dump_set_chmap_cb(idx, chmap);
}

//------------------------------------------------------------------------------------------
bool wireless_get_link_info_addr(uint8_t *wl_addr)
{
    return false;
}


u8 con_req_addr[6];
#if WIRELESS_MIC_2TNR_EN
struct con_info_t{
    uint8_t save_addr1[6];
    uint8_t save_addr2[6];
    uint8_t save_addr_sta;
    uint8_t req_con_sta;            //当前状态下需要请求连接的链路
} con_info_2tnr;

bool wireless_scan_2tnr_can_deal(uint8_t *addr, uint8_t addr_type, uint8_t *adv_ext_data)
{
    bool deal_flag = 0;

    if (wireless_role_is_adapter()) {
        if ((wireless_cb.connected_sta == 0) && (con_info_2tnr.req_con_sta&BIT(0))) {
            memcpy(con_info_2tnr.save_addr1, addr, 6);
        }

        if (((wireless_cb.connected_sta&BIT(0)) == 0) && (con_info_2tnr.req_con_sta&BIT(0)) && !memcmp(con_info_2tnr.save_addr1, addr, 6)) {
            func_adapter_set_create_con();
        }

        if ((wireless_cb.connected_sta == BIT(0)) && (con_info_2tnr.req_con_sta&BIT(1)) && !memcmp(con_info_2tnr.save_addr2, addr, 6)) {
            func_adapter_set_create_con();
        }

        deal_flag = 1;
    }

    return deal_flag;
}

void wireless_second_emit_rep_cb(uint8_t *addr, uint8_t con_sta, uint8_t status, uint8_t index)
{
    printf("wireless_second_emit_rep_cb, con_sta = %d, status = %d, index = %d, wireless_cb.connected_sta = %d\n", con_sta, status, index, wireless_cb.connected_sta);
    printf("addr: ");
    print_r(addr, 6);

    if (con_sta) {
        if (wireless_cb.connected_sta == BIT(0)) {
            memcpy(con_info_2tnr.save_addr2, addr, 6);
            con_info_2tnr.req_con_sta |= BIT(1);
        } else if (wireless_cb.connected_sta == BIT(1)) {
            memcpy(con_info_2tnr.save_addr1, addr, 6);
            con_info_2tnr.req_con_sta |= BIT(0);
        } else if (wireless_cb.connected_sta == (BIT(0) | BIT(1))) {
            ///收到断开指令后还没来得及断开或蓝牙线程没那么快回调处理完，马上又发来了连接
            if (con_info_2tnr.req_con_sta == BIT(0)) {
                memcpy(con_info_2tnr.save_addr2, addr, 6);
                con_info_2tnr.req_con_sta |= BIT(1);
            } else if (con_info_2tnr.req_con_sta == BIT(1)) {
                memcpy(con_info_2tnr.save_addr1, addr, 6);
                con_info_2tnr.req_con_sta |= BIT(0);
            }
        }
    } else {
//        if (wireless_cb.connected_sta == (BIT(0) | BIT(1))) {
            if (!memcmp(con_info_2tnr.save_addr1, addr, 6)) {
                con_info_2tnr.req_con_sta &= ~BIT(0);
            } else if (!memcmp(con_info_2tnr.save_addr2, addr, 6)) {
                con_info_2tnr.req_con_sta &= ~BIT(1);
            }

            if (status == 0x08 || status == 0x3e) {
                wireless_disconnect_req(index);
            }
//        }
    }
}

void wireless_2tnr_info_init(void)
{
    memset((u8 *)&con_info_2tnr, 0, sizeof(struct con_info_t));
    con_info_2tnr.req_con_sta = BIT(0);
}

///wireless adapter断开后，清除对应的连接消息
void wireless_2tnr_info_clr(void)
{
    wireless_2tnr_info_init();
}
#endif

//发送ble_connect_req消息之后，库获取连接地址的回调函数
AT(.text.wireless.cb)
bool wireless_get_con_req_addr_cb(uint8_t *addr)
{
    memcpy(addr, con_req_addr, 6);
    return true;
}

//发送ble_scan_set_enable消息之后，底层收到数据包的回调函数
//返回值：0=错误的地址，继续扫描；1=地址正确，自动关闭扫描，可发起连接请求
AT(.text.wireless.cb)
bool wireless_scan_rx_rep_cb(uint8_t *addr, uint8_t addr_type, uint8_t *adv_ext_data)
{
#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
    bool is_con_allowed = wireless_bonding_check(adv_ext_data);
    if(!is_con_allowed) {
        return false;
    }
#endif

    TRACE("addr: ");
    TRACE_R(addr, 6);

    memcpy(con_req_addr, addr, 6);

    ///准备发起连接，关闭scan
#if WIRELESS_MIC_2TNR_EN
    if (wireless_scan_2tnr_can_deal(addr, addr_type, adv_ext_data)) {
    } else
#endif
    {
        func_device_set_create_con();
    }

    return true;
}

//addr: 连接的设备地址
//timeout: 超时时间（单位ms，0xffff时一直连接不超时）
AT(.text.wireless.api)
void wireless_create_con_for_addr(uint8_t *addr, uint16_t timeout)
{
    memcpy(con_req_addr, addr, 6);
    wireless_connect_req(timeout);   //发消息后，通过回调函数ble_get_con_req_addr_cb获取连接地址
}

#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
static struct{
    u16 new_key;
}wl_bonding_cb;

//--------------------------------------------------------------
//device端调用
AT(.text.wireless.cb)
bool wireless_bonding_check(uint8_t *param)
{
    u8 header       = param[0];
    u8 con_flag     = param[1];
    u8 remote_bd_nb = param[2];
    u16 bd_key      = (param[4]<<8)|param[3];

    TRACE("check:%d, 0x%x, 0x%x\n", sys_cb.wl_bond_nb, bd_key, sys_cb.wl_bd_key);
    TRACE("adv_data:");
    TRACE_R(param, 4);

    if(func_cb.sta == FUNC_BT) {
#if FUNC_BT_EN
        //在BT模式下,扫到无线麦的广播包,需要转入DEVICE进行连接
        if(header== WL_PAIR_HEADER1 && con_flag == WL_PAIR_MODE && bd_key != sys_cb.wl_bd_key && remote_bd_nb < WIRELESS_CON_LINK_NB) {
            msg_enqueue(MSG_FUNC_DEVICE);
        }
#endif
    } else if(sys_cb.wl_bond_nb > 0 && header == WL_PAIR_HEADER1 && con_flag == WL_PAIR_MODE && bd_key == sys_cb.wl_bd_key) {
        //如果已经存在绑定设备, key相同, 才能发起连接(避免已绑定的TX设备连到其他RX设备)
        wl_bonding_cb.new_key = bd_key;
        return true;
    } else if(sys_cb.wl_bond_nb == 0 && header == WL_PAIR_HEADER1 && con_flag == WL_PAIR_MODE && bd_key != sys_cb.wl_bd_key && remote_bd_nb < WIRELESS_CON_LINK_NB) {
        //这边已经清除配对,key不相同才能连接(避免TX清除配对后,立刻直接回连旧设备)
        //远端的绑定设备数量小于2(避免TX连上其他已经绑定2个设备的RX)
        wl_bonding_cb.new_key = bd_key;
        return true;
    }

    return false;
}

AT(.text.wireless.cb)
void wireless_device_bond_update(void)
{
    if(sys_cb.wl_bond_nb == 0 && wl_bonding_cb.new_key != sys_cb.wl_bd_key) {
        sys_cb.wl_bond_nb = 1;
        sys_cb.wl_bd_key = wl_bonding_cb.new_key;
        sys_cb.bond_mode = BONDING_MODE_WL;

        param_bond_nb_write();
        param_bond_key_write();
        param_bond_mode_write();
        TRACE("bond_update,key:0x%x\n", sys_cb.wl_bd_key);
    }
}

AT(.text.wireless.cb)
void wireless_device_pairing_enable(void)
{
    printf("wireless_device_pairing_enable\n");

    if(wireless_cb.connected_sta) {
        //让接收端主动断开
        wirless_adapter_disconnect();
    }

    wireless_bonding_clr();

#if WIRELESS_CON_AND_BT_PAIRING_EN
    load_code_wl_device();
    lock_code_wl_com();
    wireless_scan_set_param(128,56);
    //打开无线麦扫描
    wireless_scan_set_enable(1);
    sys_cb.wl_scan_flag = 1;
#endif
}

//--------------------------------------------------------------
//adapter端调用

//把配对信息放到广播包(进入配对模式),当device端扫描到配对信息, 则进入无线麦配对模式
AT(.text.func.adapter)
void wireless_adapter_pairing_enable(uint8_t en)
{
    u8 buf[5] = {0, 0, 0, 0, 0};

    if(en) {
        buf[0] = WL_PAIR_HEADER1;
        buf[1] = WL_PAIR_MODE;
        buf[2] = sys_cb.wl_bond_nb;
        buf[3] = sys_cb.wl_bd_key&0xFF;
        buf[4] = (sys_cb.wl_bd_key>>8)&0xFF;
    }

    wl_pscan_set_adv_ext_data(buf, sizeof(buf));
}

AT(.text.wireless.cb)
void wireless_adapter_bond_update(void)
{
    u8 bond_nb = 0;
    for(u8 i = 0; i < WIRELESS_CON_LINK_NB; i++) {
        if(wireless_cb.connected_sta&BIT(i)) {
            bond_nb++;
        }
    }

    TRACE("bond_update:%d %d\n", sys_cb.wl_bond_nb, bond_nb);

    if(sys_cb.wl_bond_nb < bond_nb) {
        sys_cb.wl_bond_nb = bond_nb;
        param_bond_nb_write();
    }

    if(sys_cb.bond_mode != BONDING_MODE_WL) {
        sys_cb.bond_mode = BONDING_MODE_WL;
        param_bond_mode_write();
    }
}

//--------------------------------------------------------------
//公共接口
AT(.text.wireless.init)
void wireless_bonding_init(void)
{
    param_bond_nb_read();
    param_bond_mode_read();

    if(sys_cb.wl_bond_nb == 0) {
        sys_cb.wl_bd_key = get_random(0xffff);
        param_bond_key_write();
    } else {
        param_bond_key_read();
    }

    TRACE("wl_bond_init:%d, bd_key:0x%x\n", sys_cb.wl_bond_nb, sys_cb.wl_bd_key);
}

void wireless_bonding_clr(void)
{
    sys_cb.wl_bond_nb = 0;
    sys_cb.bond_mode  = BONDING_MODE_NONE;;

    param_bond_nb_write();
    param_bond_mode_write();

    //device端先不要更新key,
    if(wireless_role_is_adapter()) {
        sys_cb.wl_bd_key  = get_random(0xffff);
        param_bond_key_write();
    } else {
#if FUNC_BT_EN
        TRACE("bt_delete_link_info\n");
        if(bt_is_connected()){
            bt_release(0xFF);
        }
        bt_delete_link_info();
        bt_scan_enable();
#endif
    }

    TRACE("wireless_bonding_clr:%x\n", sys_cb.wl_bd_key);
}
#endif

AT(.text.wireless.init)
void printf_connect_message(void)
{
    my_printf("<TX_INTERVAL>         %d\n",cfg_wireless_tx_interval);
    my_printf("<CON_INTERVAL>        %d\n",cfg_wireless_con_interval);
    my_printf("<WIRELESS_FEAT>       %d\n",cfg_wireless_feat);
    my_printf("<WIRELESS_CODEC>      %d\n",cfg_wireless_codec[0]);
    my_printf("<FREQ_BAND>           %d\n",cfg_bb_rf_freq_bands);
    my_printf("<RETRY>               %d\n",cfg_wireless_tx_retry);
    my_printf("<DISCON_AUTO_PWROFF>  %d\n",cfg_discon_auto_pwroff);
    my_printf("<CONFIG_RSSI>         %d\n",cfg_wireless_rssi_thr);
    my_printf("<WS_NAME>             %s\n",xcfg_cb.wl_name);
}
#if DEVICE_INTERPHONE_EN
AT(.text.wireless.init)
void wireless_wl_clk_check(void)
{
    if ((DEVICE_WL_CON_CLK < SYS_60M) || (DEVICE_WL_ALG_EN_CLK < SYS_60M)) {
        printf("wl_clk too small, please increase!!!!\n");
        while(1);
    }

    u16 frame_duration_us = WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_COMB_NB*1250;

    cfg_wireless_d2a_enc_us = sys_clk_time_conver(DEVICE_WL_ALG_EN_CLK, DEVICE_WL_CON_CLK, cfg_wireless_d2a_enc_us);
    ///双向下编码优先级更高，解码时长要加上编码时长
    cfg_wireless_d2a_dec_us = sys_clk_time_conver(DEVICE_WL_ALG_EN_CLK, DEVICE_WL_CON_CLK, cfg_wireless_d2a_dec_us) + cfg_wireless_d2a_enc_us;
    cfg_wireless_a2d_enc_us = cfg_wireless_d2a_enc_us;
    cfg_wireless_a2d_dec_us = cfg_wireless_d2a_dec_us;

    if (cfg_wireless_d2a_dec_us > (frame_duration_us - 300) || cfg_wireless_d2a_enc_us > (frame_duration_us - 300)) {
        printf("wl_clk too small, please increase!!!!\n");
        while(1);
    }
}
#else
AT(.text.wireless.init)
void wireless_wl_clk_check(void)
{
    if ((DEVICE_WL_CON_CLK < SYS_60M) || (DEVICE_WL_ALG_EN_CLK < SYS_60M) || (ADAPTER_WL_CON_CLK < SYS_60M) || (ADAPTER_WL_ALG_EN_CLK < SYS_60M)) {
        printf("wl_clk too small, please increase!!!!\n");
        while(1);
    }

    u16 frame_duration_us = WIRELESS_MIC_TX_INTERVAL/WIRELESS_MIC_COMB_NB*2500;

    if (wireless_role_is_adapter()) {
        cfg_wireless_d2a_dec_us = sys_clk_time_conver(ADAPTER_WL_ALG_EN_CLK, ADAPTER_WL_CON_CLK, cfg_wireless_d2a_dec_us);
    } else {
        cfg_wireless_d2a_enc_us = sys_clk_time_conver(DEVICE_WL_ALG_EN_CLK, DEVICE_WL_CON_CLK, cfg_wireless_d2a_enc_us);
    }

    if (cfg_wireless_d2a_dec_us > (frame_duration_us - 300) || cfg_wireless_d2a_enc_us > (frame_duration_us - 300)) {
        printf("wl_clk too small, please increase!!!!\n");
        while(1);
    }
}
#endif

AT(.text.wireless.init)
void wireless_init(void)
{
    wireless_var_init();
#if TBOX_TEST_EN
    if (vusb_test_rssi_get()) {
        cfg_wireless_rssi_thr = vusb_test_rssi_get();
    }
#endif
    wireless_dump_init();

    wireless_con_adapter_init();
    wireless_con_device_init();

#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
    wireless_bonding_init();
#endif

    //print the connection base parameters
    printf_connect_message();

    wireless_wl_clk_check();
#if WIRELESS_MIC_2TNR_EN
    wireless_2tnr_info_init();
    cfg_wireless_ext_feat |= ((WIRELESS_CON_2TNR_NB << 4)&EXT_FEAT_2TNR_NB);
#endif
//    bt_setup();
}

AT(.text.wireless.adapter)
void wireless_adapter_init(void)
{
#if WIRELESS_MIC_EN
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_dec_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
    lc3f_dec_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT);
#endif

#if WIRELESS_MIC_SINGLE_PLC_EN
#if WIRELESS_MIC_24B_PCM_EN
    plc_soft_init(0, WIRELESS_MIC_SAMPLES_SELECT);
    plc_soft_init(1, WIRELESS_MIC_SAMPLES_SELECT);
#else
    plc_soft_v2_exit(0);
    plc_soft_v2_init(0, WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, 0);
    plc_soft_v2_exit(1);
    plc_soft_v2_init(1, WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, 0);
#endif
#endif
#endif // WIRELESS_MIC_EN


#if WIRELESS_SPK_EN && !DEVICE_INTERPHONE_EN
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_enc_init(WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
    lc3f_enc_init(WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT);
#endif

    spk_start();
#endif // WIRELESS_SPK_EN


#if ADAPTER_MIX_DRC_EN
    mix_drc_init();
#endif

#if WIRELESS_CON_PWR_CTR
    ws_pwr_ctr_init();
#endif

#if I2S_EN && I2S_DMA_EN
    i2s_dma_start();
#endif // I2S_EN
}

AT(.text.wireless.adapter)
void wireless_adapter_exit(u8 idx, u8 con_sta)
{
#if WIRELESS_MIC_EN
    mic_dec_reset(idx);

#if WIRELESS_MIC_SINGLE_PLC_EN
#if WIRELESS_MIC_24B_PCM_EN
    plc_soft_exit(idx);
    plc_soft_init(idx, WIRELESS_MIC_SAMPLES_SELECT);
#else
    plc_soft_v2_exit(idx);
    plc_soft_v2_init(idx, WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, 0);
#endif
#endif
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_dec_exit(idx);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
    lc3f_dec_exit(idx);
#endif
#endif // WIRELESS_MIC_EN


#if WIRELESS_SPK_EN && !DEVICE_INTERPHONE_EN
    spk_stop();
    spk_enc_reset();

#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_enc_exit();
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
    lc3f_enc_exit();
#endif
#endif // WIRELESS_SPK_EN


    wireless_cmd_reset(idx);

#if WIRELESS_CON_PWR_CTR
    ws_pwr_ctr_reset(idx);
#endif

    if(con_sta == 0) {

#if I2S_EN && I2S_DMA_EN
        i2s_dma_stop();
#endif // I2S_EN
#if WIRELESS_MIC_2TNR_EN
        wireless_2tnr_info_clr();
#endif
    }
}

AT(.text.wireless.device)
void wireless_device_init(void)
{
#if WIRELESS_MIC_EN
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_enc_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
    lc3f_enc_init(WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT);
#endif

    mic_start();
#endif // WIRELESS_MIC_EN


#if WIRELESS_SPK_EN && !DEVICE_INTERPHONE_EN
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_dec_init(WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
    lc3f_dec_init(WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT);
#endif

#if WIRELESS_SPK_SINGLE_PLC_EN
#if WIRELESS_SPK_24B_PCM_EN
    plc_soft_init(0, WIRELESS_SPK_SAMPLES_SELECT);
    plc_soft_init(1, WIRELESS_SPK_SAMPLES_SELECT);
#else
    plc_soft_v2_exit(0);
    plc_soft_v2_init(0, WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT, 0);
    plc_soft_v2_exit(1);
    plc_soft_v2_init(1, WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT, 0);
#endif
#endif
#endif // WIRELESS_SPK_EN


#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
    wireless_device_bond_update();
#endif

#if DEVICE_MIC_FADE_IN_EN
    soft_gain_init();
#endif
#if YLCRN_L3_EN || YLCRN_L2_EN || AINS4_EN || AINS5_EN || AGC_EN
    denoise_mute_set(0, DENOISE_ALL);
#endif
#if WIRELESS_MIC_2TNR_EN && WIRELESS_CON_PWR_CTR
    ws_pwr_ctr_2tnr_init();
#endif
}

AT(.text.wireless.device)
void wireless_device_exit(void)
{
#if WIRELESS_MIC_EN
    mic_stop();
    mic_enc_reset();

#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_enc_exit();
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
    lc3f_enc_exit();
#endif
#endif // WIRELESS_MIC_EN


#if WIRELESS_SPK_EN && !DEVICE_INTERPHONE_EN
    spk_dec_reset();

#if WIRELESS_SPK_SINGLE_PLC_EN
#if WIRELESS_SPK_24B_PCM_EN
    plc_soft_exit(0);
    plc_soft_init(0, WIRELESS_SPK_SAMPLES_SELECT);
#else
    plc_soft_v2_exit(0);
    plc_soft_v2_init(0, WIRELESS_SPK_SAMPLE_RATE_SELECT, WIRELESS_SPK_SAMPLES_SELECT, 0);
#endif
#endif
#if (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3S)
    lc3s_dec_exit(0);
#elif (WIRELESS_CON_CODEC_SEL == WS_CODEC_LC3F)
    lc3f_dec_exit(0);
#endif
#endif // WIRELESS_SPK_EN


    wireless_cmd_reset(0);

#if YLCRN_L3_EN || YLCRN_L2_EN || AINS4_EN || AINS5_EN || AGC_EN
    denoise_mute_set(1, DENOISE_ALL);
#endif
}

#if DEVICE_INTERPHONE_EN
void wireless_interphone_init(void)
{
    wireless_adapter_init();
    wireless_device_init();
}

void wireless_interphone_exit(u8 idx, u8 con_sta)
{
    wireless_adapter_exit(idx, con_sta);
    wireless_device_exit();
}
#endif

AT(.text.wireless)
void wireless_enter_sleep(void)
{
    if (wireless_role_is_adapter() && (!wireless_con_get_status())) {
        wireless_adv_set_interval(WL_ADV_SLEEP_INTERVAL);
    }
}

AT(.text.wireless)
void wireless_exit_sleep(void)
{
    if (wireless_role_is_adapter()) {
        wireless_adv_set_interval(WL_ADV_INTERVAL);
    }
}

bool wireless_discon_all(void)
{
    ///设一下ble_con优先级比较高，audio优先级比较低
    uint8_t con_sta = wireless_cb.connected_sta;
    u8 max_mic_nb = WIRELESS_CON_LINK_NB;
#if WIRELESS_MIC_2TNR_EN
    if (!wireless_role_is_adapter()) {
        max_mic_nb = WIRELESS_CON_2TNR_NB;
    }
#endif
    for (u8 i = 0; i<max_mic_nb; i++) {
        if (con_sta & BIT(i)) {
            wl_con_change_high_prio(i);
            wl_con_audio_stop(i);
            wireless_disconnect_req(i);
        }
    }

    return (bool)(con_sta);
}

void wireless_status_change(void)
{
    wireless_cb.change_flag = 1;
}

#else
uint16_t cfg_wireless_feat          = 0;
#endif // WIRELESS_EN
