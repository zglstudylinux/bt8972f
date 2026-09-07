#include "include.h"
#include "api.h"

const uint8_t sdp_spp_service_record[] = {
    0x36, 0x00, 0x54, 0x09, 0x00, 0x00, 0x0a, 0x00, 0x01, 0x00, 0x01, 0x09, 0x00, 0x01, 0x36, 0x00,
    0x03, 0x19, 0x11, 0x01, 0x09, 0x00, 0x04, 0x36, 0x00, 0x0e, 0x36, 0x00, 0x03, 0x19, 0x01, 0x00,
    0x36, 0x00, 0x05, 0x19, 0x00, 0x03, 0x08, SPP_RFCOMM_SERVER_CHANNEL0,   0x09, 0x00, 0x05, 0x36,
    0x00, 0x03, 0x19, 0x10, 0x02, 0x09, 0x00, 0x06, 0x36, 0x00, 0x09, 0x09, 0x65, 0x6e, 0x09, 0x00,
    0x6a, 0x09, 0x01, 0x00, 0x09, 0x00, 0x09, 0x36, 0x00, 0x09, 0x36, 0x00, 0x06, 0x19, 0x11, 0x01,
    0x09, 0x01, 0x02, 0x09, 0x01, 0x00, 0x25, 0x03, 0x53, 0x50, 0x50,
};

#if BT_SPP_EN
#define SPP_TX_MTU          256     //max=512

#define SPP_POOL_PAGE_NB     1
#define SPP_POOL_SIZE        (SPP_TX_MTU*3)

AT(.ble_buf.stack.spp)
uint8_t spp_tx_buf[SPP_TX_MTU];

AT(.ble_buf.stack.spp)
uint8_t spp_pool_buf[SPP_POOL_SIZE];

AT(.ble_buf.stack.spp)
ring_buf_t spp_pool;

AT(.rodata.ble.tbl)
const rbuf_tbl_t spp_pool_tbl[SPP_POOL_PAGE_NB] = {
    {
        .buf = spp_pool_buf,
        .size = SPP_POOL_SIZE,
    },
};

void spp_txpkt_init(void)
{
    ring_buf_init(&spp_pool, spp_pool_tbl, SPP_POOL_PAGE_NB, 0);
    txpkt_init(&spp_tx, spp_tx_buf, &spp_pool, SPP_TX_MTU, spp_send_kick);
}

u16 get_spp_mtu_size(void)
{
    return SPP_TX_MTU;
}

void spp_rx_callback(uint8_t index, uint8_t ch, uint8_t *packet, uint16_t size)
{
    if (ch == SPP_SERVICE_CH1) {
#if GFPS_EN
        gfps_spp_recv_callback(packet, size);
#endif
    }

    app_spp_rx_callback(index, ch, packet, size);
}

void spp_connect_callback(uint8_t index, uint8_t ch)
{
    printf("--->%s idx:%d ch:%d\n", __func__, index, ch);

    if (ch == SPP_SERVICE_CH1) {
#if GFPS_EN
        gfps_spp_connected_callback();
#endif
    }

    app_spp_connect_callback(index, ch);
}

void spp_disconnect_callback( uint8_t index, uint8_t ch)
{
    printf("--->%s idx:%d ch:%d\n", __func__, index, ch);
    app_spp_disconnect_callback(index, ch);
}

#endif

