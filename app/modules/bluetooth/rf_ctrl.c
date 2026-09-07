#include "include.h"
#include "api.h"


//----------------------------------------------------------------------------
//获取配置中的RF参数
const uint8_t *bt_rf_get_param(void)
{
    //优先使用FT参数，其次自定义参数，最后是库预置参数
    if(xcfg_cb.ft_rf_param_en && bt_get_ft_trim_value(&xcfg_cb.rf_pa_gain)) {
        return (const uint8_t *)&xcfg_cb.rf_pa_gain;
    } else if(xcfg_cb.bt_rf_param_en) {
        return (const uint8_t *)&xcfg_cb.rf_pa_gain;
    }
    return NULL;
}

#if BT_RF_EXT_CTL_EN
const uint8_t cfg_bb_rf_ctl = BIT(0);   //bit0=RF_EXT_CTL_EN


//此处添加外部PA/LNA控制IO，注意所有函数放公共区，不能加打印
AT(.com_text.isr.txrx)
void bb_rf_ext_ctl_cb(u32 rf_sta)
{
//	if(rf_sta & BIT(8)) {			//tx on, enable PA
//	} else if(rf_sta & BIT(9)){	    //tx down, disable PA
//	} else if(rf_sta & BIT(10)){	//rx on, enable LNA
//	} else if(rf_sta & BIT(11)){	//rx down, disable LNA
//	} else {						//idle
//	}
}

//初始化蓝牙时，初始化外部PA/LNA控制IO
void bb_rf_ext_ctl_init(void)
{
}

//关闭蓝牙时，关闭外部PA/LNA
void bb_rf_ext_ctl_exit(void)
{
}
#endif


//----------------------------------------------------------------------------
//蓝牙线程获取test_ctrl命令的回调函数
uint8_t *rf_test_get_ctrl_cmd_cb(void)
{
#if IODM_TEST_EN
    return bt_rf_get_fcc_param();
#else
    return NULL;
#endif
}

//蓝牙线程获取测试功率衰减的回调函数（单位0.5dbm，取值范围0~15）
uint8_t rf_test_get_txpwr_dec_cb(uint8_t test_type)
{
    u8 txpwr_dec = 0;

    switch(test_type) {
    case TEST_TYPE_BR:
    case TEST_TYPE_EDR:
        if(bt_is_dut_en()) {
            txpwr_dec = 0;      //dut mode
        } else if (bt_is_fcc_test()) {
            txpwr_dec = 0;      //fcc mode
        } else {
            txpwr_dec = 0;      //iodm mode
        }
        break;

    case TEST_TYPE_LE_1M:
    case TEST_TYPE_LE_2M:
    case TEST_TYPE_LE_CODED_S8:
    case TEST_TYPE_LE_CODED_S2:
        if(bt_is_bqb_rf_ble()) {
            txpwr_dec = 12;     //dut mode
        } else if (bt_is_fcc_test()) {
            txpwr_dec = 12;     //fcc mode
        } else {
            txpwr_dec = 0;      //iodm mode
        }
        break;
    }

    return txpwr_dec;
}

