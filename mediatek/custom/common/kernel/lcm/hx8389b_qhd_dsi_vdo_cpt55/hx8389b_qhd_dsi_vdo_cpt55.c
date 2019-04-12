#ifdef BUILD_LK
	#include <platform/mt_gpio.h>
#else
	#include <linux/string.h>
	#if defined(BUILD_UBOOT)
		#include <asm/arch/mt_gpio.h>
	#else
		#include <mach/mt_gpio.h>
	#endif
#endif
#include "lcm_drv.h"
// ---------------------------------------------------------------------------
//  Local Constants
// ---------------------------------------------------------------------------

#define FRAME_WIDTH  			(540)
#define FRAME_HEIGHT 			(960)

#define LCM_ID_HX8389B 0x8389

#define REGFLAG_DELAY          	0XFE
#define REGFLAG_END_OF_TABLE  	0xFFF   // END OF REGISTERS MARKER

#define LCM_DSI_CMD_MODE			0


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

#define SET_RESET_PIN(v)    	(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 		(lcm_util.udelay(n))
#define MDELAY(n) 		(lcm_util.mdelay(n))



// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)						lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)		lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg					lcm_util.dsi_read_reg()
#define read_reg_v2(cmd, buffer, buffer_size)		lcm_util.dsi_dcs_read_lcm_reg_v2(cmd, buffer, buffer_size)

static unsigned int lcm_compare_id(void);


#if 1// TYD_USE_CUSTOM_HX8389
static void init_lcm_register()
{
	unsigned int data_array[16];
	
	//push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
	data_array[0]=0x00043902;//Enable external Command
	data_array[1]=0x8983FFB9; 
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(1);//3000

	data_array[0]=0x00083902;//Enable external Command//3
	data_array[1]=0x009341ba;  
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(1);//3000

	data_array[0]=0x00143902;
	data_array[1]=0x070000B1;
	data_array[2]=0x10105FF6;
	data_array[3]=0x1C14F6F6; 
	data_array[4]=0x01421A1A; 
	data_array[5]=0x8020f73a;  
	dsi_set_cmdq(&data_array, 6, 1);
	MDELAY(1);//3000


	data_array[0]=0x00083902;
	data_array[1]=0x780000B2;
	data_array[2]=0x803F0704;
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(1);//3000

	data_array[0]=0x00183902;
	data_array[1]=0x000880b4;   
	data_array[2]=0x32001032; 
	data_array[3]=0x0000C613; 
	data_array[4]=0x40003500; 
	data_array[5]=0x440a3704; 
	data_array[6]=0x0aFFFF14; 
	dsi_set_cmdq(&data_array, 7, 1);
	MDELAY(1);//3000

	data_array[0]=0x00393902;
	data_array[1]=0x000000d5; 
	data_array[2]=0x00000100; 
	data_array[3]=0x88600000; 
	data_array[4]=0x01889988; 
	data_array[5]=0x01888845; 
	data_array[6]=0x88672345; 
	data_array[7]=0x88888888; 
	data_array[8]=0x88888888; 
	data_array[9]=0x10549988; 
	data_array[10]=0x32768888; 
	data_array[11]=0x88881054; 
	data_array[12]=0x88888888; 
	data_array[13]=0x00000088; 
	data_array[14]=0x00000000; 
	data_array[15]=0x00000000; 
	dsi_set_cmdq(&data_array, 16, 1);
	MDELAY(1);//3000

	data_array[0]=0x00053902;
	data_array[1]=0x008700b6; //8c
	data_array[2]=0x00000087; //8c
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(1);//3000

	data_array[0]=0x00233902;
	data_array[1]=0x000000e0; 
	data_array[2]=0x0C3f322E; 
	data_array[3]=0x0E0B0636; 
	data_array[4]=0x13121411; 
	data_array[5]=0x00001710; 
	data_array[6]=0x3f332d01; 
	data_array[7]=0xac04340b; 
	data_array[8]=0x1314110e; 
	data_array[9]=0x00181014; 
	dsi_set_cmdq(&data_array, 10, 1);
	MDELAY(1);//3000

	data_array[0]=0x00023902;//e
	data_array[1]=0x000002cc;//0e
	dsi_set_cmdq(&data_array, 2, 1);
	MDELAY(1);




	data_array[0] = 0x00110500; //0x11,exit sleep mode,1byte
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(120);//5000

   
    data_array[0]=0x00053902;
	data_array[1]=0x008700b6; //8c
	data_array[2]=0x00000087; //8c
	dsi_set_cmdq(&data_array, 3, 1);
	MDELAY(1);//3000


	data_array[0] = 0x00290500; //0x11,exit sleep mode,1byte
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(120);//5000
}
#endif


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

        #if (LCM_DSI_CMD_MODE)
		params->dsi.mode   = CMD_MODE;
        #else
		params->dsi.mode   = SYNC_PULSE_VDO_MODE; //SYNC_PULSE_VDO_MODE;//BURST_VDO_MODE; 
        #endif
	
		// DSI
		/* Command mode setting */
		//1 Three lane or Four lane
		params->dsi.LANE_NUM				= LCM_TWO_LANE;
		//The following defined the fomat for data coming from LCD engine.
    params->dsi.data_format.color_order = LCM_COLOR_ORDER_RGB;
    params->dsi.data_format.trans_seq	= LCM_DSI_TRANS_SEQ_MSB_FIRST;
    params->dsi.data_format.padding 	= LCM_DSI_PADDING_ON_LSB;
		params->dsi.data_format.format      = LCM_DSI_FORMAT_RGB888;

		// Video mode setting		
    params->dsi.intermediat_buffer_num = 2;
		params->dsi.PS=LCM_PACKED_PS_24BIT_RGB888;
    params->dsi.word_count=FRAME_WIDTH*3;	//DSI CMD mode need set these two bellow params, different to 6577
    params->dsi.vertical_active_line=FRAME_HEIGHT;
		
		params->dsi.vertical_sync_active				= 2;// 3    2
		params->dsi.vertical_backporch					= 6;// 20   1
		params->dsi.vertical_frontporch					= 9; // 1  12
		params->dsi.vertical_active_line				= FRAME_HEIGHT; 

		params->dsi.horizontal_sync_active				= 50;// 50  2
		params->dsi.horizontal_backporch				= 46;
		params->dsi.horizontal_frontporch				= 31;
		params->dsi.horizontal_active_pixel				= FRAME_WIDTH;
	    params->dsi.pll_div1 = 0;		// div1=0,1,2,3;div1_real=1,2,4,4 ----0: 546Mbps  1:273Mbps  0
	    params->dsi.pll_div2 = 1;		// div2=0,1,2,3;div1_real=1,2,4,4	                1
	    //params->dsi.fbk_sel = 1;
	    params->dsi.fbk_div = 18;//15      // fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)      17
}

static void lcm_init(void)
{

    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);//Must over 6 ms,SPEC request

    init_lcm_register();

}



static void lcm_suspend(void)
{
	unsigned int data_array[16];

    SET_RESET_PIN(1);
    MDELAY(10);
    SET_RESET_PIN(0);
    MDELAY(50);
    SET_RESET_PIN(1);
    MDELAY(120);//Must over 6 ms,SPEC request

	//push_table(lcm_sleep_in_setting, sizeof(lcm_sleep_in_setting) / sizeof(struct LCM_setting_table), 1);
	data_array[0] = 0x00280500; //0x11,exit sleep mode,1byte
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(20);//5000


	data_array[0] = 0x00100500; //0x10,entry sleep mode,1byte
	dsi_set_cmdq(&data_array, 1, 1);
	MDELAY(150);//5

   
}


static void lcm_resume(void)
{
	lcm_init();

    #ifdef BUILD_LK
	  printf("[LK]---cmd---hx8389b----%s------\n",__func__);
    #else
	  printk("[KERNEL]---cmd---hx8389b----%s------\n",__func__);
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
	data_array[3]= 0x00053902;
	data_array[4]= (y1_MSB<<24)|(y0_LSB<<16)|(y0_MSB<<8)|0x2b;
	data_array[5]= (y1_LSB);
	data_array[6]= 0x002c3909;

	dsi_set_cmdq(&data_array, 7, 0);

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
	//id  =  buffer[1]; 
	//id1 =  buffer[0];
	id = ((buffer[0] << 8) | buffer[1]);

	#if defined(BUILD_LK)
		printf("HX8389B uboot %s \n", __func__);
		printf("%s id = 0x%08x \n", __func__, id);
	#else
		printk("HX8389B kernel %s \n", __func__);
		printk("%s id = 0x%08x \n", __func__, id);
	#endif

	return (LCM_ID_HX8389B == id)?1:0;
}

LCM_DRIVER hx8389b_qhd_dsi_vdo_cpt55_lcm_drv = 
{
    .name			= "hx8389b_qhd_dsi_vdo_cpt55",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,
	.compare_id     = lcm_compare_id,
#if (LCM_DSI_CMD_MODE)
    .update         = lcm_update,
#endif
    };
