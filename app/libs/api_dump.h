#ifndef __API_DUMP_H_
#define __API_DUMP_H_


#define DUMP_HEADER_LEN         8
#define DUMP_TX_SIZE            8

enum DUMP_CHANNEL {
    DUMP_CH0,
    DUMP_CH1,
    DUMP_CH2,
    DUMP_CH3,
    DUMP_CH4,
    DUMP_CH5,
};

typedef struct {
    const void *ptr;
    uint len;
} dump_tx_t;

typedef struct {
    void (*dma_tx)(const void *buf, uint len);

    u8 rptr;
    u8 wptr;
    volatile u8 total;
    volatile u8 tx_busy;
    dump_tx_t *tx;
    u8 *header;

    u8 ch_num;
} dump_cb_t;

void pcm_dump_tx_done(void);
bool pcm_dump_putcs(u8 type, const void *buf, uint len, u8 frame_num);
void pcm_dump_init(dump_cb_t *dump);


#endif // __API_DUMP_H_
