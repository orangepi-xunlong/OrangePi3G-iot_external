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

#define FRAME_WIDTH  (480)
#define FRAME_HEIGHT (800)

#define REGFLAG_DELAY             							0XFFE
#define REGFLAG_END_OF_TABLE      							0xFFF   // END OF REGISTERS MARKER
#define LCM_ID_NT35510 (5510)

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
/*************/
//#define LSDA_GPIO_PIN (GPIO_DISP_LSDA_PIN)

#define SET_GPIO_OUT(n, v)  (lcm_util.set_gpio_out((n), (v)))

//#define SET_LSDA_LOW   SET_GPIO_OUT(LSDA_GPIO_PIN, 0)
//#define SET_LSDA_HIGH  SET_GPIO_OUT(LSDA_GPIO_PIN, 1)

/***************/


#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))


// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)									lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)				lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg											lcm_util.dsi_read_reg()
//#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)    

 struct LCM_setting_table {
    unsigned cmd;
    unsigned char count;
    unsigned char para_list[64];
};
//#define LCM_DSI_CMD_MODE

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
		{0xF0,5,{0x55,0xAA,0x52,0x08,0x01}},
		
		{0xB0,3,{0x09,0x09,0x09}},
		
		{0xB6,3,{0x34,0x34,0x34}},
		 
		{0xB1,3,{0x09,0x09,0x09}},
		 
		{0xB7,3,{0x24,0x24,0x24}},
		
		{0xB3,3,{0x05,0x05,0x05}}, 
		
		{0xB9,3,{0x24,0x24,0x24}},
		 
		{0xbf,1,{0x01}},
		 
		{0xB5,3,{0x0b,0x0b,0x0b}},
		 
		{0xBA,3,{0x34,0x24,0x24}},
		
		{0xBC,3,{0x00,0xA3,0X00}},
		
		{0xBD,3,{0x00,0xA3,0x00}},
		
		{0xBE,2,{0x00,0x40}},
		 
		{0xD1,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},  
		{0xD2,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},
		{0xD3,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},
		{0xD4,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},
		{0xD5,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},
		{0xD6,52,{0x00,0x37,0x00,0x64,0x00,0x84,0x00,0xA3,0x00,0xB6,0x00,0xDC,0x00,0xF4,0x01,0x2a,0x01,0x4A,0x01,0x8a,0x01,0xbb,0x02,0x0C,0x02,0x48,0x02,0x4A,0x02,0x82,0x02,0xbC,0x02,0xE1,0x03,0x0F,0x03,0x32,0x03,0x5B,0x03,0x73,0x03,0x91,0x03,0xA0,0x03,0xAF,0x03,0xBA,0x03,0xC1}},
		
		{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}},
		
		{0xB6,1,{0x0a}},
		
		{0xB7,2,{0x00,0x00}},
		
		{0xB8,4,{0x01,0x05,0x05,0x05}},
		
		{0xBa,1,{0x01}},
		
		{0xBC,3,{0x00,0x00,0x00}},
		
		{0xBD,5,{0x01,0x84,0x07,0x31,0x00}},
		
		{0xBE,5,{0x01,0x84,0x07,0x31,0x00}},
		
		{0xBF,5,{0x01,0x84,0x07,0x31,0x00}},
		
		{0xCC,3,{0x03,0x00,0x00}},
		
		{0xB1,2,{0xF8,0x00}},
		
		{0x44,2,{0x00,0xc8}},
		{0x35,1,{0x00}},		
		{0x3A,1,{0x77}},
		{0x36,1,{0x00}},

		{0x11,1,{0x00}},
		{REGFLAG_DELAY,150,{}},
		{0x29,	1,	{0x00}},
		{REGFLAG_DELAY,50,{}},
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.

	// Setting ending by predefined flag
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
		//params->dbi.te_mode				= LCM_DBI_TE_MODE_VSYNC_ONLY;
		params->dbi.te_mode 				= LCM_DBI_TE_MODE_DISABLED;
		params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;

#if defined(LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
#else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE;
#endif
	
		// DSI
		/* Command mode setting */
		params->dsi.LANE_NUM				= LCM_TWO_LANE;

		//The following defined the fomat for data coming from LCD engine.
		params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
		
		params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
		
		params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
		
		params->dsi.data_format.format	  = LCM_DSI_FORMAT_RGB888;
		
		// Highly depends on LCD driver capability.
		// Not support in MT6573 => no use in MT6573
		params->dsi.packet_size=256;

		// Video mode setting		
		params->dsi.intermediat_buffer_num = 2;

		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

		params->dsi.vertical_sync_active				= 3;
		params->dsi.vertical_backporch					= 12;
		params->dsi.vertical_frontporch					= 2;
		params->dsi.vertical_active_line				= FRAME_HEIGHT;

		params->dsi.horizontal_sync_active				= 10;
		params->dsi.horizontal_backporch				= 50;
		params->dsi.horizontal_frontporch				= 50;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
		params->dsi.ssc_disable = 0; 
		params->dsi.ssc_range = 4; 
		params->dsi.PLL_CLOCK = 192;

		
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
	unsigned int data_array[16];

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20); 
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(150);
}


static void lcm_resume(void)
{
	unsigned int data_array[16];

//	data_array[0] = 0x00110500; // Sleep Out
//	dsi_set_cmdq(&data_array, 1, 1);
//	MDELAY(150);
//	data_array[0] = 0x00290500; // Display On
//	dsi_set_cmdq(&data_array, 1, 1);
//	MDELAY(20);
	 lcm_init();     
//	lcm_compare_id();
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
	//data_array[6]= 0x002c3901;

	dsi_set_cmdq(&data_array, 7, 0);

}


static unsigned int lcm_compare_id(void)
{
	unsigned int id=0;
	unsigned char buffer[2];
	unsigned int array[16];  
 	return 1;
	//Do reset here
	SET_RESET_PIN(1);
	SET_RESET_PIN(0);
	MDELAY(25);
	
	SET_RESET_PIN(1);
	MDELAY(50);      
  
	array[0]=0x00063902;
	array[1]=0x52aa55f0;
	array[2]=0x00000108;
	dsi_set_cmdq(array, 3, 1);
	MDELAY(10);

	array[0] = 0x00083700;
	dsi_set_cmdq(array, 1, 1);

	//read_reg_v2(0x04, buffer, 3);//if read 0x04,should get 0x008000,that is both OK.
	read_reg_v2(0xc5, buffer,2);

	id = buffer[0]<<8 |buffer[1]; 
	      
#if defined(BUILD_LK)
	printf("%s,  buffer[0]=%x,buffer[1]=%x,id = 0x%x \n", __func__,buffer[0],buffer[1], id);
#else
    printk("%s,  buffer[0]=%x,buffer[1]=%x,id = 0x%x \n", __func__,buffer[0],buffer[1], id);
#endif

    if(id == LCM_ID_NT35510)
    	return 1;
    else
    	return 1;

}


LCM_DRIVER nt35510_wvga_dsi_cmd_drv = 
{
    .name			= "nt35510_dsi",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id    = lcm_compare_id,
#if defined(LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
