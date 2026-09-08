#ifndef _BSP_UART2_COM_H
#define _BSP_UART2_COM_H

typedef struct {
    u32 rx_byte_count;
    u32 rx_pending_count;
    u32 rx_irq_count;
    u32 rx_overflow_count;
    u32 tx_byte_count;
} uart2_com_stats_t;

void bsp_uart2_com_init(u32 baudrate);
void bsp_uart2_com_process(void);
u8 bsp_uart2_com_get(u8 *ch);
void bsp_uart2_com_get_stats(uart2_com_stats_t *stats);

#endif // _BSP_UART2_COM_H
