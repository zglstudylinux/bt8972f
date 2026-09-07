#ifndef _PORT_AUX_H
#define _PORT_AUX_H

bool is_aux_enter_enable(void);
bool is_aux_det_mux_micl(void);
bool aux_detect_is_busy(void);
bool aux_is_online(void);
void aux_detect_init(void);
#endif // _PORT_AUX_H
