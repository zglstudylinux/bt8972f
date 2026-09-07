#include "include.h"
#include "wireless_link_list.h"


/*****************************************************************************
 * Module    : 公共部分代码
 *****************************************************************************/

wl_link_list_t * wireless_link_list_find_next(wl_link_list_t *link_list, u8 module_count)
{
    wl_link_list_t *next_link_list = ++link_list;

    for (u8 i = 0; i < module_count; i++) {
        if (next_link_list->module_enable) {
            return next_link_list;
        }
        next_link_list++;
    }

    return NULL;
}

void wl_link_list_init(wl_link_list_t *link_list, u8 module_count, u8 sample_rate, u16 samples, u8 channel)
{
    wl_link_list_t *cur_link_list = link_list;
    wl_link_list_t *next_link_list = link_list;

    for (u8 i = 0; i < module_count; i++) {
        if (cur_link_list->module_enable) {
            if (cur_link_list->module_init) {
                cur_link_list->module_init(sample_rate, samples, channel);
            }
            if ((i + 1) < module_count) {
                next_link_list = wireless_link_list_find_next(cur_link_list, (module_count - i - 1));
                if (next_link_list) {
                    cur_link_list->module_set_callback(next_link_list->module_input);
                }
            }
        }
        cur_link_list++;
    }
}

void wireless_link_list_set_module_enable(wl_link_list_t *link_list, u8 module_count, u8 module_type, u8 enable)
{
    wl_link_list_t *cur_link_list = link_list;

    for (u8 i = 0; i < module_count; i++) {
        if (cur_link_list->module_type == module_type) {
            cur_link_list->module_enable = enable;
            return;
        }
        cur_link_list++;
    }
}
