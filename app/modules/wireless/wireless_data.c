#include "include.h"
#include "wireless.h"

#define DATA_MAX_NB          4
#define DATA_MAX_SIZE        245

struct data_tag {
    u8 buf[DATA_MAX_NB][DATA_MAX_SIZE+1];
    u8 w_idx;
    u8 r_idx;
    u8 total;
};
#if WIRELESS_MIC_2TNR_EN
static struct data_tag txdata[(WIRELESS_CON_2TNR_NB > WIRELESS_CON_LINK_NB) ? WIRELESS_CON_2TNR_NB : WIRELESS_CON_LINK_NB];
#else
static struct data_tag txdata[WIRELESS_CON_LINK_NB];
#endif

void *data_buf_alloc(struct data_tag *cbuf)
{
    if(cbuf->total < DATA_MAX_NB) {
        u8 w_idx = cbuf->w_idx;
        cbuf->w_idx++;
        if(cbuf->w_idx >= DATA_MAX_NB) {
            cbuf->w_idx = 0;
        }
        return cbuf->buf[w_idx];
    }

    return NULL;
}

void data_buf_add(struct data_tag *cbuf)
{
    cbuf->total++;
}

void *data_buf_get(struct data_tag *cbuf)
{
    u8 r_idx = cbuf->r_idx;
    if(cbuf->total > 0) {
        cbuf->r_idx++;
        if(cbuf->r_idx >= DATA_MAX_NB) {
            cbuf->r_idx = 0;
        }
        cbuf->total--;
        return cbuf->buf[r_idx];
    }

    return NULL;
}

void *data_buf_peek(struct data_tag *cbuf)
{
    u8 r_idx = cbuf->r_idx;
    if(cbuf->total != 0) {
        return cbuf->buf[r_idx];
    }

    return NULL;
}

//--------------------------------------------------------------------------------------
void wireless_data_buf_init(void)
{
    memset(&txdata, 0x00, sizeof(txdata));
}

void wireless_rcv_data(u8 index, u8 *data, u8 len)
{
//    printf("RX_DATA%d, %d: ", index, len);
//    print_r(data, len);
#if ADAPTER_AB_FOT_DEVICE_SUPPORT
    tws_set_fot_data(index, data+1, data[0]);
#endif
}

bool wireless_send_data(u8 index, u8 *data, u8 len)
{
    u8 *buf;

    if(len > DATA_MAX_SIZE) {
        return false;
    }

    buf = data_buf_alloc(&txdata[index]);
    if(buf == NULL) {
        my_printf("buf error \n");
        return false;
    }

    printf("TX_DATA%d, %d: ", index, len);
//    print_r(data, len);

    buf[0] = len;
    memcpy(buf+1, data, len);

    data_buf_add(&txdata[index]);
    if (!wireless_con_user_data_tx_req(index)) {
        data_buf_get(&txdata[index]);
        return false;
    }
    return true;
}

void wireless_send_data_until_success(u8 index, u8 *data, u8 len)
{
    if (wireless_cb.connected_sta & BIT(index)) {
        while (!wireless_send_data(index, data, len)) {
            delay_5ms(2);
            if(sys_cb.bt_is_inited) {
                bt_thread_check_trigger();
            }
        }
    }
}

ALIGNED(64)
void wireless_data_reset(u8 index)
{
    struct data_tag *cbuf = &txdata[index];

    GLOBAL_INT_DISABLE();
    cbuf->total = cbuf->r_idx = cbuf->w_idx = 0;
    GLOBAL_INT_RESTORE();
}

//--------------------------------------------------------------------------------------
//以下是库回调函数

//收到一条用户私有命令
void ble_con_user_data_rx_cb(u8 index, u8 *pdu)
{
    wireless_rcv_data(index, pdu+1, pdu[0]);
}

//获取要发送的用户私有命令
u8 *ble_con_user_data_get_tx_cb(u8 index)
{
    return data_buf_get(&txdata[index]);
}

//用户私有命令发送成功
bool ble_con_user_data_tx_cfm_cb(u8 index)
{
    //返回值：true=还有数据发送，false=数据已发完
    return data_buf_peek(&txdata[index]);
}
