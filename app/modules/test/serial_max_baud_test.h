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
 * 另有两种自发模式：TX-LA（逻辑分析仪解 TX 线）、LOOPBACK（板内 TX 短接 RX）。
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

// TX-LA 专用模式：不依赖适配器输入，板子在梯子上循环自发递增码流，
// 逻辑分析仪直接解码 TX 线验证（用于适配器接不住的 2M+ 档位验收）。
// 1=启用后 start() 走 TXLA 死循环（每档：提示 -> 1.5s 静默 -> 3s 连续发送）
#ifndef SERIAL_MAX_BAUD_TEST_TX_LA_EN
#define SERIAL_MAX_BAUD_TEST_TX_LA_EN        0
#endif

// TX-LA 每档连续发送时长
#ifndef SERIAL_MAX_BAUD_TEST_TX_LA_MS
#define SERIAL_MAX_BAUD_TEST_TX_LA_MS        3000
#endif

// TX-LA 每档发送前的静默窗口（PC 收到 arm 提示后用该窗口启动 LA 采集）
#ifndef SERIAL_MAX_BAUD_TEST_TX_LA_GAP_MS
#define SERIAL_MAX_BAUD_TEST_TX_LA_GAP_MS    1500
#endif

// UART2 自抑制探针：GPIO bitbang PE7→PB1 与 UART2DATA 硬件 TX→RX 做单字节 A/B。
// 仅用于确认普通 UART2 是否在自身发送期间屏蔽 RX，不参与波特率梯子。
#ifndef SERIAL_MAX_BAUD_TEST_UART2_PROBE_EN
#define SERIAL_MAX_BAUD_TEST_UART2_PROBE_EN  0
#endif

// 回环模式：1=启用后 start() 走板内回环梯子（TX 短接 RX，无需适配器，
// 纯板端自发码流自发收，测板子自身收发链路的最大无错波特率）
#ifndef SERIAL_MAX_BAUD_TEST_LOOPBACK_EN
#define SERIAL_MAX_BAUD_TEST_LOOPBACK_EN     0
#endif

// 统一帧式回环：每码型帧数（筛选 100 / 加压 1000），与 huart_baud_test 口径一致。
// 帧长固定 512B、5 种码型（00/FF/55/AA/递增）、逐帧逐字节比对。
#ifndef SERIAL_MAX_BAUD_TEST_LOOP_FRAMES
#define SERIAL_MAX_BAUD_TEST_LOOP_FRAMES     100
#endif

// 统一帧式回环加压态：1=梯子收敛为最大无错档单档（配 LOOP_FRAMES=1000 用）
#ifndef SERIAL_MAX_BAUD_TEST_LOOP_STRESS
#define SERIAL_MAX_BAUD_TEST_LOOP_STRESS     0
#endif

// 满吞吐回环：固定波特率下连续自发自发收、在线比对，测可达吞吐与误码。
// UART2=连续流；HUART=单块握手（SAFE）与背靠背 DMA（B2B）各测一段。
#ifndef SERIAL_MAX_BAUD_TEST_TPUT_EN
#define SERIAL_MAX_BAUD_TEST_TPUT_EN         0
#endif

#ifndef SERIAL_MAX_BAUD_TEST_TPUT_BAUD
#define SERIAL_MAX_BAUD_TEST_TPUT_BAUD       9500000
#endif

#ifndef SERIAL_MAX_BAUD_TEST_TPUT_MS
#define SERIAL_MAX_BAUD_TEST_TPUT_MS         5000
#endif

// 回环每档码流量（字节）；独立接收缓冲同尺寸（与发送源分离，HUART RX
// 回调搬运不能覆盖在途的发送源）
#ifndef SERIAL_MAX_BAUD_TEST_LOOP_SIZE
#define SERIAL_MAX_BAUD_TEST_LOOP_SIZE       32768
#endif

// 回环等待单块回环接收完成的超时（HUART 单块在途模式逐块等待）
#ifndef SERIAL_MAX_BAUD_TEST_LOOP_BLK_TIMEOUT_MS
#define SERIAL_MAX_BAUD_TEST_LOOP_BLK_TIMEOUT_MS  100
#endif

// 回环连续无回数据块数达到该值判定本档失败，提前跳档（防高波特率死等）
#ifndef SERIAL_MAX_BAUD_TEST_LOOP_MISS_ABORT
#define SERIAL_MAX_BAUD_TEST_LOOP_MISS_ABORT 8
#endif

#endif // __SERIAL_MAX_BAUD_TEST_H
