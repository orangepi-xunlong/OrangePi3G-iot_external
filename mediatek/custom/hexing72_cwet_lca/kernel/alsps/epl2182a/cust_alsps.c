#include <linux/types.h>

#ifdef MT6513
#include <mach/mt6513_pm_ldo.h>
#endif

#ifdef MT6516
#include <mach/mt6516_pm_ldo.h>
#endif

#ifdef MT6575
#include <mach/mt6575_pm_ldo.h>
#endif

#ifdef MT6577
#include <mach/mt6577_pm_ldo.h>
#endif

#include <cust_alsps.h>
//#include <mach/mt6575_pm_ldo.h>
#ifdef MT6572
#include <mach/mt_pm_ldo.h>
#endif
static struct alsps_hw cust_alsps_hw = {
	.i2c_num    = 0,
	.polling_mode_ps = 1,
	.power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
	.power_vol  = VOL_DEFAULT,          /*LDO is not used*/
	.i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
	.als_level	= {20, 45, 70, 90, 150, 300, 500, 700, 1150, 2250, 4500, 8000, 15000, 30000, 50000},
	.als_value	= {10, 30, 60, 80, 100, 200, 400, 600, 800, 1500, 3000, 6000, 10000, 20000, 40000, 60000},
	.ps_threshold_low = 100,
	.ps_threshold_high = 500,
};
struct alsps_hw *get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}

