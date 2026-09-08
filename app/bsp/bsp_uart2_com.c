#include "include.h"

#if UART2_COM_EN

#define UART2_COM_RX_BUF_SIZE           128
#define UART2_COM_RX_BUF_MASK           (UART2_COM_RX_BUF_SIZE - 1)
#define UART2_COM_TX_BUF_SIZE           128
#define UART2_COM_TX_BUF_MASK           (UART2_COM_TX_BUF_SIZE - 1)
#define UART2_XOSC_HZ                   24000000UL

#define TX2MAP_PE7                      (1 << 8)
#define RX2MAP_PB1                      (2 << 12)

typedef struct {
    volatile u16 rx_w_cnt;
    volatile u16 rx_r_cnt;
    volatile u16 tx_w_cnt;
    volatile u16 tx_r_cnt;
    u8 *rx_buf;
    u8 *tx_buf;
    u8 tx_busy;
} uart2_com_cb_t;

static uart2_com_cb_t uart2_com_cb;
static u8 uart2_com_rx_buf[UART2_COM_RX_BUF_SIZE];
static u8 uart2_com_tx_buf[UART2_COM_TX_BUF_SIZE];
static uart2_com_stats_t uart2_com_stats;

static void uart2_com_rx_push(u8 data);

AT(.com_text.uart2.com)
static u8 uart2_rx_read(void)
{
    u8 data;

    if (!(UART2CON & BIT(9))) {
        return 0;
    }

    data = UART2DATA;
    uart2_com_stats.rx_pending_count++;
    uart2_com_rx_push(data);
    UART2CPND = BIT(9);
    return 1;
}

#if UART2_COM_RX_IRQ_TEST_EN
AT(.com_text.uart2.com)
static void uart2_com_irq(void)
{
    u8 received;

#if UART1_EN
    bsp_uart1_irq_process();
#endif
    received = uart2_rx_read();
    if (received) {
        uart2_com_stats.rx_irq_count++;
    }
}
#endif

#if UART2_COM_RX_TEST_EN
AT(.com_text.uart2.com)
static void uart2_com_echo_process(void)
{
    u8 ch;

    while (uart2_com_cb.rx_r_cnt != uart2_com_cb.rx_w_cnt) {
        if ((u16)(uart2_com_cb.tx_w_cnt - uart2_com_cb.tx_r_cnt) >= UART2_COM_TX_BUF_SIZE) {
            break;
        }
        bsp_uart2_com_get(&ch);
        bsp_uart2_com_put(ch);
    }
}
#endif

AT(.com_text.uart2.com)
static void uart2_com_rx_push(u8 data)
{
    if ((u16)(uart2_com_cb.rx_w_cnt - uart2_com_cb.rx_r_cnt) >= UART2_COM_RX_BUF_SIZE) {
        uart2_com_stats.rx_overflow_count++;
        return;
    }

    uart2_com_cb.rx_buf[uart2_com_cb.rx_w_cnt & UART2_COM_RX_BUF_MASK] = data;
    uart2_com_cb.rx_w_cnt++;
    uart2_com_stats.rx_byte_count++;
}

AT(.com_text.uart2.com)
static void uart2_com_tx_process(void)
{
    if (uart2_com_cb.tx_busy) {
        if (!(UART2CON & BIT(8))) {
            return;
        }
        uart2_com_cb.tx_busy = 0;
        uart2_com_stats.tx_complete_count++;
    }

    if (uart2_com_cb.tx_r_cnt == uart2_com_cb.tx_w_cnt) {
        return;
    }

    UART2DATA = uart2_com_cb.tx_buf[uart2_com_cb.tx_r_cnt & UART2_COM_TX_BUF_MASK];
    uart2_com_cb.tx_r_cnt++;
    uart2_com_cb.tx_busy = 1;
}

AT(.com_text.uart2.com)
static u8 uart2_rx_poll(void)
{
#if UART2_COM_RX_IRQ_TEST_EN
    return 0;
#else
    return uart2_rx_read();
#endif
}

void bsp_uart2_com_init(u32 baudrate)
{
    u32 baud;

    if ((baudrate == 0) || (baudrate > UART2_XOSC_HZ)) {
        printf("uart2 invalid baud=%d\n", (int)baudrate);
        return;
    }

    memset(&uart2_com_cb, 0, sizeof(uart2_com_cb));
    memset(&uart2_com_stats, 0, sizeof(uart2_com_stats));
    uart2_com_cb.rx_buf = uart2_com_rx_buf;
    uart2_com_cb.tx_buf = uart2_com_tx_buf;

    GPIOEDE  |= BIT(7);
    GPIOEPU  |= BIT(7);
    GPIOEDIR |= BIT(7);
    GPIOEFEN |= BIT(7);

    GPIOBDE  |= BIT(1);
    GPIOBPU  |= BIT(1);
    GPIOBDIR |= BIT(1);
    GPIOBFEN |= BIT(1);

    FUNCMCON0 = (FUNCMCON0 & 0xff00ffff) | (0xff << 16);
    PWRCON0 &= ~BIT(29);
    PWRCON0 |= BIT(30);
    CLKCON1 &= ~(BIT(23) | BIT(24));
    CLKCON1 |= BIT(24);
    CLKGAT0 |= BIT(8);

    baud = ((UART2_XOSC_HZ + (baudrate / 2)) / baudrate) - 1;
    UART2CON = 0;
    UART2BAUD = (baud << 16) | baud;

    FUNCMCON2 = (FUNCMCON2 & ~0xff00) | TX2MAP_PE7 | RX2MAP_PB1;
    UART2CON = BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(0);
    UART2CON |= 0xaaa << 16;
    UART2CPND = BIT(8) | BIT(9);
    UART2CPND |= BIT(10) | BIT(15);

#if UART2_COM_RX_IRQ_TEST_EN
    UART2CON |= BIT(2);
    sys_irq_init(IRQ_UART_VECTOR, 0, uart2_com_irq);
#endif

    printf("uart2 com ready: TX=PE7 RX=PB1 baud=%d test=%d irq=%d\n",
           (int)baudrate, UART2_COM_RX_TEST_EN, UART2_COM_RX_IRQ_TEST_EN);
}

AT(.com_text.uart2.com)
u8 bsp_uart2_com_get(u8 *ch)
{
    if (uart2_com_cb.rx_r_cnt == uart2_com_cb.rx_w_cnt) {
        return 0;
    }

    *ch = uart2_com_cb.rx_buf[uart2_com_cb.rx_r_cnt & UART2_COM_RX_BUF_MASK];
    uart2_com_cb.rx_r_cnt++;
    return 1;
}

AT(.com_text.uart2.com)
u8 bsp_uart2_com_put(u8 ch)
{
    if ((u16)(uart2_com_cb.tx_w_cnt - uart2_com_cb.tx_r_cnt) >= UART2_COM_TX_BUF_SIZE) {
        uart2_com_stats.tx_overflow_count++;
        return 0;
    }

    uart2_com_cb.tx_buf[uart2_com_cb.tx_w_cnt & UART2_COM_TX_BUF_MASK] = ch;
    uart2_com_cb.tx_w_cnt++;
    uart2_com_stats.tx_queued_count++;
    return 1;
}

u16 bsp_uart2_com_write(const u8 *buf, u16 len)
{
    u16 written = 0;

    while ((written < len) && bsp_uart2_com_put(buf[written])) {
        written++;
    }
    return written;
}

u8 bsp_uart2_com_tx_idle(void)
{
    if (uart2_com_cb.tx_busy && (UART2CON & BIT(8))) {
        uart2_com_cb.tx_busy = 0;
        uart2_com_stats.tx_complete_count++;
    }
    return !uart2_com_cb.tx_busy &&
           (uart2_com_cb.tx_r_cnt == uart2_com_cb.tx_w_cnt);
}

void bsp_uart2_com_get_stats(uart2_com_stats_t *stats)
{
    *stats = uart2_com_stats;
}

AT(.com_text.uart2.com)
void bsp_uart2_com_process(void)
{
    u8 received;

    received = uart2_rx_poll();
    if (received) {
        reset_sleep_delay();
        reset_pwroff_delay();
    }

#if UART2_COM_RX_TEST_EN
    uart2_com_echo_process();
#endif
    uart2_com_tx_process();

#if UART2_COM_RX_TEST_EN
    {
        static u32 diag_tick;

        if (tick_check_expire(diag_tick, 2000)) {
            diag_tick = tick_get();
            printf("[uart2] rx=%d pending=%d irq=%d rx_ovf=%d tx_q=%d tx_done=%d tx_ovf=%d con=%08x cpnd=%08x baud=%08x mux=%08x\n",
                   (int)uart2_com_stats.rx_byte_count,
                   (int)uart2_com_stats.rx_pending_count,
                   (int)uart2_com_stats.rx_irq_count,
                   (int)uart2_com_stats.rx_overflow_count,
                   (int)uart2_com_stats.tx_queued_count,
                   (int)uart2_com_stats.tx_complete_count,
                   (int)uart2_com_stats.tx_overflow_count,
                   UART2CON, UART2CPND, UART2BAUD, FUNCMCON2);
        }
    }
#endif
}

#endif /* UART2_COM_EN */
