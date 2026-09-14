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
#if SERIAL_MAX_BAUD_TEST_TX_LA_EN
// TX-LA 梯子：适配器接不住的 2M+ 档 + 物理层上探
// （12M/24M 在 LA 24MS/s 下仅能脉宽量化分频，无法逐位解码，如实标注）
static const u32 baud_rates[] = {
    2000000,
    3000000,
    4000000,
    6000000,
    8000000,
    12000000,
    24000000,
};
#elif SERIAL_MAX_BAUD_TEST_LOOPBACK_EN
// 回环梯子：常规 8 档（同回传法阶梯）+ 探底档 4M~24M
// （分频最高可配 24M，探底档预期失败，实测"能配多高 vs 实测多高"）
static const u32 baud_rates[] = {
    115200,
    230400,
    460800,
    921600,
    1000000,
    1500000,
    2000000,
    3000000,
    4000000,
    6000000,
    8000000,
    12000000,
    24000000,
};
#else
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
#endif
#else
#define PERIPH_NAME                 "HUART"
#if SERIAL_MAX_BAUD_TEST_TX_LA_EN
// TX-LA 梯子：适配器接不住的 2M+ 档 + 回环 PHY 证据范围内的档位
static const u32 baud_rates[] = {
    2000000,
    2500000,
    3000000,
    4000000,
    6000000,
    8000000,
    9500000,
};
#elif SERIAL_MAX_BAUD_TEST_LOOPBACK_EN
// 回环梯子：常规 13 档（9.5M 对齐同事回环证据）+ 探底档 12M/16M/24M
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
    12000000,
    16000000,
    24000000,
};
#else
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
static u8 *sm_rx_dst;                                                // 接收累积目标（回传法=sm_buf / 回环=sm_loop_rx）
static u32 sm_rx_cap;                                                // 接收累积容量

#if SERIAL_MAX_BAUD_TEST_LOOPBACK_EN
static u8 sm_loop_rx[SERIAL_MAX_BAUD_TEST_LOOP_SIZE] AT(.buf.serial_max); // 回环接收缓冲(与发送源分离)
#endif

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

    if ((sm_state.rx_bytes + cnt) <= sm_rx_cap) {
        memcpy(&sm_rx_dst[sm_state.rx_bytes], sm_blk, cnt);
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
        if (sm_state.rx_bytes < sm_rx_cap) {
            sm_rx_dst[sm_state.rx_bytes++] = ch;
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
        if (sm_rx_dst[i] != (u8)(i & 0xFF)) {
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
 * @brief TX-LA 模式：把回传缓冲预填为连续递增码流（长度取 256 整除，循环发送无缝）。
 */
#define TXLA_WRAP_LEN   (SERIAL_MAX_BAUD_TEST_BUF_SIZE - (SERIAL_MAX_BAUD_TEST_BUF_SIZE % 256))

static u32 txla_pos = 0;    // 流内发送位置

static void txla_fill_stream(void)
{
    u32 i;

    for (i = 0; i < TXLA_WRAP_LEN; i++) {
        sm_buf[i] = (u8)(i & 0xFF);
    }
    txla_pos = 0;
}

/**
 * @brief TX-LA 模式：从预填码流中取一段发送（UART2 受环形队列限制按实际入队计数）。
 * @param  max_len : 单次最大块长
 * @return 实际发出的字节数
 */
static u32 txla_send_chunk(u32 max_len)
{
    u32 off = txla_pos % TXLA_WRAP_LEN;
    u32 chunk = TXLA_WRAP_LEN - off;
    u32 sent;

    if (chunk > max_len) {
        chunk = max_len;
    }

#if SERIAL_MAX_BAUD_TEST_USE_UART2
    u16 written = bsp_uart2_com_write(&sm_buf[off], (u16)chunk);

    sent = written;
#else
    u32 wait_start;

    sm_tx_done = 0;
    huart_tx(&sm_buf[off], (u16)chunk);
    wait_start = tick_get();
    while (!sm_tx_done && !tick_check_expire(wait_start, 100)) {
    }
    sent = chunk;
#endif

    txla_pos += sent;
    return sent;
}

/**
 * @brief TX-LA 模式主循环（死循环）：每档打印提示后静默窗口（给 PC 启动 LA 采集），
 *        再连续发送递增码流，逐档循环供逻辑分析仪解码验证 TX 线。
 */
static void serial_max_baud_test_txla_loop(void)
{
    u8 baud_idx;
    u32 tx_bytes;

    txla_fill_stream();
    printf("\n=== Serial Max Baud Test (%s) TX-LA Mode ===\n", PERIPH_NAME);

    while (1) {
        for (baud_idx = 0; baud_idx < BAUD_CNT; baud_idx++) {
            periph_init(baud_rates[baud_idx]);
            delay_ms(10);
            printf("[TXLA][Baud %lu] arm in %dms\n",
                   baud_rates[baud_idx], SERIAL_MAX_BAUD_TEST_TX_LA_GAP_MS);
            delay_ms(SERIAL_MAX_BAUD_TEST_TX_LA_GAP_MS);
            printf("[TXLA][Baud %lu] tx start\n", baud_rates[baud_idx]);

            {
                u32 start = tick_get();

                tx_bytes = 0;
                while (!tick_check_expire(start, SERIAL_MAX_BAUD_TEST_TX_LA_MS)) {
                    tx_bytes += txla_send_chunk(SERIAL_MAX_BAUD_TEST_BLK_SIZE);
#if SERIAL_MAX_BAUD_TEST_USE_UART2
                    // 泵空环形队列再取下一段，尽量贴近线上速率
                    {
                        u32 guard = tick_get();

                        while (!bsp_uart2_com_tx_idle() &&
                               !tick_check_expire(guard, 100)) {
                            bsp_uart2_com_process();
                        }
                    }
#endif
                }
#if !SERIAL_MAX_BAUD_TEST_USE_UART2
                {
                    u32 wait_start = tick_get();

                    while (!sm_tx_done && !tick_check_expire(wait_start, 100)) {
                    }
                }
#endif
            }

            printf("[TXLA][Baud %lu] tx done (%lu bytes)\n",
                   baud_rates[baud_idx], tx_bytes);
        }
    }
}

#if SERIAL_MAX_BAUD_TEST_LOOPBACK_EN && SERIAL_MAX_BAUD_TEST_USE_UART2 && \
    SERIAL_MAX_BAUD_TEST_UART2_PROBE_EN

static u8 uart2_probe_wait_rx(u32 loops, u32 *first_con, u32 *first_idx, u8 *data)
{
    u32 i;

    for (i = 0; i < loops; i++) {
        u32 con = UART2CON;

        if (con & BIT(9)) {
            *first_con = con;
            *first_idx = i;
            *data = UART2DATA;
            UART2CPND = BIT(9);
            return 1;
        }
    }
    return 0;
}

static void uart2_probe_bitbang_55(void)
{
    u8 i;
    u8 data = 0x55;

    GPIOECLR = BIT(7);
    delay_us(9);
    for (i = 0; i < 8; i++) {
        if (data & BIT(i)) {
            GPIOESET = BIT(7);
        } else {
            GPIOECLR = BIT(7);
        }
        delay_us(9);
    }
    GPIOESET = BIT(7);
    delay_us(18);
}

static void uart2_self_suppress_probe(void)
{
    u32 mux_saved;
    u32 con_saved;
    u32 first_con = 0;
    u32 first_idx = 0;
    u8 data = 0;
    u8 seen;
    u32 i;
    u32 bit8_idx = 0xffffffff;
    u32 bit9_idx = 0xffffffff;
    u32 bit9_con = 0;
    u8 bit9_data = 0;

    bsp_uart2_com_init(115200);
    delay_ms(10);
    mux_saved = FUNCMCON2;
    con_saved = UART2CON;
    if (UART2CON & BIT(9)) {
        data = UART2DATA;
    }
    UART2CPND = BIT(8) | BIT(9);

    FUNCMCON2 &= ~(0x0f << 8);       // 保留 RX2->PB1，移除 TX2->PE7
    GPIOEDE |= BIT(7);
    GPIOEFEN &= ~BIT(7);
    GPIOEDIR &= ~BIT(7);             // DIR=0 为 GPIO 输出
    GPIOESET = BIT(7);
    delay_us(50);
    uart2_probe_bitbang_55();
    seen = uart2_probe_wait_rx(200000, &first_con, &first_idx, &data);
    printf("[U2PROBE][GPIO] seen=%d data=%02x first=%lu con=%08x end=%08x mux=%08x\n",
           seen, data, first_idx, first_con, UART2CON, FUNCMCON2);

    FUNCMCON2 = mux_saved;
    GPIOEFEN |= BIT(7);
    GPIOEDIR |= BIT(7);
    UART2CON = con_saved;
    UART2CPND = BIT(8) | BIT(9);
    UART2DATA = 0x55;
    for (i = 0; i < 600000; i++) {
        u32 con = UART2CON;

        if ((bit8_idx == 0xffffffff) && (con & BIT(8))) {
            bit8_idx = i;
        }
        if ((bit9_idx == 0xffffffff) && (con & BIT(9))) {
            bit9_idx = i;
            bit9_con = con;
            bit9_data = UART2DATA;
            UART2CPND = BIT(9);
        }
    }
    printf("[U2PROBE][UART] bit8=%lu bit9=%lu data=%02x con9=%08x end=%08x mux=%08x\n",
           bit8_idx, bit9_idx, bit9_data, bit9_con, UART2CON, FUNCMCON2);

    // C：仅清原厂注释为 One line 的 BIT6，验证是否存在双线全双工入口。
    bit8_idx = 0xffffffff;
    bit9_idx = 0xffffffff;
    bit9_con = 0;
    bit9_data = 0;
    UART2CON = con_saved & ~BIT(6);
    UART2CPND = BIT(8) | BIT(9);
    UART2DATA = 0x55;
    for (i = 0; i < 600000; i++) {
        u32 con = UART2CON;

        if ((bit8_idx == 0xffffffff) && (con & BIT(8))) {
            bit8_idx = i;
        }
        if ((bit9_idx == 0xffffffff) && (con & BIT(9))) {
            bit9_idx = i;
            bit9_con = con;
            bit9_data = UART2DATA;
            UART2CPND = BIT(9);
        }
    }
    printf("[U2PROBE][U-NO6] bit8=%lu bit9=%lu data=%02x con9=%08x end=%08x mux=%08x\n",
           bit8_idx, bit9_idx, bit9_data, bit9_con, UART2CON, FUNCMCON2);
    printf("=== UART2 Self Suppress Probe Done ===\n");
}

#endif

#if SERIAL_MAX_BAUD_TEST_LOOPBACK_EN

#if SERIAL_MAX_BAUD_TEST_USE_UART2
/**
 * @brief 回环单档（UART2）：紧循环边发边收。TX 128B 环形队列天然做流控窗口，
 *        process() 每轮 1 进 1 出（TX 队列提交 + RX 轮询排水）；RX 过载时字节
 *        计入 overflow（drop），如实测出轮询式驱动的结构性收发上限。
 */
static void loopback_rung_uart2(void)
{
    u32 send_pos = 0;
    u32 guard;
    u32 i;

    for (i = 0; i < SERIAL_MAX_BAUD_TEST_LOOP_SIZE; i++) {
        sm_buf[i] = (u8)(i & 0xFF);
    }

    while (send_pos < SERIAL_MAX_BAUD_TEST_LOOP_SIZE) {
        u32 chunk = SERIAL_MAX_BAUD_TEST_LOOP_SIZE - send_pos;
        u16 written;

        if (chunk > UART2_TX_CHUNK) {
            chunk = UART2_TX_CHUNK;
        }
        written = bsp_uart2_com_write(&sm_buf[send_pos], (u16)chunk);
        send_pos += written;
        periph_pump();      // RX 轮询排水 + TX 队列提交
    }

    guard = tick_get();
    while (!bsp_uart2_com_tx_idle() && !tick_check_expire(guard, 1000)) {
        periph_pump();
    }
    guard = tick_get();
    while (!tick_check_expire(guard, SERIAL_MAX_BAUD_TEST_IDLE_TIMEOUT_MS)) {
        if (periph_pump()) {
            guard = tick_get();
        }
    }
}
#else
/**
 * @brief 回环单档（HUART）：单块在途（发 512B -> 等 TX 完成 -> 等本块回环接收 ->
 *        下一块），任意时刻只有一块数据在环上，规避库非环形模式背靠背连续多块
 *        接收的错位缺陷。连续无回数据块达阈值判失败跳档。
 */
static void loopback_rung_huart(void)
{
    u32 send_off = 0;
    u8 miss_runs = 0;

    while ((send_off < SERIAL_MAX_BAUD_TEST_LOOP_SIZE) &&
           (miss_runs < SERIAL_MAX_BAUD_TEST_LOOP_MISS_ABORT)) {
        u32 chunk = SERIAL_MAX_BAUD_TEST_LOOP_SIZE - send_off;
        u32 rx_before = sm_state.rx_bytes;
        u32 wait_start;

        if (chunk > HUART_TX_CHUNK) {
            chunk = HUART_TX_CHUNK;
        }

        sm_tx_done = 0;
        huart_tx(&sm_buf[send_off], (u16)chunk);
        wait_start = tick_get();
        while (!sm_tx_done && !tick_check_expire(wait_start, 100)) {
        }

        wait_start = tick_get();
        while ((sm_state.rx_bytes == rx_before) &&
               !tick_check_expire(wait_start, SERIAL_MAX_BAUD_TEST_LOOP_BLK_TIMEOUT_MS)) {
        }
        miss_runs = (sm_state.rx_bytes == rx_before) ? (u8)(miss_runs + 1) : 0;
        send_off += chunk;
    }

    delay_ms(SERIAL_MAX_BAUD_TEST_IDLE_TIMEOUT_MS);   // 等最后的在途数据落袋
}
#endif

/**
 * @brief 回环梯子：板内 TX 短接 RX，自发 32KB 递增码流自发收，板端校验打印后
 *        自动爬梯（无适配器，PC 只看 COM9 调试口记表）。
 */
static void serial_max_baud_test_loopback_ladder(void)
{
    u8 baud_idx;

    printf("\n=== Serial Max Baud Loopback (%s) Start ===\n", PERIPH_NAME);
    printf("[t] enter loopback ladder, wdt off\n");
    sm_rx_dst = sm_loop_rx;
    sm_rx_cap = SERIAL_MAX_BAUD_TEST_LOOP_SIZE;

    for (baud_idx = 0; baud_idx < BAUD_CNT; baud_idx++) {
        sm_state.drop_cnt = 0;
        sm_state.rx_bytes = 0;
        sm_state.new_data = 0;
        sm_tx_done = 0;
        periph_init(baud_rates[baud_idx]);
        delay_ms(10);
        printf("[Loop][Baud %lu] running...\n", baud_rates[baud_idx]);

#if SERIAL_MAX_BAUD_TEST_USE_UART2
        loopback_rung_uart2();
#else
        loopback_rung_huart();
#endif
        verify_and_report();
#if SERIAL_MAX_BAUD_TEST_USE_UART2
        {
            uart2_com_stats_t st;

            bsp_uart2_com_get_stats(&st);
            printf("  [rxstat] hw_pending=%lu sw=%lu ovf=%lu\n",
                   st.rx_pending_count, st.rx_byte_count, st.rx_overflow_count);
        }
#endif
    }

    printf("\n=== Serial Max Baud Loopback Done ===\n");
}

#endif /* SERIAL_MAX_BAUD_TEST_LOOPBACK_EN */

static void serial_max_baud_test_echo_ladder(void);

/**
 * @brief 串口最大波特率统一测试入口。
 * @note  默认走回传法梯子（全部档位完成后复位重启）；TX-LA 模式为死循环，
 *        逐档自发递增码流供逻辑分析仪采集，复位退出。
 */
void serial_max_baud_test_start(void)
{
    WDT_DIS();
    RTC_WDT_DIS();

#if SERIAL_MAX_BAUD_TEST_TX_LA_EN
    serial_max_baud_test_txla_loop();
#elif SERIAL_MAX_BAUD_TEST_LOOPBACK_EN && SERIAL_MAX_BAUD_TEST_USE_UART2 && \
      SERIAL_MAX_BAUD_TEST_UART2_PROBE_EN
    uart2_self_suppress_probe();
#elif SERIAL_MAX_BAUD_TEST_LOOPBACK_EN
    serial_max_baud_test_loopback_ladder();
#else
    serial_max_baud_test_echo_ladder();
#endif

    WDT_CLR();
    WDT_EN();
    RTC_WDT_EN();
}

/**
 * @brief 回传法梯子：PC 发码流 -> 板端校验（RX 真值）-> 重建回传（TX 判定）-> 自动爬梯。
 */
static void serial_max_baud_test_echo_ladder(void)
{
    u8 baud_idx;

    printf("\n=== Serial Max Baud Test (%s) Start ===\n", PERIPH_NAME);
    printf("[t] enter ladder, wdt off\n");
    sm_rx_dst = sm_buf;
    sm_rx_cap = SERIAL_MAX_BAUD_TEST_BUF_SIZE;

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
}

#endif /* SERIAL_MAX_BAUD_TEST_EN */
