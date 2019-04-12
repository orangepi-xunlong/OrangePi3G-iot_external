#ifdef BUILD_LK
#include "mt_gpio.h"
#else
    #include <linux/string.h>
    #if defined(BUILD_UBOOT)
        #include <asm/arch/mt_gpio.h>
    #else
        #include <mach/mt_gpio.h>
    #endif
#include "mach/mt_gpio.h"
#endif
#include "lcm_drv.h"
//#include "gpio_const.h"

#include "cust_gpio_usage.h"


// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  										(480)
#define FRAME_HEIGHT 										(854)
#define LCM_ID_HX8379C											(0x79)

#define REGFLAG_DELAY             							0XFFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE									0

//#define LCM_DEBUG
#if defined(BUILD_LK)
	#if defined(BUILD_LK)
	#define LCM_LOG(fmt, args...)    printf(fmt, ##args)
	#else
	#define LCM_LOG(fmt, args...)    printk(fmt, ##args)	
	#endif
#else
#define LCM_LOG(fmt, args...)	 printk(fmt, ##args)	
#endif

#ifndef TRUE
    #define   TRUE     1
#endif
 
#ifndef FALSE
    #define   FALSE    0
#endif
// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))
#define SET_GPIO_OUT(n, v)  (lcm_util.set_gpio_out((n), (v)))

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
#define read_reg_v2(cmd, buffer, buffer_size)				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

 struct LCM_setting_table {
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

	//****************************************************************************//
	//****************************** Page 1 Command ***************************
	//*************************************************************************

	{0xB9,3,{0xFF,0x83,0x79}},

	{0xB1,16,{0x44,0x19,0x19,0x31,
	0x31,0x50,0xD0,0xEE,0x54,
	0x80,0x38,0x38,0xF8,0x22,
	0x22,0x22}},

	{0xB2,9,{0x80,0xFE,0x0F,0x0D,
	0x00,0x50,0x11,0x42,0x1D}},

	{0xB4,10,{0x08,0x6E,0x08,0x6E,
	0x08,0x6E,0x12,0x86,0x13,
	0x86}},

	{0xD2,1,{0x44}},

	{0xD3,29,{0x00,0x07,0x00,0x00,
	0x00,0x0C,0x0C,0x32,0x10,
	0x08,0x00,0x08,0x03,0x62,
	0x03,0x62,0x00,0x08,0x00,
	0x08,0x37,0x33,0x0A,0x0A,
	0x37,0x0B,0x0B,0x37,0x0D}},

	{0xD5,34,{0x18,0x18,0x18,0x18,
	0x18,0x18,0x23,0x22,0x21,
	0x20,0x01,0x00,0x03,0x02,
	0x05,0x04,0x07,0x06,0x25,
	0x24,0x27,0x26,0x18,0x18,
	0x18,0x18,0x18,0x18,0x18,
	0x18,0x18,0x18,0x00,0x00}},

	{0xD6,32,{0x18,0x18,0x18,0x18,
	0x18,0x18,0x26,0x27,0x24,
	0x25,0x00,0x01,0x06,0x07,
	0x04,0x05,0x02,0x03,0x22,
	0x23,0x20,0x21,0x18,0x18,
	0x18,0x18,0x18,0x18,0x18,
	0x18,0x18,0x18}},

	{0xE0,42,{0x00,0x01,0x11,0x19,
	0x1B,0x3F,0x31,0x3E,0x0A,
	0x10,0x12,0x1A,0x11,0x13,
	0x17,0x16,0x16,0x08,0x12,
	0x12,0x17,0x00,0x01,0x11,
	0x19,0x1B,0x3F,0x31,0x3D,
	0x0B,0x11,0x11,0x1A,0x11,
	0x15,0x17,0x15,0x15,0x07,
	0x12,0x13,0x17}},

	{0xB6,2,{0x62,0x62}},

	{0xCC,1,{0x0A}},

	{0x11,1,{0x00}},                 // Sleep-Out
	{REGFLAG_DELAY, 120, {}},
	{0x29,1,{0x00}},                 // Display On

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}

};




static struct LCM_setting_table lcm_set_window[] = {
	{0x2A,	4,	{0x00, 0x00, (FRAME_WIDTH>>8), (FRAME_WIDTH&0xFF)}},
	{0x2B,	4,	{0x00, 0x00, (FRAME_HEIGHT>>8), (FRAME_HEIGHT&0xFF)}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
    {REGFLAG_DELAY, 200, {}},

    // Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
	{REGFLAG_DELAY, 50, {}},

    // Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 200, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
	{0x53, 1, {0x24}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void push_table(struct LCM_setting_table *table, unsigned int count, unsigned char force_update)
{
	unsigned int i;

    for(i = 0; i < count; i++) {
		
        unsigned int cmd;
        cmd = table[i].cmd;
		
        switch (cmd) {
			
            case REGFLAG_DELAY :
                MDELAY(table[i].count);
                break;
				
            case REGFLAG_END_OF_TABLE :
                break;
				
            default:
				dsi_set_cmdq_V2(cmd, table[i].count, table[i].para_list, force_update);
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

		// enable tearing-free
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;//LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		//params->dsi.mode   = SYNC_EVENT_VDO_MODE;
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	

		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
	
		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
		params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format		= LCM_DSI_FORMAT_RGB888;
		
		params->dsi.intermediat_buffer_num = 2;//because DSI/DPI HW design change, this parameters should be 0 when video mode in MT658X; or memory leakage
		
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
		
		params->dsi.word_count=480*3;	//DSI CMD mode need set these two bellow params, different to 6577

		params->dsi.vertical_sync_active				= 4;
		params->dsi.vertical_backporch				= 10;
		params->dsi.vertical_frontporch				= 18;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 4;
		params->dsi.horizontal_backporch				= 32;
		params->dsi.horizontal_frontporch				= 32;
		params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
		
		// Bit rate calculation
#if 0//def CONFIG_MT6589_FPGA
		params->dsi.pll_div1=2; 	// div1=0,1,2,3;div1_real=1,2,4,4
		params->dsi.pll_div2=2; 	// div2=0,1,2,3;div2_real=1,2,4,4
		params->dsi.fbk_sel=0;		 // fbk_sel=0,1,2,3;fbk_sel_real=1,2,4,4
		params->dsi.fbk_div =8; 	// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
#else
		params->dsi.pll_div1=0; 	// div1=0,1,2,3;div1_real=1,2,4,4
		params->dsi.pll_div2=1; 	// div2=0,1,2,3;div2_real=1,2,4,4
		params->dsi.fbk_sel=1;		 // fbk_sel=0,1,2,3;fbk_sel_real=1,2,4,4
		params->dsi.fbk_div =15;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		
#endif

}

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(10);
    SET_RESET_PIN(1);
    MDELAY(120);
    push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
}

static void lcm_suspend(void)
{
#ifndef BUILD_LK
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	LCM_LOG("tek.xing lcm_suspend\n");	
 
 
        SET_RESET_PIN(0);
    	MDELAY(10);
#endif
}


static void lcm_resume(void)
{
	lcm_init();
	LCM_LOG("tek.xing lcm_resume\n");
    //    lcm_initialization_setting[14].para_list[0]+=1;	

}
         
#if (LCM_DSI_CMD_MODE)
static void lcm_update(unsigned int x, unsigned int y,
                       unsigned int width, unsigned int height)
{
	unsigned int x0 = x;
	unsigned int y0 = y;
	unsigned int x1 = x0 + width - 1;
	unsigned int y1 = y0 + height - 1;

	unsigned char x0_MSB = ((x0>>8)&0xFF);
	unsigned char x0_LSB = (x0&0xFF);
	unsigned char x1_MSB = ((x1>>8)&0xFF);
	unsigned char x1_LSB = (x1&0xFF);
	unsigned char y0_MSB = ((y0>>8)&0xFF);
	unsigned char y0_LSB = (y0&0xFF);
	unsigned char y1_MSB = ((y1>>8)&0xFF);
	unsigned char y1_LSB = (y1&0xFF);

	unsigned int data_array[16];

	data_array[0]= 0x00053902;
	data_array[1]= (x1_MSB<<24)|(x0_LSB<<16)|(x0_MSB<<8)|0x2a;
	data_array[2]= (x1_LSB);
	dsi_set_cmdq(data_array, 3, 1);
	
	data_array[0]= 0x00053902;
	data_array[1]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[2]= (y1_LSB);
	dsi_set_cmdq(data_array, 3, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);

}
#endif

static unsigned int lcm_compare_id(void)
{
	unsigned int id,id1=0;
	unsigned char buffer[2];
	unsigned int array[16];  

	SET_RESET_PIN(0);
	MDELAY(20); 
	SET_RESET_PIN(1);
	MDELAY(20); 


	array[0]=0x00043902;
	array[1]=0x8983FFB9;// page enable
	dsi_set_cmdq(array, 2, 1);
	//MDELAY(10);
//{0x39,0xBA,7,{0x41,0x93,0x00,0x16,0xA4,0x10,0x18}},	
	array[0]=0x00083902;
	array[1]=0x009341BA;// page enable
	array[2]=0x1810a416;
	dsi_set_cmdq(array, 3, 1);

	array[0] = 0x00043700;// return byte number
	dsi_set_cmdq(array, 1, 1);
	MDELAY(10);

	read_reg_v2(0xF4, buffer, 3);
	id  =  buffer[1]; 
	id1 =  buffer[0];
	
#ifdef BUILD_LK
	printf("%s, id = 0x%08x id1=%x \n", __func__, id,id1);
#else
	printk("%s, id = 0x%08x  id1=%x \n",__func__, id,id1);
#endif

	return (LCM_ID_HX8379C == id)?1:0;

}



LCM_DRIVER hx8379c_fwvga_dsi_vdo_ivo_lcm_drv = 
{
    .name			= "hx8379c_fwvga_dsi_vdo_ivo",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,	
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
