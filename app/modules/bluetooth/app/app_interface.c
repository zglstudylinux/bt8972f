#include "include.h"
#include "app.h"

static gatt_characteristic_cb_info_t *characteristic_cb_info[ATT_NUM] AT(.ble_cache.att);
static uint8_t  gatts_profile_table[ATT_NUM * 10] AT(.ble_cache.att);
void ble_txpkt_init(void);
void latt_txpkt_init(void);
void priv_ble_gatts_service_init(void);

void app_ble_connect_callback(void)
{
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    fot_ble_connect_callback();
#endif
    app_ctrl_ble_connect_callback();
}

void app_ble_disconnect_callback(void)
{
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    fot_ble_disconnect_callback();
#endif
    app_ctrl_ble_disconnect_callback();
}

void app_process(void)
{
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    bsp_fot_process();
#endif
    app_ctrl_process();
}

void app_var_init(void)
{
#if FOT_EN
    fot_var_init();
#endif
    app_ctrl_var_init();
}

void app_txpkt_init(void)
{
#if LE_EN
    ble_txpkt_init();
#endif
#if BT_ATT_EN
    latt_txpkt_init();
#endif
}

void app_gatt_init(void)
{
    gatts_init(gatts_profile_table, sizeof(gatts_profile_table),
                    characteristic_cb_info,
                    ATT_NUM);
#if BLE_PRIV_EN
    priv_ble_gatts_service_init();
#endif
    app_ctrl_gatt_init();
}

void app_init_do(void)
{
    app_ctrl_init_do();
}

u8 app_eq_set(void)
{
    return app_ctrl_eq_set();
}

void app_enter_sleep(void)
{
    app_ctrl_enter_sleep();
}

void app_exit_sleep(void)
{
    app_ctrl_exit_sleep();
}

bool app_need_wakeup(void)
{
#if FOT_EN
    if (bsp_fot_is_connect()) {
        return true;
    }
#endif
    return app_ctrl_need_wakeup();
}

void app_bt_evt_notice(uint evt, void *params)
{
    app_ctrl_bt_evt_notice(evt, params);
}

uint16_t app_ble_role_switch_get_data(uint8_t *data_ptr)
{
    return app_ctrl_ble_role_switch_get_data(data_ptr);
}

uint16_t app_ble_role_switch_set_data(uint8_t *data_ptr, uint16_t len)
{
    return app_ctrl_ble_role_switch_set_data(data_ptr, len);
}

void app_sleep_mode_process(void)
{
    app_ctrl_sleep_mode_process();
}

void app_tws_user_key_process(uint32_t *opcode)
{
    app_ctrl_tws_user_key_process(opcode);
}

void app_tws_connect_callback(void)
{
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    fot_tws_connect_callback();
#endif
}

void app_tws_disconnect_callback(void)
{
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    fot_tws_disconnect_callback();
#endif
}

void app_spp_connect_callback(uint8_t index, uint8_t ch)
{
    if (ch == SPP_SERVICE_CH0) {
#if FOT_EN
        fot_spp_connect_callback();
#endif
        app_ctrl_spp_connect_callback(index);
    } else if (ch == SPP_SERVICE_CH1){
        app_ctrl_spp1_connect_callback(index);
    } else if (ch == SPP_SERVICE_CH2){
        app_ctrl_spp2_connect_callback(index);
    }
}

void app_spp_disconnect_callback(uint8_t index, uint8_t ch)
{
    if (ch == SPP_SERVICE_CH0) {
#if FOT_EN
        fot_spp_disconnect_callback();
#endif
        app_ctrl_spp_disconnect_callback(index);
    } else if (ch == SPP_SERVICE_CH1){
        app_ctrl_spp1_disconnect_callback(index);
    } else if (ch == SPP_SERVICE_CH2){
        app_ctrl_spp2_disconnect_callback(index);
    }
}

bool app_spp_rx_callback(uint8_t index, uint8_t ch, uint8_t *data, u16 len)
{
    if (ch == SPP_SERVICE_CH0) {
#if FOT_EN
        if(fot_app_connect_auth(data, len, FOTA_CON_SPP)){
            fot_recv_proc(data, len);
            return true;
        }
#endif
        if (app_test_cmd_process(data, len, 0)) {
            return true;
        }
        return app_ctrl_spp_rx_callback(index, data, len);
    } else if (ch == SPP_SERVICE_CH1){
        return app_ctrl_spp1_rx_callback(index, data, len);
    } else if (ch == SPP_SERVICE_CH2){
        return app_ctrl_spp2_rx_callback(index, data, len);
    }

    return true;
}
