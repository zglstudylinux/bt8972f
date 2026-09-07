#include "include.h"

#if BT_MAP_EN

int nibble_for_char(char c);
void bt_map_start(void);
void bt_get_time(char *ptr)
{
    my_printf("-->set time\n");
#if UART0_PRINTF_SEL
    bool ios_flag = bt_is_ios_device();
    u16 year = ios_flag? 2000 + nibble_for_char(ptr[2])*10 + nibble_for_char(ptr[3]) : nibble_for_char(ptr[0])*1000 + nibble_for_char(ptr[1])*100 + nibble_for_char(ptr[2])*10 + nibble_for_char(ptr[3]);
    u8 month = nibble_for_char(ptr[4])*10 + nibble_for_char(ptr[5]);
    u8 day = nibble_for_char(ptr[6])*10 + nibble_for_char(ptr[7]);
    u8 hour = nibble_for_char(ptr[9 - ios_flag])*10 + nibble_for_char(ptr[10 - ios_flag]);
    u8 min = nibble_for_char(ptr[11 - ios_flag])*10 + nibble_for_char(ptr[12 - ios_flag]);
    u8 sec = nibble_for_char(ptr[13 - ios_flag])*10 + nibble_for_char(ptr[14 - ios_flag]);
    if((year >= 2000) && (year <= 3000) && (month <= 12) && (day <= 31) && (hour <= 24) && (min <= 60) && sec <= 60){
        ios_flag ? printf("IOS get time:\n") : printf("Android get time:\n");
        printf("date:%04d.%02d.%02d time:%02d:%02d:%02d\n",year,month,day,hour,min,sec);
        if(!ios_flag){
            sys_cb.map_retry = 0;
        }
    }else{
        if(!ios_flag){
            sys_cb.map_retry = 1;
			bt_map_start();
        }
    }
#endif
}

void bt_map_data_callback(uint8_t *packet)     //获取时间例程
{
    char *ptr = (char *)(packet+13);
    bt_get_time(ptr);
}
#endif

