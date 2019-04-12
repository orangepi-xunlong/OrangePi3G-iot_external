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

#define FRAME_WIDTH  										(540)
#define FRAME_HEIGHT 										(960)
#define LCM_ID											(0x9261)

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

	{0xbf,3,{0x92,0x61,0xf3}},
	{0xb3,2,{0x00,0x45}},//43
	{0xb4,2,{0x00,0x45}},
	{0xb8,6,{0x00,0xCf,0x01,0x00,0xCf,0x01}},
	{0xc3,1,{0x04}},
	{0xc4,2,{0x00,0x78}},
	{0xc7,9,{0x00,0x01,0x32,0x08,0x68,0x2a,0x15,0xa5,0xa5}},
	//GAMMA2.5
	{0xC8,38,{0x7F,0x5F,0x4A,0x3A,0x32,0x21,0x24,0x0E,0x28,0x28,0x27,0x42,0x2C,0x37,0x2C,0x29,0x1C,0x0B,0x02,
	          0x7F,0x5F,0x4A,0x3A,0x32,0x21,0x24,0x0E,0x28,0x28,0x27,0x42,0x2C,0x37,0x2C,0x29,0x1C,0x0B,0x02}},
	{0xD4,19,{0x1F,0x1F,0x01,0x03,0x1F,0x1F,0x07,0x0B,0x1F,0x1F,0x09,0x05,0x13,0x11,0x1F,0x1F,0x1F,0x1F,0x1F}},
	{0xD5,19,{0x00,0x02,0x1F,0x1F,0x06,0x0A,0x1F,0x1F,0x08,0x04,0x1F,0x1F,0x1F,0x1F,0x12,0x10,0x1F,0x1F,0x1F}},
	{0xD6,19,{0x1F,0x1F,0x12,0x10,0x1F,0x1F,0x04,0x08,0x1F,0x1F,0x0A,0x06,0x00,0x02,0x1F,0x1F,0x1F,0x1F,0x1F}},
	{0xd7,19,{0x13,0x11,0x1f,0x1f,0x05,0x09,0x1f,0x1f,0x0b,0x07,0x1f,0x1f,0x1f,0x1f,0x01,0x03,0x1f,0x1f,0x1f}},
	{0xd8,20,{0x00,0x00,0x00,0x30,0x03,0x30,0x01,0x02,0x30,0x01,0x02,0x06,0x70,0x73,0xc7,0x72,0x06,0x30,0x70,0x08}},
	{0xd9,21,{0x00,0x0a,0x0a,0x88,0x00,0x00,0x06,0x7b,0x00,0x00,0x00,0x33,0x33,0x1f,0x00,0x00,0x00,0x06,0x70,0x01,0xe0}},
	{0xBE,1,{0x01}},
	{0xCC,10,{0x34,0x20,0x38,0x60,0x11,0x91,0x00,0x40,0x00,0x00}},
	{0xBE,1,{0x00}},
	{0x35,1,{0x00}},
	{0x55,1,{0xB0}},
	{0x11,1,{0x00}},
	{REGFLAG_DELAY,150,{}},
	{0x29,1,{0x00}},
	{REGFLAG_DELAY,50,{}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}


/*
	{0xbf,3,{0x92,0x61,0xf3}},
	{0xb3,2,{0x00,0x43}},
	{0xb4,2,{0x00,0x43}},
	{0xb8,6,{0x00,0x9f,0x01,0x00,0x9f,0x01}},
	//{0xba,3,{0x3e,0x23,0x00}},
	{0xc3,1,{0x04}},
	{0xc4,2,{0x00,0x78}},

	{0xc7,9,{0x00,0x01,0x32,0x08,0x68,0x2a,0x15,0xa5,0xa5}},

	//gama2.2
	{0xc8,38,{0x7f,0x62,0x4f,0x3f,0x38,0x27,0x2b,0x15,0x2f,0x30,0x30,0x4d,0x3a,0x3f,0x2e,0x29,0x1c,0x0b,0x02,0x7f,0x62,0x4f,0x3f,0x38,0x27,0x2b,0x15,0x2f,0x30,0x30,0x4d,0x3a,0x3f,0x2e,0x29,0x1c,0x0b,0x02}},

	//{0xc8,38,{0x7f,0x61,0x4d,0x3e,0x36,0x26,0x28,0x12,0x2c,0x2b,0x2b,0x45,0x2e,0x31,0x20,0x1b,0x10,0x04,0x00,0x7f,0x61,0x4d,0x3e,0x36,0x26,0x28,0x12,0x2c,0x2b,0x2b,0x45,0x2e,0x31,0x20,0x1b,0x10,0x04,0x00}},
	{0xd4,19,{0x1f,0x1f,0x01,0x03,0x1f,0x1f,0x07,0x0b,0x1f,0x1f,0x09,0x05,0x13,0x11,0x1f,0x1f,0x1f,0x1f,0x1f}},

	{0xd5,19,{0x00,0x02,0x1f,0x1f,0x06,0x0a,0x1f,0x1f,0x08,0x04,0x1f,0x1f,0x1f,0x1f,0x12,0x10,0x1f,0x1f,0x1f}},

	{0xd6,19,{0x1f,0x1f,0x12,0x10,0x1f,0x1f,0x04,0x08,0x1f,0x1f,0x0a,0x06,0x00,0x02,0x1f,0x1f,0x1f,0x1f,0x1f}},

	{0xd7,19,{0x13,0x11,0x1f,0x1f,0x05,0x09,0x1f,0x1f,0x0b,0x07,0x1f,0x1f,0x1f,0x1f,0x01,0x03,0x1f,0x1f,0x1f}},

	{0xd8,20,{0x00,0x00,0x00,0x30,0x03,0x30,0x01,0x02,0x30,0x01,0x02,0x06,0x70,0x73,0xc7,0x72,0x06,0x30,0x70,0x08}},

	{0xd9,21,{0x00,0x0a,0x0a,0x88,0x00,0x00,0x06,0x7b,0x00,0x00,0x00,0x33,0x33,0x1f,0x00,0x00,0x00,0x06,0x70,0x01,0xe0}},

        //{0xBE,1,{0x01}},
        //{0xCC,10,{0x34,0x20,0x38,0x60,0x11,0x91,0x00,0x40,0x00,0x00}},
        //{0xBE,1,{0x00}},
	{0x35,1,{0x00}},

	{0x11,1,{0x00}},
	{REGFLAG_DELAY,150,{}},
	{0x29,	1,	{0x00}},
	{REGFLAG_DELAY,50,{}},

	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}

*/
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

		params->dsi.vertical_sync_active				= 2;
		params->dsi.vertical_backporch				= 8;
		params->dsi.vertical_frontporch				= 5;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				=10;
		params->dsi.horizontal_frontporch				= 10;
		params->dsi.horizontal_active_pixel			= FRAME_WIDTH;
		
		// Bit rate calculation
#ifdef CONFIG_MT6589_FPGA
		params->dsi.pll_div1=2; 	// div1=0,1,2,3;div1_real=1,2,4,4
		params->dsi.pll_div2=2; 	// div2=0,1,2,3;div2_real=1,2,4,4
		params->dsi.fbk_sel=0;		 // fbk_sel=0,1,2,3;fbk_sel_real=1,2,4,4
		params->dsi.fbk_div =8; 	// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)
#else
		params->dsi.pll_div1=0; 	// div1=0,1,2,3;div1_real=1,2,4,4
		params->dsi.pll_div2=1; 	// div2=0,1,2,3;div2_real=1,2,4,4
		//params->dsi.fbk_sel=1;		 // fbk_sel=0,1,2,3;fbk_sel_real=1,2,4,4
		params->dsi.fbk_div =14;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		
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

//extern void FY6203_en_ready(void);
//extern void FY6203_en_pulse_count(int count);
//extern void FY6203_led_off(void);

//extern int FL_disable(void);
//extern int FL_enable(void);
//extern int FL_init(void);

static void lcm_suspend(void)
{
#ifndef BUILD_LK
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);
	LCM_LOG("tek.xing lcm_suspend\n");	
        SET_RESET_PIN(0);
        MDELAY(120);
 
#endif
       //FY6203_led_off();
      // FL_disable();
}


static void lcm_resume(void)
{
#if !defined(BUILD_LK)
	lcm_init();
	LCM_LOG("tek.xing lcm_resume\n");	
#endif
      ////FY6203_en_ready();
      //FY6203_en_pulse_count(15);
      //
      //FL_init();
      //FL_enable();
}


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
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

}

static unsigned int lcm_esd_recover()
{
    lcm_init();
    return TRUE;
}


static void lcm_setpwm(unsigned int divider)
{
	// TBD
}

static unsigned int lcm_compare_id(void)
{
	unsigned int id1 = 0, id2 = 0, id3 = 0, id4 = 0,id;
	unsigned char buffer[4];
	unsigned int data_array[16];
	 
	SET_RESET_PIN(1);	//NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(50);
	
	data_array[0]=0x00043902;
	data_array[1]=0xF36192BF;
	dsi_set_cmdq(data_array, 2, 1);
	MDELAY(10);

	// read id return five byte : 0x01 0x8B 0x96 0x08 0xFF
	data_array[0] = 0x00023700;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);
	
//	read_reg_v2(0x01, buffer, 1);
//	id1= buffer[0]; //should be 
	read_reg_v2(0x04, buffer, 2);
	id1= buffer[0]; //92
	id2= buffer[1]; //61
	
	id=(id1 << 8) | id2;

#if defined(BUILD_LK)
	printf("ili9806 id=%x, %x %x \n", id, id1, id2);
#else
	printk("ili9806 id=%x, %x %x \n", id, id1, id2);
#endif
	return (LCM_ID == id) ? 1 : 0;
}


static void lcm_setbacklight(unsigned int level)
{
	unsigned int default_level = 145;
	unsigned int mapped_level = 0;

	//for LGE backlight IC mapping table
	if(level > 255) 
			level = 255;

	if(level >0) 
			mapped_level = default_level+(level)*(255-default_level)/(255);
	else
			mapped_level=0;
	
#if defined(BUILD_LK)
			printf("brightness_set_cust %s  \n", __func__);
#else
			printk(" brightness_set_cust %s  \n", __func__);
#endif
	// Refresh value of backlight level.
	//lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}

LCM_DRIVER jd9261_lcm_drv= 
{
    .name			= "jd9261",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,	
#if (LCM_DSI_CMD_MODE)
	.set_backlight	= lcm_setbacklight,
    .update         = lcm_update,
#endif
};
