#include "include.h"
#include "api.h"
#include "huart_baud_test.h"

#if HUART_BAUD_TEST_EN

/*
 * HUART 波特率压力测试（双机回传）：PC 发 50KB 递增码流 -> 板子收满后
 * 原样回传 -> PC 逐字节比对。移植自同事 huart_baud_test，仅保留双机模式。
 *
 * 使用约束（详见 docs/peripheral/huart_dual_failure_analysis.md）：
 * 1. HUART 单外设：使能本模块须关闭 EQ_DBG_IN_UART/UART2_COM_EN/HUART_COM_EN；
 * 2. PC 须逐块节流发送（--chunk-size 与 HUART_BAUD_TEST_BUF_SIZE 一致，
 *    --send-delay-ms 10），背靠背连续流会触发库非环形模式块重启缺陷；
 * 3. PC 须使用递增码流（huart_dual_inc.bin），恒值码流会掩盖块错位。
 */

// 测试波特率表，从小到大排列
static const u32 baud_rates[] = {
    115200,
    230400,
    460800,
    921600,
    1000000,
    1500000,
    2000000,
    3000000,
};

#define BAUD_CNT            (sizeof(baud_rates) / sizeof(baud_rates[0]))  // 波特率档位数量
#define DUAL_TX_CHUNK       512                                           // 单次回传发送块大小
#define DUAL_ECHO_BUF_SIZE  HUART_BAUD_TEST_DUAL_BUF_SIZE                 // 双机回传缓冲区大小

/**
 * @brief 双机回传接收状态
 */
typedef struct {
    volatile u32 drop_cnt;  // 回传缓冲区满时丢失的字节数
    volatile u32 rx_bytes;  // 当前波特率档位累计接收字节数
    volatile u8 new_data;   // RX 中断是否写入新数据
} huart_dual_state_t;

static volatile u8 dual_tx_done = 0;                                 // 双机回传 TX 完成标志
static huart_dual_state_t dual_state;                                // 双机回传接收状态
static u8 dual_echo_buf[DUAL_ECHO_BUF_SIZE] AT(.buf.huart_dual);          // 双机回传缓冲区
static u8 dual_rx_buf[HUART_BAUD_TEST_BUF_SIZE] AT(.buf.huart_dual);      // DMA 接收缓冲（与 PC chunk-size 对齐）

/**
 * @brief HUART RX 块完成回调（库在块满或线路空闲时触发）。
 * @note  ISR 上下文，按实际接收长度拷入累积缓冲；库的 rxcnt 在消费后自动归零。
 */
AT(.com_huart.text)
void huart_rx_done_cb(void)
{
    u16 cnt = (u16)huart_get_rxcnt();

    if (cnt > HUART_BAUD_TEST_BUF_SIZE) {
        cnt = HUART_BAUD_TEST_BUF_SIZE;
    }

    dual_state.new_data = 1;

    if ((dual_state.rx_bytes + cnt) <= DUAL_ECHO_BUF_SIZE) {
        memcpy(&dual_echo_buf[dual_state.rx_bytes], dual_rx_buf, cnt);
        dual_state.rx_bytes += cnt;
    } else {
        dual_state.drop_cnt += cnt;
    }
}

/**
 * @brief HUART TX 完成回调。
 */
AT(.com_huart.text)
void huart_tx_done_cb(void)
{
    dual_tx_done = 1;
}

/**
 * @brief 在接收结束后统一回传缓冲区数据。
 * @param  buf : 数据缓冲区
 * @param  len : 数据长度
 */
static void dual_echo_send(const u8 *buf, u32 len)
{
    u32 off = 0;
    u32 wait_start;
    u8 started = 0;

    if (len == 0) {
        return;
    }

    while (off < len) {
        u32 chunk = len - off;

        if (chunk > DUAL_TX_CHUNK) {
            chunk = DUAL_TX_CHUNK;
        }

        if (started) {
            wait_start = tick_get();
            while (!dual_tx_done && !tick_check_expire(wait_start, 100)) {
            }
        }

        dual_tx_done = 0;
        huart_tx(&buf[off], chunk);
        started = 1;
        off += chunk;
    }

    wait_start = tick_get();
    while (!dual_tx_done && !tick_check_expire(wait_start, 100)) {
    }
    dual_tx_done = 0;
}

/**
 * @brief 使用独立 TX/RX 引脚初始化 HSUART。
 * @param  baud : 目标波特率
 */
static void huart_baud_test_init(u32 baud)
{
    huart_t huart0;

    huart_exit();
    memset(&huart0, 0, sizeof(huart0));
    huart0.rx_port = HUART_BAUD_TEST_RX_PORT;
    huart0.tx_port = HUART_BAUD_TEST_TX_PORT;
    huart0.rxisr_en = 1;
    huart0.txisr_en = 1;
    huart0.rxbuf_loop = 0;
    huart0.rxbuf = dual_rx_buf;
    huart0.rxbuf_size = HUART_BAUD_TEST_BUF_SIZE;

    huart_init(&huart0, baud);
}

/**
 * @brief 串口波特率压力测试入口。
 */
void huart_baud_test_start(void);
void huart_baud_test_dual_loop(void);

void huart_baud_test_start(void)
{
    WDT_DIS();
    RTC_WDT_DIS();
    huart_baud_test_init(baud_rates[0]);
    huart_baud_test_dual_loop();
    WDT_CLR();
    WDT_EN();
    RTC_WDT_EN();
}

/**
 * @brief 执行 MCU 与 PC 之间的双机回传测试。
 * @note  接收阶段仅写入连续缓冲区，待 PC 停止发送后统一回传；
 *        每档收完自动切换下一档波特率，全部档位完成后返回（随后复位重启）。
 */
void huart_baud_test_dual_loop(void)
{
    u8 baud_idx = 0;
    u8 data_received = 0;
    u32 last_activity_tick;

    printf("\n=== HUART Dual Test (Echo) Start ===\n");

    // 初始化当前波特率档位的接收状态
    dual_state.drop_cnt = 0;
    dual_state.rx_bytes = 0;
    dual_state.new_data = 0;
    dual_tx_done = 0;
    data_received = 0;

    // 清空 RX FIFO，确保从干净状态开始等待 PC 数据
    huart_rxfifo_clear();
    delay_ms(10);
    printf("[Baud %lu] waiting PC data...\n", baud_rates[baud_idx]);
    last_activity_tick = tick_get();

    while (1) {
        // RX 中断已经写入新数据，刷新空闲计时
        if (dual_state.new_data) {
            dual_state.new_data = 0;
            data_received = 1;
            last_activity_tick = tick_get();
        }

        // PC 停止发送超过空闲超时后，结束当前档接收
        if (data_received &&
            tick_check_expire(last_activity_tick, HUART_BAUD_TEST_DUAL_IDLE_TIMEOUT_MS)) {
            if (dual_state.drop_cnt) {
                printf("  [drop] %lu bytes\n", dual_state.drop_cnt);
                dual_state.drop_cnt = 0;
            }

            // 输出接收统计，并统一回传
            printf("  [rx] %lu bytes\n", dual_state.rx_bytes);
            dual_echo_send(dual_echo_buf, dual_state.rx_bytes);

            if ((baud_idx + 1) < BAUD_CNT) {
                // 还有下一档波特率，切换并复位接收状态
                baud_idx++;
                data_received = 0;
                dual_state.drop_cnt = 0;
                dual_state.rx_bytes = 0;
                dual_state.new_data = 0;
                dual_tx_done = 0;
                huart_baud_test_init(baud_rates[baud_idx]);
                huart_rxfifo_clear();
                delay_ms(10);
                printf("[Baud %lu] waiting PC data...\n", baud_rates[baud_idx]);
                last_activity_tick = tick_get();
            } else {
                // 所有波特率档位测试完成
                printf("\n=== HUART Dual Test (Echo) Done ===\n");
                break;
            }
        }
    }
}

#endif /* HUART_BAUD_TEST_EN */
