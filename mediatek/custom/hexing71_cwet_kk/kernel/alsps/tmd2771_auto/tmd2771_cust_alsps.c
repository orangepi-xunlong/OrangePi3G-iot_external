
#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <cust_alsps.h>


#if defined(K95H_PROJ)
int ZOOM_TIME = 60;
int TMD2771_CMM_PPCOUNT_VALUE = 0x12; //pulses 1`0xFF
int TMD2771_CMM_CONTROL_VALUE = 0xE0; //current 0x20-100mA 060 0xA0 0xE0
#elif defined(J1A_HNWX_PROJ)
int ZOOM_TIME = 10;
int TMD2771_CMM_PPCOUNT_VALUE = 0x12; //pulses 1`0xFF
int TMD2771_CMM_CONTROL_VALUE = 0xE0; //current 0x20-100mA 060 0xA0 0xE0
#elif defined(C1_PROJ)
int ZOOM_TIME = 60;
int TMD2771_CMM_PPCOUNT_VALUE = 0x18; //pulses 1`0xFF
int TMD2771_CMM_CONTROL_VALUE = 0xA1; //current 0x20-100mA 060 0xA0 0xE0
#else
int ZOOM_TIME = 5;//60;
int TMD2771_CMM_PPCOUNT_VALUE = 0x11;//0x18; //pulses 1`0xFF
int TMD2771_CMM_CONTROL_VALUE = 0x21;//0xA0; //current 0x20-100mA 060 0xA0 0xE0
#endif

static struct alsps_hw cust_alsps_hw = {
    .i2c_num    = 1,
#if 1//defined(J1A_HNWX_PROJ)
    .polling_mode_ps =0,
#else
    .polling_mode_ps =1,
#endif
    .polling_mode_als =1,
    .power_id   = MT65XX_POWER_NONE,    /*LDO is not used*/
    .power_vol  = VOL_DEFAULT,          /*LDO is not used*/
    //.i2c_addr   = {0x0C, 0x48, 0x78, 0x00},
#if defined(C1C_PROJ)||defined(C1J_PROJ)
    .als_level  = { 0,	0,	40,		100,	150,	300,	300,	700,	1000,	1200,	3000,	4000,	5000,	10000,	20000,	65535},
    .als_value  = {40,	40,	40,		40,		90,		160,	160,	225,	320,	640,	1280,	1280,	2600,	2600,	10240,	10240,10240},
#else
    .als_level  = { 0,  20, 41,   86,  108,  146,  180, 236, 282,  462,  804, 1288, 1560, 4240, 8000},
    .als_value  = {10, 40, 90,  150, 240, 300,  360,  460,  540,  680,  1280,  2800,  5600, 8600,  9840, 10240},
#endif
   // .ps_threshold_high = 53,
   // .ps_threshold_low = 46,
#if defined(C1C_PROJ)||defined(C1J_PROJ)   
    .ps_threshold_high = 500,
    .ps_threshold_low = 450,
#else
    .ps_threshold_high = 600,//208,//300,
    .ps_threshold_low =500,//160,// 250,
#endif
   
    };
struct alsps_hw *tmd2771_get_cust_alsps_hw(void) {
    return &cust_alsps_hw;
}
