#include "include.h"
#include "func.h"
#include "func_adapter.h"
//#include "bsp_usbfot.h"

#define TRACE_EN                1

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

#if FUNC_ADAPTER_EN

enum {
    ADAPTER_STA_INIT_IDLE,
    ADAPTER_STA_INIT_W4_CONNECT,
    ADAPTER_STA_START_ACTION,
    ADAPTER_STA_IDLE,

    ADAPTER_STA_INIT_CONNECT,
    ADAPTER_STA_DELAY,
    ADAPTER_STA_SCAN,
};

enum {
    PARAM_STA_GET_VERS,
    PARAM_STA_GET_SN,
    PARAM_STA_GET_MAC,
    PARAM_STA_GET_BAT,
    PARAM_STA_ESTABLISH,

    PARAM_STA_W4_SN         = 0x80 | PARAM_STA_GET_VERS,
    PARAM_STA_W4_MAC        = 0x80 | PARAM_STA_GET_SN,
    PARAM_STA_W4_BAT        = 0x80 | PARAM_STA_GET_MAC,
    PARAM_STA_W4_VERS       = 0x80 | PARAM_STA_GET_BAT,
};

static struct {
#if WIRELESS_MIC_CMD_EN
    struct device_info_tag {
        u16 fw_ver;                 //固件版本号
        u8 fw_sn[16];               //序列号
        u8 mac_addr[6];             //设备地址
        u8 nr_en;                   //降噪开关
        u8 nr_level;                //降噪强度
#if ECHO_EN
        u8 echo_en;                 //混响开关
        u8 echo_level;              //混响等级
#endif
        u8 bat_level;               //电池电量

        u8 param_sta;               //参数同步状态
        u8 update_flag;             //参数更新标志
    } device[WIRELESS_CON_LINK_NB];
#endif

    u8 nr_en;                   //降噪开关
    u8 nr_level;                //降噪强度
#if ECHO_EN
    u8 echo_en;                 //混响开关
    u8 echo_level;              //混响等级
#endif

    u8 disp_sta;
    u8 init_state;
    u32 ticks;
    uint tick_delay;
    bool usbdev_insert;
    bool create_con_flag;
} adapter_cb;

#if WIRELESS_MIC_EN
//无线MIC适配器处理流程
//ADAPTER_RX -> wireless_d2a_set_rxpkt_cb -> decoder_prio_trans_audio_input -> mic_dec_audio_input --> mic_mix_audio_input-> dac0_out_audio_input -> DAC
//                                                                                                 |-> dac0_out_audio_input -> DAC
static wl_link_list_t mic_rx_cfg_tbl[] = {
/*  模块类型，                  使能，          初始化，                    输入接口，                  设置输出*/
    {DECODER_PRIO_TRANS_TYPE,   1,              decoder_prio_trans_init,    NULL,                       decoder_prio_trans_audio_output_callback_set},
    {MIC_DEC_TYPE,              1,              mic_dec_init,               mic_dec_audio_input,        mic_dec_audio_output_callback_set},
#if ADAPTER_LOCAL_MIC_MIX_EN
    {MIC_MIX_TYPE,              1,              mic_mix_init,               mic_mix_audio_input,        mic_mix_output_callback_set},
#endif
#if ADAPTER_YLCRN_L3_EN
    {YLCRN_L3_TYPE,             1,              ylcrn_L3_mic_init,          ylcrn_L3_mic_audio_input,   ylcrn_L3_mic_output_callback_set},
#endif
#if ADAPTER_YLCRN_L2_EN
    {YLCRN_L2_TYPE,             1,              ylcrn_L2_mic_init,          ylcrn_L2_mic_audio_input,   ylcrn_L2_mic_output_callback_set},
#endif
#if ADAPTER_ECHO_EN
    {ECHO_TYPE,                 1,              echo_audio_init,            echo_audio_input,           echo_audio_output_callback_set},
#endif
#if ADAPTER_I2S_OUTPUT_EN
    {I2S_OUT_TYPE,              1,              i2s_audio_out_init,         i2s_audio_out_input,        i2s_audio_output_callback_set},
#endif
#if ADAPTER_I2S_IN_OUT_EN
    {I2S_OUT_TYPE,              1,              i2s_audio_in_out_init,      i2s_audio_in_out_input,     i2s_audio_in_out_output_callback_set},
#endif

#if ADAPTER_HUART_OUTPUT_EN
    {HUART_TX_TYPE,             1,              huart_audio_out_init,       huart_audio_out_input,      huart_audio_out_output_callback_set},
#endif
#if ADAPTER_HUART_INPUT_EN
    {HUART_RX_TYPE,             1,              huart_audio_in_init,        huart_audio_in_input,       huart_audio_in_output_callback_set},
#endif

#if ADAPTER_DAC_OUTPUT_EN
    {DAC_OUT_TYPE,              1,              dac0_out_init,              dac0_out_audio_input,       dac0_out_audio_output_callback_set},
#endif
#if ADAPTER_USB_MIC_EQ_DRC_EN
    {USBMIC_EQ_DRC_TYPE,        1,              usb_mic_eq_drc_init,        usb_mic_eq_drc_audio_input, usb_mic_eq_drc_audio_output_callback_set},
#endif
#if ADAPTER_USB_MIC_RX_EN
    {USB_MIC_IN_TYPE,           1,              usb_mic_in_init,            usb_mic_in_audio_input,     usb_mic_in_audio_output_callback_set},
#endif
};
#endif

#if WIRELESS_SPK_EN && !DEVICE_INTERPHONE_EN
#if ADAPTER_USB_SPK_TX_EN
static wl_link_list_t spk_tx_cfg_tbl[] = {//测试usb speaker下行音频#if FUNC_TEST_PRINT//调试打印
/*  模块类型，                  使能，            初始化，                  输入接口，                      设置输出*/
    {USB_SPK_OUT_TYPE,          1,              usb_audio_out_init,         NULL,                           usb_audio_out_audio_output_callback_set},
    {ENCODER_PRIO_TRANS_TYPE,   1,              encoder_prio_trans_init,    encoder_prio_trans_audio_input, encoder_prio_trans_audio_output_callback_set},   //缓存到编码缓冲区，并转到低优先级处理
    {DAC_OUT_TYPE,              1,              dac1_out_init,              usb_speaker_dac1_out_audio_input,       dac1_out_audio_output_callback_set},
};
#else
static wl_link_list_t spk_tx_cfg_tbl[] = {
/*  模块类型，                  使能，          初始化，                    输入接口，                      设置输出*/
    {SPK_TYPE,                  1,              spk_init,                   NULL,                           spk_audio_output_callback_set},
    {ENCODER_PRIO_TRANS_TYPE,   1,              encoder_prio_trans_init,    encoder_prio_trans_audio_input, encoder_prio_trans_audio_output_callback_set},    //缓存到编码缓冲区，并转到低优先级处理
    {SPK_ENC_TYPE,              1,              spk_enc_init,               spk_enc_audio_input,            spk_enc_audio_output_callback_set},
};
#endif
#endif

AT(.text.func.adapter)
void func_adapter_init(void)
{
    memset(&adapter_cb, 0x00, sizeof(adapter_cb));
    adapter_cb.disp_sta = 0xff;
    adapter_cb.init_state = ADAPTER_STA_INIT_IDLE;

    load_code_wl_adapter();
    lock_code_wl_com();

#if WIRELESS_MIC_EN
    wl_link_list_init(mic_rx_cfg_tbl, (sizeof(mic_rx_cfg_tbl)/sizeof(wl_link_list_t)), WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_CHANNEL_SELECT);
#endif

#if WIRELESS_SPK_EN && !DEVICE_INTERPHONE_EN
//    //测试usb speaker下行音频#if FUNC_TEST_PRINT//调试打印
    wl_link_list_init(spk_tx_cfg_tbl, (sizeof(spk_tx_cfg_tbl)/sizeof(wl_link_list_t)), WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_CHANNEL_SELECT);
#endif
}


#if WIRELESS_MIC_CMD_EN
static void adapter_send_set_nr_cmd(u8 idx)
{
    u8 buf[2];
    buf[0] = adapter_cb.device[idx].nr_en = adapter_cb.nr_en;
    buf[1] = adapter_cb.device[idx].nr_level = adapter_cb.nr_level;
    wireless_tx_mic_cmd(idx, ADAPTER_SET_NR_STA, buf, 2);
}

static void func_adapter_cmd_tx(u8 idx)
{
    adapter_cb.device[idx].param_sta = PARAM_STA_ESTABLISH;  //for test

    switch(adapter_cb.device[idx].param_sta) {
//    case PARAM_STA_GET_VERS:
//        wireless_tx_mic_cmd0(idx, ADAPTER_GET_VERS, 0);
//        adapter_cb.device[idx].param_sta = PARAM_STA_W4_VERS;
//        break;
//
//    case PARAM_STA_GET_SN:
//        wireless_tx_mic_cmd0(idx, ADAPTER_GET_SN, 0);
//        adapter_cb.device[idx].param_sta = PARAM_STA_W4_SN;
//        break;
//
//    case PARAM_STA_GET_MAC:
//        wireless_tx_mic_cmd0(idx, ADAPTER_GET_MAC, 0);
//        adapter_cb.device[idx].param_sta = PARAM_STA_W4_MAC;
//        break;
//
//    case PARAM_STA_GET_BAT:
//        wireless_tx_mic_cmd0(idx, ADAPTER_GET_BAT, 0);
//        adapter_cb.device[idx].param_sta = PARAM_STA_W4_BAT;
//        break;
//
    case PARAM_STA_ESTABLISH:
        adapter_cb.device[idx].update_flag = false;
        adapter_send_set_nr_cmd(idx);
        break;
    }
}

void func_adapter_cmd_rx_cb(u8 idx, u8 opcode, u8 *buf)
{
}

AT(.text.func.process.adapter)
static void func_adapter_cmd_process(void)
{
    for(uint i=0; i<WIRELESS_CON_LINK_NB; i++) {
        if(wireless_cb.connected_sta & BIT(i)) {
            if(!wireless_cmd_is_full(i)
               && (adapter_cb.device[i].param_sta < PARAM_STA_ESTABLISH || adapter_cb.device[i].update_flag != 0)) {

                func_adapter_cmd_tx(i);
            }
        } else {
            memset(&adapter_cb.device[i], 0x00, sizeof(struct device_info_tag));
        }
    }
}
#endif

void func_adapter_set_nr_sta(void)
{
    adapter_cb.nr_en = !adapter_cb.nr_en;
    adapter_cb.disp_sta = 0xff;
#if WIRELESS_MIC_CMD_EN
    for(uint i=0; i<WIRELESS_CON_LINK_NB; i++) {
        if(wireless_cb.connected_sta & BIT(i)) {
            adapter_cb.device[i].update_flag = true;
        }
    }
#endif
}

#if WIRELESS_MIC_2TNR_EN
AT(.text.func.adapter)
static void func_adapter_process_do(void)
{
    u8 addr[6];

    if(wireless_cb.change_flag) {
        wireless_cb.change_flag = 0;

        if (adapter_cb.create_con_flag) {
            TRACE("adapter, create_con\n");
            adapter_cb.create_con_flag = 0;
            wireless_scan_set_enable(0);
            wireless_connect_req(500);
            adapter_cb.init_state  = ADAPTER_STA_INIT_CONNECT;
        } else {
            switch(wireless_cb.change_sta) {
            case 0:     //connect success
                adapter_cb.init_state = ADAPTER_STA_START_ACTION;
                break;

            case 1:     //connect fail
                if(!wireless_mic_is_bonding()) {
                    adapter_cb.ticks      = tick_get();
                    adapter_cb.tick_delay = 100+get_random(0xffff)%500;
                    adapter_cb.init_state = ADAPTER_STA_DELAY;
                    break;
                }
                //no break

            case 2:     //disconnect
                adapter_cb.ticks      = tick_get();
                adapter_cb.tick_delay = 400+get_random(0xffff)%500;
                adapter_cb.init_state = ADAPTER_STA_DELAY;
                break;
            }
        }
    }

    switch(adapter_cb.init_state) {
    case ADAPTER_STA_INIT_IDLE:
        TRACE("adapter, init(%d)\n", wireless_mic_is_bonding());
        if(wireless_get_link_info_addr(addr)) {
            //有回连信息，开始回连
            TRACE("adapter, con_req: ");
            TRACE_R(addr, 6);
            wireless_create_con_for_addr(addr, 1500);
            adapter_cb.init_state  = ADAPTER_STA_INIT_CONNECT;
        } else {
            //没有回连信息，搜索组队
            TRACE("adapter, scan_en\n");
            wireless_scan_set_enable(1);
            adapter_cb.init_state  = ADAPTER_STA_SCAN;
            adapter_cb.ticks       = tick_get();
        }
        break;

    case ADAPTER_STA_INIT_CONNECT:
        break;

    case ADAPTER_STA_START_ACTION:
        TRACE("adapter, con_sta(%d): %x\n", wireless_mic_is_bonding(), wireless_cb.connected_sta);

        if(wireless_cb.connected_sta == WIRELESS_CON_STA_MASK) {
            //已连接，关闭扫描
            TRACE("adapter, con_complete\n");
            adapter_cb.init_state = ADAPTER_STA_IDLE;
        } else if(wireless_mic_is_bonding() && wireless_get_link_info_addr(addr)) {
            //有回连信息，开始回连
            TRACE("adapter, con_req: ");
            TRACE_R(addr, 6);
            wireless_create_con_for_addr(addr, 1500);
            adapter_cb.init_state  = ADAPTER_STA_INIT_CONNECT;
        } else {
            //没有回连信息，搜索组队
            TRACE("adapter, scan_en\n");
            wireless_scan_set_enable(1);
            adapter_cb.init_state = ADAPTER_STA_SCAN;
            adapter_cb.ticks      = tick_get();
        }
        break;

    case ADAPTER_STA_SCAN:
//        if(tick_check_expire(adapter_cb.ticks, 400)) {     //省电时可以间歇性扫描
//            wireless_scan_set_enable(0);
//            adapter_cb.ticks      = tick_get();
//            adapter_cb.tick_delay = 500+get_random(0xffff)%200;
//            adapter_cb.init_state = ADAPTER_STA_DELAY;
//        }
        break;

    case ADAPTER_STA_DELAY:
        if(tick_check_expire(adapter_cb.ticks, adapter_cb.tick_delay)) {
            adapter_cb.init_state = ADAPTER_STA_START_ACTION;
        }
        break;
    }
}
#else
AT(.text.func.adapter)
static void func_adapter_process_do(void)
{
    u8 link_nb;

    if(wireless_cb.change_flag) {
        wireless_cb.change_flag = 0;
        adapter_cb.init_state = ADAPTER_STA_START_ACTION;
#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
        wireless_adapter_bond_update();
#endif
    }

    switch(adapter_cb.init_state) {
    case ADAPTER_STA_INIT_IDLE:
        link_nb = bt_get_link_info_nb();
        TRACE("adapter, init(%d): %x, %d\n", wireless_mic_is_bonding(), wireless_cb.connected_sta, link_nb);

        if(link_nb != 0) {
            //有回连信息，等待被连接（关闭可被发现）
            wireless_adv_set_enable(1, 0);
            adapter_cb.init_state  = ADAPTER_STA_INIT_W4_CONNECT;
            adapter_cb.ticks       = tick_get();
        } else {
#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
            if(sys_cb.wl_bond_nb > 0) {
                wireless_adapter_pairing_enable(1);
            }
#endif
            //没有回连信息，等待被发现和连接
            wireless_adv_set_enable(1, 1);
            adapter_cb.init_state  = ADAPTER_STA_IDLE;
        }
        break;

    case ADAPTER_STA_INIT_W4_CONNECT:
        if(tick_check_expire(adapter_cb.ticks, 2000)) {
            adapter_cb.init_state = ADAPTER_STA_START_ACTION;
        }
        break;

    case ADAPTER_STA_START_ACTION:
        link_nb = bt_get_link_info_nb();
        TRACE("adapter, con_sta(%d): %x, %d\n", wireless_mic_is_bonding(), wireless_cb.connected_sta, link_nb);

        if(wireless_cb.connected_sta == WIRELESS_CON_STA_MASK) {
            //两个都连上了，关闭可被发现和可被连接
            wireless_adv_set_enable(0, 0);
        } else if(wireless_mic_is_bonding() && link_nb >= WIRELESS_CON_LINK_NB) {
            //组队绑定时，切配对过两个，等待被连接（关闭可被发现）
            wireless_adv_set_enable(1, 0);
        } else {
#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
            if(sys_cb.wl_bond_nb > 0) {
                wireless_adapter_pairing_enable(1);
            }
#endif
            //等待被发现和连接
            wireless_adv_set_enable(1, 1);
        }
        adapter_cb.init_state = ADAPTER_STA_IDLE;
        break;
    }
}
#endif

AT(.text.func.process.wireless)
void func_adapter_sta_proc(void)
{
    if(wireless_cb.connected_sta != adapter_cb.disp_sta) {
        adapter_cb.disp_sta = wireless_cb.connected_sta;

        if(adapter_cb.disp_sta != 0) {
//            if(adapter_cb.disp_sta & BIT(0)) {
//                led_bt0_connected(); //mic0
//            }
//            if(adapter_cb.disp_sta & BIT(1)) {
//                led_bt1_connected(); //mic1
//            }
            if(adapter_cb.nr_en && NR_ALG_EN) {
                led_bt_play(); //mic0&1
            } else {
                led_bt_connected(); //mic0&1
            }
        } else {
            led_bt_idle();
        }
    }

    wireless_dump_proc();
}

AT(.text.func.process.adapter)
void func_adapter_process(void)
{
#if TRACE_EN
    static u8 sta = 0xff;
    if(sta != adapter_cb.init_state) {
        sta = adapter_cb.init_state;
        TRACE("adapter, state: %d\n", sta);
    }
#endif
#if WIRELESS_MIC_2TNR_EN
    if(((adapter_cb.init_state != ADAPTER_STA_IDLE) && (adapter_cb.init_state != ADAPTER_STA_INIT_CONNECT)) || wireless_cb.change_flag) {
#else
    if(adapter_cb.init_state != ADAPTER_STA_IDLE || wireless_cb.change_flag) {
#endif
        func_adapter_process_do();
    }
#if WIRELESS_MIC_CMD_EN
    func_adapter_cmd_process();
#endif



    func_adapter_sta_proc();
    func_process();
#if ADAPTER_USB_UPDATE_EN
	usb_dev_update_process();
#endif
}

AT(.text.func.adapter)
static void func_adapter_enter(void)
{
    func_adapter_init();
    led_bt_idle();
    func_bt_init();

    wireless_adv_set_interval(WIRELESS_MIC_TX_INTERVAL*16);
#if WIRELESS_MIC_2TNR_EN
    wireless_scan_set_param(64,32);
    wireless_set_con_flag(CON_FLAG_T2R, CON_FLAG_T2R);
#endif
#if ADAPTER_USB_MIC_RX_EN
    if(dev_is_online(DEV_USBPC)) {
        func_usbdev_insert(FUNC_ADAPTER);
    }
#endif

#if WIRELESS_CON_AND_BT_PAIRING_EN || WIRELESS_CON_BONDING_EN
    if(sys_cb.wl_bond_nb > 0) {
        wireless_adapter_pairing_enable(1);
    }
#endif
}

AT(.text.func.adapter)
static void func_adapter_exit(void)
{
//    le_mic_discon_all();
    bt_off();
    sys_cb.bt_is_inited = 0;
    func_cb.last = FUNC_ADAPTER;

#if ADAPTER_USB_MIC_RX_EN
    if(dev_is_online(DEV_USBPC)) {
        func_usbdev_remove(FUNC_ADAPTER);
    }
#endif
}

AT(.text.func.adapter)
void func_adapter_set_create_con(void)
{
    adapter_cb.create_con_flag = 1;
    wireless_cb.change_flag = 1;
}

AT(.text.func.adapter)
void func_adapter(void)
{
    printf("%s\n", __func__);

    func_adapter_enter();

    while (func_cb.sta == FUNC_ADAPTER) {
        func_adapter_process();
        func_adapter_message(msg_dequeue());
        //func_adapter_display();
    }

    func_adapter_exit();
}

#endif // FUNC_ADAPTER_EN
