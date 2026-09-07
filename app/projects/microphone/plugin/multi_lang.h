#ifndef _MULTI_LANG_H
#define _MULTI_LANG_H

typedef struct {
    u32 *ptr;
    u32 *len;
} res_addr_t;

extern const res_addr_t en_lang_tbl[RES_IDX_MAX];
extern const res_addr_t zh_lang_tbl[RES_IDX_MAX];

#if (LANG_SELECT == LANG_EN_ZH)
void multi_lang_init(uint lang_id);
#elif (LANG_SELECT == LANG_EN || LANG_SELECT == LANG_ZH)
#else
    #error "请选择提示音语言\n"
#endif

#endif
