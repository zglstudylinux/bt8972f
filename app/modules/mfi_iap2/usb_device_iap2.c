#include "include.h"

#define TRACE_EN                0
#if TRACE_EN
#define TRACE(...)              my_printf(__VA_ARGS__)
#define TRACE_R(...)            my_print_r(__VA_ARGS__)
#else
#define TRACE(...)
#define TRACE_R(...)
#endif

#if UDE_IAP_PROCESS_EN

#define IAP2_CERTEN_INFO_MAX_LEN        300                  //认证信息最大为300字节

const char iap2_product_name[]  = UDE_PROD_NAME;             //认证信息需要和USB信息一致
const char iap2_serial_number[] = UDE_SERIAL_NB;
const char iap2_model[]         = UDE_MODEL;
const char iap2_manufacture[]   = UDE_MANUFACTURE;

WEAK uint8_t ude_iap_process_en = 1;
extern uds_t uds_0 AT(.udev_buf.udev.usb);

u8 authentication_data_buf[APPLE_CERTI_SIZE] AT(.iap2.authen_info.buf);   //认证信息buf
u8 device_authen_buf[IAP2_CERTEN_INFO_MAX_LEN] AT(.iap2.authen_info.buf);

mfi_iap2_cfg_t iap2_cfg AT(.iap2_buf.cfg);
iap2_buf_cfg_t iap2_buf_cfg AT(.iap2_buf.buf_cfg);;
iap2_authen_cfg_t iap2_authen_cfg AT(.iap2.authen_info.buf);

static ios_data_cache_cb ios_data_cache AT(.buf.iap2.data_pool);

static ring_buf_t apple_rec_cbuf;
static ring_buf_t apple_send_cbuf;
static ring_buf_t apple_ota_cbuf;

static const rbuf_tbl_t iap_buf_tbl[3] = {
    {
        .buf = iap2_buf_cfg.apple_data_pool,
        .size = APPLE_DATA_POOL_SIZE,
    },

    {
        .buf = iap2_buf_cfg.device_data_pool,
        .size = APPLE_DATA_POOL_SIZE,
    },

    {
        .buf = ios_data_cache.ios_data_pool,
        .size = APPLE_DATA_POOL_SIZE,
    }
};

AT(.rodata.iap)
const u8 iap2_generial_info1[16] = {
//-----------------------------------------------------------
//起始2行,协议头部信息
//第1行的第4byte为整个数组的长度0xD2(210)
//第2行的第4byte为整个数组的长度减10, 0xC8(210-10=200)
//-----------------------------------------------------------
    0xFF, 0x5A, 0x00, 0xD2, 0x40, 0x04, 0xC6, 0x01, 0xB1,
    0x40, 0x40, 0x00, 0xC8, 0x1D, 0x01, 0x00
};

AT(.rodata.iap)
const u8 iap2_generial_info2[131] = {
    0x15, 0x00, 0x22, 0x32, 0x35, 0x39, 0x36, 0x34, 0x62, 0x32, 0x64, 0x33, 0x38, 0x65, 0x39, 0x34, 0x65, 0x65, 0x65, 0x00, 0x00,
    //固件版本："1.1.0"
    0x0A, 0x00, 0x04, 0x31, 0x2E, 0x31, 0x2E, 0x30, 0x00, 0x00,
    //硬件版本："1.1.0"
    0x0A, 0x00, 0x05, 0x31, 0x2E, 0x31, 0x2E, 0x30, 0x00, 0x00,
    //其他
    0x0C, 0x00, 0x06, 0xAE, 0x00, 0xAE, 0x02, 0xAE, 0x03, 0xEA, 0x03, 0x00,
    0x0A, 0x00, 0x07, 0xAE, 0x01, 0xEA, 0x00, 0xEA, 0x01, 0x00,
    0x05, 0x00, 0x08, 0x02, 0x00,
    0x06, 0x00, 0x09, 0x00, 0x64, 0x00,
    0x13, 0x00, 0x0A, 0x00, 0x05, 0x00, 0x00, 0xB1, 0x00, 0x05, 0x00, 0x01, 0x00, 0x00, 0x05, 0x00, 0x02, 0x00, 0x00,
    0x07, 0x00, 0x0C, 0x65, 0x6E, 0x00, 0x00,
    0x07, 0x00, 0x0D, 0x65, 0x6E, 0x00, 0x00,
    0x18, 0x00, 0x10, 0x00, 0x06, 0x00, 0x00, 0x00, 0x01, 0x00, 0x0A, 0x00, 0x01, 0x69, 0x41, 0x50, 0x32, 0x48, 0x00, 0x00, 0x04, 0x00, 0x02, 0xAB
};

AT(.rodata.iap)
static const u8 detect_sequence[6] = {0xFF,0x55,0x02,0x00,0xEE,0x10};

AT(.rodata.iap)
static const u8 iap_syn_sequence[26] = {0xFF,0x5A,0x00,0x1A,0x80,0x01,0x10,0x00,0xFC,0x01,0x05,0x04,0x00,0x13,0x88,0x00,0xFF,0x1E,0x03,0x01,0x00,0x01,0x02,0x02,0x01,0xF8};
//AT(.rodata.iap)
//static const u8 iap_syn_sequence[26] = {0xFF,0x5A,0x00,0x1A,0x80,0x01,0x10,0x00,0xFC,0x01,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x01,0x00,0x01,0x02,0x02,0x01,0xF8};

AT(.rodata.iap)
static const u8 iap_fourth_cmd[19] = {0xFF,0x5A,0x03,0xE0,0x40,0x02,0xF5,0x01,0x8C,0x40,0x40,0x03,0xD6,0xAA,0x01,0x03,0xD0,0x00,0x00};

AT(.rodata.iap)
static const u8 iap_challenage_cmd[19] = {0xFF,0x5A,0x00,0x94,0x40,0x03,0xF6,0x01,0xD9,0x40,0x40,0x00,0x8A,0xAA,0x03,0x00,0x84,0x00,0x00};

AT(.rodata.iap)
static const u8 iap_startPowerUpdates_cmd[32] = {0xFF,0x5A,0x00,0x20,0x40,0x30,0x17,0x01,0xff,0x40,0x40,0x00,0x16,0xae,0x00,0x00,0x04,0x00,0x00,0x00,0x04,0x00,0x01,0x00,0x04,0x00,0x04,0x00,0x04,0x00,0x05,0xa2};   //StartPowerUpdates

AT(.rodata.iap)
static const u8 iap_stopPowerUpdates_cmd[17] = {0xFF,0x5A,0x00,0x11,0x40,0x30,0x17,0x01,0xff,0x40,0x00,0x16,0xae,0x02,0x00,0x05,0xa2};

AT(.rodata.iap)
static const u8 powersource_update[27] = {0xFF,0x5A,0x00,0x1B,0x40,0x06,0xF9,0x01,0x4D,0x40,0x40,0x00,0x11,0xAE,0x03,0x00,0x06,0x00,0x00,0x09,0x60,0x00,0x05,0x00,0x01,0x01,0xC6};//2.4A

AT(.rodata.iap)
static const u8 ea_session_status[27] = {0xFF, 0x5A, 0x00, 0x1B, 0x40, 0x07, 0xFC, 0x01, 0x48, 0x40, 0x40, 0x00, 0x11, 0xEA, 0x03, 0x00, 0x06, 0x00, 0x00, 0x00, 0x08, 0x00, 0x05, 0x00, 0x01, 0x00, 0x6E};

AT(.rodata.iap)
static const u8 ea_session_packet[11] = {0xFF, 0x5A, 0x00, 0x4C, 0x40, 0x08, 0xFD, 0x02, 0x36, 0x00, 0x03};

//AT(.rodata.iap)
//static const u8 app_launch[37] = {0xFF, 0x5A, 0x00, 0x25, 0x40, 0x05, 0xC7, 0x01, 0x75, 0x40, 0x40, 0x00, 0x1B, 0xEA, 0x02, 0x00, 0x10, 0x00, 0x00, 0x63, 0x6F, 0x6D, 0x2E, 0x62, 0x6F, 0x79, 0x61, 0x2E, 0x65, 0x63, 0x6F, 0x00, 0x05, 0x00, 0x01, 0x00, 0xE6};

AT(.rodata.iap)
static const u8 iap_ack_cmd[9] = {0xFF, 0x5A, 0x00, 0x09, 0x40, 0x2f, 0x16, 0x01, 0x18};//第五位是SEQ，第六位是ACKnumber，最后一位是校验位

//AT(.rodata.iap)
//const uint16_t ccitt_crc16_table[256] = {
//    0x0000, 0x1021, 0x2042, 0x3063, 0x4084, 0x50a5, 0x60c6, 0x70e7, 0x8108, 0x9129, 0xa14a, 0xb16b, 0xc18c, 0xd1ad, 0xe1ce, 0xf1ef,
//    0x1231, 0x0210, 0x3273, 0x2252, 0x52b5, 0x4294, 0x72f7, 0x62d6, 0x9339, 0x8318, 0xb37b, 0xa35a, 0xd3bd, 0xc39c, 0xf3ff, 0xe3de,
//    0x2462, 0x3443, 0x0420, 0x1401, 0x64e6, 0x74c7, 0x44a4, 0x5485, 0xa56a, 0xb54b, 0x8528, 0x9509, 0xe5ee, 0xf5cf, 0xc5ac, 0xd58d,
//    0x3653, 0x2672, 0x1611, 0x0630, 0x76d7, 0x66f6, 0x5695, 0x46b4, 0xb75b, 0xa77a, 0x9719, 0x8738, 0xf7df, 0xe7fe, 0xd79d, 0xc7bc,
//    0x48c4, 0x58e5, 0x6886, 0x78a7, 0x0840, 0x1861, 0x2802, 0x3823, 0xc9cc, 0xd9ed, 0xe98e, 0xf9af, 0x8948, 0x9969, 0xa90a, 0xb92b,
//    0x5af5, 0x4ad4, 0x7ab7, 0x6a96, 0x1a71, 0x0a50, 0x3a33, 0x2a12, 0xdbfd, 0xcbdc, 0xfbbf, 0xeb9e, 0x9b79, 0x8b58, 0xbb3b, 0xab1a,
//    0x6ca6, 0x7c87, 0x4ce4, 0x5cc5, 0x2c22, 0x3c03, 0x0c60, 0x1c41, 0xedae, 0xfd8f, 0xcdec, 0xddcd, 0xad2a, 0xbd0b, 0x8d68, 0x9d49,
//    0x7e97, 0x6eb6, 0x5ed5, 0x4ef4, 0x3e13, 0x2e32, 0x1e51, 0x0e70, 0xff9f, 0xefbe, 0xdfdd, 0xcffc, 0xbf1b, 0xaf3a, 0x9f59, 0x8f78,
//    0x9188, 0x81a9, 0xb1ca, 0xa1eb, 0xd10c, 0xc12d, 0xf14e, 0xe16f, 0x1080, 0x00a1, 0x30c2, 0x20e3, 0x5004, 0x4025, 0x7046, 0x6067,
//    0x83b9, 0x9398, 0xa3fb, 0xb3da, 0xc33d, 0xd31c, 0xe37f, 0xf35e, 0x02b1, 0x1290, 0x22f3, 0x32d2, 0x4235, 0x5214, 0x6277, 0x7256,
//    0xb5ea, 0xa5cb, 0x95a8, 0x8589, 0xf56e, 0xe54f, 0xd52c, 0xc50d, 0x34e2, 0x24c3, 0x14a0, 0x0481, 0x7466, 0x6447, 0x5424, 0x4405,
//    0xa7db, 0xb7fa, 0x8799, 0x97b8, 0xe75f, 0xf77e, 0xc71d, 0xd73c, 0x26d3, 0x36f2, 0x0691, 0x16b0, 0x6657, 0x7676, 0x4615, 0x5634,
//    0xd94c, 0xc96d, 0xf90e, 0xe92f, 0x99c8, 0x89e9, 0xb98a, 0xa9ab, 0x5844, 0x4865, 0x7806, 0x6827, 0x18c0, 0x08e1, 0x3882, 0x28a3,
//    0xcb7d, 0xdb5c, 0xeb3f, 0xfb1e, 0x8bf9, 0x9bd8, 0xabbb, 0xbb9a, 0x4a75, 0x5a54, 0x6a37, 0x7a16, 0x0af1, 0x1ad0, 0x2ab3, 0x3a92,
//    0xfd2e, 0xed0f, 0xdd6c, 0xcd4d, 0xbdaa, 0xad8b, 0x9de8, 0x8dc9, 0x7c26, 0x6c07, 0x5c64, 0x4c45, 0x3ca2, 0x2c83, 0x1ce0, 0x0cc1,
//    0xef1f, 0xff3e, 0xcf5d, 0xdf7c, 0xaf9b, 0xbfba, 0x8fd9, 0x9ff8, 0x6e17, 0x7e36, 0x4e55, 0x5e74, 0x2e93, 0x3eb2, 0x0ed1, 0x1ef0
//};

WEAK uint8_t cfg_auth_cp_version = 1;       //0:Auth2.0  1:Auth3.0

///0x100减去前八位数据之和的一个校验位
AT(.usbdev.com.iap2)
uint8_t check_before_eight(u8 *ptr, u16 len)
{
    uint16_t i ;
    uint8_t sum = 0;

    for(i=0; i<len; i++){
        sum += ptr[i];
    }
    return (uint8_t)(0x100 - sum);
}

void set_iap_send_flag(void)
{
    iap2_cfg.iap_data_send_flag = true;

}

AT(.com_text.usbdev)
void uds_iap_tmr_isr(void)
{
    if(iap2_cfg.time_out_tick) {
        iap2_cfg.time_out_tick--;
    }

//    if(!iap2_cfg.iap_rec_packet_flag) {
//        iap2_cfg.app_init_tick++;
//        if(iap2_cfg.app_init_tick  >= 100) {
//            iap2_cfg.app_init_tick = 0;
//        }
//    }

}

AT(.text.iap2_info)
void iap2_cfg_str_convert(u8 *in, u8 *out, int in_len, int out_len, u8 id)
{
    int i, char_size;
    u16 unicode;
    int cnt = 0;

    out[cnt++] = out_len;
    out[cnt++] = 0x00;
    out[cnt++] = id;
    for (i = 0; i < in_len; ) {
        if (!in[i]) {
            break;
        }
        char_size = utf8_char_size(in[i]);
        unicode = utf8_convert_to_unicode(&in[i], char_size);
        out[cnt++] = (u8)unicode;
        i += char_size;
        if (cnt >= out_len) {
            break;
        }
    }
    out[cnt++] = 0x00;
    out[cnt++] = 0x00;
    out[0] = cnt;                           //更新实际长度
}

AT(.text.iap2_name)
static void iap2_product_name_init(void)
{
    iap2_cfg_str_convert((u8 *)iap2_product_name, (u8 *)iap2_authen_cfg.product_name_buf, sizeof(iap2_product_name), sizeof(iap2_authen_cfg.product_name_buf), NAME);
//    TRACE("product_name\n");
//    TRACE_R(iap2_authen_cfg.product_name_buf, sizeof(iap2_authen_cfg.product_name_buf));
}

AT(.text.iap2_serial_number)
static void iap2_serial_number_init(void)
{
    iap2_cfg_str_convert((u8 *)iap2_serial_number, (u8 *)iap2_authen_cfg.serial_number_buf, sizeof(iap2_serial_number), sizeof(iap2_authen_cfg.serial_number_buf), SERIAL_NUMBER);
//    TRACE("serial_number\n");
//    TRACE_R(iap2_authen_cfg.serial_number_buf, sizeof(iap2_authen_cfg.serial_number_buf));
}

AT(.text.iap2_model)
static void iap2_model_init(void)
{
    iap2_cfg_str_convert((u8 *)iap2_model, (u8 *)iap2_authen_cfg.model, sizeof(iap2_model), sizeof(iap2_authen_cfg.model), MODEL_IDENTIFIER);
//    TRACE("model\n");
//    TRACE_R(iap2_authen_cfg.model, sizeof(iap2_authen_cfg.model));
}

AT(.text.iap2_manufacture)
static void iap2_manufacture_init(void)
{
    iap2_cfg_str_convert((u8 *)iap2_manufacture, (u8 *)iap2_authen_cfg.manufacture, sizeof(iap2_manufacture), sizeof(iap2_authen_cfg.manufacture), MANUFACTURER);
//    TRACE("manufacture\n");
//    TRACE_R(iap2_authen_cfg.manufacture, sizeof(iap2_authen_cfg.manufacture));
}

AT(.text.got_info)
void got_iap2_authen_info(void)
{
    u8 *iap2_authen_ptr = &device_authen_buf[0];
    u16 iap2_cfg_ifo_len;

    memcpy(iap2_authen_ptr, iap2_generial_info1, sizeof(iap2_generial_info1));
    iap2_authen_ptr += sizeof(iap2_generial_info1);

    memcpy(iap2_authen_ptr, (u8 *)iap2_authen_cfg.product_name_buf, iap2_authen_cfg.product_name_buf[0]);
    iap2_authen_ptr += iap2_authen_cfg.product_name_buf[0];

    memcpy(iap2_authen_ptr, (u8 *)iap2_authen_cfg.model, iap2_authen_cfg.model[0]);
    iap2_authen_ptr += iap2_authen_cfg.model[0];

    memcpy(iap2_authen_ptr, (u8 *)iap2_authen_cfg.manufacture, iap2_authen_cfg.manufacture[0]);
    iap2_authen_ptr += iap2_authen_cfg.manufacture[0];

    memcpy(iap2_authen_ptr, (u8 *)iap2_authen_cfg.serial_number_buf, iap2_authen_cfg.serial_number_buf[0]);
    iap2_authen_ptr += iap2_authen_cfg.serial_number_buf[0];

     memcpy(iap2_authen_ptr, iap2_generial_info2, sizeof(iap2_generial_info2));
    iap2_authen_ptr += sizeof(iap2_generial_info2);


    iap2_cfg_ifo_len = (u32)iap2_authen_ptr - (u32)device_authen_buf;
    device_authen_buf[2] = (u8)(iap2_cfg_ifo_len >> 8);
    device_authen_buf[3] = (u8)(iap2_cfg_ifo_len);
    device_authen_buf[11] = (u8)((iap2_cfg_ifo_len - 10) >> 8);
    device_authen_buf[12] = (u8)(iap2_cfg_ifo_len-10);

//    TRACE("IAP2 INFO LEN:%d\n DATA:", iap2_cfg_ifo_len);
//    TRACE_R(device_authen_buf, iap2_cfg_ifo_len);
}


AT(.com_text.usbdev)
void uds_iap_reset(void)                  //端点reset后需要重新认证
{
    iap2_cfg.iap_send_step = 0;
}

///发送数据给iphone
AT(.com_text.uds_iap)
void ude_iap_transfer(uds_t *uds, uint len)
{
    iap2_cfg.time_out_tick = 10;
    usb_ep_start_transfer(uds->bulk_in, len);
    while ((!usb_ep_transfer(uds->bulk_in)) && (!uds->error) && (iap2_cfg.time_out_tick)){
        WDT_CLR();
        if((iap2_cfg.time_out_tick <= 1) && (sys_cb.phone_type == DEVICE_TYPE)) {   //认证数据交互即将超时，且没有通过第一次苹果认证
            sys_cb.phone_type = ANDROID;
            TRACE("Android PHONE\n");

        }
    }
    iap2_cfg.time_out_tick=0;
}

///获取MFI认证数据
AT(.com_text.uds_iap)
static u32 mfi_accessory_certificate_data_get(void)
{
    u32 i;
    u8 buf[2], *ceri_buf = &cfg_desc_buf[19];
    u16 length;
    u16 length_temp;
    u16 block_size;
    mfi_i2c_read_data(0x30, buf, 2);
    if(cfg_auth_cp_version){
        length = ((buf[0] << 8) + buf[1]);
        length_temp = length;
        block_size = length_temp / 128;
        memset(ceri_buf, 0, sizeof(cfg_desc_buf)-19);
        for (i = 0; i < block_size; i++){
            mfi_i2c_read_data(0x31 + i, &ceri_buf[i*128], 128);
            length_temp -= 128;
        }
        if(length_temp != 0){
            mfi_i2c_read_data(0x31 + i, &ceri_buf[i*128], length_temp);
        }
        return length;
    } else {
        block_size = (((u16)buf[0] << 8) + buf[1] + 127) / 128;
        memset(ceri_buf, 0, sizeof(cfg_desc_buf)-19);
        mfi_i2c_read_data(0x31, &ceri_buf[0], 128*block_size);
        return block_size * 128;
    }
}

AT(.com_text.uds_iap)
void ude_iap_transfer_cerificate_data(uds_t *uds)
{
    u32 length = 0, i, tx_len;
    u8 *txbuf = uds->bulk_in->buf;
    u8 *ceri_buf = cfg_desc_buf;

    if(cfg_auth_cp_version){
        length = mfi_accessory_certificate_data_get();
        length += 4;//3.0CP
    } else {
        length = mfi_accessory_certificate_data_get();
    }

    memcpy(ceri_buf, iap_fourth_cmd, sizeof(iap_fourth_cmd));
    length += 20;
    ceri_buf[2] = length >> 8;
    ceri_buf[3] = (u8)length;
    ceri_buf[6] = iap2_cfg.rec_seq;
    ceri_buf[8] = check_before_eight(ceri_buf, 8);
    ceri_buf[11] = (length + 6 - 20) >> 8;
    ceri_buf[12] = (u8)(length + 6 - 20);
    ceri_buf[15] = (length - 20) >> 8;
    ceri_buf[16] = (u8)(length - 20);
    ceri_buf[length - 1] = check_before_eight(ceri_buf, length-1);
    for (i = 0; i < length; ) {
        tx_len = length - i;
        if (tx_len > 512) {
            tx_len = 512;
        }
        memcpy(txbuf, ceri_buf, tx_len);
        ude_iap_transfer(uds, tx_len);
        i += tx_len;
        ceri_buf += tx_len;
    }
}

AT(.com_text.uds_iap)
void ude_iap_update_packet_send(void *tcb, u8 ack, u8 *data, u16 len)
{
    uds_t *uds = (uds_t *)tcb;
    u8 *txbuf = uds->bulk_in->buf;
    u16 packet_len = 12 + len;

    memcpy(txbuf, ea_session_packet, sizeof(ea_session_packet));            //数据包头
    txbuf[3] = packet_len;
    txbuf[5] = iap2_cfg.send_seq++;
    txbuf[6] = ack;
    txbuf[7] = iap2_cfg.session_identifier;
    txbuf[8] = check_before_eight(txbuf, 8);
    txbuf[9] = (iap2_cfg.ea_identifiter >> 8);
    txbuf[10] = (iap2_cfg.ea_identifiter & 0xFF);
    if(len) {
        memcpy((&txbuf[11]), data, len);
    }
    memset(data, 0, len);
    len = 0;
    txbuf[(packet_len - 1)] = check_before_eight(txbuf, (packet_len - 1));
    ude_iap_transfer(uds, packet_len);
    TRACE("session send:%x, %x\n", (iap2_cfg.ea_identifiter >> 8), (iap2_cfg.ea_identifiter & 0xFF));
    return;
}


AT(.com_text.uds_iap)
void ude_iap_packet_send(void *tcb, u8 ack)
{
    u16 device_packet_len = ring_buf_get_count(&(apple_send_cbuf));
    u16 packet_len = 12 + device_packet_len;

    uds_t *uds = (uds_t *)tcb;
    u8 *txbuf = uds->bulk_in->buf;

    if(device_packet_len) {
        ring_buf_get(&apple_send_cbuf, iap2_buf_cfg.device_data_cache, device_packet_len);

        packet_len = 12 + device_packet_len;
        memcpy(txbuf, ea_session_packet, sizeof(ea_session_packet));            //数据包头
        txbuf[2] = (u8)(packet_len >> 8);
        txbuf[3] = (u8)packet_len;
        txbuf[5] = iap2_cfg.send_seq++;
        txbuf[6] = ack;
        txbuf[7] = iap2_cfg.session_identifier;
        txbuf[8] = check_before_eight(txbuf, 8);
        txbuf[9] = (iap2_cfg.ea_identifiter >> 8);
        txbuf[10] = (iap2_cfg.ea_identifiter & 0xFF);
        memcpy((&txbuf[11]), iap2_buf_cfg.device_data_cache, device_packet_len);
        txbuf[(packet_len - 1)] = check_before_eight(txbuf, (packet_len - 1));
        ude_iap_transfer(uds, packet_len);
        memset(iap2_buf_cfg.device_data_cache, 0, device_packet_len);
        device_packet_len = 0;
    }
    TRACE("session send:%x, %x\n", (iap2_cfg.ea_identifiter >> 8), (iap2_cfg.ea_identifiter & 0xFF));
    return;
}

AT(.com_text.uds_iap) WEAK
void ude_iap_tx_process(void)
{
    if(iap2_cfg.ea_start_flag && iap2_cfg.iap_data_send_flag) {
        ude_iap_packet_send((void *)&uds_0, iap2_cfg.rec_seq);
    }
}


AT(.com_text.uds_iap)
void apple_data_send(u8 *data, u16 len, u8 report_id, u8 type)
{
    if(sys_cb.phone_type == APPLE) {
        if(type) {                                                         //type,0:正常通信，1:升级
            iap2_cfg.apple_pyload[0] = report_id;
            if(len > APPLE_PYLOAD_SIZE) {
                memcpy((iap2_cfg.apple_pyload + 1), data, (APPLE_PYLOAD_SIZE - 1));
                iap2_cfg.data_len = APPLE_PYLOAD_SIZE;
            } else {
                memcpy(iap2_cfg.apple_pyload + 1, data, len);
                iap2_cfg.data_len = len + 1;
            }
            ude_iap_update_packet_send((void *)&uds_0, iap2_cfg.rec_seq, iap2_cfg.apple_pyload, iap2_cfg.data_len);
        } else {
            if(len > APPLE_PYLOAD_SIZE) {
                memcpy(iap2_cfg.apple_pyload, data, APPLE_PYLOAD_SIZE);
                iap2_cfg.data_len = APPLE_PYLOAD_SIZE;
            } else {
                memcpy(iap2_cfg.apple_pyload, data, len);
                iap2_cfg.data_len = len;
            }
            ring_buf_put(&apple_send_cbuf, iap2_cfg.apple_pyload, iap2_cfg.data_len);
        }

    }
}

//AT(.com_text.uds_iap)
//uint16_t ccitt_crc16(uint8_t *data, uint16_t len)
//{
//    uint16_t crc16 = 0x0000;
//    uint16_t crc_h8, crc_l8;
//
//    while( len-- ) {
//        crc_h8 = (crc16 >> 8);
//        crc_l8 = (crc16 << 8);
//        crc16 = crc_l8 ^ ccitt_crc16_table[crc_h8 ^ *data];
//        data++;
//    }
//    return crc16;
//}

AT(.com_text.uds_iap)
void iap_packet_analy(void)
{
    u16 recv_pkg_size = iap2_cfg.packet_size;

    if(iap2_cfg.iap_rec_packet_flag && (ring_buf_get_count(&(apple_rec_cbuf)) >= recv_pkg_size)) {
        iap2_cfg.iap_rec_packet_flag = 0;
        ring_buf_get(&apple_rec_cbuf, iap2_buf_cfg.mfi_data_cache, recv_pkg_size);

        if(recv_pkg_size > 15 && iap2_buf_cfg.mfi_data_cache[11] == 0x55 && (iap2_buf_cfg.mfi_data_cache[12] == 0x10 || iap2_buf_cfg.mfi_data_cache[12] == 0x11 || iap2_buf_cfg.mfi_data_cache[12] == 0x12)) {
            ios_data_cache.ios_outlen_buf[ios_data_cache.w_idx] = (recv_pkg_size - 12);

            ring_buf_t *ring_buf = &(ios_data_cache.ios_data_cbuf);
            ring_buf_put(ring_buf, (iap2_buf_cfg.mfi_data_cache + 11), (recv_pkg_size - 12));

            ios_data_cache.w_idx++;
            if(ios_data_cache.w_idx >= 100) {
                ios_data_cache.w_idx = 0;
            }
        }

        iap2_cfg.control_byte = iap2_buf_cfg.mfi_data_cache[4];
        iap2_cfg.rec_seq = iap2_buf_cfg.mfi_data_cache[5];
        iap2_cfg.rec_ack = iap2_buf_cfg.mfi_data_cache[6];
        iap2_cfg.session_identifier = iap2_buf_cfg.mfi_data_cache[7];

        if(memcmp(iap2_buf_cfg.mfi_data_cache, detect_sequence, 6) == 0){
            iap2_cfg.iap_send_step = DEVICE_LINK_SYN;
//            TRACE("DEVICE_LINK_SYN,APPLE PHONE\n");
        } else if((iap2_buf_cfg.mfi_data_cache[0] != 0xFF) || (iap2_buf_cfg.mfi_data_cache[1] != 0x5A)) {                      //数据包，包头不对直接修改为无效包，不做解析
            iap2_cfg.control_byte = APPLE_PACKET_IVALID;
        } else if(iap2_cfg.control_byte == APPLE_PACKET_SYN_ACK) {
            iap2_cfg.iap_send_step = DEVICE_LINK_ACK;
//            TRACE("APPLE_PACKET_SYN_ACK\n");
        } else if((iap2_cfg.control_byte == APPLE_PACKET_ACK) && (recv_pkg_size == 0x09)) {  //没有负载的ack，不回复
            iap2_cfg.iap_send_step = DEVICE_IDLE;
//            TRACE("APPLE_PACKET_ACK\n");
        } else if((iap2_cfg.control_byte == APPLE_PACKET_ACK) && (recv_pkg_size > 0x09)) {    //带有负载的ACK，需要解析数据包
            iap2_cfg.summary_id = iap2_buf_cfg.mfi_data_cache[13];
            iap2_cfg.summary_id = ((iap2_cfg.summary_id << 8) | iap2_buf_cfg.mfi_data_cache[14]);
//            TRACE("iap2_cfg.summary_id:%x\n",iap2_cfg.summary_id);
            switch(iap2_cfg.summary_id) {
                case APPLE_REQ_AUTHENT_CERT:
                    iap2_cfg.iap_send_step = DEVICE_AUTHENT_CERT;
                    break;
                case APPLE_REQ_AUTHENT_CHALL_RESP:
                    iap2_cfg.iap_send_step = DEVICE_AUTHENT_CERT_RESPONSE;
                    break;
                case APPLE_AUTHENT_SUCCESSED:
                    iap2_cfg.iap_send_step = DEVICE_LINK_ACK;
                    break;
                case APPLE_START_IDENTIFICATION:
                    iap2_cfg.iap_send_step = DEVICE_IDENTIFICATION;
                    break;
                case APPLE_IDENTIFICATION_ACCEPTED:
                    iap2_cfg.iap_send_step = DEVICE_START_POWER_UPDATE;                    //需要在认证通过后1s内发送电源更新参数
                    break;
                case APPLE_START_EA_SESSION:
                    iap2_cfg.ea_identifiter = 0;
                    iap2_cfg.ea_identifiter = iap2_buf_cfg.mfi_data_cache[24];
                    iap2_cfg.ea_identifiter = ((iap2_cfg.ea_identifiter << 8) | iap2_buf_cfg.mfi_data_cache[25]);
                    iap2_cfg.ea_start_flag = true;
                    iap2_cfg.app_init_flag = false;                                             //重新进入APP，初始化速度提升
                    sys_cb.phone_type = APPLE;
                    iap2_cfg.iap_send_step = DEVICE_STATUS_EA_SESSION;
                    break;
                case APPLE_STOP_EA_SESSION:
                    iap2_cfg.ea_identifiter = 0;
                    iap2_cfg.ea_identifiter = iap2_buf_cfg.mfi_data_cache[19];
                    iap2_cfg.ea_identifiter = ((iap2_cfg.ea_identifiter << 8) | iap2_buf_cfg.mfi_data_cache[20]);
                    iap2_cfg.ea_start_flag = false;
                    iap2_cfg.iap_send_step = DEVICE_STATUS_EA_SESSION;                     //EA start/stop 设备都需要通过EA status更新状态
                    break;
                case APPLE_POWER_UPDATE:
                    iap2_cfg.mfi_inited_flag = true;
                    iap2_cfg.iap_send_step = DEVICE_STOP_POWER_UPFATE;
                    break;
                case APPLE_AUTHENT_FILED:
//                    my_printf("authentication Failed\n");
                    break;
                default:                           //暂时将EA协议心跳包放在这里处理
                    break;
            }
//            TRACE("iap2_cfg.iap_send_step:%x\n",iap2_cfg.iap_send_step);
        }
    }
}

AT(.com_text.uds_iap) WEAK
void ude_iap_run_loop_execute(void *tcb)
{
    uds_t *uds = (uds_t *)tcb;
    u8 *txbuf = uds->bulk_in->buf;
    u8  databuf[2];
    u8  response_date_len[2];
     u16 iap2_info_len;
    u32 challenge_data_len = 0;

    if(ude_cfgval_get() && (uds->state != UDI_STATE_DONE) && (ude_iap_process_en) && (LOAD_DONE == iap2_cfg.upload_step)) {
//        if(iap2_cfg.mfi_inited_flag && (iap2_cfg.last_power_sta != (GPIOB & BIT(2)))) {
//            iap2_cfg.last_power_sta = (GPIOB & BIT(2));
//            memcpy(txbuf, powersource_update, sizeof(powersource_update));  //在发送startpoweruodata之后需要马上更新电源参数给手机
//            txbuf[5] = iap2_cfg.send_seq++;
//            txbuf[6] = iap2_cfg.rec_seq;                                    //rx收到的ack作为发送数据的seq
//            txbuf[8] = check_before_eight(txbuf, 8);
//            if(GPIOB & BIT(2)) {
//                iap2_cfg.last_power_sta = POWER_ENTER;
//                txbuf[19] = 0x09;
//                txbuf[20] = 0x60;
//            } else {
//                iap2_cfg.last_power_sta = POWER_EXIT;
//                txbuf[19] = 0x00;
//                txbuf[20] = 0x00;
//            }
//            txbuf[26] = check_before_eight(txbuf, 26);
//            ude_iap_transfer(uds, 27);
//        }
        switch(iap2_cfg.iap_send_step){
            case DEVICE_LINK_INIT:
                memcpy(&cfg_desc_buf[19], authentication_data_buf, iap2_cfg.load_info_len);
                memcpy(txbuf, detect_sequence, sizeof(detect_sequence));
                uds->data_size = sizeof(detect_sequence);
                ude_iap_transfer(uds, sizeof(detect_sequence));
                break;
            case DEVICE_LINK_SYN:
                memcpy(&txbuf[0], iap_syn_sequence, sizeof(iap_syn_sequence));
                txbuf[25] = check_before_eight(txbuf, 25);
                iap2_cfg.send_seq = 1;
                uds->data_size = sizeof(iap_syn_sequence);
                ude_iap_transfer(uds, sizeof(iap_syn_sequence));
                break;
            case DEVICE_LINK_ACK:
                memcpy(txbuf, iap_ack_cmd, sizeof(iap_ack_cmd));
                txbuf[5] = iap2_cfg.send_seq++;
                txbuf[6] = iap2_cfg.rec_seq;
                txbuf[7] = iap2_cfg.session_identifier;
                txbuf[8] = check_before_eight(txbuf, 8);
                uds->data_size = sizeof(iap_ack_cmd);
                ude_iap_transfer(uds, 9);
                break;
            case DEVICE_AUTHENT_CERT:
                mfi_i2c_init();
                iap2_cfg.send_seq++;
                ude_iap_transfer_cerificate_data(uds);                    //第四次收发
                break;
            case DEVICE_IDENTIFICATION:
                iap2_info_len = device_authen_buf[2];
                iap2_info_len = ((iap2_info_len <<8 )| device_authen_buf[3]);

                memcpy(txbuf, device_authen_buf, iap2_info_len);
                txbuf[5] = iap2_cfg.rec_ack + 1;                        //rx收到的ack作为发送数据的seq
                iap2_cfg.send_seq = txbuf[5] + 1;                       //后续发包该seq + 1
                txbuf[6] = iap2_cfg.rec_seq;
                txbuf[8] = check_before_eight(txbuf, 8);
                txbuf[iap2_info_len - 1] = check_before_eight(txbuf, iap2_info_len-1);
                ude_iap_transfer(uds, iap2_info_len);              //第六次收发
                break;
            case DEVICE_AUTHENT_CERT_RESPONSE:
                memcpy(txbuf, iap_ack_cmd, sizeof(iap_ack_cmd));   //发送challenage之前需要提前发送一个ack，避免获取数据时间过长，ack回复超时
                txbuf[5] = iap2_cfg.send_seq++;
                txbuf[6] = iap2_cfg.rec_seq;
                txbuf[8] = check_before_eight(txbuf, 8);
                uds->data_size = sizeof(iap_ack_cmd);
                ude_iap_transfer(uds, 9);

                memcpy(txbuf, &iap2_buf_cfg.mfi_data_cache[19], 32);                            //第五次收发（challenage）
                mfi_i2c_write_data(0x21, txbuf, 32);                      //write challenage data
                mfi_i2c_read_data(0x20, databuf, 2);                      //read challenage length
                txbuf[0] = 0x01;
                iap2_cfg.send_seq++;
                mfi_i2c_write_data(0x10, txbuf, 1);                       //write authentication control and status
                if(cfg_auth_cp_version){
                    mfi_i2c_read_data(0x10, txbuf, 1);                   //read authentication status
                    mfi_i2c_read_data(0x11, response_date_len, 2);         //read challenage response data length
                    mfi_i2c_read_data(0x12, &txbuf[19], 64);             //read response data
                    challenge_data_len = ((response_date_len[0]<<8) + response_date_len[1]) + 20;
                    memcpy(txbuf, iap_challenage_cmd, sizeof(iap_challenage_cmd));
                    txbuf[2] = challenge_data_len >> 8;
                    txbuf[3] = (u8)challenge_data_len;
                    txbuf[6] = iap2_cfg.rec_seq;
                    txbuf[8] = check_before_eight(txbuf, 8);
                    txbuf[11] = (challenge_data_len - 10) >> 8;
                    txbuf[12] = (u8)(challenge_data_len - 10);
                    txbuf[15] = (challenge_data_len - 16) >> 8;
                    txbuf[16] = (u8)(challenge_data_len - 16);
                    txbuf[83] = check_before_eight(txbuf, 83);
                    ude_iap_transfer(uds, 84);
                } else {
                    mfi_i2c_read_data(0x12, &txbuf[19], 128);              //read response data
                    memcpy(txbuf, iap_challenage_cmd, sizeof(iap_challenage_cmd));
                    txbuf[6] = iap2_cfg.rec_seq;
                    txbuf[8] = check_before_eight(txbuf, 8);
                    txbuf[147] = check_before_eight(txbuf, 147);
                    ude_iap_transfer(uds, 148);
                }
                TRACE("5\n");
                break;
            case DEVICE_POWER_SOURCE_UPDATE:
                memcpy(txbuf, powersource_update, sizeof(powersource_update));
                txbuf[5] = iap2_cfg.send_seq++;
                txbuf[6] = iap2_cfg.rec_seq;                                //rx收到的ack作为发送数据的seq
                txbuf[8] = check_before_eight(txbuf, 8);
                txbuf[26] = check_before_eight(txbuf, 26);
                ude_iap_transfer(uds, 27);                            //第八次收发
                break;
            case DEVICE_REQUEST_APP_LAUNCH:
//                memcpy(txbuf, app_launch, sizeof(app_launch));
//                txbuf[5] = iap2_cfg.send_seq++;
//                txbuf[6] = iap2_cfg.rec_seq;                                //rx收到的ack作为发送数据的seq
//                txbuf[8] = check_before_eight(txbuf, 8);
//                txbuf[36] = check_before_eight(txbuf, 36);
//                ude_iap_transfer(uds, 37);
                break;
            case DEVICE_STATUS_EA_SESSION:
                memcpy(txbuf, ea_session_status, sizeof(ea_session_status));
                txbuf[5] = iap2_cfg.send_seq++;
                txbuf[6] = iap2_cfg.rec_seq;                                //rx收到的ack作为发送数据的seq
                txbuf[8] = check_before_eight(txbuf, 8);
                txbuf[19] = iap2_cfg.ea_identifiter >> 8;
                txbuf[20] = (iap2_cfg.ea_identifiter & 0xFF);
                txbuf[25] = (!iap2_cfg.ea_start_flag);                      //SessionClose
                txbuf[26] = check_before_eight(txbuf, 26);
                ude_iap_transfer(uds, 27);
                break;
            case DEVICE_START_POWER_UPDATE:
                memcpy(txbuf, iap_startPowerUpdates_cmd, sizeof(iap_startPowerUpdates_cmd));
                txbuf[5] = iap2_cfg.send_seq++;;
                txbuf[6] = iap2_cfg.rec_seq;                                    //rx收到的ack作为发送数据的seq
                txbuf[8] = check_before_eight(txbuf, 8);
                txbuf[31] = check_before_eight(txbuf, 31);
                ude_iap_transfer(uds, 32);

                memcpy(txbuf, powersource_update, sizeof(powersource_update));  //在发送startpoweruodata之后需要马上更新电源参数给手机
                txbuf[5] = iap2_cfg.send_seq++;
                txbuf[6] = iap2_cfg.rec_seq;                                    //rx收到的ack作为发送数据的seq
                txbuf[8] = check_before_eight(txbuf, 8);
//                if(GPIOB & BIT(2)) {
//                    iap2_cfg.last_power_sta = POWER_ENTER;
//                    txbuf[19] = 0x09;
//                    txbuf[20] = 0x60;
//                } else {
//                    iap2_cfg.last_power_sta = POWER_EXIT;
//                    txbuf[19] = 0x00;
//                    txbuf[20] = 0x00;
//                }
                txbuf[26] = check_before_eight(txbuf, 26);
                ude_iap_transfer(uds, 27);

                break;
            case DEVICE_STOP_POWER_UPFATE:
                memcpy(txbuf, iap_stopPowerUpdates_cmd, sizeof(iap_stopPowerUpdates_cmd));
                txbuf[5] = iap2_cfg.send_seq++;;
                txbuf[6] = iap2_cfg.rec_seq;                                //rx收到的ack作为发送数据的seq
                txbuf[8] = check_before_eight(txbuf, 8);
                txbuf[16] = check_before_eight(txbuf, 16);
                ude_iap_transfer(uds, 17);
                break;
            case DEVICE_EA_SESSION_PROCESS:
                while(ring_buf_get_count(&(ios_data_cache.ios_data_cbuf))&& (ios_data_cache.r_idx != ios_data_cache.w_idx)) {
                    ring_buf_get(&(ios_data_cache.ios_data_cbuf), ios_data_cache.ios_cache, ios_data_cache.ios_outlen_buf[ios_data_cache.r_idx]);
                    ios_data_cache.r_idx++;
                    if(ios_data_cache.r_idx >= 100) {
                        ios_data_cache.r_idx = 0;
                    }
                }
                iap2_cfg.iap_data_send_flag = true;
                ude_iap_tx_process();
                break;
            case DEVICE_IDLE:
//                memset(txbuf, 0, uds->data_size);
                break;
            default:
                break;
        }

        iap2_cfg.iap_send_step = DEVICE_IDLE;
    } else {
//        printf("ude_cfgval_get():%x,uds->state:%x,iap2_cfg.upload_step:%x\n",ude_cfgval_get(),uds->state,iap2_cfg.upload_step);
    }

}


AT(.com_text.uds_iap)
void ude_iap_get_data_callback(u8 *ptr, u8 len)
{
    iap2_cfg.iap_data_send_flag = false;
    memcpy(iap2_buf_cfg.ota_data_cache, ptr, len);

    u8 *ptr_iap = iap2_buf_cfg.ota_data_cache;

    ring_buf_t *ring_buf = &(apple_rec_cbuf);
    ring_buf_put(ring_buf, iap2_buf_cfg.ota_data_cache, len);

    if(memcmp(ptr_iap, detect_sequence, 6) == 0) {
        iap2_cfg.packet_size = 6;
    } else if((ptr_iap[0] == 0xFF) && (ptr_iap[1] == 0x5A)){
        iap2_cfg.packet_size = ptr_iap[2];
        iap2_cfg.packet_size = ((iap2_cfg.packet_size << 8) | ptr_iap[3]);
    }
//    my_printf("rec\n");
//    my_print_r(ptr, len);
    iap2_cfg.iap_rec_packet_flag = 1;
    iap_packet_analy();

}

void iap2_info_init(void)
{
    memset(&iap2_authen_cfg, 0x00, sizeof(iap2_authen_cfg_t));
    memset(device_authen_buf, 0x00, sizeof(device_authen_buf));
    iap2_product_name_init();
    iap2_serial_number_init();
    iap2_model_init();
    iap2_manufacture_init();
}

///库函数回调
void ude_iap_app_init(void)
{
    memset(&iap2_cfg, 0, sizeof(mfi_iap2_cfg_t));
    memset(&authentication_data_buf, 0, sizeof(authentication_data_buf));
    memset(&iap2_buf_cfg, 0, sizeof(iap2_buf_cfg_t));

    memset(&apple_rec_cbuf, 0, sizeof(apple_rec_cbuf));
    memset(&apple_send_cbuf, 0, sizeof(apple_send_cbuf));
    memset(&apple_ota_cbuf, 0, sizeof(apple_ota_cbuf));
    memset(&ios_data_cache, 0, sizeof(ios_data_cache));


    ring_buf_init(&apple_rec_cbuf, &iap_buf_tbl[0], 1, 0);
    ring_buf_init(&apple_send_cbuf, &iap_buf_tbl[1], 1, 0);
    ring_buf_init(&(ios_data_cache.ios_data_cbuf), &iap_buf_tbl[2], 1, 0);

    iap2_info_init();
    got_iap2_authen_info();

    if(0 == auth_cp_oneline_en){
        iap2_cfg.upload_step = LOAD_DONE;
    }
}

#endif
