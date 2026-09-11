/**
 * @file         : serial_max_baud_test.h
 * @date         : 2026-09-10
 * @brief        : BT897x 普通串口(UART2)/高速串口(HUART) 最大波特率统一测试接口
 *
 * 统一方法测四个测试项（UART2-RX/TX、HUART-RX/TX）：PC 发 50KB 递增码流 ->
 * 板端收满做递增连续性校验（RX 方向真值）-> 整块回传（TX 方向，PC 比对）->
 * 自动切下一档。外设由 SERIAL_MAX_BAUD_TEST_USE_UART2 编译期二选一，
 * 两外设共用 PE7/PB1 接线，烧不同固件即可，接线不变。
 * 节流约束与归因规则见 docs/peripheral/serial_max_baud_test_plan.md。
 */
#ifndef __SERIAL_MAX_BAUD_TEST_H
#define __SERIAL_MAX_BAUD_TEST_H

// 总开关（config.h 控制；未定义时缺省关闭，模块整体编译排除）
#ifndef SERIAL_MAX_BAUD_TEST_EN
#define SERIAL_MAX_BAUD_TEST_EN              0
#endif

// 外设选择：1=UART2 普通串口(bsp_uart2_com 驱动) 0=HUART 高速串口(库+DMA)
#ifndef SERIAL_MAX_BAUD_TEST_USE_UART2
#define SERIAL_MAX_BAUD_TEST_USE_UART2       1
#endif

// 回传缓冲区大小（与测试码流文件 huart_dual_inc.bin 50760 字节一致）
#ifndef SERIAL_MAX_BAUD_TEST_BUF_SIZE
#define SERIAL_MAX_BAUD_TEST_BUF_SIZE        50760
#endif

// HUART DMA 接收块长（PC 发送 chunk-size 需与此一致）
#ifndef SERIAL_MAX_BAUD_TEST_BLK_SIZE
#define SERIAL_MAX_BAUD_TEST_BLK_SIZE        512
#endif

// 每档接收空闲超时（PC 停发超过该时间判定本档结束）
#ifndef SERIAL_MAX_BAUD_TEST_IDLE_TIMEOUT_MS
#define SERIAL_MAX_BAUD_TEST_IDLE_TIMEOUT_MS 500
#endif

// 启动统一最大波特率测试（阻塞运行，全部档位完成后复位重启）
void serial_max_baud_test_start(void);

#endif // __SERIAL_MAX_BAUD_TEST_H
