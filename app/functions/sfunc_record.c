#include "include.h"
#include "func.h"
#include "sfunc_record.h"

#if MUSIC_SDCARD_EN
void record_enter(void) {};           //用于lock代码，暂时先不lock
void record_exit(void) {};
rec_src_t rec_src AT(.buf.record.vars);
rec_cb_t  rec_cb AT(.buf.record.vars);
rec_enc_t rec_enc_cb AT(.buf.record.vars);

u8 rec_obuf[REC_OBUF_SIZE] AT(.buf.record.obuf);
u8 rec_encbuf[REC_ENC_SIZE] AT(.buf.record.encbuf);
char fname_buf[100] AT(.buf.record.fname);
char fname_time_buf[100] AT(.buf.record.fname_time);
static u8 record_time_stamp[512] AT(.buf.record.timestamp);
static u8 stamp_num;
static u8 file_mode;
volatile u8 record_stop_sta = 0;
static u8 w_cnt;

#if WRITE_2K
u8 rec_2k_buf[2000] AT(.buf.record.rec);
#else
u8 rec_2k_buf[2] AT(.buf.record.rec);
#endif
extern volatile u8 sd_write_cnt;
u8 wav_header_buf[512] AT(.buf.record.header);

AT(.text.record.table)
const u8 wav_header_tbl[52] = {
    0x52, 0x49, 0x46, 0x46, 0x00, 0x00, 0x00, 0x00, 0x57, 0x41, 0x56, 0x45, 0x66, 0x6D, 0x74, 0x20,
	0x14, 0x00, 0x00, 0x00, 0x01, 0x00, 0x02, 0x00, 0x44, 0xAC, 0x00, 0x00, 0x10, 0xB1, 0x02, 0x00,
	0x04, 0x00, 0x10, 0x00, 0x02, 0x00, 0x00, 0x00, 0x66, 0x61, 0x63, 0x74, 0xC8, 0x01, 0x00, 0x00,
	0x00, 0x00, 0x00, 0x00
};

/*-------------/
以长文件名R20210412-112234.WAV为例
/----------- */
AT(.text.record.table)
const char file_name_mic[] = "MIC1001.WAV";

AT(.text.record.table)
const char file_name_time[] = ".txt";

AT(.text.record.table_time)
const char time_char[] = "0123456789:";

AT(.text.func.record)
bool sfunc_rec_mkdir(void)
{
    FRESULT res = FR_OK;
    u8 len;
    len = 0;
//    const char *path;
    memset(fname_buf, 0 ,sizeof(fname_buf));

    if ((res == FR_OK) || (res == FR_EXIST)) {
//        memcpy(fname_buf, path, len);
        fname_buf[len++] = '/';
        fname_buf[99] = len;            //保存目录PATH的位置，生成文件PATH时使用
        return true;
    }
    return false;
}

static bool rec_file_sfn_inc(u8 *fn)
{
    u8 *ptr = fn + SFN_NUM_LEN - 1;

    *ptr += 1;
    for (u8 i = 0; i < (SFN_NUM_LEN - 1); i++) {
        if(*ptr > '9') {
            *ptr-- = '0';
            *ptr += 1;
        } else {
            return true;
        }
    }

    return (*ptr > '9')? false : true;
}

AT(.text.func.record)
bool sfunc_rec_create_file(void)
{
    FRESULT res;
    u8 pos = fname_buf[99];
    if(file_mode) {
        strcpy((char *)&fname_buf[pos], fname_time_buf);
    } else {
        strcpy((char *)&fname_buf[pos], file_name_mic);
    }
    pos += MIC_SFN_NUM_OFS;
//    fs_create_time_inc();
//    rtc_rec_set_time();
    while (1) {
        res = fs_open((const char *)fname_buf, FA_WRITE|FA_CREATE_NEW);
        if (res == FR_OK) {
            printf("creat file ok\n");
            break;
        } else if (res == FR_EXIST) {
            if (!rec_file_sfn_inc((u8 *)&fname_buf[pos])) {
                return false;
            }
        } else {
            return false;
        }
    }

    if(!file_mode) {
        strcpy((char *)fname_time_buf, (char *)(&fname_buf[fname_buf[99]]));
        strcpy((char *)&fname_time_buf[strlen(fname_time_buf) - 4], (char *)file_name_time);
    }
    my_printf("%s: %s\n", __func__, fname_buf);
    return true;
}


AT(.text.func.record)
void creat_time_stamp(void)                //创建录音文件时间戳
{
    u8 hour = 0, minute = 0, second = 0;
    u8 timestamp_buf[9] = {0x30, 0x30, 0x3A, 0x30, 0x30, 0x3A, 0x30, 0x30, 0x0A};       //00:00:00/n
    if(sys_cb.fs_recording_cnt > 3600) {        //小时
        hour = (u8)sys_cb.fs_recording_cnt/3600;
        minute = (u8)((sys_cb.fs_recording_cnt - hour*3600)/60);
        second = (u8)(sys_cb.fs_recording_cnt - hour*3600 - minute*60);
    } else if(sys_cb.fs_recording_cnt > 60) {
        minute = (u8)(sys_cb.fs_recording_cnt/60);
        second = (u8)(sys_cb.fs_recording_cnt - minute*60);
    } else {
        second = (u8)(sys_cb.fs_recording_cnt);
    }
    if(hour) {
        timestamp_buf[0] = time_char[(u8)(hour/10)];
        timestamp_buf[1] = time_char[(u8)(hour%10)];
    }
    if(minute) {
        timestamp_buf[3] = time_char[(u8)(minute/10)];
        timestamp_buf[4] = time_char[(u8)(minute%10)];
    }
    if(second) {
        timestamp_buf[6] = time_char[(u8)(second/10)];
        timestamp_buf[7] = time_char[(u8)(second%10)];
    }

    memcpy(&record_time_stamp[stamp_num*9], timestamp_buf, sizeof(timestamp_buf));
    stamp_num++;
}

//可能在DMA中断调用，必须放公共区。缓存ADC数据
AT(.com_text.record)
void puts_rec_obuf(u8 *inbuf, u16 len)
{
    u16 clen, clen2, rest;
//    s32 temp = 0;
    rec_cb_t *rec = &rec_cb;
    if ((!rec->src) || (rec->len + len) > REC_OBUF_SIZE) {
    } else {
        rest = rec->obuf + REC_OBUF_SIZE - rec->wptr;
        clen = (rest < len) ? rest : len;
        clen2 = len - clen;
        if (clen) {
            memcpy(rec->wptr, inbuf, clen);
        }
        if (clen2) {
            memcpy(rec->obuf, inbuf + clen, clen2);

            rec->wptr = rec->obuf + clen2;
        } else {
            rec->wptr += clen;
        }
        GLOBAL_INT_DISABLE();
        rec->len += len;
        GLOBAL_INT_RESTORE();
    }
    music_enc_control(ENC_MSG_WAV);
}

//读取len的ADC数据
AT(.com_text.record)
bool gets_rec_obuf(u8 *buf, u16 len)
{
    u16 clen, clen2, rest;
    rec_cb_t *rec = &rec_cb;

    if (rec->len < len) {
        return false;
    }

    rest = rec->obuf + REC_OBUF_SIZE - rec->rptr;
    clen = (rest < len) ? rest : len;
    clen2 = len - clen;
    if (clen) {
        memcpy(buf, rec->rptr, clen);
    }
    if (clen2) {
        memcpy(buf + clen, rec->obuf, clen2);
        rec->rptr = rec->obuf + clen2;
    } else {
        rec->rptr += clen;
    }
    GLOBAL_INT_DISABLE();
    rec->len -= len;
    GLOBAL_INT_RESTORE();

    return true;
}

//缓存压缩好的数据
AT(.com_text.record)
bool puts_rec_encbuf(u8 *buf, u16 len)
{
    u16 clen, clen2, rest;
    rec_enc_t *enc = rec_cb.enc;
    if ((!enc) || (enc->len + len) > REC_ENC_SIZE) {
        return false;             //record enc obuf full
    }
    rest = enc->buf + REC_ENC_SIZE - enc->wptr;
    clen = (rest < len) ? rest : len;
    clen2 = len - clen;
    if (clen) {
        memcpy(enc->wptr, buf, clen);
    }
    if (clen2) {
        memcpy(enc->buf, buf + clen, clen2);
        enc->wptr = enc->buf + clen2;
    } else {
        enc->wptr += clen;
    }
    GLOBAL_INT_DISABLE();
    enc->len += len;
    GLOBAL_INT_RESTORE();

    return true;
}

//wave只是将数据从pcmbuf搬到encbuf
AT(.com_text.record)
void rec_wave_process(void)
{
//#if FUNC_REC_EN && (REC_TYPE_SEL != REC_WAV)
//
//    u8 *rbuf = &avio_buf[0];
//
//    if (!gets_rec_obuf(rbuf, 512)) {
//        return;
//    }
//    puts_rec_encbuf(rbuf, 512);
//#endif
}

//取len压缩好的数据
AT(.com_text.record)
bool gets_rec_encbuf(u8 *buf, u16 len)
{
    u16 clen, clen2, rest;
    rec_cb_t *rec = &rec_cb;
    rec_enc_t *enc = rec->enc;

    if (enc->len < len) {
        return false;
    }

    rest = enc->buf + REC_ENC_SIZE - enc->rptr;
    clen = (rest < len) ? rest : len;
    clen2 = len - clen;
    if (clen) {
        memcpy(buf, enc->rptr, clen);
    }
    if (clen2) {
        memcpy(buf + clen, enc->buf, clen2);
        enc->rptr = enc->buf + clen2;
    } else {
        enc->rptr += clen;
    }
    GLOBAL_INT_DISABLE();
    enc->len -= len;
    GLOBAL_INT_RESTORE();
    return true;
}

AT(.text.func.record)
bool sfunc_rec_write_file(u8 *buf)
{
    FRESULT res;

    #if WRITE_2K
    res = fs_write(buf, 8192);
    sd_write_cnt = 0;
    #else
    res = fs_write(buf, 512);
    #endif
    if (res != FR_OK) {
        if (res == FR_NOT_ENOUGH_CORE) {
            printf("record disk full\n");
        } else {
            printf("record disk failed: %d\n", res);
        }
        sfunc_record_stop();
        rec_cb.flag_play = 1;
        return false;
    }

    return true;
}

AT(.text.func.record)
bool rec_wave_init(rec_cb_t *rec)
{
    u8 *wbuf = avio_buf;
    wav_header_t *wavhead = (wav_header_t *)wav_header_buf;
    rec->src->nchannel = 1;

    memset(wav_header_buf, 0, 512);
    memcpy(wav_header_buf, wav_header_tbl, sizeof(wav_header_tbl));

    wavhead->wFormatTag = WAVE_FORMAT_PCM;
    wavhead->nChannels  = rec->src->nchannel & 0x03;
    wavhead->nSamplesPerSec  = tbl_sample_rate[rec->src->spr];
    wavhead->nAvgBytesPerSec = (wavhead->nSamplesPerSec * wavhead->nChannels * rec->src->pcm_bits) / 8;
    wavhead->nBlockAlign = (wavhead->nChannels * rec->src->pcm_bits) / 8;
    wavhead->wBitsPerSample = rec->src->pcm_bits;
    wavhead->data_id = DATA_CKID;
    if(file_mode) {
        memcpy(wbuf, record_time_stamp, 512);
    } else {
        memcpy(wbuf, wav_header_buf, 512);
    }
    if (fs_write(wbuf, 512) != FR_OK) {
        return false;
    }
    rec->fssect = fs_get_file_ssect();
    return true;
}

AT(.text.func.record)
static bool rec_wav_header_sync(rec_cb_t *rec)
{
    u8 *wbuf = avio_buf;
    wav_header_t *wavhead = (wav_header_t *)wav_header_buf;
    u32 fsize = fs_get_file_size();
    wavhead->riff_size = fsize - 8;
    wavhead->data_size = fsize - 512;
    wavhead->dwSampleLength = (fsize - 512)/wavhead->nBlockAlign;  //Number of samples

    if(file_mode) {
        memcpy(wbuf, record_time_stamp, 512);
    } else {
        memcpy(wbuf, wav_header_buf, 512);
    }
    if (disk_writep(wbuf, rec->fssect)) {
        return false;
    }
    return true;
}

AT(.text.func.record)
void sfunc_rec_proc(void)
{
    #if WRITE_2K
    u8 *recbuf = &rec_2k_buf[0];
    #endif
    u8 *rbuf = &avio_buf[0];
    if (!sfunc_is_recording()) {
        return;
    }

    if (!gets_rec_encbuf(rbuf, 512)) {
        return;
    }
    #if WRITE_2K
    memcpy(&recbuf[w_cnt*512], rbuf, 512);
    w_cnt ++;
    if (w_cnt == 32) {
        w_cnt = 0;
        if (!sfunc_rec_write_file(rec_2k_buf)) {
            printf("write err!\n");
            return;
        } else {
            fs_sync();                            //同步一下文件
        }
        memset(rec_2k_buf, 0, sizeof(rec_2k_buf));
    }
    #else

    if (!sfunc_rec_write_file(rbuf)) {
        printf("write err!\n");
        return;
    } else {
        sys_cb.recording_size_cnt++;
    }
    if(sys_cb.recording_size_cnt > 0x3FFFF600) {         //超过文件上限大小(4G)，重新生成下一个文件继续录音
        if (sfunc_is_recording()) {
            printf("file next\n");
            sfunc_record_stop();
            msg_enqueue(MSG_VOL_UP);
        }
    }
    #endif
}

AT(.text.func.record)
static bool rec_file_close(rec_cb_t *rec)
{
     if (1 == sys_cb.rec_format) {
        if (rec->sco_flag || (REC_TYPE_SEL == REC_WAV) || (REC_TYPE_SEL == REC_ADPCM)) {
            if (!rec_wav_header_sync(rec)) {
                return false;
            }
         }
     }

    if (fs_close() != FR_OK) {
        return false;
    }
    return true;
}

AT(.com_text.func.record)
bool sfunc_is_recording(void)
{
    if (rec_cb.sta == REC_RECORDING) {
        return true;
    }
    return false;
}

AT(.text.func.record)
bool sfunc_record_is_pause(void)
{
    if (rec_cb.sta == REC_PAUSE) {
        return true;
    }
    return false;
}

AT(.text.func.record)
void sfunc_record_stop_do(void)
{
    rec_cb_t *rec = &rec_cb;
    if (rec->sta == REC_STOP) {
        return;
    }
    record_exit();
    if (rec->flag_file) {
        rec_file_close(rec);
    }
    rec->flag_file = 0;
    rec->flag_dir = 0;
    rec->tm_sec = 0;
    rec->src = 0;
    rec->enc = 0;
    rec->sta = REC_STOP;
    if (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_SDCARD1)) {
        sd0_stop(1);
    }
    w_cnt = 0;
    memset(rec_2k_buf, 0, sizeof(rec_2k_buf));
}

AT(.text.func.record)
void sfunc_record_stop(void)
{
    printf("record stop\n");
    sys_cb.fs_recording_cnt = 0;
    sys_cb.recording_size_cnt = 0;
    sfunc_record_stop_do();

    file_mode = 1;
    sfunc_record_start();
    sfunc_record_stop_do();
    file_mode = 0;

}

AT(.text.func.record)
void sfunc_record_pause(void)
{
    rec_cb_t *rec = &rec_cb;

    if (rec->sta == REC_RECORDING) {
        rec->src->source_stop();
        record_exit();
        if (dev_is_online(DEV_SDCARD) || dev_is_online(DEV_SDCARD1)) {
            sd0_stop(1);
        }
        rec->src = 0;
        rec->enc = 0;
        rec->sta = REC_PAUSE;
        printf("record pause\n");
    }
}

AT(.text.func.record)
void sfunc_record_continue(void)
{
    if (rec_cb.sta == REC_PAUSE) {
        printf("record continue\n");
        sfunc_record_start();
    }
}

AT(.text.func.record)
bool sfunc_fwrite_sync(void)
{
    rec_cb_t *rec = &rec_cb;

    rec->tm_sec++;
    if (rec->tm_sec % REC_SYNC_TIMES) {
        return true;
    }
    if (1 == sys_cb.rec_format) {
        if (rec->sco_flag || (REC_TYPE_SEL == REC_WAV) || (REC_TYPE_SEL == REC_ADPCM)) {
            if (!rec_wav_header_sync(rec)) {
                return false;
            }
        }
    }
    if (fs_sync() != FR_OK) {
        return false;
    }
    return true;
}

AT(.text.func.record)
void sfunc_record_process(void)
{
    sfunc_rec_proc();
}

AT(.text.func.record)
bool sfunc_record_switch_device(void)
{
    if (sys_cb.cur_dev == DEV_SDCARD) {
        if (dev_is_online(DEV_UDISK)) {
            sys_cb.cur_dev = DEV_UDISK;
            return true;
        } else if (dev_is_online(DEV_SDCARD1)) {
            sys_cb.cur_dev = DEV_SDCARD1;
            return true;
        }
    } else if (sys_cb.cur_dev == DEV_UDISK) {
        if (dev_is_online(DEV_SDCARD1)) {
            sys_cb.cur_dev = DEV_SDCARD1;
            return true;
        } else if (dev_is_online(DEV_SDCARD)) {
            sys_cb.cur_dev = DEV_SDCARD;
            return true;
        }
    } else if (sys_cb.cur_dev == DEV_SDCARD1) {
        if (dev_is_online(DEV_SDCARD)) {
            sys_cb.cur_dev = DEV_SDCARD;
            return true;
        } else if (dev_is_online(DEV_UDISK)) {
            sys_cb.cur_dev = DEV_UDISK;
            return true;
        }
    }
    return false;
}

void record_mode(u8 mode)
{
    if(mode == FUNC_BT) {
        rec_src.spr = 6;              //0:48KHz,6:16K
        rec_src.pcm_bits = 16;        //16bit/32bit
    } else if(mode == FUNC_DEVICE) {
        rec_src.spr = 0;              //0:48KHz,6:16K
        rec_src.pcm_bits = 32;        //16bit/32bit
    }
}

AT(.text.func.record)
bool sfunc_record_start(void)
{
    if(!sdadc_mic_is_use()) {           //ADC没启动，禁止录音
        return false;
    }

    rec_cb_t *rec = &rec_cb;
    sys_cb.rec_format = 1;
    printf("record start\n");
    record_mode(func_cb.sta);      //匹配录音模式
    rec->sta = REC_STARTING;

    fsdisk_callback_init(sys_cb.cur_dev);
    if (!rec->flag_file) {
        if (!fs_mount()) {
            return false;
        }
    }

    //创建录音文件夹
    if (!rec->flag_dir) {
        if (!sfunc_rec_mkdir()) {
            return false;
        }
        rec->flag_dir = 1;
    }

    //创建录音文件
    if (!rec->flag_file) {
        if (!sfunc_rec_create_file()) {
            return false;
        }
        rec->flag_file = 1;
    }


    rec->src = &rec_src;
    do {
    if (1 == sys_cb.rec_format) {
        if (!rec_wave_init(rec)) {
            my_printf("%s %d\n",__func__,__LINE__);
            rec->src = 0;
            return false;
        }
    }
    } while(0);

    record_enter();
    memset(&rec_enc_cb, 0, sizeof(rec_enc_cb));
    rec->enc = &rec_enc_cb;
    {
        rec->enc->buf = rec->enc->rptr = rec->enc->wptr = rec_encbuf;
        rec->obuf = rec->wptr = rec->rptr = rec_obuf;
    }
    rec->len = 0;
//    rec->src->source_start();
    rec->sta = REC_RECORDING;

    return true;
}

void record_var_init(void)            //初始化，默认录音文件为48K 16bit
{
    memset(&rec_cb, 0, sizeof(rec_cb));
    memset(&rec_src, 0, sizeof(rec_src));
    rec_src.spr = 6;              //0:48KHz,6:16K
    rec_src.pcm_bits = 16;        //16bit/32bit
    rec_cb.first_flag = 1;
}

AT(.text.func.record)
void sfunc_record_enter(void)
{
    if (dev_is_online(DEV_UDISK)) {
        sys_cb.cur_dev = DEV_UDISK;
    } else if (dev_is_online(DEV_SDCARD1)) {
        sys_cb.cur_dev = DEV_SDCARD1;
    } else if (dev_is_online(DEV_SDCARD)) {
        sys_cb.cur_dev = DEV_SDCARD;
    } else {
        return;
    }
    sfunc_record_display_enter();
    sys_cb.rec_format = 1;
    rec_cb.src->nchannel = 1;

    ///待优化模块
//    if (!sfunc_record_start()) {
//        if (sfunc_record_switch_device()) {
//            sfunc_record_start();
//        }
//    }
//    sfunc_record_stop();
}

AT(.text.func.record)
void sd_record_init(void)       //初始化，挂在SD卡文件系统
{
    sd_disk_init();
    sd_insert();
    sd_enable_user();
    memset(record_time_stamp, 0, sizeof(record_time_stamp));
    if(!sd0_init()) {
        printf("sd0 init faile\n");
    }
    sys_cb.sd_card_init_flag = 1;
    fsdisk_callback_init(0);
    if(!fs_mount()) {
        printf("fs_mount faile\n");
    }
}

AT(.text.func.record)
void kick_record_start(void)       //初始化并启动麦，开始录音
{
#if DEVICE_LOCAL_REC_EN
    if(func_cb.sta == FUNC_DEVICE) {
        sys_clk_req(INDEX_MUSIC, SYS_120M);
        sys_clk_req(INDEX_MUSIC, DEVICE_WL_ALG_EN_CLK);                     //先抬高主频
        wireless_device_init();
        mic_dma_start();
        wireless_cb.alg_en = 1;
        if(wireless_cb.connected_sta == 0) {                              //未连接的时候mute一下算法
            ylcrn_L3_mic_mute_set(1);
        }
    } else if(func_cb.sta == FUNC_BT && !sdadc_mic_is_use()) {
//        sco_buffer_init();
//        sco_audio_init();
//        sco_adc_dma_kick();
        //kick SCO链路mic,切换模式需要完全退出
    }
#endif
}


AT(.text.func.record)
void sfunc_record_exit(void)
{
    if (rec_cb.sta != REC_PAUSE) {
        sfunc_record_stop();
        rec_cb.flag_play = 1;
    }

    if (sfunc_is_recording()) {
        sfunc_record_stop();
    }

}

AT(.text.func.record)
void sfunc_mic_rec_process(void)
{
    if(((rec_cb.sta == REC_RECORDING) || (rec_cb.sta == REC_PAUSE)) && (!wireless_role_is_adapter())) {
        sfunc_record_process();
    }
}
#else
void record_var_init(void){}
bool sfunc_record_start(void){return false;}
void sfunc_record_stop(void){}
void sd_record_init(void){}
bool sfunc_fwrite_sync(void){return false;}
void sfunc_record_exit(void){}
void sfunc_record_process(void){}
bool sfunc_is_recording(void){return false;}
void kick_record_start(void){}
void sfunc_mic_rec_process(void){}
#endif
