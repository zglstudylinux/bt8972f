#ifndef __VBAT_H
#define __VBAT_H


void vbat_init(void);
void vbat_voltage_init(void);
uint16_t vbat_get_voltage(void);
uint vbat_get_lpwr_sta(void);

#endif // __VBAT_H
