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
// 统一帧式回环梯子（两外设同口径，与 huart_baud_test 帧回环一致）：低速常规档 +
// 9M~12M 边界密集档；UART2 另加 16M/20M/24M 探底延伸。
// 注意分频量化：16M/20M 档实际速率为 12M/24M（24M 只能整除），标注仍按名义值。
static const u32 baud_rates[] = {
#if SERIAL_MAX_BAUD_TEST_LOOP_STRESS
    12000000,                       // 加压态：仅统一口径最大无错档
#else
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
    5000000,
    6000000,
    7000000,
    8000000,
    9000000,
    9500000,
    10000000,
    10500000,
    11000000,
    12000000,
    16000000,
    20000000,
    24000000,
#endif
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
// 统一帧式回环梯子（与 UART2 侧完全同口径）：115200~12M，含 9M~12M 边界密集档
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
    5000000,
    6000000,
    7000000,
    8000000,
    9000000,
    9500000,
    10000000,
    10500000,
    11000000,
    12000000,
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

/*
 * UART2 自回环探针 v2（按手册 Register 12-1/12-2/12-3 逐位核对后重写）：
 *   - CON[27:24]/[23:20]/[19:16] 是 KEYIE/KEYEN/RSTEN 三个功能域，写 0xa 使能、
 *     0x5 关闭。原厂 UART1 模板的 0xaaa 是把三个 key 检测功能全开，探针全关。
 *   - ONELINE(bit6)=0 为 TX/RX separate；RXEN(bit7)=1；FIXBAUD(5)、SB2EN(4)。
 *   - RXPND=CON[9]、RX_BCNT=CON[14:11]（RX 4 字节缓冲计数）、RX_4BUF_ERROR=[15]。
 *   - CPND 写 BIT9 = RX 计数减 1；写 DATA 自动清 TXPND。
 *   - BAUD 只有低 16 位是分频（Baud=Fudet/(BAUD+1)），高 16 位 DARTBAUD 只读。
 * 三组同轮对比：GPIO 正对照 / ONELINE=0 双线自发 / ONELINE=1 单线自发。
 */
#define UART2_PROBE_CON_KEYS_OFF    ((0x5u << 24) | (0x5u << 20) | (0x5u << 16))

static u8 uart2_probe_wait_rx(u32 loops, u32 *first_con, u32 *first_idx, u8 *data)
{
    u32 i;

    for (i = 0; i < loops; i++) {
        u32 con = UART2CON;

        // RXPND 置位或 RX_BCNT 非零都算收到（手册 bit9 / bit14:11）
        if ((con & BIT(9)) || ((con >> 11) & 0xf)) {
            *first_con = con;
            *first_idx = i;
            *data = UART2DATA;
            UART2CPND = BIT(9);
            return 1;
        }
    }
    return 0;
}

static void uart2_probe_bitbang_55(u32 *pe7_low, u32 *pe7_tog)
{
    u8 i;
    u8 data = 0x55;
    u8 last = 1;

    *pe7_low = 0;
    *pe7_tog = 0;
    GPIOECLR = BIT(7);              // start
    delay_us(9);
    for (i = 0; i < 8; i++) {
        if (data & BIT(i)) {
            GPIOESET = BIT(7);
        } else {
            GPIOECLR = BIT(7);
        }
        delay_us(9);
        {
            u8 now = (GPIOE & BIT(7)) ? 1 : 0;

            if (!now) {
                (*pe7_low)++;
            }
            if (now != last) {
                (*pe7_tog)++;
                last = now;
            }
        }
    }
    GPIOESET = BIT(7);              // 2 stop（SB2EN=1）
    delay_us(18);
}

static void uart2_probe_manual_init(u8 oneline)
{
    u32 con = UART2_PROBE_CON_KEYS_OFF | BIT(7) | BIT(5) | BIT(4);

    if (oneline) {
        con |= BIT(6);
    }
    UART2CON = con & ~BIT(0);       // UTEN=0 下改配置
    UART2BAUD = (24000000UL / 115200) - 1;   // 手册 12-3：只写低 16 位 BAUD
    UART2CPND = BIT(15) | BIT(11) | BIT(10) | BIT(9) | BIT(8);
    UART2CON = con | BIT(0);        // UTEN=1
}

static void uart2_probe_poll_tx_rx(const char *tag)
{
    u32 i;
    u32 txd_idx = 0xffffffff;       // TXPND 1->0（写 DATA 后开始移位）
    u32 done_idx = 0xffffffff;      // TXPND 0->1（一字节完成）
    u32 rxpnd_idx = 0xffffffff;
    u32 rxpnd_con = 0;
    u32 bcnt_max = 0;
    u32 pe7_low = 0;
    u32 pe7_tog = 0;
    u32 pb1_low = 0;
    u32 pb1_tog = 0;
    u8 rx_data = 0;
    u8 prev8 = (UART2CON & BIT(8)) ? 1 : 0;
    u8 pe7_last = (GPIOE & BIT(7)) ? 1 : 0;
    u8 pb1_last = (GPIOB & BIT(1)) ? 1 : 0;

    UART2DATA = 0x55;
    for (i = 0; i < 600000; i++) {
        u32 con = UART2CON;
        u8 now8 = (con & BIT(8)) ? 1 : 0;
        u32 bcnt = (con >> 11) & 0xf;
        u8 pe7_now = (GPIOE & BIT(7)) ? 1 : 0;
        u8 pb1_now = (GPIOB & BIT(1)) ? 1 : 0;

        if (!pe7_now) {
            pe7_low++;
        }
        if (pe7_now != pe7_last) {
            pe7_tog++;
            pe7_last = pe7_now;
        }
        if (!pb1_now) {
            pb1_low++;
        }
        if (pb1_now != pb1_last) {
            pb1_tog++;
            pb1_last = pb1_now;
        }
        if (bcnt > bcnt_max) {
            bcnt_max = bcnt;
        }
        if (prev8 && !now8 && (txd_idx == 0xffffffff)) {
            txd_idx = i;
        }
        if (!prev8 && now8 && (done_idx == 0xffffffff)) {
            done_idx = i;
        }
        if ((rxpnd_idx == 0xffffffff) && (con & BIT(9))) {
            rxpnd_idx = i;
            rxpnd_con = con;
            rx_data = UART2DATA;
            UART2CPND = BIT(9);
        }
        prev8 = now8;
    }
    printf("[U2PROBE][%s] txd=%lu done=%lu rxpnd=%lu data=%02x bcnt_max=%lu rx4err=%lu pe7(low=%lu tog=%lu) pb1(low=%lu tog=%lu)\n",
           tag, txd_idx, done_idx, rxpnd_idx, rx_data, bcnt_max,
           (UART2CON >> 15) & 1, pe7_low, pe7_tog, pb1_low, pb1_tog);
}

static void uart2_self_suppress_probe(void)
{
    u32 mux_saved;
    u32 first_con = 0;
    u32 first_idx = 0;
    u8 data = 0;
    u8 seen;

    bsp_uart2_com_init(115200);     // 引脚/复用/时钟沿用驱动，CON/BAUD 随后按手册重配
    delay_ms(10);
    mux_saved = FUNCMCON2;
    UART2CPND = BIT(15) | BIT(11) | BIT(10) | BIT(9) | BIT(8);
    printf("[U2PROBE] init con=%08x baud=%08x mux=%08x\n",
           UART2CON, UART2BAUD, FUNCMCON2);

    // 正对照：ONELINE=0 手册双线配置下，GPIO PE7 发 0x55，UART2 只负责收
    u32 pe7_low = 0;
    u32 pe7_tog = 0;

    uart2_probe_manual_init(0);
    FUNCMCON2 &= ~(0x0f << 8);      // 解除 TX2->PE7，保留 RX2->PB1
    GPIOEDE |= BIT(7);
    GPIOEFEN &= ~BIT(7);
    GPIOEDIR &= ~BIT(7);            // DIR=0 为 GPIO 输出
    GPIOESET = BIT(7);
    delay_us(50);
    uart2_probe_bitbang_55(&pe7_low, &pe7_tog);
    seen = uart2_probe_wait_rx(200000, &first_con, &first_idx, &data);
    printf("[U2PROBE][GPIO] seen=%d data=%02x first=%lu con=%08x pe7(low=%lu tog=%lu)\n",
           seen, data, first_idx, first_con, pe7_low, pe7_tog);

    // 被测组 A：ONELINE=0（TX/RX separate）+ UART2 自己发。
    // 手册 12.3 User Guide 第 1 步 "Set IO in the correct direction"：v3 实测
    // separate 模式下 TX 脚为输入方向时 pad 不驱动（单线模式外设自控方向不受限），
    // 本组把 TX 脚改为输出方向验证。
    FUNCMCON2 = mux_saved;
    GPIOEFEN |= BIT(7);
    GPIOEDIR &= ~BIT(7);            // TX 脚输出方向
    uart2_probe_manual_init(0);
    delay_ms(2);
    uart2_probe_poll_tx_rx("U-SEP-OUT");

    // 对照组 A'：同配置但 TX 脚输入方向（复现 v3 的 pad 平直现象）
    UART2CPND = BIT(15) | BIT(11) | BIT(10) | BIT(9) | BIT(8);
    GPIOEDIR |= BIT(7);
    delay_ms(2);
    uart2_probe_poll_tx_rx("U-SEP-IN");

    // 被测组 B：ONELINE=1（one-line）同流程，同轮 A/B
    uart2_probe_manual_init(1);
    delay_ms(2);
    uart2_probe_poll_tx_rx("U-1LINE");

    printf("=== UART2 Self Suppress Probe Done ===\n");
}

#endif

#if SERIAL_MAX_BAUD_TEST_TPUT_EN

/*
 * 满吞吐回环：固定波特率下连续自发自发收、在线比对，测可达吞吐与误码。
 *   UART2 = 连续流（TX 128B 环形队列流控 + 逐字节排水比对）；
 *   HUART = SAFE（单块握手，规避库非环形连续块缺陷）与 B2B（背靠背 DMA，
 *   预期暴露"重武装窗口丢字节"库缺陷——如实区分库缺陷与 PHY 误码）。
 * 码型统一为 0~255 递增循环 + 按位置比对（非恒值）：丢字节/位错/块错位均可检出。
 */
#define TPUT_BLOCK      512
#define TPUT_RX_WIN     4096                // HUART 接收校验窗口（缓冲第 i 字节应为 i&0xFF）
#define TPUT_RX_TIMEOUT_MS  200

static u8 tput_rx[TPUT_RX_WIN] AT(.buf.serial_max);   // HUART 吞吐接收校验缓冲

static void tput_report(const char *tag, u32 ms, u32 tx, u32 rx, u32 err, s32 first_err)
{
    u32 kb;

    if (ms == 0) {
        ms = 1;
    }
    kb = (u32)((u64)rx * 1000 / ms / 1024);         // 实收吞吐 KB/s
    printf("[TPUT][%s] time=%lums tx=%lu rx=%lu err=%lu first_err=%ld tput=%luKB/s\n",
           tag, ms, tx, rx, err, first_err, kb);
}

#if SERIAL_MAX_BAUD_TEST_USE_UART2
/**
 * @brief 满吞吐（UART2）：256B 递增图案无缝循环发送，逐字节在线比对。
 */
static void tput_uart2_run(void)
{
    u32 start;
    u32 mark;
    u32 sent = 0;
    u32 recv = 0;
    u32 err = 0;
    s32 first_err = -1;
    u8 ch;

    for (ch = 0; ch < 255; ch++) {                  // 0..255 递增图案
        sm_buf[ch] = ch;
    }
    sm_buf[255] = 255;

    start = tick_get();
    mark = start;
    while (!tick_check_expire(start, SERIAL_MAX_BAUD_TEST_TPUT_MS)) {
        u32 off = sent & 0xFF;
        u32 chunk = 256 - off;

        if (chunk > 64) {
            chunk = 64;
        }
        sent += bsp_uart2_com_write(&sm_buf[off], (u16)chunk);
        bsp_uart2_com_process();
        while (bsp_uart2_com_get(&ch)) {
            if (ch != (u8)(recv & 0xFF)) {
                err++;
                if (first_err < 0) {
                    first_err = (s32)recv;
                }
            }
            recv++;
        }
        if (tick_check_expire(mark, 1000)) {
            mark = tick_get();
            printf("[TPUT][UART2] t=%lums sent=%lu recv=%lu err=%lu\n",
                   tick_get() - start, sent, recv, err);
        }
    }
    delay_us(500);                                  // 排空最后的在途字节
    bsp_uart2_com_process();
    while (bsp_uart2_com_get(&ch)) {
        if (ch != (u8)(recv & 0xFF)) {
            err++;
        }
        recv++;
    }
    tput_report("UART2", tick_get() - start, sent, recv, err, first_err);
}
#else
/**
 * @brief 满吞吐（HUART SAFE）：单块握手——发 512B 等 TX 完成、等 RX 块完成、
 *        比对后下一块（规避库非环形连续块缺陷）。
 */
static void tput_huart_safe(void)
{
    u32 start;
    u32 mark;
    u32 tx = 0;
    u32 rx = 0;
    u32 err = 0;
    u16 i;

    for (i = 0; i < TPUT_BLOCK; i++) {
        sm_buf[i] = (u8)i;                          // 递增图案（位置相关）
    }
    start = tick_get();
    mark = start;
    while (!tick_check_expire(start, SERIAL_MAX_BAUD_TEST_TPUT_MS)) {
        u32 wait_start;

        sm_state.rx_bytes = 0;
        sm_state.drop_cnt = 0;
        sm_state.new_data = 0;
        sm_tx_done = 0;
        huart_rxfifo_clear();
        huart_tx(sm_buf, TPUT_BLOCK);
        tx += TPUT_BLOCK;
        wait_start = tick_get();
        while (!sm_tx_done && !tick_check_expire(wait_start, 100)) {
        }
        wait_start = tick_get();
        while ((sm_state.rx_bytes < TPUT_BLOCK) &&
               !tick_check_expire(wait_start, TPUT_RX_TIMEOUT_MS)) {
        }
        rx += sm_state.rx_bytes;
        for (i = 0; i < (u16)sm_state.rx_bytes; i++) {
            if (tput_rx[i] != (u8)i) {              // 按位置比对：丢字节即错位可检出
                err++;
            }
        }
        if (tick_check_expire(mark, 1000)) {
            mark = tick_get();
            printf("[TPUT][HUART-SAFE] tx=%lu rx=%lu err=%lu\n", tx, rx, err);
        }
    }
    tput_report("HUART-SAFE", tick_get() - start, tx, rx, err, -1);
}

/**
 * @brief 满吞吐（HUART B2B）：TX 块完成后立即续发下一块（0x55 图案），RX 连续
 *        接收。tx/rx 差额即"重武装窗口丢字节"（库缺陷），非 0x55 字节即位/帧错。
 */
static void tput_huart_b2b(void)
{
    u32 start;
    u32 tx = 0;
    u32 err = 0;
    u16 i;

    for (i = 0; i < TPUT_BLOCK; i++) {
        sm_buf[i] = (u8)i;                          // 递增图案（位置相关）
    }
    sm_state.rx_bytes = 0;
    sm_state.drop_cnt = 0;
    sm_state.new_data = 0;
    sm_tx_done = 0;
    huart_rxfifo_clear();
    huart_tx(sm_buf, TPUT_BLOCK);
    tx += TPUT_BLOCK;

    start = tick_get();
    while (!tick_check_expire(start, SERIAL_MAX_BAUD_TEST_TPUT_MS)) {
        if (sm_tx_done) {
            sm_tx_done = 0;
            huart_tx(sm_buf, TPUT_BLOCK);
            tx += TPUT_BLOCK;
        }
        if (sm_state.new_data) {
            u32 end = sm_state.rx_bytes;
            u32 st0 = (end >= TPUT_BLOCK) ? end - TPUT_BLOCK : 0;

            sm_state.new_data = 0;
            for (i = (u16)st0; i < end; i++) {
                if (tput_rx[i] != (u8)(i & 0xFF)) {  // 缓冲第 i 个字节应为 i&0xFF
                    err++;
                }
            }
        }
    }
    {
        u32 kept = sm_state.rx_bytes;
        u32 lost = (tx > (kept + sm_state.drop_cnt)) ?
                   (tx - kept - sm_state.drop_cnt) : 0;

        printf("[TPUT][HUART-B2B] tx=%lu kept=%lu drop=%lu lost=%lu err=%lu\n",
               tx, kept, sm_state.drop_cnt, lost, err);
        tput_report("HUART-B2B", tick_get() - start, kept, kept, err, -1);
    }
}
#endif

/**
 * @brief 满吞吐回环入口：固定波特率，按外设执行对应吞吐段。
 */
static void serial_max_baud_test_tput(void)
{
    printf("\n=== Serial Max Baud Throughput (%s @ %lu) ===\n",
           PERIPH_NAME, (u32)SERIAL_MAX_BAUD_TEST_TPUT_BAUD);
    printf("[t] pattern=inc(w UART2 inc), %lums window, theoretical max = baud/10 B/s\n",
           (u32)SERIAL_MAX_BAUD_TEST_TPUT_MS);
    sm_rx_dst = tput_rx;
    sm_rx_cap = TPUT_RX_WIN;

    periph_init(SERIAL_MAX_BAUD_TEST_TPUT_BAUD);
    delay_ms(10);

#if SERIAL_MAX_BAUD_TEST_USE_UART2
    tput_uart2_run();
#else
    tput_huart_safe();
    // 复位后重配再跑 B2B（SAFE 结束后 FIFO/块状态复位，保证两段独立）
    periph_init(SERIAL_MAX_BAUD_TEST_TPUT_BAUD);
    delay_ms(10);
    tput_huart_b2b();
#endif

    printf("\n=== Serial Max Baud Throughput Done ===\n");
}

#endif /* SERIAL_MAX_BAUD_TEST_TPUT_EN */

#if SERIAL_MAX_BAUD_TEST_LOOPBACK_EN

/*
 * 统一帧式回环口径（两外设完全一致，与 huart_baud_test 帧回环同构）：
 *   512B 帧 × 5 码型（00/FF/55/AA/递增）× SERIAL_MAX_BAUD_TEST_LOOP_FRAMES 帧，
 *   逐帧逐字节比对；连续 8 帧收不满 512B 提前终止该码型。
 *   stop bit：UART2 驱动配 1 stop（SB2EN=0，与 HUART RX 行为对齐）。
 */
#define LOOP_FRAME_SIZE     512
#define LOOP_MODE_CNT       5
#define LOOP_TIMEOUT_MS     200
#define LOOP_MISS_ABORT     8

static void loopback_fill_pattern(u8 mode)
{
    u16 i;

    for (i = 0; i < LOOP_FRAME_SIZE; i++) {
        if (mode == 0) {
            sm_buf[i] = 0x00;
        } else if (mode == 1) {
            sm_buf[i] = 0xff;
        } else if (mode == 2) {
            sm_buf[i] = 0x55;
        } else if (mode == 3) {
            sm_buf[i] = 0xaa;
        } else {
            sm_buf[i] = (u8)i;
        }
    }
}

#if SERIAL_MAX_BAUD_TEST_USE_UART2
/**
 * @brief 发送一帧 512B 并等接收排空（边发边收 + 短排水），返回实收字节数。
 */
static u32 loopback_uart2_send_frame(void)
{
    u32 sent = 0;
    u32 guard;

    while (sent < LOOP_FRAME_SIZE) {
        u32 chunk = LOOP_FRAME_SIZE - sent;
        u16 written;

        if (chunk > UART2_TX_CHUNK) {
            chunk = UART2_TX_CHUNK;
        }
        written = bsp_uart2_com_write(&sm_buf[sent], (u16)chunk);
        sent += written;
        periph_pump();      // 发送期间持续排水 RX
    }
    guard = tick_get();
    while (!bsp_uart2_com_tx_idle() && !tick_check_expire(guard, 1000)) {
        periph_pump();
    }
    delay_us(200);          // 覆盖最后字节的 stop 位采样（任意档 >=2 字节时间）
    while (periph_pump()) {
    }
    return sm_state.rx_bytes;
}
#endif

/**
 * @brief 比较一帧，返回误码字节数。
 */
static u32 loopback_compare_frame(u32 got, u32 *err_bytes)
{
    u32 i;
    u32 fe = 0;

    for (i = 0; i < got; i++) {
        if (sm_loop_rx[i] != sm_buf[i]) {
            fe++;
        }
    }
    *err_bytes += fe + (LOOP_FRAME_SIZE - got);
    return fe;
}

/**
 * @brief 回环梯子：板内 TX 短接 RX，统一帧式口径自动爬梯（无适配器，PC 只看
 *        COM9 调试口记表）。每档 5 码型，PASS 需全部帧 512B 零误码。
 */
static void serial_max_baud_test_loopback_ladder(void)
{
    u8 baud_idx;
    u8 mode_idx;

    printf("\n=== Serial Max Baud Loopback (%s) Start ===\n", PERIPH_NAME);
    printf("[t] unified frame loopback: %lu frames x 512B x 5 modes per baud\n",
           (u32)SERIAL_MAX_BAUD_TEST_LOOP_FRAMES);
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

        for (mode_idx = 0; mode_idx < LOOP_MODE_CNT; mode_idx++) {
            u32 frame;
            u32 bad_frames = 0;
            u32 err_bytes = 0;
            u32 miss = 0;
            u32 last_got = 0;

            loopback_fill_pattern((u8)mode_idx);
            for (frame = 0; frame < SERIAL_MAX_BAUD_TEST_LOOP_FRAMES; frame++) {
                u32 got;
                u32 fe;

                sm_state.rx_bytes = 0;
                sm_state.drop_cnt = 0;
                sm_state.new_data = 0;
#if SERIAL_MAX_BAUD_TEST_USE_UART2
                got = loopback_uart2_send_frame();
#else
                {
                    u32 wait_start;

                    sm_tx_done = 0;
                    huart_tx(sm_buf, LOOP_FRAME_SIZE);
                    wait_start = tick_get();
                    while (!sm_tx_done && !tick_check_expire(wait_start, 100)) {
                    }
                    wait_start = tick_get();
                    while ((sm_state.rx_bytes < LOOP_FRAME_SIZE) &&
                           !tick_check_expire(wait_start, LOOP_TIMEOUT_MS)) {
                    }
                    got = sm_state.rx_bytes;
                }
#endif
                if (got != LOOP_FRAME_SIZE) {
                    bad_frames++;
                    miss++;
                    last_got = got;
                    if (miss >= LOOP_MISS_ABORT) {
                        break;
                    }
                    continue;
                }
                miss = 0;
                last_got = got;
                fe = loopback_compare_frame(got, &err_bytes);
                if (fe) {
                    bad_frames++;
                }
            }

            if (bad_frames == 0) {
                printf("  [mode%d] PASS (%lu frames)\n", mode_idx,
                       (u32)SERIAL_MAX_BAUD_TEST_LOOP_FRAMES);
            } else {
                printf("  [mode%d] FAIL, bad_frames=%lu err_bytes=%lu last_got=%lu\n",
                       mode_idx, bad_frames, err_bytes, last_got);
            }
        }
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
#elif SERIAL_MAX_BAUD_TEST_TPUT_EN
    serial_max_baud_test_tput();
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
