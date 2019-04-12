/* BEGIN PN: , Added by h84013687, 2013.08.13*/
#ifndef BUILD_LK
#include <linux/string.h>
#endif
#include "lcm_drv.h"

#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#elif defined(BUILD_UBOOT)
	#include <asm/arch/mt_gpio.h>
#else
	#include <mach/mt_gpio.h>
#endif

static unsigned int lcm_compare_id(void);
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  (540)
#define FRAME_HEIGHT (960)

#define LCM_ID_NT35517 (0x5517)

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    (lcm_util.set_reset_pin((v)))

#define UDELAY(n) (lcm_util.udelay(n))
#define MDELAY(n) (lcm_util.mdelay(n))
#define REGFLAG_DELAY             							0xAB
#define REGFLAG_END_OF_TABLE      							0x1FF //AA   // END OF REGISTERS MARKER

struct LCM_setting_table {
    unsigned char cmd;
    unsigned char count;
    unsigned char para_list[64];
};

// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	        lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
#define read_reg_v2(cmd, buffer, buffer_size)   				lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)   

#if 0 //defined(MTK_WFD_SUPPORT)
#define   LCM_DSI_CMD_MODE							1
#else
#define   LCM_DSI_CMD_MODE							0
#endif

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

	params->dbi.te_mode                             = LCM_DBI_TE_MODE_VSYNC_ONLY;
	//params->dbi.te_mode                               = LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity            = LCM_POLARITY_RISING;
	//#if (LCM_DSI_CMD_MODE)
	//	params->dsi.mode   = CMD_MODE;
	// #else
	params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
	// #endif

	// DSI
	/* Command mode setting */
	//1 Three lane or Four lane
	params->dsi.LANE_NUM				= LCM_TWO_LANE;
	//The following defined the fomat for data coming from LCD engine.
	params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

	// Video mode setting		
	params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;

	params->dsi.vertical_sync_active				= 0x05;// 3    2
	params->dsi.vertical_backporch					= 14;// 20   1
	params->dsi.vertical_frontporch					= 12; // 1  12
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 

	params->dsi.horizontal_sync_active				= 0x16;// 50  2
	params->dsi.horizontal_backporch				= 80;//30; //80;
	params->dsi.horizontal_frontporch				= 50;
	params->dsi.horizontal_active_pixel				= FRAME_WIDTH;

	//improve clk quality
	params->dsi.PLL_CLOCK = 240; //this value must be in MTK suggested table
	params->dsi.compatibility_for_nvk = 1;
	params->dsi.ssc_disable = 1;
}

static struct LCM_setting_table lcm_initialization_setting[] = {
	//CMI5.0_F050A10_NT35517_VIDEO_V1

	{0xFF,4,{0xAA,0x55,0x25,0x81}}, 
	{0xF9,42,{0x02,0xA0,0x00,0x00, 0x15, 0x15, 0x04, 0x0B, 0x08, 0x0A, 0x09, 0x10, 0x12, 0x05, 0x00, 0x15, 0x0C, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x15, 0x06, 0x15, 0x12, 0x0D, 0x00, 0x02, 0x09, 0x08, 0x0A, 0x07, 0x0E, 0x15, 0x15}}, 

	{0xF9,4,{0x00,0x1A,0x00,0x40}}, 

	// CMD2 Page0
	{0xF0,5,{0x55,0xAA,0x52,0x08,0x00}}, 

	// Source hold time, 05=1.8us 09=3.25us
	{0xb6,1,{0x09}}, 

	//Gate EQ
	{0xB7,2,{0x70,0x70}}, 

	//Source EQ
	{0xB8,4,{0x01,0x04,0x04,0x04}}, 
	//REGW 0xB8,0x01,0x04,0x00,0x00}}, 
	//Inversion
	{0XBC,1,{0X00}}, 

	//Display timing:dual 8-phase 4-overlap

	{0xC1,1,{0x01}}, 
	{0xCA,11,{0x01,0xE4,0xE4,0xE4,0x00,0x00,0x00,0x02,0x02,0x00,0x00}}, 


	//CMD2 Page1
	{0xF0, 5,{0x55, 0xAA, 0x52, 0x08, 0x01}}, 

	//AVDD: 5.5V =>6.2V
	//REGW 0xB0,0x0A
	{0xb0,1,{0x03}}, 

	//AVEE: -5.5V => -6.2V
	//REGW 0XB1,0X0A
	{0xb1,1,{0x03}}, 


	//VCL: -2.5V
	{0XB2,1,{0X00}}, 

	//VGH: 15V=>18V
	//REGW 0XB3,0X10
	{0xb3,1,{0x16}}, 

	//VGLX: -10V
	{0XB4,1,{0X06}}, 

	//AVDD: 2.5XVDDB
	{0XB6,1,{0X44}}, 

	//AVEE: -VDDB+1.5XVCL
	{0XB7,1,{0X34}}, 

	//VCL:-1XVDDB
	{0XB8,1,{0X14}}, 

	//VGH: 2XAVDD-AVEE
	{0XB9,1,{0X33}}, 

	//VGLX:AVEE-AVDD
	{0XBA,1,{0X14}}, 

	//VGMP=4.8V
	{0XBC,3,{0X00,0X90,0X00}}, 

	//VGMN=-4.8V
	{0XBD,3,{0X00,0X90,0X00}}, 

	//VCOM
	{0XBE,1,{0X4C}}, 

	//PUMP
	{0XC2,1,{0X00}}, 

	//1 Gamma
	{0XCF,1,{0X04}}, 

	// Positive Gamma
	{0xD1,16,{0x00,0x00,0x00,0x01,0x00,0x09,0x00,0x1D,0x00,0x36,0x00,0x6F,0x00,0x9B,0x00,0xEA}}, 
	{0xD2,16,{0x01,0x18,0x01,0x65,0x01,0x95,0x01,0xE0,0x02,0x19,0x02,0x1A,0x02,0x49,0x02,0x78}}, 
	{0xD3,16,{0x02,0x92,0x02,0xB1,0x02,0xC3,0x02,0xDC,0x02,0xEF,0x02,0xFF,0x03,0x0F,0x03,0x28}}, 
	{0xD4,4,{0x03,0x65,0x03,0xD6}},

	{0x35,1,{0x00}},
	
	{0x11,  1,  {0x00}}, 
	{REGFLAG_DELAY, 120, {}},	    
	// Display ON
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 20, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}},
	// Note
	// Strongly recommend not to set Sleep out / Display On here. That will cause messed frame to be shown as later the backlight is on.


	// Setting ending by predefined flag
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static void lcm_init(void)
{
	int i;
	unsigned char buffer[10];
	unsigned int  array[16];
	unsigned int data_array[16];

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(120);
	//lcm_compare_id(); //test
	push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);

}


static void lcm_suspend(void)
{
	unsigned int data_array[16];

	data_array[0]=0x00280500; // Display Off
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(10); 
	data_array[0] = 0x00100500; // Sleep In
	dsi_set_cmdq(data_array, 1, 1);
	MDELAY(120); 

	//SET_RESET_PIN(1);	
	SET_RESET_PIN(0);
	MDELAY(10); // 1ms

	SET_RESET_PIN(1);
	//MDELAY(120);
}


static void lcm_resume(void)
{
	lcm_init();

	#ifdef BUILD_LK
	printf("[LK]---cmd---nt35590----%s------\n",__func__);
	#else
	printk("[KERNEL]---cmd---nt35590----%s------\n",__func__);
	#endif	
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
	unsigned int id=0;
	unsigned char buffer[3];
	unsigned int array[16];  
	unsigned int data_array[16];

	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(50);

	SET_RESET_PIN(1);
	MDELAY(120); 

	data_array[0] = 0x00063902;
	data_array[1] = 0x52AA55F0;  
	data_array[2] = 0x00000108;                
	dsi_set_cmdq(&data_array, 3, 1); 

	array[0] = 0x00033700;// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);

	read_reg_v2(0xC5, buffer, 3);
	id = buffer[1]; //we only need ID
	#ifdef BUILD_LK
	printf("%s, LK nt35517 debug:  id = 0x%08x buffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n", __func__, id,buffer[0],buffer[1],buffer[2]);
	#else
	printk("%s, LK nt35517 debug:  id = 0x%08x buffer[0]=0x%08x,buffer[1]=0x%08x,buffer[2]=0x%08x\n", __func__, id,buffer[0],buffer[1],buffer[2]);
	#endif

	// if(id == LCM_ID_NT35517)
	if(buffer[0]==0x55 && buffer[1]==0x17)
	return 1;
	else
	return 0;
}

LCM_DRIVER nt35517_qhd_dsi_vdo_cmi_lcm_drv =
{
    .name           	= "nt35517_qhd_dsi_vdo_cmi",
    .set_util_funcs 	= lcm_set_util_funcs,
    .get_params     	= lcm_get_params,
    .init           	= lcm_init,
    .suspend        	= lcm_suspend,
    .resume         	= lcm_resume,
    .compare_id     	= lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
};
