/**
 * @file         : huart_baud_test.h
 * @date         : 2026-09-10
 * @brief        : BT897x HUART 波特率压力测试模块接口（双机回传）
 *
 * 移植自同事双机回传测试（sdk_bt897x_le_mic2_v012_s8074_20260202），
 * 适配本工程：自包含缓冲（不依赖 eq_rx_buf）、直接提供 huart_rx_done_cb/
 * huart_tx_done_cb（本工程关闭 EQ 时 bsp_huart.c 整体编译排除）。
 * 块长/节流约束与缺陷背景见 docs/peripheral/huart_dual_failure_analysis.md。
 */
#ifndef __HUART_BAUD_TEST_H
#define __HUART_BAUD_TEST_H

// HSUART RX 引脚（与 UART2_COM 接线一致：PC TX -> PB1）
#ifndef HUART_BAUD_TEST_RX_PORT
#define HUART_BAUD_TEST_RX_PORT              HUART_TR_PB1
#endif

// HSUART TX 引脚（PC RX <- PE7）
#ifndef HUART_BAUD_TEST_TX_PORT
#define HUART_BAUD_TEST_TX_PORT              HUART_TR_PE7
#endif

// DMA 接收块长（PC 发送 chunk-size 需与此一致）
#ifndef HUART_BAUD_TEST_BUF_SIZE
#define HUART_BAUD_TEST_BUF_SIZE             512
#endif

// 双机回传接收缓冲区大小
#ifndef HUART_BAUD_TEST_DUAL_BUF_SIZE
#define HUART_BAUD_TEST_DUAL_BUF_SIZE        51200
#endif

// 双机测试接收空闲超时时间（PC 停发超过该时间判定本档结束）
#ifndef HUART_BAUD_TEST_DUAL_IDLE_TIMEOUT_MS
#define HUART_BAUD_TEST_DUAL_IDLE_TIMEOUT_MS 500
#endif

// 启动双机回传压力测试（阻塞运行，全部档位完成后复位重启）
void huart_baud_test_start(void);

#endif // __HUART_BAUD_TEST_H
