#include "include.h"
#include "api.h"

#if BT_ATT_EN
#define LATT_TX_MTU          256     //max=512

#define LATT_POOL_PAGE_NB     1
#define LATT_POOL_SIZE        (LATT_TX_MTU*3)

AT(.ble_buf.stack.latt)
uint8_t latt_tx_buf[LATT_TX_MTU];

AT(.ble_buf.stack.latt)
uint8_t latt_pool_buf[LATT_POOL_SIZE];

AT(.ble_buf.stack.latt)
ring_buf_t latt_pool;

AT(.rodata.ble.tbl)
const rbuf_tbl_t latt_pool_tbl[LATT_POOL_PAGE_NB] = {
    {
        .buf = latt_pool_buf,
        .size = LATT_POOL_SIZE,
    },
};

void latt_txpkt_init(void)
{
    ring_buf_init(&latt_pool, latt_pool_tbl, LATT_POOL_PAGE_NB, 0);
    txpkt_init(&latt_notify_tx, latt_tx_buf, &latt_pool, LATT_TX_MTU, latt_send_kick);
}
#endif
