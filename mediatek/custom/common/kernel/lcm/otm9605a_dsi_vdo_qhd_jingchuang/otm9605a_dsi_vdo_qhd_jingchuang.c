/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 */
/* MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

/*****************************************************************************
*  Copyright Statement:
*  --------------------
*  This software is protected by Copyright and the information contained
*  herein is confidential. The software may not be copied and the information
*  contained herein may not be used or disclosed except with the written
*  permission of MediaTek Inc. (C) 2008
*
*  BY OPENING THIS FILE, BUYER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
*  THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
*  RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO BUYER ON
*  AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
*  EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
*  MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
*  NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
*  SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
*  SUPPLIED WITH THE MEDIATEK SOFTWARE, AND BUYER AGREES TO LOOK ONLY TO SUCH
*  THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. MEDIATEK SHALL ALSO
*  NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE RELEASES MADE TO BUYER'S
*  SPECIFICATION OR TO CONFORM TO A PARTICULAR STANDARD OR OPEN FORUM.
*
*  BUYER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND CUMULATIVE
*  LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
*  AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
*  OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY BUYER TO
*  MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
*
*  THE TRANSACTION CONTEMPLATED HEREUNDER SHALL BE CONSTRUED IN ACCORDANCE
*  WITH THE LAWS OF THE STATE OF CALIFORNIA, USA, EXCLUDING ITS CONFLICT OF
*  LAWS PRINCIPLES.  ANY DISPUTES, CONTROVERSIES OR CLAIMS ARISING THEREOF AND
*  RELATED THERETO SHALL BE SETTLED BY ARBITRATION IN SAN FRANCISCO, CA, UNDER
*  THE RULES OF THE INTERNATIONAL CHAMBER OF COMMERCE (ICC).
*
*****************************************************************************/


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
#define LCM_ID                   (0x9605)//     (0x0680)   // 

#define REGFLAG_DELAY             	(0XFE)
#define REGFLAG_END_OF_TABLE      	(0x100)	// END OF REGISTERS MARKER


#define LCM_DSI_CMD_MODE									0

#ifndef TRUE
    #define TRUE 1
#endif

#ifndef FALSE
    #define FALSE 0
#endif

// ---------------------------------------------------------------------------
//  Local Variables
// ---------------------------------------------------------------------------

static LCM_UTIL_FUNCS lcm_util = {0};

#define SET_RESET_PIN(v)    								(lcm_util.set_reset_pin((v)))

#define UDELAY(n) 											(lcm_util.udelay(n))
#define MDELAY(n) 											(lcm_util.mdelay(n))

static unsigned int lcm_esd_test = FALSE;      ///only for ESD test

static int lcm_id = 0 ;   //  0 : default  1:晶创   2： 鸿
// ---------------------------------------------------------------------------
//  Local Functions
// ---------------------------------------------------------------------------

#define dsi_set_cmdq_V2(cmd, count, ppara, force_update)	lcm_util.dsi_set_cmdq_V2(cmd, count, ppara, force_update)
#define dsi_set_cmdq(pdata, queue_size, force_update)		lcm_util.dsi_set_cmdq(pdata, queue_size, force_update)
#define wrtie_cmd(cmd)										lcm_util.dsi_write_cmd(cmd)
#define write_regs(addr, pdata, byte_nums)					lcm_util.dsi_write_regs(addr, pdata, byte_nums)
#define read_reg(cmd)											lcm_util.dsi_dcs_read_lcm_reg(cmd)
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
 
#if 1 // JX  晶创
{0x00,1,{0x00}},
{0xFF,3,{0x96,0x05,0x01}},
{0x00,1,{0x80}},
{0xFF,2,{0x96,0x05}},

{0x00,1,{0x92}},
{0xFF,2,{0x10,0x02}},

{0x00,1,{0x00}},
{0x00,1,{0x00}},
{0x00,1,{0x00}},
{0x00,1,{0x00}},
{0x00,1,{0x00}},
{0x00,1,{0x00}},
{0x00,1,{0x00}},
{0x00,1,{0x00}},

{0x00,1,{0xB4}},
{0xC0,1,{0x50}},

{0x00,1,{0x89}},
{0xC0,1,{0x01}},
{0x00,1,{0xA0}},
{0xC1,1,{0x00}},
{0x00,1,{0x00}},
{0xA0,1,{0x00}},
{0x00,1,{0xA2}},
{0xC0,3,{0x01,0x10,0x00}},
{0x00,1,{0x91}},
{0xC5,1,{0x77}},

{0x00,1,{0x80}},
{0xD6,1,{0x58}},
{0x00,1,{0x00}},
{0xD7,1,{0x00}},
{0x00,1,{0x00}},
{0xD8,2,{0x6f,0x6f}},
{0x00,1,{0x00}},
{0xD9,1,{0x2A}},

////////////// CE improve code ///////////////////
{0x00,1,{0x80}}, 
{0xC1,2,{0x36,0x66}},
{0x00,1,{0xb1}}, 
{0xC5,1,{0x28}},
{0x00,1,{0xB2}}, 
{0xF5,4,{0x15,0x00,0x15,0x00}},
//{0x00,0xC0}}, 
//{0xC5,0x00}},
//{0x00,0x88}},
//{0xC4,0x01}},
//{0x00,0xA2}}, 
//{0xC0,0x20,0x18,0x09}},
//{0x00,0x80}},
//{0xC4,0x9C}}, 
////////////// CE improve code end ///////////////

{0x00,1,{0x00}},
{0xE1,16,{0x00,0x0F,0x15,0x0E,0x07,0x0E,0x0B,0x09,0x04,0x08,0x0F,0x0A,0x11,0x12,0x0A,0x03}},
{0x00,1,{0x00}},
{0xE2,16,{0x00,0x0F,0x15,0x0E,0x07,0x0E,0x0B,0x09,0x04,0x08,0x0F,0x0A,0x11,0x12,0x0A,0x03}},

{0x00,1,{0x80}},
{0xCB,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0x00,1,{0x90}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0x00,1,{0xA0}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0x00,1,{0xB0}},
{0xCB,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0x00,1,{0xC0}},
{0xCB,15,{0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0x00,1,{0xD0}},
{0xCB,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x04,0x04,0x04,0x04,0x04,0x00,0x00,0x00}},
{0x00,1,{0xE0}},
{0xCB,10,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0x00,1,{0xF0}},
{0xCB,10,{0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff}},
{0x00,1,{0x80}},
{0xCC,10,{0x00,0x00,0x09,0x0B,0x01,0x25,0x26,0x00,0x00,0x00}},
{0x00,1,{0x90}},
{0xCC,15,{0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x0A,0x0C,0x02}},
{0x00,1,{0xA0}},
{0xCC,15,{0x25,0x26,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00}},
{0x00,1,{0x80}},
{0xCE,12,{0x86,0x01,0x06,0x85,0x01,0x06,0x0F,0x00,0x00,0x0f,0x00,0x00}},
{0x00,1,{0x90}},
{0xCE,14,{0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0xF0,0x00,0x00,0x00,0x00}},
{0x00,1,{0xA0}},
{0xCE,14,{0x18,0x05,0x03,0xC0,0x00,0x06,0x00,0x18,0x04,0x03,0xC1,0x00,0x06,0x00}},
{0x00,1,{0xB0}},
{0xCE,14,{0x18,0x03,0x03,0xC2,0x00,0x06,0x00,0x18,0x02,0x03,0xC3,0x00,0x06,0x00}},
{0x00,1,{0xC0}},
{0xCE,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
{0x00,1,{0xD0}},
{0xCE,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
{0x00,1,{0x80}},
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
{0x00,1,{0x90}},
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
{0x00,1,{0xA0}},
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
{0x00,1,{0xB0}},
{0xCF,14,{0xF0,0x00,0x00,0x10,0x00,0x00,0x00,0xF0,0x00,0x00,0x10,0x00,0x00,0x00}},
{0x00,1,{0xC0}},
{0xCF,10,{0x02,0x02,0x10,0x10,0x00,0x00,0x01,0x81,0x00,0x08}},

////////////////////// CABC CODE /////////////////////////////////
//{0x00,0x80}},
//W_COM_1A_28P(0xca,0x80,0xB9,0xBE,0xC3,0xC8,0xCD,0xD2,0xD7,0xDC,0xE1,0xE6,0xEB,0xF0,0xF5,0xFF,0xFF,0xC3,0xFF,0xC3,0xFF,0xC3,0xFF,0x05,0x03,0x05,0x03,0x05,0x03}},
//{0x00,0x00}},
//{0xc7,0x10}},
//W_COM_1A_18P(0xc8,0x80,0x99,0x99,0x99,0xA9,0xAA,0x99,0x99,0x99,0x99,0x99,0x99,0x89,0x78,0x67,0x56,0x45,0x33}}, 
//{0x00,0x00}},
//{0xc7,0x11}},
//W_COM_1A_18P(0xc8,0x80,0x99,0x99,0x99,0xA9,0xAA,0x99,0x99,0x99,0x99,0x99,0x99,0x89,0x78,0x67,0x56,0x45,0x33}},
//{0x00,0x00}},
//{0xc7,0x12}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x99,0x99,0x9A,0x99,0x99,0x99,0x99,0x99,0x99,0x88,0x88,0x77,0x66,0x45,0x33}}, 
//{0x00,0x00}},
//{0xc7,0x13}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x99,0x99,0x89,0x99,0x99,0x99,0x99,0x99,0x99,0x88,0x88,0x78,0x67,0x45,0x33}}, 
//{0x00,0x00}},
//{0xc7,0x14}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x99,0x99,0x88,0x99,0x99,0x99,0x99,0x99,0x89,0x88,0x88,0x88,0x77,0x45,0x33}}, 
//{0x00,0x00}},
//{0xc7,0x15}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x99,0x99,0x88,0x98,0x99,0x89,0x99,0x99,0x89,0x88,0x88,0x88,0x78,0x46,0x33}}, 
//{0x00,0x00}},
//{0xc7,0x16}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x99,0x89,0x88,0x98,0x99,0x89,0x98,0x99,0x89,0x88,0x88,0x88,0x78,0x56,0x34}}, 
//{0x00,0x00}},
//{0xc7,0x17}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x99,0x89,0x88,0x88,0x98,0x99,0x88,0x99,0x89,0x88,0x88,0x88,0x88,0x57,0x34}}, 
//{0x00,0x00}},
//{0xc7,0x18}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x99,0x89,0x88,0x88,0x88,0x98,0x99,0x99,0x88,0x88,0x88,0x88,0x77,0x67,0x45}}, 
//{0x00,0x00}},
//{0xc7,0x19}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x99,0x89,0x88,0x88,0x88,0x88,0x98,0x99,0x88,0x88,0x88,0x88,0x88,0x67,0x45}}, 
//{0x00,0x00}},
//{0xc7,0x1a}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x99,0x88,0x88,0x88,0x88,0x88,0x88,0x99,0x88,0x88,0x88,0x88,0x88,0x77,0x55}}, 
//{0x00,0x00}},
//{0xc7,0x1b}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x89,0x88,0x88,0x88,0x88,0x88,0x98,0x88,0x88,0x88,0x88,0x88,0x88,0x77,0x57}}, 
//{0x00,0x00}},
//{0xc7,0x1c}},
//W_COM_1A_18P(0xC8,0x80,0x99,0x99,0x88,0x88,0x88,0x88,0x88,0x88,0x98,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x56}}, 
//{0x00,0x00}},
//{0xc7,0x1d}},
//W_COM_1A_18P(0xC8,0x80,0x98,0x99,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x88,0x67}}, 
//{0x00,0x00}},
//{0xC7,0x00}},
////////////////////// CABC CODE END //////////////////////////////
//////////////////////////////////////
{0x00,1,{0xB1}},
{0xC5,1,{0x28}},

{0x00,1,{0x80}},
{0xC4,1,{0x9C}},

{0x00,1,{0xC0}},
{0xC5,1,{0x00}},

{0x00,1,{0xB2}},
{0xF5,4,{0x15,0x00,0x15,0x00}},

{0x00,1,{0x93}},
{0xC5,1,{0x03}},

{0x00,1,{0x80}},
{0xC1,2,{0x36,0x66}},

{0x00,1,{0x89}},
{0xC0,1,{0x01}},

{0x00,1,{0xA0}},
{0xC1,1,{0x00}},

{0x00,1,{0xC5}},
{0xB0,1,{0x03}},
///////////////////////////////////////
{0x00,1,{0x00}},
{0xFF,3,{0xFF,0xFF,0xFF}},

//{0x51,0x00}},
//{0x53,0x24}},
//{0x55,0x01}},

{0x11,1,{0x00}},
{REGFLAG_DELAY,120,{}},
{0x29,1,{0x00}},

#endif
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_sleep_out_setting[] = {
    // Sleep Out
	{0x11, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},
	// Display ON
	//{0x2C, 1, {0x00}},
	//{0x13, 1, {0x00}},
	{0x29, 1, {0x00}},
	{REGFLAG_DELAY, 200, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};


static struct LCM_setting_table lcm_deep_sleep_mode_in_setting[] = {
	// Display off sequence
	{0x28, 1, {0x00}},
    {REGFLAG_DELAY, 150, {}},

	// Sleep Mode On
	{0x10, 1, {0x00}},
	{REGFLAG_DELAY, 150, {}},

	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_compare_id_setting[] = {
	// Display off sequence
	{0xf0, 5, {0x55, 0xaa, 0x52, 0x08, 0x01}},
	{REGFLAG_DELAY, 10, {}},
	{REGFLAG_END_OF_TABLE, 0x00, {}}
};

static struct LCM_setting_table lcm_backlight_level_setting[] = {
	{0x51, 1, {0xFF}},
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
	params->dbi.te_mode 			= LCM_DBI_TE_MODE_DISABLED;
	params->dbi.te_edge_polarity		= LCM_POLARITY_RISING;



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
	params->dsi.vertical_sync_active				= 4;
	params->dsi.vertical_backporch					= 13;
	params->dsi.vertical_frontporch					= 13;
	params->dsi.vertical_active_line				= FRAME_HEIGHT; 
	params->dsi.horizontal_sync_active				= 35;
	params->dsi.horizontal_backporch				= 55;
	params->dsi.horizontal_frontporch				= 60;
	params->dsi.horizontal_blanking_pixel		       = 60;
	params->dsi.horizontal_active_pixel		       = FRAME_WIDTH;
	// Bit rate calculation
	params->dsi.pll_div1=0;		// div1=0,1,2,3;div1_real=1,2,4,4
	params->dsi.pll_div2=1;		// div2=0,1,2,3;div2_real=1,2,4,4
	params->dsi.fbk_sel=1;		 // fbk_sel=0,1,2,3;fbk_sel_real=1,2,4,4
	params->dsi.fbk_div =19;		// fref=26MHz, fvco=fref*(fbk_div+1)*2/(div1_real*div2_real)	
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
#ifdef BUILD_LK
	printf("[erick-lk]%s\n", __func__);
#else
	printk("[erick-k]%s\n", __func__);
#endif
}

static unsigned int lcm_compare_id(void);

static void lcm_suspend(void)
{


extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
#ifndef   BUILD_LK
	int k = 0 ; 
	int data[4] = {0,0};

	//k = PMIC_IMM_GetOneChannelValue_au0(0, 10, 1);
	IMM_GetOneChannelValue(0,  data, &k);
	printk("[wangli LCD_ID=%d\n] ",k);
	lcm_compare_id();
#endif



#ifndef BUILD_LK
	SET_RESET_PIN(1);
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(10);
	SET_RESET_PIN(1);
	MDELAY(20);
	push_table(lcm_deep_sleep_mode_in_setting, sizeof(lcm_deep_sleep_mode_in_setting) / sizeof(struct LCM_setting_table), 1);	//wqtao. enable
        SET_RESET_PIN(0);
	#ifdef BUILD_LK
		printf("[erick-lk]%s\n", __func__);
	#else
		printk("[erick-k]%s\n", __func__);
	#endif
#endif
}


static void lcm_resume(void)
{
#ifndef BUILD_LK
	lcm_init();

	//push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);

	#ifdef BUILD_LK
		printf("[erick-lk]%s\n", __func__);
	#else
		printk("[erick-k]%s\n", __func__);
	#endif
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

	data_array[0]= 0x00290508; //HW bug, so need send one HS packet
	dsi_set_cmdq(data_array, 1, 1);

	data_array[0]= 0x002c3909;
	dsi_set_cmdq(data_array, 1, 0);
}
#endif

#if 0	//wqtao.		
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

	// Refresh value of backlight level.
	lcm_backlight_level_setting[0].para_list[0] = mapped_level;

	push_table(lcm_backlight_level_setting, sizeof(lcm_backlight_level_setting) / sizeof(struct LCM_setting_table), 1);
}
#endif

static unsigned int lcm_esd_check(void)
{
#ifndef BUILD_UBOOT
	if(lcm_esd_test)
	{
	    lcm_esd_test = FALSE;
	    return TRUE;
	}

	/// please notice: the max return packet size is 1
	/// if you want to change it, you can refer to the following marked code
	/// but read_reg currently only support read no more than 4 bytes....
	/// if you need to read more, please let BinHan knows.
	/*
	        unsigned int data_array[16];
	        unsigned int max_return_size = 1;
	        
	        data_array[0]= 0x00003700 | (max_return_size << 16);    
	        
	        dsi_set_cmdq(&data_array, 1, 1);
	*/

	if(read_reg(0x0a) == 0x9c)
	{
	    return FALSE;
	}
	else
	{            
	    return TRUE;
	}
#endif
}

static unsigned int lcm_esd_recover(void)
{
    unsigned char para = 0;

    SET_RESET_PIN(1);
    SET_RESET_PIN(0);
    MDELAY(1);
    SET_RESET_PIN(1);
    MDELAY(120);
	  push_table(lcm_initialization_setting, sizeof(lcm_initialization_setting) / sizeof(struct LCM_setting_table), 1);
    MDELAY(10);
	  push_table(lcm_sleep_out_setting, sizeof(lcm_sleep_out_setting) / sizeof(struct LCM_setting_table), 1);
    MDELAY(10);
    dsi_set_cmdq_V2(0x35, 1, &para, 1);     ///enable TE
    MDELAY(10);

    return TRUE;
}


extern int IMM_GetOneChannelValue(int dwChannel, int data[4], int* rawdata);
static unsigned int lcm_compare_id(void)
{
	unsigned int id = 0;
	unsigned char buffer[5];
	unsigned int array[16];
	int ADC_ID = 0 ;
	int data[4] = {0,0};

	SET_RESET_PIN(1);	//NOTE:should reset LCM firstly
	MDELAY(10);
	SET_RESET_PIN(0);
	MDELAY(50);
	SET_RESET_PIN(1);
	MDELAY(50);

	array[0] = 0x00053700;	// read id return two byte,version and id
	dsi_set_cmdq(array, 1, 1);
	read_reg_v2(0xa1, buffer, 5);
	id = ((buffer[2] << 8) | buffer[3]);	//we only need ID

#ifdef BUILD_LK
	printf("[erick-lk]%s,  otm9605a id = 0x%08x\n", __func__, id);
	{
		extern int PMIC_IMM_GetOneChannelValue(int dwChannel, int deCount, int trimd);
	}
#else
	printk("[erick-k]%s,  otm9605a id = 0x%08x\n", __func__, id);

	{
		extern int PMIC_IMM_GetOneChannelValue_au0(int dwChannel, int deCount, int trimd);

	}
#endif

	IMM_GetOneChannelValue(0,  data, &ADC_ID);
	//printk("[wangli LCD_ID=%d\n] ",ADC_ID);

#ifdef BUILD_LK
	printf("[erick-lk - jc  wangli ]%s,  ADC_ID = %d \n", __func__, ADC_ID);
#endif  

         //1743--- 1754 
	if(ADC_ID >= 3000 && ADC_ID <= 3600 ) 	return (LCM_ID == id)?1:0;
	else   return 0 ;
		

}


// ---------------------------------------------------------------------------
//  Get LCM Driver Hooks
// ---------------------------------------------------------------------------
LCM_DRIVER otm9605a_dsi_vdo_qhd_JC = 
{
	.name			= "otm9605a_dsi_vdo_qhd_JC",
	.set_util_funcs = lcm_set_util_funcs,
	.get_params     = lcm_get_params,
	.init           = lcm_init,
	.suspend        = lcm_suspend,
	.resume         = lcm_resume,	
	.compare_id    = lcm_compare_id,	
#if (LCM_DSI_CMD_MODE)
	//.set_backlight	= lcm_setbacklight,
    //.esd_check   = lcm_esd_check,	
    //.esd_recover   = lcm_esd_recover,	
    .update         = lcm_update,
#endif	//wqtao
};

