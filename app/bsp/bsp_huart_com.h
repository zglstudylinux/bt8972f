#ifndef _BSP_HUART_COM_H
#define _BSP_HUART_COM_H

typedef struct {
    u32 rx_block_count;     /* rx 块回调次数 */
    u32 rx_byte_count;      /* 累计收到的字节数 */
    u32 rx_overflow_count;  /* 软件环形缓冲满丢弃的字节数 */
    u32 tx_byte_count;      /* huart_tx 累计提交字节数 */
    u32 tx_done_count;      /* tx 完成回调次数 */
    u32 tx_busy_skip;       /* tx 忙期间带数据跳过的轮询次数 */
    u32 rxcnt_snapshot;     /* 最近一次诊断时 huart_get_rxcnt() 快照 */
} huart_com_stats_t;

#if HUART_COM_EN
void bsp_huart_com_init(u32 baudrate);
void bsp_huart_com_process(void);
u8 bsp_huart_com_get(u8 *ch);
u8 bsp_huart_com_tx_idle(void);
void bsp_huart_com_get_stats(huart_com_stats_t *stats);
#endif

#endif // _BSP_HUART_COM_H
