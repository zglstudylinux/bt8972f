#include "include.h"
#include "api.h"

#if HUART_COM_EN

/*
 * HUART 高速串口测试口：DMA 块接收 + 库回调搬入软件环形缓冲，主循环消费。
 * 收发用法照抄原厂 huart_audio_in_mix.c（定长块 rxbuf，回调内取数）和
 * vusb_test.c（回调内 huart_get_rxcnt() 取实际长度）；
 * 测试帧格式与 bsp_uart2_com.c 完全一致，主机脚本可直接复用。
 * 约束：HUART 是单外设，本模块使能期间 EQ_DBG_IN_UART 必须关闭；
 * 且与 UART2_COM 共用 PE7/PB1 接线，两者不能同时初始化。
 * 回调语义（收满 rxbuf_size 或线路空闲后触发、rxcnt 为本次块长）来自原厂
 * 用例源码推断，未在 PDF 中明文，见 docs/peripheral/huart_com_bringup.md。
 */

#define HUART_COM_RING_SIZE             1024
#define HUART_COM_RING_MASK             (HUART_COM_RING_SIZE - 1)
#define HUART_COM_ECHO_STAGE_SIZE       128
#define HUART_TX_TEST_FRAME_SIZE        72
#define HUART_TX_TEST_PAYLOAD_SIZE      64
#define HUART_TX_TEST_INTERVAL          100

/* 与 UART2 相同的接线：PC TX -> PB1(RX)，PC RX <- PE7(TX) */
#define HUART_COM_TX_PORT               HUART_TR_PE7
#define HUART_COM_RX_PORT               HUART_TR_PB1

typedef struct {
    volatile u16 rx_w_cnt;
    volatile u16 rx_r_cnt;
    u8 *rx_buf;
    volatile u8 tx_busy;
} huart_com_cb_t;

static huart_com_cb_t huart_com_cb;
static u8 huart_com_rx_ring[HUART_COM_RING_SIZE];
ALIGNED(4)
static u8 huart_com_dma_buf[HUART_COM_BLOCK_SIZE];
#if HUART_COM_RX_TEST_EN
static u8 huart_com_echo_stage[HUART_COM_ECHO_STAGE_SIZE];
#endif
static huart_com_stats_t huart_com_stats;

AT(.com_text.huart.com)
static void huart_com_rx_push_block(const u8 *buf, u16 len)
{
    u16 i;

    for (i = 0; i < len; i++) {
        if ((u16)(huart_com_cb.rx_w_cnt - huart_com_cb.rx_r_cnt) >= HUART_COM_RING_SIZE) {
            huart_com_stats.rx_overflow_count += (u32)(len - i);
            return;
        }
        huart_com_cb.rx_buf[huart_com_cb.rx_w_cnt & HUART_COM_RING_MASK] = buf[i];
        huart_com_cb.rx_w_cnt++;
    }
}

AT(.com_text.huart.com)
void huart_rx_done_cb(void)
{
    u16 cnt = (u16)huart_get_rxcnt();

    huart_com_stats.rx_block_count++;
    if (cnt > HUART_COM_BLOCK_SIZE) {
        cnt = HUART_COM_BLOCK_SIZE;
    }
    huart_com_rx_push_block(huart_com_dma_buf, cnt);
    huart_com_stats.rx_byte_count += cnt;
}

AT(.com_text.huart.com)
void huart_tx_done_cb(void)
{
    huart_com_cb.tx_busy = 0;
    huart_com_stats.tx_done_count++;
}

AT(.com_text.huart.com)
u8 bsp_huart_com_get(u8 *ch)
{
    if (huart_com_cb.rx_r_cnt == huart_com_cb.rx_w_cnt) {
        return 0;
    }

    *ch = huart_com_cb.rx_buf[huart_com_cb.rx_r_cnt & HUART_COM_RING_MASK];
    huart_com_cb.rx_r_cnt++;
    return 1;
}

AT(.com_text.huart.com)
u8 bsp_huart_com_tx_idle(void)
{
    return !huart_com_cb.tx_busy;
}

#if HUART_COM_RX_TEST_EN
AT(.com_text.huart.com)
static void huart_com_echo_process(void)
{
    u16 n = 0;
    u8 ch;

    if (huart_com_cb.tx_busy) {
        if (huart_com_cb.rx_r_cnt != huart_com_cb.rx_w_cnt) {
            huart_com_stats.tx_busy_skip++;
        }
        return;
    }

    while ((n < HUART_COM_ECHO_STAGE_SIZE) && bsp_huart_com_get(&ch)) {
        huart_com_echo_stage[n++] = ch;
    }
    if (n == 0) {
        return;
    }

    huart_com_cb.tx_busy = 1;
    huart_com_stats.tx_byte_count += n;
    huart_tx(huart_com_echo_stage, n);
}
#endif

#if HUART_COM_TX_TEST_EN
static u8 huart_com_tx_frame[HUART_TX_TEST_FRAME_SIZE];

static u16 huart_com_crc16(const u8 *buf, u8 len)
{
    u16 crc = 0xffff;
    u8 i;

    while (len--) {
        crc ^= *buf++;
        for (i = 0; i < 8; i++) {
            crc = (crc & 1) ? ((crc >> 1) ^ 0xa001) : (crc >> 1);
        }
    }
    return crc;
}

AT(.com_text.huart.com)
static void huart_com_tx_test_process(void)
{
    static u32 test_tick;
    static u16 sequence;
    u16 crc;
    u8 i;

    if (!tick_check_expire(test_tick, HUART_TX_TEST_INTERVAL)) {
        return;
    }
    if (huart_com_cb.tx_busy) {
        huart_com_stats.tx_busy_skip++;
        return;
    }
    test_tick = tick_get();

    /* huart_tx 为异步 DMA，帧缓冲必须静态，栈数组会在函数返回后被复用 */
    huart_com_tx_frame[0] = 0x55;
    huart_com_tx_frame[1] = 0xaa;
    huart_com_tx_frame[2] = 0x5a;
    huart_com_tx_frame[3] = 0xa5;
    huart_com_tx_frame[4] = (u8)sequence;
    huart_com_tx_frame[5] = (u8)(sequence >> 8);
    for (i = 0; i < HUART_TX_TEST_PAYLOAD_SIZE; i++) {
        huart_com_tx_frame[6 + i] = (u8)(i + sequence);
    }
    crc = huart_com_crc16(huart_com_tx_frame, HUART_TX_TEST_FRAME_SIZE - 2);
    huart_com_tx_frame[HUART_TX_TEST_FRAME_SIZE - 2] = (u8)crc;
    huart_com_tx_frame[HUART_TX_TEST_FRAME_SIZE - 1] = (u8)(crc >> 8);

    huart_com_cb.tx_busy = 1;
    huart_tx(huart_com_tx_frame, sizeof(huart_com_tx_frame));
    huart_com_stats.tx_byte_count += sizeof(huart_com_tx_frame);
    sequence++;
}
#endif

void bsp_huart_com_init(u32 baudrate)
{
    huart_t huart0;

    if (baudrate == 0) {
        printf("huart com invalid baud=%d\n", (int)baudrate);
        return;
    }

    memset(&huart_com_cb, 0, sizeof(huart_com_cb));
    memset(&huart_com_stats, 0, sizeof(huart_com_stats));
    huart_com_cb.rx_buf = huart_com_rx_ring;
    memset(huart_com_dma_buf, 0, sizeof(huart_com_dma_buf));

    memset(&huart0, 0x00, sizeof(huart0));
    huart0.rx_port = HUART_COM_RX_PORT;
    huart0.tx_port = HUART_COM_TX_PORT;
    huart0.rxisr_en = 1;
    huart0.txisr_en = 1;
    huart0.rxbuf = huart_com_dma_buf;
    huart0.rxbuf_size = HUART_COM_BLOCK_SIZE;
    huart_init(&huart0, baudrate);

    printf("huart com ready: TX=PE7 RX=PB1 baud=%d block=%d rx_test=%d tx_test=%d\n",
           (int)baudrate, HUART_COM_BLOCK_SIZE, HUART_COM_RX_TEST_EN,
           HUART_COM_TX_TEST_EN);
}

void bsp_huart_com_get_stats(huart_com_stats_t *stats)
{
    *stats = huart_com_stats;
}

AT(.com_text.huart.com)
void bsp_huart_com_process(void)
{
    if (huart_com_cb.rx_r_cnt != huart_com_cb.rx_w_cnt) {
        reset_sleep_delay();
        reset_pwroff_delay();
    }

#if HUART_COM_RX_TEST_EN
    huart_com_echo_process();
#endif

#if HUART_COM_TX_TEST_EN
    huart_com_tx_test_process();
#endif

#if HUART_COM_RX_TEST_EN || HUART_COM_TX_TEST_EN
    {
        static u32 diag_tick;

        if (tick_check_expire(diag_tick, 2000)) {
            diag_tick = tick_get();
            huart_com_stats.rxcnt_snapshot = huart_get_rxcnt();
            printf("[huart] blk=%d rx=%d rx_ovf=%d tx=%d tx_done=%d tx_skip=%d rxcnt=%d\n",
                   (int)huart_com_stats.rx_block_count,
                   (int)huart_com_stats.rx_byte_count,
                   (int)huart_com_stats.rx_overflow_count,
                   (int)huart_com_stats.tx_byte_count,
                   (int)huart_com_stats.tx_done_count,
                   (int)huart_com_stats.tx_busy_skip,
                   (int)huart_com_stats.rxcnt_snapshot);
        }
    }
#endif
}

#endif /* HUART_COM_EN */
