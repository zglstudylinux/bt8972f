#include "include.h"

/**************************************************************************
WKPINMAP[15:0]
WAKEUP SOURCE   0   [1:0]     00:PA7    01:PA6    10:PA5    11:PA4
WAKEUP SOURCE   1   [3:2]     00:PB1    01:PE0    10:PE4    11:PG0
WAKEUP SOURCE   2   [5:4]     00:PB2    01:PE5    10:PE6    11:PG1
WAKEUP SOURCE   3   [7:6]     00:PB3    01:PE7    10:PF0    11:PG2
WAKEUP SOURCE   4   [9:8]     00:PB4    01:PF1    10:PG3    11:PG4
WAKEUP SOURCE   5   [11:10]   00:PB5    01:PB5    10:PG5    11:UNUSED
WAKEUP SOURCE   6   [13:12]   00:PE0    01:UNUSED 10:UNUSED 11:PALL_FALL
WAKEUP SOURCE   7   [15:14]   00:UNUSED 01:UNUSED 10:UNUSED 11:PALL_RISE
***************************************************************************/

#define TRACE_EN                0

#if TRACE_EN
#define TRACE(...)              printf(__VA_ARGS__)
#else
#define TRACE(...)
#endif

#if 0

//GPIO中断示例
AT(.com_text.port)
void port_isr(void)
{
//    if ((io_num == IO_PA7) || (io_num == IO_PA6) || (io_num == IO_PA5) || (io_num == IO_PA4)) {
//        if (WKUPEDG & (BIT(0) << 16)) {
//            WKUPCPND = (BIT(0) << 16);  //CLEAR PENDING
////            port_int_callback(io_num);
//        }
//    }
}

void bsp_port_int_io_init(gpio_t *g, u8 io_num, u8 edg)
{
    bsp_gpio_cfg_init(g, io_num);
    if (g->sfr == NULL) {
        return;
    }
    g->sfr[GPIOxDE] |= BIT(g->num);
    g->sfr[GPIOxDIR] |= BIT(g->num);
    g->sfr[GPIOxFEN] &= ~BIT(g->num);
    g->sfr[GPIOxPD - edg] |= BIT(g->num);
}

#define PINT_EN        BIT(0)      //bit[23]=VUSB_PIN, bit[22:17]=PG[5:0], bit[16:15]=PF[1:0], bit[14:11]=PE[7:4], bit[10]=PE[0], bit[9:4]=PB[5:0], bit[3:0]=PA[7:4]
#define PINT_EDG       BIT(0)      //bit[23]=VUSB_PIN, bit[22:17]=PG[5:0], bit[16:15]=PF[1:0], bit[14:11]=PE[7:4], bit[10]=PE[0], bit[9:4]=PB[5:0], bit[3:0]=PA[7:4]


//测试OK
void port_int_init(void)
{
    sys_irq_init(IRQ_PORT_VECTOR, 0, port_isr);

    PORTINTEN = PINT0_EN;
    PORTINTEDG = PINT0_EDG;

    WKPINMAP = BIT(1) | BIT(0);    //WKWKPINMAP[15:0]
    WKUPEDG = BIT(0);              //0:rising, 1:falling
    WKUPIE = BIT(0);               //input[7:0] interrupt enable
    WKUPCON = BIT(0);              //input[7:0] enable
}

#endif // 1
