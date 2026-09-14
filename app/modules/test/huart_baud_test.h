/**
 * @file         : huart_baud_test.h
 * @date         : 2026-09-14
 * @brief        : BT897x HUART 板内回环/双机回传波特率测试接口
 */
#ifndef __HUART_BAUD_TEST_H
#define __HUART_BAUD_TEST_H

#define HUART_BAUD_TEST_MODE_LOOPBACK        0
#define HUART_BAUD_TEST_MODE_DUAL            1

#ifndef HUART_BAUD_TEST_MODE
#define HUART_BAUD_TEST_MODE                 HUART_BAUD_TEST_MODE_DUAL
#endif

#define HUART_BAUD_TEST_PINSET_PE7_PB1       0
#define HUART_BAUD_TEST_PINSET_PB4_PB3       1

#ifndef HUART_BAUD_TEST_PINSET
#define HUART_BAUD_TEST_PINSET               HUART_BAUD_TEST_PINSET_PE7_PB1
#endif

#if HUART_BAUD_TEST_PINSET == HUART_BAUD_TEST_PINSET_PE7_PB1
#define HUART_BAUD_TEST_RX_PORT              HUART_TR_PB1
#define HUART_BAUD_TEST_TX_PORT              HUART_TR_PE7
#else
#define HUART_BAUD_TEST_RX_PORT              HUART_TR_PB3
#define HUART_BAUD_TEST_TX_PORT              HUART_TR_PB4
#endif

#ifndef HUART_BAUD_TEST_BUF_SIZE
#define HUART_BAUD_TEST_BUF_SIZE             512
#endif

#ifndef HUART_BAUD_TEST_FRAMES
#define HUART_BAUD_TEST_FRAMES               100
#endif

#ifndef HUART_BAUD_TEST_FRAME_TIMEOUT_MS
#define HUART_BAUD_TEST_FRAME_TIMEOUT_MS     200
#endif

#ifndef HUART_BAUD_TEST_TIMEOUT_ABORT
#define HUART_BAUD_TEST_TIMEOUT_ABORT        8
#endif

#ifndef HUART_BAUD_TEST_DUAL_BUF_SIZE
#define HUART_BAUD_TEST_DUAL_BUF_SIZE        51200
#endif

#ifndef HUART_BAUD_TEST_DUAL_IDLE_TIMEOUT_MS
#define HUART_BAUD_TEST_DUAL_IDLE_TIMEOUT_MS 500
#endif

void huart_baud_test_start(void);
u8 huart_baud_test_rx_done_cb(void);
void huart_baud_test_tx_done_hook(void);

#endif // __HUART_BAUD_TEST_H
