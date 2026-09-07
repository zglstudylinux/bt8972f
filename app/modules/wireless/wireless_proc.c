#include "include.h"
#include "wireless.h"

#if WIRELESS_EN

struct wireless_cb_tag wireless_cb;

#define wireless_set_alg_en(en)     wireless_cb.alg_en = en
#define wireless_clr_alg_en()       wireless_cb.alg_en = 0

AT(.com_text.wireless)
u8 wireless_get_status(void)
{
    return (wireless_cb.connected_sta);
}

AT(.text.wireless.cb)
void wireless_emit_notice(uint evt, void *params)
{
//    u32 tmp;
    u8 *packet = params;
    u8 mic_num;
    u8 max_mic_nb = WIRELESS_CON_LINK_NB;
#if WIRELESS_MIC_2TNR_EN
    if (!wireless_role_is_adapter()) {
        max_mic_nb = WIRELESS_CON_2TNR_NB;
    }
#endif

    switch(evt) {
    case BT_NOTICE_WIRELESS_CONNECTED:
        mic_num = packet[0];
        printf("WL_CON, %d, %d\n", mic_num, packet[8]);
        if(mic_num < max_mic_nb) {
#if DEVICE_INTERPHONE_EN
            bool alg_en = (bool)(wireless_cb.bypass_cnt == 0);

            wireless_cb.connected_sta |= BIT(mic_num);
            wireless_cb.change_sta = 0;
            wireless_cb.change_flag = 1;
            func_device_interphone_set_con_cmp();
            sys_clk_req(INDEX_WL_CON, DEVICE_WL_CON_CLK);    //先抬高主频
            wireless_interphone_init();
            dis_auto_pwroff();
            wireless_set_alg_en(alg_en);                //再使能算法
#else
            if(wireless_cb.connected_sta == 0) {
                bool alg_en = (bool)(wireless_cb.bypass_cnt == 0);

                if(alg_en) {
                    if(wireless_role_is_adapter()) {
                        sys_clk_req(INDEX_WL_CON, ADAPTER_WL_CON_CLK);
                    } else {
                        sys_clk_req(INDEX_WL_CON, DEVICE_WL_CON_CLK);
                    }
                }

                if(wireless_role_is_adapter()) {
                    wireless_adapter_init();
                    dis_auto_pwroff();
                } else {
                    wireless_device_init();
                }
                wireless_set_alg_en(alg_en);                //再使能算法
            }
            wireless_cb.connected_sta |= BIT(mic_num);
            wireless_cb.change_sta = 0;
            wireless_cb.change_flag = 1;
#if WIRELESS_MIC_2TNR_EN
        } else if (mic_num == max_mic_nb) {
            ///跑到这里说明时d2d链路，或者侦听发射链路，做特殊处理
            wireless_cb.device_con_sta = 1;
            wireless_cb.device_con_role = packet[8];
            wireless_cb.change_flag = 1;
#endif
#endif
        }
        break;

    case BT_NOTICE_WIRELESS_CONNECT_FAIL:
        mic_num = packet[0];
        printf("WL_CON_FAIL, %d\n", mic_num);
        if(mic_num < max_mic_nb) {
            wireless_cb.change_sta = 1;
            wireless_cb.change_flag = 1;
#if WIRELESS_MIC_2TNR_EN
        } else if (mic_num == max_mic_nb) {
            wireless_cb.device_con_sta = 0;
            wireless_cb.device_con_role = 0;
            wireless_cb.change_flag = 1;
#endif
        }
        break;

    case BT_NOTICE_WIRELESS_DISCONNECT:
        mic_num = packet[0];
        printf("WL_DISCON, %d\n", mic_num);
        if(mic_num < max_mic_nb) {
            wireless_cb.connected_sta &= ~BIT(mic_num);
            wireless_cb.change_sta = 2;
            wireless_cb.change_flag = 1;

            wireless_dump_reset(mic_num);
            if(wireless_cb.connected_sta == 0) {
#if !DEVICE_LOCAL_REC_EN
                wireless_clr_alg_en();                  //先关闭算法
#else
                if(!sys_cb.sd_card_init_flag) {         //断连且拔出SD卡
                    wireless_clr_alg_en();
                }
#endif
#if DEVICE_INTERPHONE_EN
                wireless_interphone_exit(mic_num, 0);
                en_auto_pwroff();
#else
                if(wireless_role_is_adapter()) {
                    if (!bt_is_connected()) {
                        en_auto_pwroff();
                    }
                    wireless_adapter_exit(mic_num, 0);
                } else {
#if !DEVICE_LOCAL_REC_EN
                    wireless_device_exit();
#else
                    if(!sys_cb.sd_card_init_flag) {         //断连且拔出SD卡
                        wireless_device_exit();
                    }
#endif
                }
#endif
                sys_clk_free(INDEX_WL_CON);
                sys_clk_free(INDEX_WL_ALG_EN);
#if TBOX_TEST_EN
                if (vusb_test_is_sucess() && cfg_discon_auto_pwroff) {
                    func_cb.sta = FUNC_PWROFF;
                }
#endif
            } else {
                if(wireless_role_is_adapter()) {
                    wireless_adapter_exit(mic_num, wireless_cb.connected_sta);
                }
            }
#if WIRELESS_MIC_2TNR_EN
        } else if (mic_num == max_mic_nb) {
            wireless_cb.device_con_sta = 0;
            wireless_cb.device_con_role = 0;
            wireless_cb.change_flag = 1;
#endif
        }
        break;

    default:
        break;
    }
}

AT(.text.wireless.cb)
void wireless_device_fast_exit(void)
{
    wireless_cb.connected_sta = 0;
    wireless_cb.change_sta = 2;
    wireless_cb.change_flag = 1;

    wireless_clr_alg_en();                  //先关闭算法
    wireless_device_exit();

    sys_clk_free(INDEX_WL_CON);
    sys_clk_free(INDEX_WL_ALG_EN);
    led_bt_idle();
}

AT(.text.wireless_cmd)
void wireless_rx_user_cmd(u8 index, u8 *ptr, u8 len)
{
//    printf("user_cmd%d: \n", index);
//    print_r(ptr, len);
}

AT(.text.wireless.init)
void wireless_mic_role_init(void)
{
#if WIRELESS_MIC_ROLE == 0
    cfg_wireless_role = false;
#elif WIRELESS_MIC_ROLE == 1
    cfg_wireless_role = true;
#elif WIRELESS_MIC_ROLE == 2
    if (xcfg_cb.wireless_adapter_en) {
        cfg_wireless_role = true;
    } else if (xcfg_cb.wireless_device_en) {
        cfg_wireless_role = false;
    } else {
        ///都被去掉默认发射端
        cfg_wireless_role = false;
    }
#endif
}

AT(.text.wireless.init)
void wireless_var_init(void)
{
    memset(&wireless_cb, 0x00, sizeof(wireless_cb));
//    wireless_cb.alg_en = 0;

#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    wireless_data_buf_init();
    if(xcfg_cb.wireless_adapter_en) {
        usb_dev_update_var_init();
    }
#endif

    wireless_mic_role_init();
    wireless_cmd_init();
}
#endif
