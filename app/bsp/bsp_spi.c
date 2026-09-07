#include "include.h"

#if SPI_HW_EN

u8 spi_rx_buf[SPI_RX_BUF_LEN];

u8 spi_init_flag = 0;

static struct {
    uint16_t rxbuf_size;
    uint8_t *rxbuf;
} spi_cb;

//----------------------------------------------------------------------
//SPI TX/RX 1 BYTE
AT(.com_text.spi)
void spi_byte_tx(u8 data)
{
    if(!spi_init_flag) {
        return;
    }
    SPI1CON &= ~BIT(4);                       //set TX
    SPI1BUF = data;
    while(!(SPI1CON & BIT(16)));              //Wait pending
}

void spi_byte_rx(u8 *data)
{
    SPI1CON |= BIT(4);                        //set RX
    SPI1BUF = 0xFF;
    while(!(SPI1CON & BIT(16)));
    *data = SPI1BUF;
}

//----------------------------------------------------------------------
//SPI DMA TX/RX
AT(.com_text.spi)
void spi_dma_tx(u8 *buf, uint len)
{
    if(!spi_init_flag) {
        return;
    }

    SPI1CON &= ~BIT(4);
    SPI1DMAADR = DMA_ADR(buf);
    SPI1DMACNT = len;
    //while(!(SPI1CON & BIT(16)));
}

void spi_dma_rx(u8 *buf, uint len)
{
    SPI1CON |= BIT(4);
    SPI1DMAADR = DMA_ADR(buf);
    SPI1DMACNT = len;
    //while(!(SPI1CON & BIT(16)));
}

#if SPI_IRQ_EN
#if SPI_MASTER_EN
AT(.com_text.spi.isr)
static void spi_dma_tx_done_cb(void)
{
    //SPI DMA 已发送完SPI1DMACNT长度的数据
}

#else
AT(.com_text.spi.isr)
static void spi_dma_rx_done_cb(void)
{
    //SPI DMA 已收到spi_cb.rxbuf_size长度的数据
}

static void spi_dma_rx_kick(void)
{
    SPI1CON |= BIT(4);
    SPI1DMAADR = DMA_ADR(spi_cb.rxbuf);
    SPI1DMACNT = spi_cb.rxbuf_size;
}
#endif

AT(.com_text.spi.isr)
static void spi_isr_func(void)
{
    //DMA TX or RX finish
    if(SPI1CON & BIT(16)) {
        SPI1CPND = BIT(16);  //clear pending
        #if SPI_MASTER_EN
        spi_dma_tx_done_cb();
        #else
        spi_dma_rx_done_cb();
        spi_dma_rx_kick();
        #endif
    }
}
#endif // SPI_IRQ_EN

void bsp_spi_init(void)
{
    printf("%s\n", __func__);

    spi_cb.rxbuf      = spi_rx_buf;
    spi_cb.rxbuf_size = SPI_RX_BUF_LEN;

#if SPI_MASTER_EN
    SPI_MASTER_CLK_SDO_INIT();
    #if LCD_DISPLAY_EN //LCD使用双线协议,CPOL=1
      SPI1CON =  BIT(SPIEN) | BIT(SPI_BUSMODE_LOWBIT) | BIT(SPIIE) | BIT(SPI_CLKIDS);
    #elif SPI_2_WIRE_EN
      //2线模式
      SPI1CON =  BIT(SPIEN) | BIT(SPI_BUSMODE_LOWBIT) | BIT(SPIIE) ;
    #else
      //3线模式
      SPI1CON =  BIT(SPIEN) | BIT(SPIIE) ;
      SPI_MASTER_SDI_INIT();
    #endif

    SPI1BAUD = 24000000/SPI_BAUD_RATE - 1;
#else
    SPI_SALVE_CLK_SDO_INIT();
    SPI1CON =  BIT(SPIEN)  | BIT(SPI_BUSMODE_LOWBIT) | BIT(SPIIE) | BIT(SPI_RXSEL)| BIT(SPISM);

    SPI1DMAADR = DMA_ADR(spi_cb.rxbuf);
    SPI1DMACNT = SPI_RX_BUF_LEN;                  //kick start spi receive
#endif

    CLKGAT0 |= BIT(13);                           //SPI1 CLK_EN
    FUNCMCON1 |= (0x0F<<4);
    FUNCMCON1 |= SPI_MAPPING;

#if SPI_CS_EN
    SPI_CS_IO_INIT() ;
#endif

#if SPI_IRQ_EN
    sys_irq_init(IRQ_SPI_VECTOR, 0, spi_isr_func);
#endif

    spi_init_flag = 1;
}


#endif


