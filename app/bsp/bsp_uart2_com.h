#ifndef _BSP_UART2_COM_H
#define _BSP_UART2_COM_H

//UART2普通串口测试口(TX=PE7,RX=PB1)，宏开关见config.h: UART2_COM_EN
void bsp_uart2_com_init(u32 baudrate);
void bsp_uart2_com_process(void);
u8 bsp_uart2_com_get(u8 *ch);

#endif // _BSP_UART2_COM_H
