#ifndef _USB_DEVICE_IAP2_H
#define _USB_DEVICE_IAP2_H


#define APPLE_CACHE_SIZE         560
#define APPLE_DATA_POOL_SIZE     2048
#define APPLE_CERTI_SIZE         560
#define APPLE_PYLOAD_SIZE        560


enum  {
    UDI_STATE_IDLE = 0,

    UDI_STATE_DONE,
};

enum{
    LOAD_INFO,
    LOAD_CERTI,
    LOAD_DONE,
};

enum{
    DEVICE_LINK_INIT = 0,
    DEVICE_LINK_SYN,
    DEVICE_LINK_ACK,
    DEVICE_AUTHENT_CERT,
    DEVICE_AUTHENT_CERT_RESPONSE,
    DEVICE_IDENTIFICATION,
    DEVICE_START_POWER_UPDATE,
    DEVICE_POWER_SOURCE_UPDATE,
    DEVICE_REQUEST_APP_LAUNCH,
    DEVICE_STATUS_EA_SESSION,
    DEVICE_EA_SESSION_PROCESS,
    DEVICE_STOP_POWER_UPFATE,

    DEVICE_IDLE = 20,
};

enum{
    DEVICE_TYPE   = 0,
    APPLE         = 1,
    ANDROID       = 2,
};

enum{
    POWER_EXIT    = 0,
    POWER_ENTER   = 1,
};

enum{
    NORMAL_DATA    = 0,
    FOTA_DATA      = 1,
};

enum{
    APPLE_PACKET_IVALID   = 0x00,
    APPLE_PACKET_SYN      = 0x80,
    APPLE_PACKET_SYN_ACK  = 0xC0,
    APPLE_PACKET_ACK      = 0x40,
};

enum{
    APPLE_REQ_AUTHENT_CERT         = 0xAA00,
    APPLE_REQ_AUTHENT_CHALL_RESP   = 0xAA02,
    APPLE_AUTHENT_FILED            = 0xAA04,
    APPLE_AUTHENT_SUCCESSED        = 0xAA05,
    APPLE_START_IDENTIFICATION     = 0x1D00,
    APPLE_IDENTIFICATION_ACCEPTED  = 0x1D02,
    APPLE_START_EA_SESSION         = 0xEA00,
    APPLE_STOP_EA_SESSION          = 0xEA01,
    APPLE_POWER_SOURCE_UPDATE      = 0xAE03,
    APPLE_POWER_UPDATE             = 0xAE01,
};


typedef struct{
    u8 rec_ack;
    u8 rec_seq;
    u8 send_ack;
    u8 send_seq;
    u8 ack_start_flag;
    u8 iap_rec_packet_flag;
    u8 ea_start_flag;
    u8 mfi_inited_flag;         //MFI身份认证完成
    u8 last_power_sta;
    u8 control_byte;
    u8 session_identifier;
    u8 ea_identifier_set_flag;
    u8 apple_pyload[APPLE_PYLOAD_SIZE];
    u8 send_packet_len[30];
    u8 send_packet_num;
    u8 update_data_flag;
    u8 iap_data_send_flag;
    u8 app_init_flag;            //优化IOS APP初始化速度
    u8 iap_send_step;
    u8 upload_step;

    u16 update_data_len;
    u16 crc_data;
    u16 iap_rec_cnt;
    u16 ea_identifiter;
    u16 packet_size;
    u16 summary_id;
    u16 time_out_tick;
    u16 data_len;
    u16  load_info_len;
    u32 app_init_tick;           //优化IOS初始化计数
}mfi_iap2_cfg_t;

typedef struct {
    ring_buf_t ios_data_cbuf;

    u8  w_idx;
    u8  r_idx;

    u16 ios_outlen_buf[100];
    u8 ios_cache[APPLE_CACHE_SIZE];
    u8 ios_data_pool[APPLE_DATA_POOL_SIZE];

} ios_data_cache_cb;

enum{
    NAME                 = 0,
    MODEL_IDENTIFIER,
    MANUFACTURER,
    SERIAL_NUMBER,
    FIRMWARE_VERSION,
    HARDWARE_VERSION,
    MSG_SEND_BY_ACESSORY,
    MSG_RECV_FROM_DEVICE,
    POWER_PROVIDING_CAPABILITY,
    MAX_CURRENT_DRAWN_FROME_DEVICE,
    SUPPORT_EA,
};

typedef struct{
    u8 product_name_buf[74];
    u8 model[38];
    u8 serial_number_buf[34];
    u8 manufacture[34];
}iap2_authen_cfg_t;

typedef struct{
    u8 mfi_data_cache[APPLE_CACHE_SIZE];
    u8 ota_data_cache[APPLE_CACHE_SIZE];
    u8 ota_data_pool[APPLE_DATA_POOL_SIZE];
    u8 device_data_cache[APPLE_CACHE_SIZE];
    u8 device_data_pool[APPLE_DATA_POOL_SIZE];
    u8 apple_data_pool[APPLE_DATA_POOL_SIZE];
}iap2_buf_cfg_t;

extern u8 cfg_desc_buf[0x400];          //配置描述符
u8 ude_cfgval_get(void);
void mfi_i2c_read_data(u8 addr, u8 *buf, u16 len);
void mfi_i2c_write_data(u8 addr, u8 *buf, u8 len);
void mfi_i2c_init(void);
void apple_data_send(u8 *data, u16 len, u8 report_id, u8 type);

bool get_c9_send_en(void);
u16 get_read_info_len(void);
void auth_chip_sdq_get_length(void);
bool get_read_certi_sta(void);
bool get_auth_respon_sta(void);
void set_auth_respon_sta(bool bo_set);
void get_challenge_data(u8 *buf);
void ude_iap_get_data(void);
void uds_iap_tmr_isr(void);
void set_iap_send_flag(void);
void cfdl_run(void);
void uds_iap_reset(void);
#endif
