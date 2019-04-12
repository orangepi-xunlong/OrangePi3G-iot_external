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

#define FRAME_WIDTH  										(540)
#define FRAME_HEIGHT 										(960)
#define LCM_ID											(0x9816)

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

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))
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

static unsigned int lcm_compare_id(void);
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

{0xFF,5,{0xFF,0x98,0x07,0x00,0x01}},
{0x31,1,{0x02}},//00 02
{0x40,1,{0x56}},
{0x41,1,{0x77}},  
{0x42,1,{0x22}}, 
{0x43,1,{0x09}},
{0x44,1,{0x09}}, 
{0x46,1,{0xdC}},
//{0x47,1,{0xCC}},

{0x50,1,{0x78}},
{0x51,1,{0x78}},
{0x52,1,{0x00}},  
{0x53,1,{0x60}},//45
 
{0x60,1,{0x03}}, 
{0x61,1,{0x00}}, 
{0x62,1,{0x08}}, 
{0x63,1,{0x00}}, 

{0xA0,1,{0x00}}, 
{0xA1,1,{0x0e}}, 
{0xA2,1,{0x16}},
{0xA3,1,{0x0d}}, 
{0xA4,1,{0x06}}, 
{0xA5,1,{0x0b}},
{0xA6,1,{0x07}}, 
{0xA7,1,{0x05}},
{0xA8,1,{0x07}}, 
{0xA9,1,{0x0b}},
{0xAA,1,{0x11}}, 
{0xAB,1,{0x08}}, 
{0xAC,1,{0x0e}},
{0xAD,1,{0x12}}, 
{0xAE,1,{0x07}}, 
{0xAF,1,{0x00}}, 

{0xC0,1,{0x00}}, 
{0xC1,1,{0x0e}},
{0xC2,1,{0x16}},
{0xC3,1,{0x0d}},
{0xC4,1,{0x06}}, 
{0xC5,1,{0x0b}}, 
{0xC6,1,{0x07}}, 
{0xC7,1,{0x06}},
{0xC8,1,{0x07}}, 
{0xC9,1,{0x0B}},
{0xCA,1,{0x0f}},
{0xCB,1,{0x08}},
{0xCC,1,{0x0f}},
{0xCD,1,{0x11}},
{0xCE,1,{0x07}}, 
{0xCF,1,{0x00}}, 


{0xFF,5,{0xFF,0x98,0x07,0x00,0x06}},
{0x00,1,{0x21}}, 
{0x01,1,{0x0A}}, 
{0x02,1,{0x00}}, 
{0x03,1,{0x00}}, 
{0x04,1,{0x01}}, 
{0x05,1,{0x01}}, 
{0x06,1,{0x80}}, 
{0x07,1,{0x06}}, 
{0x08,1,{0x01}}, 
{0x09,1,{0x00}}, 
{0x0A,1,{0x00}}, 
{0x0B,1,{0x00}}, 
{0x0C,1,{0x01}}, 
{0x0D,1,{0x01}},  
{0x0E,1,{0x00}}, 
{0x0F,1,{0x00}}, 

{0x10,1,{0xF0}}, 
{0x11,1,{0xF4}}, 
{0x12,1,{0x01}}, 
{0x13,1,{0x00}}, 
{0x14,1,{0x00}}, 
{0x15,1,{0xC0}}, 
{0x16,1,{0x08}}, 
{0x17,1,{0x00}}, 
{0x18,1,{0x00}}, 
{0x19,1,{0x00}}, 
{0x1A,1,{0x00}}, 
{0x1B,1,{0x00}}, 
{0x1C,1,{0x00}}, 
{0x1D,1,{0x00}}, 

{0x20,1,{0x01}}, 
{0x21,1,{0x23}}, 
{0x22,1,{0x45}}, 
{0x23,1,{0x67}}, 
{0x24,1,{0x01}}, 
{0x25,1,{0x23}}, 
{0x26,1,{0x45}}, 
{0x27,1,{0x67}}, 

{0x30,1,{0x01}}, 
{0x31,1,{0x11}}, 
{0x32,1,{0x00}}, 
{0x33,1,{0xEE}}, 
{0x34,1,{0xFF}}, 
{0x35,1,{0x22}}, 
{0x36,1,{0xCB}}, 
{0x37,1,{0x22}}, 
{0x38,1,{0xDA}}, 
{0x39,1,{0x22}}, 
{0x3A,1,{0xAD}}, 
{0x3B,1,{0x22}}, 
{0x3C,1,{0xBC}}, 

{0x3D,1,{0x76}}, 
{0x3E,1,{0x67}}, 
{0x3F,1,{0x22}}, 
{0x40,1,{0x22}}, 
{0x41,1,{0x22}}, 
{0x42,1,{0x22}}, 
{0x43,1,{0x22}}, 

{0xFF,5,{0xFF,0x98,0x07,0x00,0x07}},
{0x02,1,{0x77}}, 
{0x17,1,{0x22}}, 
{0x18,1,{0x1d}}, 
{0x00,1,{0x11}}, 
       	
{0xFF,5,{0xFF,0x98,0x07,0x00,0x00}},
 
{0x11,1,{0x00}},                 // Sleep-Out
{REGFLAG_DELAY, 120, {}},

{0x29,1,{0x00}},                 // Display On
{REGFLAG_DELAY, 50, {}},
   

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

		params->dsi.vertical_sync_active				= 6;
		params->dsi.vertical_backporch				= 14;
		params->dsi.vertical_frontporch				= 20;
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 80;
		params->dsi.horizontal_frontporch				= 80;
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
		params->dsi.fbk_sel=1;		 // fbk_sel=0,1,2,3;fbk_sel_real=1,2,4,4
		params->dsi.fbk_div =19;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)		
#endif

}

static void lcm_init(void)
{
    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
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

    //MDELAY(120);
#endif
       //FY6203_led_off();
      // FL_disable();

#if 0
	lcm_compare_id();
	SET_RESET_PIN(0);
	MDELAY(50);
#endif
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
	unsigned char buffer[4]={0,};
	unsigned int data_array[16];

#if 1	 
	SET_RESET_PIN(1);	//NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(100);

	data_array[0]=0x00063902;
	data_array[1]=0x0798FFFF;
	data_array[2]=0x00000100;
	dsi_set_cmdq(data_array, 3, 1);
	MDELAY(10);

	// read id return five byte : 0x01 0x8B 0x96 0x08 0xFF
	data_array[0] = 0x00023700;
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10);

	read_reg_v2(0x00, buffer, 1);
	id1= buffer[0]; //should be 
	read_reg_v2(0x01, buffer, 1);
	id2= buffer[0]; //should be 0x98
	
	id=(id1 << 8) | id2;

	//SET_RESET_PIN(0);
	//MDELAY(10);

#endif

#if defined(BUILD_LK)
 //   printf("ili9806 buffer[0] = %x,buffer[3] = %x,buffer[1] = %x,buffer[3] = %x\n",  buffer[1], buffer[2]);
#else
    printk("ili9806 id = %x,buffer[0] = %x,buffer[1] = %x,buffer[2] = %x,buffer[3] = %x \n", id,
           buffer[0],buffer[1],buffer[2], buffer[3]);
    
#endif
	return   (0x9807 == id) ? 1 : 0;
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

LCM_DRIVER ili9807_dsi_vdo_qhd_drv= 
{
    .name			= "ili9807_dsi_vdo_qhd",
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
