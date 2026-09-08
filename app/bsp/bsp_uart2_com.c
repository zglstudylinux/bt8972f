#include "include.h"

#if UART2_COM_EN

//===== BT8970 普通串口驱动（uart2 收发一体版）=================================
//TX：uart2 @ PE7（TX2-G1），波特率=config.h 的 UART2_COM_BAUD，实测无码上限24M
//RX：uart2 @ PB1（RX2-G2），同波特率，盲读DATA保持寄存器变化检测
//
//★本轮假设（待验证）：uart2 的 DATA 与 uart0 同为"保持寄存器"语义——BIT9粘滞/
//  中断不挂，但寄存器随字节更新。此前"uart2 RX全矩阵失败"的判定全部依赖ISR/
//  BIT9，从未用盲读变化检测验证，可能是检测方法假象。
//
//uart0 教训（不再触碰）：重配115200+读GPIOB会干扰其接收（轨迹实锤：线上有帧、
//  寄存器死）；使能态写CPND/BAUD挂总线。本版完全不碰uart0，console保持1.5M。
//
//uart2 关键经验（详见docs/peripheral/uart2_tx_bringup.md）：
//  先写DATA后等CON BIT(8)；写BAUD前必须UART2CON=0；重写CON后必须补KEY(0xaaa<<16)；
//  映射域写0不清除(0xF=禁用)；CON未文档化位(BIT1/BIT3)禁止试探(实测触发复位)
//=============================================================================

#define UART2_COM_BUF_MASK              0x7f            //128字节环形缓冲

//UART2引脚复用（手册5.6.4：TX2MAP=FUNCMCON2[11:8], RX2MAP=FUNCMCON2[15:12]）
#define TX2MAP_PE7                      (1 << 8)        //G1 uart2 tx: PE7
#define RX2MAP_PB1                      (2 << 12)       //G2 uart2 rx: PB1

typedef struct {
    volatile u8 w_cnt;
    volatile u8 r_cnt;
    u8 resv0;
    u8 resv1;
    u8 *buf;
} uart2_com_cb_t;

static uart2_com_cb_t uart2_com_cb;
static u8 uart2_com_rx_buf[UART2_COM_BUF_MASK + 1];

//RX状态（保持寄存器变化检测）
static u8 uart2_last_byte;                                //最近推送字节
static u32 uart2_ff_cnt;                                  //空读0xFF丢弃计数

//发送一个字节：先写DATA后等完成（TX完成标志复位值为0，不能先等后写），带超时
AT(.com_text.uart2.com)
static u8 uart2_com_tx_byte(u8 ch)
{
    u32 tmo = 200000;
    UART2DATA = ch;
    while (!(UART2CON & BIT(8))) {
        if (--tmo == 0) return 0;
    }
    return 1;
}

static u8 uart2_com_puts(const char *s)
{
    while (*s) {
        if (!uart2_com_tx_byte(*s++)) {
            return 0;
        }
    }
    return 1;
}

//u8转两位hex字符串，返回静态缓冲
static const char *uart2_com_hex(u8 v)
{
    static char buf[4];
    const char hex[] = "0123456789abcdef";
    buf[0] = hex[v >> 4];
    buf[1] = hex[v & 0x0f];
    buf[2] = 0;
    return buf;
}

//u32转十进制字符串，返回静态缓冲
static const char *uart2_com_dec(u32 v)
{
    static char buf[11];
    char *p = buf + 10;
    *p = 0;
    do {
        *--p = '0' + v % 10;
        v /= 10;
    } while (v);
    return p;
}

//u32转十进制字符串（写入调用方缓冲），返回长度
static u8 uart2_com_u32_to_str(char *p, u32 v)
{
    char tmp[10];
    u8 n = 0, len = 0;
    do {
        tmp[n++] = '0' + v % 10;
        v /= 10;
    } while (v);
    while (n) {
        *p++ = tmp[--n];
        len++;
    }
    *p = 0;
    return len;
}

//RX轮询：循环顶无条件清挂起/错误标志（BIT8~15） + 变化检测兜底 —— 最终定版
//实测定论：CON BIT(9)=电平语义（窗口内保持置位，非每字节脉冲）——事件法会重复推送同字节
//（b→63×4、d→64×6 实锤），已回退；重复字节折叠为保持寄存器+电平标志的固有局限
//该寄存器写安全（官方init反复写），循环顶清挂起+错误标志根治了RX节流/锁死
AT(.com_text.uart2.com)
static void uart2_rx_poll(void)
{
    u8 data;
    UART2CPND = 0xFF00;                                 //循环顶全清BIT8~15（挂起+错误标志）
    for (u8 n = 0; n < 8; n++) {
        data = UART2DATA;
        if (data == 0xFF || data == uart2_last_byte) {  //空读标记/保持寄存器驻留
            continue;
        }
        uart2_last_byte = data;
        uart2_com_cb.buf[uart2_com_cb.w_cnt & UART2_COM_BUF_MASK] = data;
        uart2_com_cb.w_cnt++;
    }
}

void bsp_uart2_com_init(u32 baudrate)
{
    u32 baud;

    memset(&uart2_com_cb, 0, sizeof(uart2_com_cb));
    uart2_com_cb.buf = uart2_com_rx_buf;

    //TX引脚PE7（仿system.c uart0 PE7 模板）
    GPIOEDE  |= BIT(7);
    GPIOEPU  |= BIT(7);
    GPIOEDIR |= BIT(7);
    GPIOEFEN |= BIT(7);

    //RX引脚PB1（仿bsp_uart.c的uart1 RX模板）
    GPIOBDE  |= BIT(1);
    GPIOBPU  |= BIT(1);
    GPIOBDIR |= BIT(1);
    GPIOBFEN |= BIT(1);

    //官方uart2_key_mode时钟/电源序列（反汇编逐条解码）
    FUNCMCON0 = (FUNCMCON0 & 0xff00ffff) | (0xff << 16);
    PWRCON0 &= ~BIT(29);
    PWRCON0 |= BIT(30);
    CLKCON1 &= ~(BIT(23) | BIT(24));                    //uart2_sel=CLKCON1[24:23]
    CLKCON1 |= BIT(24);                                 //选xosc时钟源
    CLKGAT0 |= BIT(8);                                  //uart2_clken

    baud = ((get_sysclk_nhz() + (baudrate / 2)) / baudrate) - 1;
    UART2CON = 0;                                       //配置前先关外设
    UART2BAUD = (baud << 16) | baud;

    FUNCMCON2 = (FUNCMCON2 & ~0xff00) | TX2MAP_PE7 | RX2MAP_PB1;
    //0xF1=BIT(7)RX采样+BIT(6)TX使能+BIT(5)fixbaud+BIT(4)2stop+BIT(0)EN+KEY（官方款全配置）
    UART2CON = BIT(7) | BIT(6) | BIT(5) | BIT(4) | BIT(0);
    UART2CON |= 0xaaa << 16;                            //KEY RESET MODE
    UART2CPND = BIT(8) | BIT(9);
    UART2CPND |= BIT(10) | BIT(15);

    printf("uart2 com ready: TX=PE7 RX=PB1 baud=%d\n", (int)baudrate);
    uart2_com_puts("\r\nuart2 RX@PB1 ready\r\n");
}

//取一个收到的字节（供业务代码消费），返回1=有数据
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

//主循环轮询：RX采样 + 回显（PE7） + 心跳/诊断（console+PE7双通道），喂休眠/关机延时
AT(.com_text.uart2.com)
void bsp_uart2_com_process(void)
{
    u8 ch;
    static u32 hb_tick;
    static u32 hb_cnt;
    char line[48];
    char *p;
    const char *pre = "uart2 hb #";

    uart2_rx_poll();                                    //采样保持寄存器变化→环形缓冲

    while (uart2_com_cb.r_cnt != uart2_com_cb.w_cnt) {  //回显收到的数据（hex，紧凑3字符）
        bsp_uart2_com_get(&ch);
        reset_sleep_delay();
        reset_pwroff_delay();
        uart2_com_puts(uart2_com_hex(ch));
        uart2_com_tx_byte(' ');
    }

    if (tick_check_expire(hb_tick, 2000)) {             //每2秒心跳+诊断行
        hb_tick = tick_get();
        hb_cnt++;
        p = line;
        while (*pre) {
            *p++ = *pre++;
        }
        p += uart2_com_u32_to_str(p, hb_cnt);
        *p++ = '\r';
        *p++ = '\n';
        *p = 0;
        uart2_com_puts(line);                           //PE7
        //console诊断（1.5M官方工具）+PE7诊断，双通道交叉验证
        printf("[uart2] hb=%d rx=%d last=%02x ff=%d con2=%08x cpnd=%x\n",
               (int)hb_cnt, (int)uart2_com_cb.w_cnt, uart2_last_byte,
               (int)uart2_ff_cnt, UART2CON, UART2CPND);
        uart2_com_puts("hb=");
        uart2_com_puts(uart2_com_dec(hb_cnt));
        uart2_com_puts(" rx=");
        uart2_com_puts(uart2_com_dec(uart2_com_cb.w_cnt));
        uart2_com_puts(" last=");
        uart2_com_puts(uart2_com_hex(uart2_last_byte));
        uart2_com_puts(" con2=");
        uart2_com_puts(uart2_com_hex((u8)(UART2CON >> 24)));
        uart2_com_puts(uart2_com_hex((u8)(UART2CON >> 16)));
        uart2_com_puts(uart2_com_hex((u8)(UART2CON >> 8)));
        uart2_com_puts(uart2_com_hex((u8)UART2CON));
        uart2_com_puts("\r\n");
        reset_sleep_delay();
        reset_pwroff_delay();
    }
}

#endif /* UART2_COM_EN */
