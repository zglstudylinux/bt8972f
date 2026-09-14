#include "include.h"
#include "api.h"
#include "huart_baud_test.h"

#if HUART_BAUD_TEST_EN

#if HUART_BAUD_TEST_MODE == HUART_BAUD_TEST_MODE_LOOPBACK
static const u32 baud_rates[] = {
    9500000,
};
#else
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

#define BAUD_CNT            (sizeof(baud_rates) / sizeof(baud_rates[0]))
#define MODE_CNT            5
#define DUAL_TX_CHUNK       HUART_BAUD_TEST_BUF_SIZE
#define DUAL_ECHO_BUF_SIZE  HUART_BAUD_TEST_DUAL_BUF_SIZE
#define ERR_LOG_MAX         8

typedef enum {
    HUART_BAUD_TEST_IDLE = 0,
    HUART_BAUD_TEST_LOOPBACK,
    HUART_BAUD_TEST_DUAL,
} huart_baud_test_state_t;

typedef struct {
    u32 baud;
    u8 mode;
    u16 index;
    u8 send;
    u8 recv;
} huart_baud_err_t;

typedef struct {
    volatile u32 drop_cnt;
    volatile u32 rx_bytes;
    volatile u8 new_data;
} huart_dual_state_t;

static volatile huart_baud_test_state_t huart_test_state = HUART_BAUD_TEST_IDLE;
static volatile u8 loopback_rx_done = 0;
static volatile u8 dual_tx_done = 0;
static huart_dual_state_t dual_state;
static u8 dual_echo_buf[DUAL_ECHO_BUF_SIZE] AT(.buf.huart_dual);
static u8 dual_rx_buf[HUART_BAUD_TEST_BUF_SIZE] AT(.buf.huart_dual);
static u8 loop_tx_buf[HUART_BAUD_TEST_BUF_SIZE] AT(.buf.huart_dual);
static u8 loop_rx_buf[HUART_BAUD_TEST_BUF_SIZE] AT(.buf.huart_dual);
static huart_baud_err_t huart_err_log[ERR_LOG_MAX];
static u8 huart_err_log_cnt;

static void fill_pattern(u8 *buf, u16 len, u8 mode)
{
    u16 i;

    for (i = 0; i < len; i++) {
        if (mode == 0) {
            buf[i] = 0x00;
        } else if (mode == 1) {
            buf[i] = 0xff;
        } else if (mode == 2) {
            buf[i] = 0x55;
        } else if (mode == 3) {
            buf[i] = 0xaa;
        } else {
            buf[i] = (u8)i;
        }
    }
}

static void err_log_clear(void)
{
    huart_err_log_cnt = 0;
}

static void err_log_add(u32 baud, u8 mode, u16 index, u8 send, u8 recv)
{
    huart_baud_err_t *log;

    if (huart_err_log_cnt >= ERR_LOG_MAX) {
        return;
    }
    log = &huart_err_log[huart_err_log_cnt++];
    log->baud = baud;
    log->mode = mode;
    log->index = index;
    log->send = send;
    log->recv = recv;
}

static void err_log_flush(void)
{
    u8 i;

    for (i = 0; i < huart_err_log_cnt; i++) {
        printf("[ERR][%lu][mode%d][%d] send=0x%02X recv=0x%02X\n",
               huart_err_log[i].baud, huart_err_log[i].mode,
               huart_err_log[i].index, huart_err_log[i].send,
               huart_err_log[i].recv);
    }
    huart_err_log_cnt = 0;
}

static u16 check_data(const u8 *tx, const u8 *rx, u16 len, u32 baud, u8 mode)
{
    u16 i;
    u16 err = 0;

    for (i = 0; i < len; i++) {
        if (tx[i] != rx[i]) {
            if (err == 0) {
                err_log_add(baud, mode, i, tx[i], rx[i]);
            }
            err++;
        }
    }
    return err;
}

AT(.com_huart.text)
u8 huart_baud_test_rx_done_cb(void)
{
    if (huart_test_state == HUART_BAUD_TEST_LOOPBACK) {
        loopback_rx_done = 1;
        return 1;
    }

    if (huart_test_state == HUART_BAUD_TEST_DUAL) {
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
        return 1;
    }
    return 0;
}

AT(.com_huart.text)
void huart_baud_test_tx_done_hook(void)
{
    dual_tx_done = 1;
}

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
#if HUART_BAUD_TEST_MODE == HUART_BAUD_TEST_MODE_LOOPBACK
    huart0.rxbuf = loop_rx_buf;
    huart0.rxbuf_size = HUART_BAUD_TEST_BUF_SIZE;
#else
    huart0.rxbuf = dual_rx_buf;
    huart0.rxbuf_size = HUART_BAUD_TEST_BUF_SIZE;
#endif
    huart_init(&huart0, baud);
}

static void dual_echo_send(const u8 *buf, u32 len)
{
    u32 off = 0;
    u8 started = 0;

    while (off < len) {
        u32 chunk = len - off;
        u32 start;

        if (chunk > DUAL_TX_CHUNK) {
            chunk = DUAL_TX_CHUNK;
        }
        if (started) {
            start = tick_get();
            while (!dual_tx_done && !tick_check_expire(start, 100)) {
            }
        }
        dual_tx_done = 0;
        huart_tx(&buf[off], chunk);
        started = 1;
        off += chunk;
    }
}

static void huart_baud_test_loopback(void)
{
    u8 baud_idx;
    u8 mode_idx;

    printf("\n=== HUART Frame Loopback (TX=%s RX=%s, %lu frames/mode, %dB/frame) ===\n",
#if HUART_BAUD_TEST_PINSET == HUART_BAUD_TEST_PINSET_PE7_PB1
           "PE7", "PB1",
#else
           "PB4", "PB3",
#endif
           (u32)HUART_BAUD_TEST_FRAMES, HUART_BAUD_TEST_BUF_SIZE);
    huart_test_state = HUART_BAUD_TEST_LOOPBACK;

    for (baud_idx = 0; baud_idx < BAUD_CNT; baud_idx++) {
        u32 baud = baud_rates[baud_idx];

        if (baud_idx != 0) {
            huart_baud_test_init(baud);
        }
        delay_ms(10);
        printf("\n[Baud %lu]\n", baud);

        for (mode_idx = 0; mode_idx < MODE_CNT; mode_idx++) {
            u32 frame;
            u32 err_total = 0;
            u32 frame_errors = 0;
            u32 timeout_cnt = 0;

            fill_pattern(loop_tx_buf, HUART_BAUD_TEST_BUF_SIZE, mode_idx);
            err_log_clear();
            for (frame = 0; frame < HUART_BAUD_TEST_FRAMES; frame++) {
                u32 start;
                u16 frame_err;

                loopback_rx_done = 0;
                huart_rxfifo_clear();
                memset(loop_rx_buf, 0, HUART_BAUD_TEST_BUF_SIZE);
                huart_tx(loop_tx_buf, HUART_BAUD_TEST_BUF_SIZE);
                start = tick_get();
                while (!loopback_rx_done &&
                       !tick_check_expire(start, HUART_BAUD_TEST_FRAME_TIMEOUT_MS)) {
                }
                if (!loopback_rx_done) {
                    timeout_cnt++;
                    err_total += HUART_BAUD_TEST_BUF_SIZE;
                    frame_errors++;
                    if (timeout_cnt >= HUART_BAUD_TEST_TIMEOUT_ABORT) {
                        break;
                    }
                    continue;
                }
                frame_err = check_data(loop_tx_buf, loop_rx_buf,
                                       HUART_BAUD_TEST_BUF_SIZE, baud, mode_idx);
                if (frame_err) {
                    err_total += frame_err;
                    frame_errors++;
                }
            }
            if (err_total == 0) {
                printf("  [mode%d] PASS (%lu frames)\n", mode_idx,
                       (u32)HUART_BAUD_TEST_FRAMES);
            } else {
                printf("  [mode%d] FAIL, total_err=%lu, bad_frames=%lu, timeout=%lu\n",
                       mode_idx, err_total, frame_errors, timeout_cnt);
            }
            err_log_flush();
        }
    }
    huart_test_state = HUART_BAUD_TEST_IDLE;
    printf("\n=== HUART Frame Loopback Done ===\n");
}

static void huart_baud_test_dual_loop(void)
{
    u8 baud_idx = 0;
    u8 data_received = 0;
    u32 last_activity_tick;

    printf("\n=== HUART Dual Test (Echo) Start ===\n");
    huart_test_state = HUART_BAUD_TEST_DUAL;
    dual_state.drop_cnt = 0;
    dual_state.rx_bytes = 0;
    dual_state.new_data = 0;
    dual_tx_done = 0;
    huart_rxfifo_clear();
    delay_ms(10);
    printf("[Baud %lu] waiting PC data...\n", baud_rates[baud_idx]);
    last_activity_tick = tick_get();

    while (1) {
        if (dual_state.new_data) {
            dual_state.new_data = 0;
            data_received = 1;
            last_activity_tick = tick_get();
        }
        if (data_received &&
            tick_check_expire(last_activity_tick, HUART_BAUD_TEST_DUAL_IDLE_TIMEOUT_MS)) {
            printf("  [rx] %lu bytes drop=%lu\n", dual_state.rx_bytes, dual_state.drop_cnt);
            dual_echo_send(dual_echo_buf, dual_state.rx_bytes);
            if ((baud_idx + 1) >= BAUD_CNT) {
                break;
            }
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
        }
    }
    huart_test_state = HUART_BAUD_TEST_IDLE;
    printf("\n=== HUART Dual Test (Echo) Done ===\n");
}

void huart_baud_test_start(void)
{
    WDT_DIS();
    RTC_WDT_DIS();
    huart_baud_test_init(baud_rates[0]);
#if HUART_BAUD_TEST_MODE == HUART_BAUD_TEST_MODE_LOOPBACK
    huart_baud_test_loopback();
#else
    huart_baud_test_dual_loop();
#endif
    WDT_CLR();
    WDT_EN();
    RTC_WDT_EN();
}

#endif /* HUART_BAUD_TEST_EN */
