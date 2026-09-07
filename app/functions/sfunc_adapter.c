#include "include.h"
#include "func.h"
#include "sfunc_adapter.h"

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#define TRACE_R(...)            print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif // TRACE_EN

#if FUNC_ADAPTER_EN

enum {
    ADAPTER_STA_OFF,
    ADAPTER_STA_W4_REINIT,
    ADAPTER_STA_W4_DISCON,
    ADAPTER_STA_INIT_IDLE,
    ADAPTER_STA_INIT_W4_CONNECT,
    ADAPTER_STA_START_ACTION,
    ADAPTER_STA_IDLE,
};

static struct {
    bool init_flag;
    bool off_flag;
    bool disconnect_flag;
    uint8_t state;
    uint32_t ticks;
} adapter_cb;

//无线MIC适配器处理流程
//ADAPTER_RX -> wireless_d2a_set_rxpkt_cb -> decoder_prio_trans_audio_input -> mic_dec_audio_input --> mic_mix_audio_input-> dac0_out_audio_input -> DAC
//                                                                                                 |-> dac0_out_audio_input -> DAC
static wl_link_list_t mic_rx_cfg_tbl[] = {
/*  模块类型，                  使能，          初始化，                    输入接口，                  设置输出*/
    {DECODER_PRIO_TRANS_TYPE,   1,              decoder_prio_trans_init,    NULL,                       decoder_prio_trans_audio_output_callback_set},
    {MIC_DEC_TYPE,              1,              mic_dec_init,               mic_dec_audio_input,        mic_dec_audio_output_callback_set},

#if (I2S_EN)
    {ALG_PRIO_TRANS_TYPE,       1,              alg_prio_trans_init,        alg_prio_trans_audio_input, alg_prio_trans_audio_output_callback_set},
#endif

#if ADAPTER_HOWLING_EN
    {HOWLING_AIMASK_TYPE,       1,              howling_mic_init,           howling_mic_audio_input,    howling_mic_output_callback_set},
#endif
#if ADAPTER_LOCAL_MIC_MIX_EN
    {MIC_MIX_TYPE,              1,              karaok_wireless_mic_mix_init, karaok_wireless_mic_mix_audio_input, karaok_wireless_mic_mix_output_callback_set},
#endif
#if ADAPTER_ECHO_EN
    {ECHO_TYPE,                 1,              echo_audio_init,           echo_audio_input,                echo_audio_output_callback_set},
#endif

#if ADAPTER_MAGIC_EN
    {MAGCI_PITCH_SHIFT_TYPE,    1,              magic_audio_init,          magic_audio_input,               magic_audio_output_callback_set},
#endif

#if I2S_AUDIO_OUT_EN
    {I2S_OUT_TYPE,              1,              i2s_audio_out_init,        i2s_audio_out_input,         i2s_audio_output_callback_set},               //I2S传输
#endif

#if I2S_AUDIO_IN_EN
    {I2S_IN_TYPE,              1,              i2s_audio_in_init,        i2s_audio_in_input,         i2s_audio_input_callback_set},               //I2S传输
#endif // I2S_AUDIO_IN_EN

#if I2S_AUDIO_IN_AND_OUT_EN
    {I2S_IN_AND_OUT_TYPE,      1,              i2s_audio_in_and_out_init, i2s_audio_in_and_out_input,   i2s_audio_in_and_output_callback_set},               //I2S传输
#endif // I2S_AUDIO_IN_AND_OUT_EN

#if ADAPTER_DAC_OUTPUT_EN
    {DAC_OUT_TYPE,              1,              dac1_out_init,              dac1_out_audio_input,       dac1_out_audio_output_callback_set},
#endif
};

AT(.text.func.adapter)
static void sfunc_adapter_process_do(void)
{
    u8 link_nb;

    if(wireless_cb.change_flag) {
        wireless_cb.change_flag = 0;
        if(adapter_cb.state > ADAPTER_STA_W4_DISCON) {
            adapter_cb.state = ADAPTER_STA_START_ACTION;
        }
    }

    switch(adapter_cb.state) {
    case ADAPTER_STA_INIT_IDLE:
        link_nb = bt_get_link_info_nb();
        TRACE("adapter, init(%d): %x, %d\n", wireless_mic_is_bonding(), wireless_cb.connected_sta, link_nb);

        if(link_nb != 0) {
            //有回连信息，等待被连接（关闭可被发现）
            wireless_adv_set_enable(1, 0);
            adapter_cb.state  = ADAPTER_STA_INIT_W4_CONNECT;
            adapter_cb.ticks       = tick_get();
        } else {
            //没有回连信息，等待被发现和连接
            wireless_adv_set_enable(1, 1);
            adapter_cb.state  = ADAPTER_STA_IDLE;
        }
        break;

    case ADAPTER_STA_INIT_W4_CONNECT:
        if(tick_check_expire(adapter_cb.ticks, 2000)) {
            adapter_cb.state = ADAPTER_STA_START_ACTION;
        }
        break;

    case ADAPTER_STA_START_ACTION:
        link_nb = bt_get_link_info_nb();
        TRACE("adapter, con_sta(%d): %x, %d\n", wireless_mic_is_bonding(), wireless_cb.connected_sta, link_nb);

        if(adapter_cb.off_flag || adapter_cb.disconnect_flag || bt_sco_get_status() != SCO_STATUS_IDLE) {
            //关闭可被发现和可被连接
            wireless_adv_set_enable(0, 0);
            //断开所有连接
            if(!wireless_discon_all()) {
                wireless_cb.change_flag = 1;
            }
            adapter_cb.state = ADAPTER_STA_W4_DISCON;
            adapter_cb.ticks = tick_get();
        } else {
            if(wireless_cb.connected_sta == WIRELESS_CON_STA_MASK) {
                //两个都连上了，关闭可被发现和可被连接
                wireless_adv_set_enable(0, 0);
            } else if(wireless_mic_is_bonding() && link_nb >= WIRELESS_CON_LINK_NB) {
                //组队绑定时，切配对过两个，等待被连接（关闭可被发现）
                wireless_adv_set_enable(1, 0);
            } else {
                //等待被发现和连接
                wireless_adv_set_enable(1, 1);
            }
            adapter_cb.state = ADAPTER_STA_IDLE;
        }
        break;

    case ADAPTER_STA_W4_DISCON:
        TRACE("adapter, w4_dis(%d): %x, %d\n", wireless_mic_is_bonding(), wireless_cb.connected_sta, wireless_pscan_adv_is_close());
        {
            uint16_t timeout = 50;
            while(!wireless_pscan_adv_is_close() && timeout) {
                bt_thread_check_trigger();
                delay_5ms(1);
                timeout--;
            }

            if(wireless_cb.connected_sta != 0) {    //等待过程中又有无线麦连上了
                wireless_discon_all();
                break;
            }
            if (adapter_cb.off_flag == 0 && adapter_cb.disconnect_flag) {
                adapter_cb.disconnect_flag = 0;
                adapter_cb.state = ADAPTER_STA_START_ACTION;
                break;
            }

            unlock_code_wl_com();
            adapter_cb.init_flag = 0;
            adapter_cb.state = adapter_cb.off_flag? ADAPTER_STA_OFF : ADAPTER_STA_W4_REINIT;
            wireless_off();
        }
        break;

    case ADAPTER_STA_W4_REINIT:
        if(adapter_cb.off_flag) {
            adapter_cb.state = ADAPTER_STA_OFF;
        } else {
            if(bt_sco_get_status() == SCO_STATUS_IDLE) {
                sfunc_adapter_init();
            }
        }
        break;
    }
}

AT(.text.func.process.adapter)
void sfunc_adapter_process(void)
{
#if TRACE_EN
    static u8 sta = 0xff;
    if(sta != adapter_cb.state || wireless_cb.change_flag) {
        sta = adapter_cb.state;
        TRACE("adapter, state: %d\n", sta);
    }
#endif
    if(adapter_cb.state == ADAPTER_STA_W4_DISCON && tick_check_expire(adapter_cb.ticks, 2000)) {
        wireless_cb.change_flag = true;
    }
    if((adapter_cb.state > ADAPTER_STA_W4_DISCON && adapter_cb.state < ADAPTER_STA_IDLE) || wireless_cb.change_flag) {
        sfunc_adapter_process_do();
    }
}

AT(.com_text.func.adapter)
void sfunc_adapter_set_disconnect(void)
{
    adapter_cb.disconnect_flag = 1;
    wireless_cb.change_flag = 1;
}

AT(.text.func.adapter)
void sfunc_adapter_init(void)
{
    if (!adapter_cb.init_flag) {

        load_code_wl_adapter();
        lock_code_wl_com();

        adapter_cb.init_flag = 1;
        adapter_cb.off_flag = false;
        wl_link_list_init(mic_rx_cfg_tbl, (sizeof(mic_rx_cfg_tbl)/sizeof(wl_link_list_t)), WIRELESS_MIC_SAMPLE_RATE_SELECT, WIRELESS_MIC_SAMPLES_SELECT, WIRELESS_MIC_CHANNEL_SELECT);

        adapter_cb.state = ADAPTER_STA_INIT_IDLE;

        wireless_setup();

    }
}

AT(.text.func.adapter)
void sfunc_adapter_exit(void)
{
    uint16_t timeout = 300;     //1.5s

    wireless_audio_bypass();

    adapter_cb.off_flag = true;
    wireless_cb.change_flag = 1;

    while (adapter_cb.state != ADAPTER_STA_OFF && timeout) {
        func_process();
        delay_5ms(1);
        timeout--;
    }

    if(timeout == 0) {
        TRACE("adapter_exit: tout\n");
    }

    wireless_audio_enable();
}

#endif
