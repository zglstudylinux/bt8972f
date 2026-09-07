#ifndef  _BSP_PIANO_H_
#define  _BSP_PIANO_H_

#define PIANO_MAX_LEN           10      //一条tone音最多几个音调

enum {
    WARNING_PIANO,
    WARNING_TONE,
};

//与warning_tone_tbl数组对应
enum {
    TONE_POWER_ON,
    TONE_POWER_OFF,
    TONE_PAIR,
    TONE_BT_DISCONNECT,
    TONE_BT_CONNECT,
    TONE_BT_RING,
    TONE_MAX_VOL,
    TONE_LOW_BATTERY,
    TONE_REDIALING,
    TONE_BT_MODE,
    TONE_MAX_NB,
};

//与warning_piano_tbl数组对应
enum {
    PIANO_POWER_ON,
    PIANO_POWER_OFF,
    PIANO_BT_MODE,
    PIANO_CAMERA_MODE,
    PIANO_AUX_MODE,
    PIANO_FINEMY_SOUND,
    PIANO_MAX_NB,
};

typedef struct {
    u32 res;
    u32 last_time;  //持续时间
} tone_tbl_t;

typedef u32 piano_tbl_t;

typedef struct {
    u32 len;
    const tone_tbl_t *res_tbl;
} warning_tone_tbl_t;

typedef struct {
    u32 len;
    const piano_tbl_t *res_tbl;
} warning_piano_tbl_t;

typedef struct {
    bool dac1_en;
    bool flag_need_kick;
    u8 len;
    u8 type;
    u8 spr;
    u8 base_key;                //音阶：CDEFGAB
    u8 harmen;                  //泛音
    s16 base_delay;             //音节持续时间
    volatile u16 write_cnt;
    u32 res_buf[PIANO_MAX_LEN*2];
} piano_cb_t;

extern const warning_piano_tbl_t warning_piano_tbl[];
extern const warning_tone_tbl_t warning_tone_tbl[];

bool piano_is_playing(void);
void piano_sync_kick(void);

u8 piano_tone_init(uint type, void *res, bool kick, uint8_t res_flag);
#define piano_tone_is_playing()     piano_is_playing()
void piano_tone_exit(void);

#endif
