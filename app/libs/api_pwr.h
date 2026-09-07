#ifndef _API_PWR_H_
#define _API_PWR_H_

#define WKO_10SRST_EN(x)       { RTCCON10 = BIT(10);\
                                RTCCON1 &= ~BIT(8);\
                                if (x) {RTCCON1 |= BIT(8);}\
                                RTCCON12 = (RTCCON12 & ~(0x03 << 8 | 0x03)) | (1 << 8); }

#define WKO_10SRST_DIS()        RTCCON12 |= 0x03


extern uint8_t cfg_pmu_vddio_lp_enable;

void pmu_init(uint cfg);        //bit[3:0]=ldo/buck, bit[7:4]=vcm_set
void pmu_set_mode(uint cfg);    //bit[3:0]=ldo/buck

u32 getcfg_pmu_mode(void);

#endif // _API_PWR_H_

