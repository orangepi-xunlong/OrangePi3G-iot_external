#ifdef BUILD_LK
#else
#include <linux/string.h>
#if defined(BUILD_UBOOT)
#include <asm/arch/mt_gpio.h>
#else
#include <mach/mt_gpio.h>
#endif
#endif
#include "lcm_drv.h"

#if defined(BUILD_LK)
#else

#include <linux/proc_fs.h>   //proc file use 
#endif

// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)

#define REGFLAG_DELAY             							0XFE11
#define REGFLAG_END_OF_TABLE      							0xFF11   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

static struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};


static struct LCM_setting_table lcm_initialization_setting[] = {
	
	/*
	Note :

	Data ID will depends on the following rule.
	
		count of parameters > 1	=> Data ID = 0x39
		count of parameters = 1	=> Data ID = 0x15
		count of parameters = 0	=> Data ID = 0x05

	Structure Format :

	{DCS command, count of parameters, {parameter list}}
	{REGFLAG_DELAY, milliseconds of time, {}},

	...

	Setting ending by predefined flag
	
	{REGFLAG_END_OF_TABLE, 0x00, {}}
	*/

	{0xF0, 5,{0x55, 0xAA, 0x52, 0x08, 0x02}},

	{0xF6, 2,{0x60, 0x40}},

	{0xFE, 4,{0x01, 0x80, 0x09, 0x09}},

	{REGFLAG_DELAY, 50, {}},

	{0xF0, 5,{0x55, 0xAA, 0x52, 0x08, 0x01}},

	{0xB0, 1,{0x07}},

	{0xB1, 1,{0x07}},

	{0xB5, 1,{0x08}},

	{0xB6, 1,{0x54}},

	{0xB7, 1,{0x44}},

	{0xB8, 1,{0x24}},

	{0xB9, 1,{0x34}},

	{0xBA, 1,{0x14}},

	{0xBC, 3,{0x00, 0x78, 0x13}},

	{0xBD, 3,{0x00, 0x78, 0x13}},

	{0xBE, 2,{0x00, 0x1E}},

	{0xD1, 52,{0x00, 0x00, 0x00, 0x14, 0x00, 0x36, 0x00, 0x53, 0x00, 0x6C, 0x00, 0x98, 0x00, 0xBB, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x67, 0x01, 0x9D, 0x01, 0xEF, 0x02, 0x32, 0x02, 0x34, 0x02, 0x75, 0x02, 0xC1, 0x02, 0xF3, 0x03, 0x36, 0x03, 0x62, 0x03, 0x99, 0x03, 0xB8, 0x03, 0xD9, 0x03, 0xE7, 0x03, 0xF3, 0x03, 0xF9, 0x03, 0xFF}},

	{0xD2, 52,{0x00, 0x00, 0x00, 0x14, 0x00, 0x36, 0x00, 0x53, 0x00, 0x6C, 0x00, 0x98, 0x00, 0xBB, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x67, 0x01, 0x9D, 0x01, 0xEF, 0x02, 0x32, 0x02, 0x34, 0x02, 0x75, 0x02, 0xC1, 0x02, 0xF3, 0x03, 0x36, 0x03, 0x62, 0x03, 0x99, 0x03, 0xB8, 0x03, 0xD9, 0x03, 0xE7, 0x03, 0xF3, 0x03, 0xF9, 0x03, 0xFF}},

	{0xD3, 52,{0x00, 0x00, 0x00, 0x14, 0x00, 0x36, 0x00, 0x53, 0x00, 0x6C, 0x00, 0x98, 0x00, 0xBB, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x67, 0x01, 0x9D, 0x01, 0xEF, 0x02, 0x32, 0x02, 0x34, 0x02, 0x75, 0x02, 0xC1, 0x02, 0xF3, 0x03, 0x36, 0x03, 0x62, 0x03, 0x99, 0x03, 0xB8, 0x03, 0xD9, 0x03, 0xE7, 0x03, 0xF3, 0x03, 0xF9, 0x03, 0xFF}},

	{0xD4, 52,{0x00, 0x00, 0x00, 0x14, 0x00, 0x36, 0x00, 0x53, 0x00, 0x6C, 0x00, 0x98, 0x00, 0xBB, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x67, 0x01, 0x9D, 0x01, 0xEF, 0x02, 0x32, 0x02, 0x34, 0x02, 0x75, 0x02, 0xC1, 0x02, 0xF3, 0x03, 0x36, 0x03, 0x62, 0x03, 0x99, 0x03, 0xB8, 0x03, 0xD9, 0x03, 0xE7, 0x03, 0xF3, 0x03, 0xF9, 0x03, 0xFF}},

	{0xD5, 52,{0x00, 0x00, 0x00, 0x14, 0x00, 0x36, 0x00, 0x53, 0x00, 0x6C, 0x00, 0x98, 0x00, 0xBB, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x67, 0x01, 0x9D, 0x01, 0xEF, 0x02, 0x32, 0x02, 0x34, 0x02, 0x75, 0x02, 0xC1, 0x02, 0xF3, 0x03, 0x36, 0x03, 0x62, 0x03, 0x99, 0x03, 0xB8, 0x03, 0xD9, 0x03, 0xE7, 0x03, 0xF3, 0x03, 0xF9, 0x03, 0xFF}},

	{0xD6, 52,{0x00, 0x00, 0x00, 0x14, 0x00, 0x36, 0x00, 0x53, 0x00, 0x6C, 0x00, 0x98, 0x00, 0xBB, 0x00, 0xF4, 0x01, 0x22, 0x01, 0x67, 0x01, 0x9D, 0x01, 0xEF, 0x02, 0x32, 0x02, 0x34, 0x02, 0x75, 0x02, 0xC1, 0x02, 0xF3, 0x03, 0x36, 0x03, 0x62, 0x03, 0x99, 0x03, 0xB8, 0x03, 0xD9, 0x03, 0xE7, 0x03, 0xF3, 0x03, 0xF9, 0x03, 0xFF}},

	{REGFLAG_DELAY, 50, {}},

	{0xF0, 5,{0x55, 0xAA, 0x52, 0x08, 0x03}},

	{0xB0, 7,{0x05, 0x17, 0xF9, 0x53, 0x53, 0x00, 0x30}},

	{0xB1, 7,{0x05, 0x17, 0xFB, 0x55, 0x53, 0x00, 0x30}},

	{0xB2, 9,{0xFC, 0xFD, 0xFE, 0xFF, 0xF0, 0xED, 0x00, 0xC4, 0x08}},

	{0xB3, 6,{0x5B, 0x00, 0xFC, 0x5A, 0x5A, 0x03}},

	{0xB4, 11,{0x00, 0x01, 0x02, 0x03, 0x00, 0x40, 0x04, 0x08, 0xED, 0x00, 0x00}},

	{0xB5, 11,{0x40, 0x00, 0x00, 0x80, 0x5F, 0x5E, 0x50, 0x50, 0x33, 0x33, 0x55}},

	{0xB6, 7,{0xBC, 0x00, 0x00, 0x00, 0x2A, 0x80, 0x00}},

	{0xB7, 8,{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}},

	{0xB8, 3,{0x11, 0x60, 0x00}},

	{0xB9, 1,{0x90}},

	{0xBA, 16,{0x44, 0x44, 0x08, 0xAC, 0xE2, 0x64, 0x44, 0x44, 0x44, 0x44, 0x47, 0x3F, 0xDB, 0x91, 0x54, 0x44}},

	{0xBB, 16,{0x44, 0x43, 0x79, 0xFD, 0xB5, 0x14, 0x44, 0x44, 0x44, 0x44, 0x40, 0x4A, 0xCE, 0x86, 0x24, 0x44}},

	{0xBC, 4,{0xE0, 0x1F, 0xF8, 0x07}},

	{0xBD, 4,{0xE0, 0x1F, 0xF8, 0x07}},

	{REGFLAG_DELAY, 50, {}},

	{0xF0, 5,{0x55, 0xAA, 0x52, 0x08, 0x00}},

	{0xB0, 2,{0x00, 0x10}},

	{0xB4, 1,{0x10}},

	{0xB5, 1,{0x6B}},

	{0xBC, 1,{0x00}},

	{0x35, 1,{0x00}},

	{0x11, 0,{0x00}},
	{REGFLAG_DELAY, 100, {}},

	{0x29, 0,{0x00}},
	{REGFLAG_DELAY, 200, {}},

	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};



static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 0, {0x00}},
    {REGFLAG_DELAY, 120, {}},

    // Display ON
	{0x29, 0, {0x00}},
    {REGFLAG_DELAY, 100, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 0, {0x00}},

    // Sleep Mode On
	{0x10, 0, {0x00}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};





static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
				//UDELAY(5);//soso add or it will fail to send register
       	}
    }
	
}


// ---------------------------------------------------------------------------
//  LCM Driver Implementations
// ---------------------------------------------------------------------------

static void lcm_set_util_funcs(const LCM_UTIL_FUNCS *util)
{
    memcpy(&lcm_util, util, sizeof(LCM_UTIL_FUNCS));
}


static void lcm_get_params(LCM_PARAMS *params)
{
	memset(params, 0, sizeof(LCM_PARAMS));

	params->type   = LCM_TYPE_DSI;

	params->width  = FRAME_WIDTH;
	params->height = FRAME_HEIGHT;


	params->dsi.mode   = SYNC_EVENT_VDO_MODE;

	// DSI
	/* Command mode setting */
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
	params->dsi.data_format.trans_seq   = LCM_DSI_TRANS_SEQ_MSB_FIRST;
	params->dsi.data_format.padding     = LCM_DSI_PADDING_ON_LSB;
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Highly depends on LCD driver capability.
	// Not support in MT6573
	params->dsi.packet_size=256;

	// Video mode setting		
	params->dsi.intermediat_buffer_num = 2;

	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 2;
	params->dsi.vertical_backporch					= 14;
	params->dsi.vertical_frontporch					= 16;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 8;
	params->dsi.horizontal_backporch				= 24;
	params->dsi.horizontal_frontporch				= 32;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	// Bit rate calculation
	params->dsi.pll_div1=0;	
	params->dsi.pll_div2=1;
        params->dsi.fbk_div=17; 
}

static unsigned int lcm_compare_id(void);

static void lcm_init(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(6);//Must > 5ms
	SET_RESET_PIN(1);
	MDELAY(50);//Must > 50ms

	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_suspend(void)
{
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(6);//Must > 5ms
	SET_RESET_PIN(1);
	MDELAY(50);//Must > 50ms

	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
}


static void lcm_resume(void)
{
	lcm_init();
	
	push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
}

#define LCM_RM68172_ID          (0x8172)

static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xf0, 5, {0x55, 0xaa, 0x52, 0x08, 0x01}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static unsigned int lcm_compare_id(void)
{
	unsigned int id;
	unsigned char buffer[5];
	unsigned int array[5];

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(20);
	SET_RESET_PIN(1);
	MDELAY(150);

	push_table(lcm_compare_id_setting,sizeof(lcm_compare_id_setting)/sizeof(struct LCM_setting_table), 1);

	array[0] = 0x00023700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xc5, buffer, 2);
	//id = ((buffer[0] << 8) | buffer[1]);
	id = ((buffer[1] << 8) | buffer[0]);

	#if defined(BUILD_LK)
		printf("RM68172 uboot %s \n", __func__);
		printf("%s id = 0x%08x \n", __func__, id);
	#else
		printk("RM68172 kernel %s \n", __func__);
		printk("%s id = 0x%08x \n", __func__, id);
	#endif

	return ((LCM_RM68172_ID == id)? 1 : 0);
	//return 1;
}


LCM_DRIVER ry_rm68172_fwvga_dsi_vdo_hf_lcm_drv = 
{
    .name			= "ry_rm68172_fwvga_dsi_vdo_hf",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
};

