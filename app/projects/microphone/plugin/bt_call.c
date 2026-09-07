#include "include.h"

#if BT_SCO_SMIC_AI_EN
    static dnn_cb_t dnn_cb AT(.buf.dnn_npu);
#elif BT_SCO_SMIC_AI_PRO_EN
    static dnn_pro_cb_t dnn_pro_cb AT(.buf.dnn_pro_npu);
#endif

void bt_sco_rec_exit(void);

void bt_sco_code_init(void)
{
    if (!bt_sco_is_msbc()){
        unlock_dcode();
    }
    load_code_bt_voice();
    load_code_sco_comm();
}

void bt_sco_nr_init(u8 *sysclk, nr_cb_t *nr)
{
#if BT_SCO_AGC_EN
	///MIC上行AGC算法初始化
	bt_sco_agc_init();
#endif

    ///MIC上行降噪算法
    bt_sco_near_nr_init(sysclk, nr);

    if (!xcfg_cb.bt_nr_en) {
        nr->nr_type = ((nr->nr_type & ~NR_CFG_TYPE_MASK) | NR_TYPE_NONE);
    }
}

void bt_sco_nr_exit(void)
{
    bt_sco_near_nr_exit();
}

#if BT_SCO_SMIC_AI_EN
void bt_sco_dnn_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_dnn_en) {
        printf("DNN xcfg init warning!\n");
        return;
    }
    nr->nr_type = NR_TYPE_SMIC_AI;
    memset(&dnn_cb, 0, sizeof(dnn_cb_t));

	dnn_cb.param_printf           	= 0;
	dnn_cb.nt                     	= BT_SCO_SMIC_AI_LEVEL;
	dnn_cb.nt_post                	= 0;
	dnn_cb.exp_range_H			   	= 1;
	dnn_cb.exp_range_L			   	= 0;
	dnn_cb.noise_ps_rate          	= 1;
	dnn_cb.prior_opt_idx	       	= 3;
	dnn_cb.prior_opt_ada_en	   		= 1;
	dnn_cb.wind_level			   	= 0;
	dnn_cb.wind_range			   	= 0;
	dnn_cb.low_fre_range          	= 16;
	dnn_cb.low_fre_range0         	= 0;
	dnn_cb.mask_floor			   	= 1600;
	dnn_cb.mask_floor_r		   		= 0;
	dnn_cb.music_lev			   	= 11;
	dnn_cb.comforN_level		   	= 1;
	dnn_cb.gain_expand			   	= 1024;
	dnn_cb.nn_only				   	= 0;
	dnn_cb.nn_only_len			   	= 16;
	dnn_cb.gain_assign			   	= 16666;
	dnn_cb.sin_gain_post_en	   		= 0;
	dnn_cb.sin_gain_post_len	   	= 128;
	dnn_cb.sin_gain_post_len_f	   	= 256;

	bt_dnn_init(&dnn_cb);
    *sysclk = *sysclk < SYS_48M ? SYS_48M : *sysclk;
}
#endif

#if BT_SCO_SMIC_AI_PRO_EN
void bt_sco_dnn_pro_init(u8 *sysclk, nr_cb_t *nr)
{
    if (!xcfg_cb.bt_dnn_en) {
        printf("DNN_PRO xcfg init warning!\n");
        return;
    }
    nr->nr_type = NR_TYPE_SMIC_PRO_AI;
    memset(&dnn_pro_cb, 0, sizeof(dnn_pro_cb_t));

	dnn_pro_cb.param_printf           	= 0;
	dnn_pro_cb.gain_floor          	    = 1100;
//	dnn_pro_cb.nt_post                	= 0;
//	dnn_pro_cb.exp_range_H			   	= 1;
//	dnn_pro_cb.exp_range_L			   	= 0;
	dnn_pro_cb.noise_ps_rate          	= 1;
	dnn_pro_cb.prior_opt_idx	       	= 10;
	dnn_pro_cb.prior_opt_ada_en	   		= 1;
//	dnn_pro_cb.wind_level			   	= 0;
//	dnn_pro_cb.wind_range			   	= 0;
	dnn_pro_cb.low_fre_range          	= 15;
	dnn_pro_cb.music_lev			   	= 11;
	dnn_pro_cb.nn_only				   	= 0;
	dnn_pro_cb.nn_only_len			   	= 16;
	dnn_pro_cb.gain_assign			   	= 16666;
	dnn_pro_cb.sin_gain_post_en	   		= 0;
	dnn_pro_cb.sin_gain_post_len	   	= 128;
	dnn_pro_cb.sin_gain_post_len_f	   	= 256;

	bt_dnn_pro_init(&dnn_pro_cb);
    *sysclk = *sysclk < SYS_160M ? SYS_160M : *sysclk;
}
#endif

