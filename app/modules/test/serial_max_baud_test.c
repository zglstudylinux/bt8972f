#include "include.h"
#include "api.h"
#include "bsp_uart2_com.h"
#include "serial_max_baud_test.h"

#if SERIAL_MAX_BAUD_TEST_EN

#if HUART_BAUD_TEST_EN
#error "SERIAL_MAX_BAUD_TEST_EN 与 HUART_BAUD_TEST_EN 互斥(huart 回调重复定义)，请只开一个"
#endif
#if SERIAL_MAX_BAUD_TEST_USE_UART2 && !UART2_COM_EN
#error "UART2 测试构建需打开 UART2_COM_EN(测试模块直接使用 bsp_uart2_com 驱动)"
#endif

/*
 * 普通串口/高速串口 最大波特率统一测试（单板 + PC 适配器）：
 * 每档波特率固定三段式，四个测试项（UART2-RX/TX、HUART-RX/TX）方法完全一致：
 *   1. RX：PC 发 50KB 递增码流 -> 板收满/500ms 空闲 -> 板端递增连续性校验（RX 方向真值）
 *   2. TX：板按递增基准重建码流后整块回传 -> PC 逐字节比对（TX 方向独立判定，
 *      不受 RX 实收损坏影响；适配器回传方向的丢失由 PC 比对捕获，配合 LA 可归因）
 *   3. 打印统计后自动切下一档；PC 脚本靠 COM9 的 "[Baud xxx] waiting PC data..." 同步
 * PC 须按 profile 节流发送（HUART 512B+10ms 规避库非环形块缺陷；UART2 64B+8ms
 * 适配 128B 环形缓冲+主循环轮询排水），码流必须用递增（恒值会掩盖块错位）。
 */

#if SERIAL_MAX_BAUD_TEST_USE_UART2
#define PERIPH_NAME                 "UART2"
// UART2 阶梯：3M 为 CH340 规格外档（CH340 规格上限 2M），用 CP210x 交叉验证
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
#else
#define PERIPH_NAME                 "HUART"
// HUART 阶梯：9.5M 与回环 PHY 证据对齐（回环 10M 起误码，不设更高档）
static const u32 baud_rates[] = {
    115200,
    230400,
    460800,
    921600,
    1000000,
    1500000,
    2000000,
    2500000,
    3000000,
    4000000,
    6000000,
    8000000,
    9500000,
};
#endif

#define BAUD_CNT            (sizeof(baud_rates) / sizeof(baud_rates[0]))  // 波特率档位数量
#define UART2_TX_CHUNK      64                                            // UART2 回传单次入队块大小(环形队列 128B 留余量)
#define HUART_TX_CHUNK      SERIAL_MAX_BAUD_TEST_BLK_SIZE                 // HUART 回传单次 DMA 块大小

/**
 * @brief 每档波特率的接收状态
 */
typedef struct {
    volatile u32 drop_cnt;  // 回传缓冲区满时丢弃的字节数
    volatile u32 rx_bytes;  // 当前档位累计接收字节数
    volatile u8  new_data;  // 本轮泵询是否有新数据
} serial_max_state_t;

static volatile u8 sm_tx_done = 0;                                   // HUART TX 完成标志
static serial_max_state_t sm_state;                                  // 接收状态
static u8 sm_buf[SERIAL_MAX_BAUD_TEST_BUF_SIZE] AT(.buf.serial_max); // 回传缓冲区
static u8 sm_blk[SERIAL_MAX_BAUD_TEST_BLK_SIZE] AT(.buf.serial_max); // HUART DMA 接收块缓冲

#if !SERIAL_MAX_BAUD_TEST_USE_UART2

/**
 * @brief HUART RX 块完成回调（库在块满或线路空闲时触发）。
 * @note  ISR 上下文，按实际接收长度拷入累积缓冲；库的 rxcnt 在消费后自动归零。
 */
AT(.com_huart.text)
void huart_rx_done_cb(void)
{
    u16 cnt = (u16)huart_get_rxcnt();

    if (cnt > SERIAL_MAX_BAUD_TEST_BLK_SIZE) {
        cnt = SERIAL_MAX_BAUD_TEST_BLK_SIZE;
    }

    sm_state.new_data = 1;

    if ((sm_state.rx_bytes + cnt) <= SERIAL_MAX_BAUD_TEST_BUF_SIZE) {
        memcpy(&sm_buf[sm_state.rx_bytes], sm_blk, cnt);
        sm_state.rx_bytes += cnt;
    } else {
        sm_state.drop_cnt += cnt;
    }
}

/**
 * @brief HUART TX 完成回调。
 */
AT(.com_huart.text)
void huart_tx_done_cb(void)
{
    sm_tx_done = 1;
}

#endif /* !SERIAL_MAX_BAUD_TEST_USE_UART2 */

/**
 * @brief 按目标波特率重新初始化被测外设。
 * @param  baud : 目标波特率
 */
static void periph_init(u32 baud)
{
#if SERIAL_MAX_BAUD_TEST_USE_UART2
    printf("[t] uart2 init %lu...\n", baud);
    bsp_uart2_com_init(baud);
#else
    huart_t huart0;

    huart_exit();
    memset(&huart0, 0, sizeof(huart0));
    huart0.rx_port = HUART_TR_PB1;
    huart0.tx_port = HUART_TR_PE7;
    huart0.rxisr_en = 1;
    huart0.txisr_en = 1;
    huart0.rxbuf_loop = 0;
    huart0.rxbuf = sm_blk;
    huart0.rxbuf_size = SERIAL_MAX_BAUD_TEST_BLK_SIZE;

    huart_init(&huart0, baud);
    huart_rxfifo_clear();
#endif
}

/**
 * @brief 泵询被测外设：把新到的数据搬入累积缓冲。
 * @return 本轮是否有新数据（用于空闲计时）
 * @note   HUART 的搬运在 RX ISR 完成，这里只回报活动；UART2 在此同时驱动
 *         bsp_uart2_com_process()（RX 轮询 + TX 队列提交）。
 */
static u8 periph_pump(void)
{
#if SERIAL_MAX_BAUD_TEST_USE_UART2
    u8 ch;
    u8 active = 0;

    bsp_uart2_com_process();
    while (bsp_uart2_com_get(&ch)) {
        active = 1;
        if (sm_state.rx_bytes < SERIAL_MAX_BAUD_TEST_BUF_SIZE) {
            sm_buf[sm_state.rx_bytes++] = ch;
        } else {
            sm_state.drop_cnt++;
        }
    }
    return active;
#else
    u8 active = sm_state.new_data;

    sm_state.new_data = 0;
    return active;
#endif
}

/**
 * @brief 把缓冲区数据整块回传（阻塞到发送完成）。
 * @param  buf : 数据缓冲区
 * @param  len : 数据长度
 */
static void periph_send(const u8 *buf, u32 len)
{
    u32 off = 0;
    u32 wait_start;

    if (len == 0) {
        return;
    }

#if SERIAL_MAX_BAUD_TEST_USE_UART2
    while (off < len) {
        u32 chunk = len - off;
        u16 written;

        if (chunk > UART2_TX_CHUNK) {
            chunk = UART2_TX_CHUNK;
        }
        written = bsp_uart2_com_write(&buf[off], (u16)chunk);
        off += written;
        bsp_uart2_com_process();    // 队列逐字节提交，吞吐为驱动结构上限(~28.6KB/s)
    }

    wait_start = tick_get();
    while (!bsp_uart2_com_tx_idle() && !tick_check_expire(wait_start, 1000)) {
        bsp_uart2_com_process();
    }
#else
    u8 started = 0;

    while (off < len) {
        u32 chunk = len - off;

        if (chunk > HUART_TX_CHUNK) {
            chunk = HUART_TX_CHUNK;
        }

        if (started) {
            wait_start = tick_get();
            while (!sm_tx_done && !tick_check_expire(wait_start, 100)) {
            }
        }

        sm_tx_done = 0;
        huart_tx(&buf[off], chunk);
        started = 1;
        off += chunk;
    }

    wait_start = tick_get();
    while (!sm_tx_done && !tick_check_expire(wait_start, 100)) {
    }
    sm_tx_done = 0;
#endif
}

/**
 * @brief 板端递增连续性校验并打印 RX 方向统计（码流规律：buf[i] = i & 0xFF）。
 */
static void verify_and_report(void)
{
    u32 err_cnt = 0;
    s32 first_err = -1;
    u32 i;

    for (i = 0; i < sm_state.rx_bytes; i++) {
        if (sm_buf[i] != (u8)(i & 0xFF)) {
            if (first_err < 0) {
                first_err = (s32)i;
            }
            err_cnt++;
        }
    }

    printf("  [rx] got=%lu err=%lu first_err=%ld drop=%lu\n",
           sm_state.rx_bytes, err_cnt, first_err, sm_state.drop_cnt);
}

/**
 * @brief 串口最大波特率统一测试入口（阻塞运行，全部档位完成后复位重启）。
 */
void serial_max_baud_test_start(void)
{
    u8 baud_idx;

    WDT_DIS();
    RTC_WDT_DIS();

    printf("\n=== Serial Max Baud Test (%s) Start ===\n", PERIPH_NAME);
    printf("[t] enter ladder, wdt off\n");

    for (baud_idx = 0; baud_idx < BAUD_CNT; baud_idx++) {
        u8 data_received = 0;
        u32 last_activity_tick;

        sm_state.drop_cnt = 0;
        sm_state.rx_bytes = 0;
        sm_state.new_data = 0;
        sm_tx_done = 0;
        periph_init(baud_rates[baud_idx]);
        delay_ms(10);
        printf("[Baud %lu] waiting PC data...\n", baud_rates[baud_idx]);
        last_activity_tick = tick_get();

        // RX 阶段：收到首个字节起，PC 停发超过空闲超时判定本档结束
        while (1) {
            if (periph_pump()) {
                data_received = 1;
                last_activity_tick = tick_get();
            }

            if (data_received &&
                tick_check_expire(last_activity_tick, SERIAL_MAX_BAUD_TEST_IDLE_TIMEOUT_MS)) {
                break;
            }
        }

        // 板端校验（RX 方向真值）。回传前按递增基准重建码流：TX 方向与 RX
        // 实收内容解耦，高波特率下 RX 的结构性丢失不会污染 TX 判定。
        verify_and_report();
        {
            u32 i;

            for (i = 0; i < sm_state.rx_bytes; i++) {
                sm_buf[i] = (u8)(i & 0xFF);
            }
        }
        periph_send(sm_buf, sm_state.rx_bytes);
        printf("  [echo] %lu bytes sent (regen)\n", sm_state.rx_bytes);
    }

    printf("\n=== Serial Max Baud Test Done ===\n");

    WDT_CLR();
    WDT_EN();
    RTC_WDT_EN();
}

#endif /* SERIAL_MAX_BAUD_TEST_EN */
