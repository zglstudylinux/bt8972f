#include "include.h"

#if UART2_COM_EN

#define UART2_COM_BUF_SIZE              128
#define UART2_COM_BUF_MASK              (UART2_COM_BUF_SIZE - 1)
#define UART2_XOSC_HZ                   24000000UL

#define TX2MAP_PE7                      (1 << 8)
#define RX2MAP_PB1                      (2 << 12)

typedef struct {
    volatile u16 w_cnt;
    volatile u16 r_cnt;
    u8 *buf;
} uart2_com_cb_t;

static uart2_com_cb_t uart2_com_cb;
static u8 uart2_com_rx_buf[UART2_COM_BUF_SIZE];
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
#if UART1_EN
    bsp_uart1_irq_process();
#endif
    uart2_com_stats.rx_irq_count++;
    uart2_rx_read();
}
#endif

#if UART2_COM_RX_TEST_EN
static u8 uart2_com_tx_busy;

AT(.com_text.uart2.com)
static void uart2_com_echo_process(void)
{
    u8 ch;

    if (uart2_com_tx_busy && !(UART2CON & BIT(8))) {
        return;
    }
    if (!bsp_uart2_com_get(&ch)) {
        uart2_com_tx_busy = 0;
        return;
    }

    UART2DATA = ch;
    uart2_com_tx_busy = 1;
    uart2_com_stats.tx_byte_count++;
}
#endif

AT(.com_text.uart2.com)
static void uart2_com_rx_push(u8 data)
{
    if ((u16)(uart2_com_cb.w_cnt - uart2_com_cb.r_cnt) >= UART2_COM_BUF_SIZE) {
        uart2_com_stats.rx_overflow_count++;
        return;
    }

    uart2_com_cb.buf[uart2_com_cb.w_cnt & UART2_COM_BUF_MASK] = data;
    uart2_com_cb.w_cnt++;
    uart2_com_stats.rx_byte_count++;
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

    memset(&uart2_com_cb, 0, sizeof(uart2_com_cb));
    memset(&uart2_com_stats, 0, sizeof(uart2_com_stats));
    uart2_com_cb.buf = uart2_com_rx_buf;

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
    if (uart2_com_cb.r_cnt == uart2_com_cb.w_cnt) {
        return 0;
    }

    *ch = uart2_com_cb.buf[uart2_com_cb.r_cnt & UART2_COM_BUF_MASK];
    uart2_com_cb.r_cnt++;
    return 1;
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

    {
        static u32 diag_tick;

        if (tick_check_expire(diag_tick, 2000)) {
            diag_tick = tick_get();
            printf("[uart2] rx=%d pending=%d irq=%d overflow=%d tx=%d con=%08x cpnd=%08x baud=%08x mux=%08x\n",
                   (int)uart2_com_stats.rx_byte_count,
                   (int)uart2_com_stats.rx_pending_count,
                   (int)uart2_com_stats.rx_irq_count,
                   (int)uart2_com_stats.rx_overflow_count,
                   (int)uart2_com_stats.tx_byte_count,
                   UART2CON, UART2CPND, UART2BAUD, FUNCMCON2);
        }
    }
#endif
}

#endif /* UART2_COM_EN */
