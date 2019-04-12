/*****************************************************************************
 *  Copyright Statement:
 *  --------------------
 *  This software is protected by Copyright and the information contained
 *  herein is confidential. The software may not be copied and the information
 *  contained herein may not be used or disclosed except with the written
 *  permission of MediaTek Inc. (C) 2005
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

/*****************************************************************************
 *
 * Filename:
 * ---------
 *   gc0329yuv_Sensor.c
 *
 * Project:
 * --------
 *   MAUI
 *
 * Description:
 * ------------
 *   Image sensor driver function
 *   V1.2.3
 *
 * Author:
 * -------
 *   LANKING
 *
 *=============================================================
 *             HISTORY
 * Below this line, this part is controlled by GCoreinc. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Log$
 * 2013.01.23  kill bugs
 *   
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by GCoreinc. DO NOT MODIFY!!
 *=============================================================
 ******************************************************************************/
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "gc0329yuv_Sensor.h"
#include "gc0329yuv_Camera_Sensor_para.h"
#include "gc0329yuv_CameraCustomized.h"

//#define GC0329YUV_DEBUG
#ifdef GC0329YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

//#define DEBUG_SENSOR_GC0329

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

kal_uint16 GC0329_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
    char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};
	
	iWriteRegI2C(puSendCmd , 2, GC0329_WRITE_ID);

}
kal_uint16 GC0329_read_cmos_sensor(kal_uint8 addr)
{
	kal_uint16 get_byte=0;
    char puSendCmd = { (char)(addr & 0xFF) };
	iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte, 1, GC0329_WRITE_ID);
	
    return get_byte;
}

#ifdef DEBUG_SENSOR_GC0329
#define gc0329_OP_CODE_INI		0x00		/* Initial value. */
#define gc0329_OP_CODE_REG		0x01		/* Register */
#define gc0329_OP_CODE_DLY		0x02		/* Delay */
#define gc0329_OP_CODE_END		0x03		/* End of initial setting. */
static kal_uint16 fromsd;

typedef struct
{
	u16 init_reg;
	u16 init_val;	/* Save the register value and delay tick */
	u8 op_code;		/* 0 - Initial value, 1 - Register, 2 - Delay, 3 - End of setting. */
} gc0329_initial_set_struct;

gc0329_initial_set_struct gc0329_Init_Reg[5000];

static u32 strtol(const char *nptr, u8 base)
{

	printk("gc0329___%s____\n",__func__); 

	u8 ret;
	if(!nptr || (base!=16 && base!=10 && base!=8))
	{
		printk("gc0329 %s(): NULL pointer input\n", __FUNCTION__);
		return -1;
	}
	for(ret=0; *nptr; nptr++)
	{
		if((base==16 && *nptr>='A' && *nptr<='F') || 
				(base==16 && *nptr>='a' && *nptr<='f') || 
				(base>=10 && *nptr>='0' && *nptr<='9') ||
				(base>=8 && *nptr>='0' && *nptr<='7') )
		{
			ret *= base;
			if(base==16 && *nptr>='A' && *nptr<='F')
				ret += *nptr-'A'+10;
			else if(base==16 && *nptr>='a' && *nptr<='f')
				ret += *nptr-'a'+10;
			else if(base>=10 && *nptr>='0' && *nptr<='9')
				ret += *nptr-'0';
			else if(base>=8 && *nptr>='0' && *nptr<='7')
				ret += *nptr-'0';
		}
		else
			return ret;
	}
	return ret;
}

static u8 GC0329_Initialize_from_T_Flash()
{
	//FS_HANDLE fp = -1;				/* Default, no file opened. */
	//u8 *data_buff = NULL;
	u8 *curr_ptr = NULL;
	u32 file_size = 0;
	//u32 bytes_read = 0;
	u32 i = 0, j = 0;
	u8 func_ind[4] = {0};	/* REG or DLY */

	printk("gc0329___%s____11111111111111\n",__func__); 



	struct file *fp; 
	mm_segment_t fs; 
	loff_t pos = 0; 
	static u8 data_buff[10*1024] ;

	fp = filp_open("/mnt/sdcard/gc0329_sd.txt", O_RDONLY , 0); 
	if (IS_ERR(fp)) 
	{ 
		printk("0329 create file error 1111111\n");  
		return -1; 
	} 
	else
	{
		printk("0329 create file error 2222222\n");  
	}
	fs = get_fs(); 
	set_fs(KERNEL_DS); 

	file_size = vfs_llseek(fp, 0, SEEK_END);
	vfs_read(fp, data_buff, file_size, &pos); 
	//printk("%s %d %d\n", buf,iFileLen,pos); 
	filp_close(fp, NULL); 
	set_fs(fs);


	printk("gc0329___%s____22222222222222222\n",__func__); 



	/* Start parse the setting witch read from t-flash. */
	curr_ptr = data_buff;
	while (curr_ptr < (data_buff + file_size))
	{
		while ((*curr_ptr == ' ') || (*curr_ptr == '\t'))/* Skip the Space & TAB */
			curr_ptr++;				

		if (((*curr_ptr) == '/') && ((*(curr_ptr + 1)) == '*'))
		{
			while (!(((*curr_ptr) == '*') && ((*(curr_ptr + 1)) == '/')))
			{
				curr_ptr++;		/* Skip block comment code. */
			}

			while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
			{
				curr_ptr++;
			}

			curr_ptr += 2;						/* Skip the enter line */

			continue ;
		}

		if (((*curr_ptr) == '/') || ((*curr_ptr) == '{') || ((*curr_ptr) == '}'))		/* Comment line, skip it. */
		{
			while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
			{
				curr_ptr++;
			}

			curr_ptr += 2;						/* Skip the enter line */

			continue ;
		}
		/* This just content one enter line. */
		if (((*curr_ptr) == 0x0D) && ((*(curr_ptr + 1)) == 0x0A))
		{
			curr_ptr += 2;
			continue ;
		}
		//printk(" curr_ptr1 = %s\n",curr_ptr);
		memcpy(func_ind, curr_ptr, 3);


		if (strcmp((const char *)func_ind, "REG") == 0)		/* REG */
		{
			curr_ptr += 6;				/* Skip "REG(0x" or "DLY(" */
			gc0329_Init_Reg[i].op_code = gc0329_OP_CODE_REG;

			gc0329_Init_Reg[i].init_reg = strtol((const char *)curr_ptr, 16);
			curr_ptr += 5;	/* Skip "00, 0x" */

			gc0329_Init_Reg[i].init_val = strtol((const char *)curr_ptr, 16);
			curr_ptr += 4;	/* Skip "00);" */

		}
		else									/* DLY */
		{
			/* Need add delay for this setting. */ 
			curr_ptr += 4;	
			gc0329_Init_Reg[i].op_code = gc0329_OP_CODE_DLY;

			gc0329_Init_Reg[i].init_reg = 0xFF;
			gc0329_Init_Reg[i].init_val = strtol((const char *)curr_ptr,  10);	/* Get the delay ticks, the delay should less then 50 */
		}
		i++;


		/* Skip to next line directly. */
		while (!((*curr_ptr == 0x0D) && (*(curr_ptr+1) == 0x0A)))
		{
			curr_ptr++;
		}
		curr_ptr += 2;
	}

	/* (0xFFFF, 0xFFFF) means the end of initial setting. */
	gc0329_Init_Reg[i].op_code = gc0329_OP_CODE_END;
	gc0329_Init_Reg[i].init_reg = 0xFF;
	gc0329_Init_Reg[i].init_val = 0xFF;
	i++;
	//for (j=0; j<i; j++)
	printk("gc0329 %x  ==  %x\n",gc0329_Init_Reg[j].init_reg, gc0329_Init_Reg[j].init_val);
	
	printk("gc0329___%s____3333333333333333\n",__func__); 

	/* Start apply the initial setting to sensor. */
#if 1
	for (j=0; j<i; j++)
	{
		if (gc0329_Init_Reg[j].op_code == gc0329_OP_CODE_END)	/* End of the setting. */
		{
			printk("gc0329 REG OK -----------------END!\n");
		
			break ;
		}
		else if (gc0329_Init_Reg[j].op_code == gc0329_OP_CODE_DLY)
		{
			msleep(gc0329_Init_Reg[j].init_val);		/* Delay */
			printk("gc0329 REG OK -----------------DLY!\n");			
		}
		else if (gc0329_Init_Reg[j].op_code == gc0329_OP_CODE_REG)
		{

			GC0329_write_cmos_sensor(gc0329_Init_Reg[j].init_reg, gc0329_Init_Reg[j].init_val);
			printk("gc0329 REG OK!-----------------REG(0x%x,0x%x)\n",gc0329_Init_Reg[j].init_reg, gc0329_Init_Reg[j].init_val);			
			printk("gc0329 REG OK!-----------------REG(0x%x,0x%x)\n",gc0329_Init_Reg[j].init_reg, gc0329_Init_Reg[j].init_val);			
			printk("gc0329 REG OK!-----------------REG(0x%x,0x%x)\n",gc0329_Init_Reg[j].init_reg, gc0329_Init_Reg[j].init_val);			
			
		}
		else
		{
			printk("gc0329 REG ERROR!\n");
		}
	}
#endif
	return 1;	
}

#endif


/*******************************************************************************
 * // Adapter for Winmo typedef
 ********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT

kal_bool   GC0329_MPEG4_encode_mode = KAL_FALSE;
kal_uint16 GC0329_dummy_pixels = 0, GC0329_dummy_lines = 0;
kal_bool   GC0329_MODE_CAPTURE = KAL_FALSE;
kal_bool   GC0329_NIGHT_MODE = KAL_FALSE;

kal_uint32 GC0329_isp_master_clock;
static kal_uint32 GC0329_g_fPV_PCLK = 26;

kal_uint8 GC0329_sensor_write_I2C_address = GC0329_WRITE_ID;
kal_uint8 GC0329_sensor_read_I2C_address = GC0329_READ_ID;

UINT8 GC0329PixelClockDivider=0;

MSDK_SENSOR_CONFIG_STRUCT GC0329SensorConfigData;

#define GC0329_SET_PAGE0 	GC0329_write_cmos_sensor(0xfe, 0x00)
#define GC0329_SET_PAGE1 	GC0329_write_cmos_sensor(0xfe, 0x01)


/*************************************************************************
 * FUNCTION
 *	GC0329_SetShutter
 *
 * DESCRIPTION
 *	This function set e-shutter of GC0329 to change exposure time.
 *
 * PARAMETERS
 *   iShutter : exposured lines
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0329_Set_Shutter(kal_uint16 iShutter)
{
} /* Set_GC0329_Shutter */


/*************************************************************************
 * FUNCTION
 *	GC0329_read_Shutter
 *
 * DESCRIPTION
 *	This function read e-shutter of GC0329 .
 *
 * PARAMETERS
 *  None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint16 GC0329_Read_Shutter(void)
{
    	kal_uint8 temp_reg1, temp_reg2;
	kal_uint16 shutter;

	temp_reg1 = GC0329_read_cmos_sensor(0x04);
	temp_reg2 = GC0329_read_cmos_sensor(0x03);

	shutter = (temp_reg1 & 0xFF) | (temp_reg2 << 8);

	return shutter;
} /* GC0329_read_shutter */


/*************************************************************************
 * FUNCTION
 *	GC0329_write_reg
 *
 * DESCRIPTION
 *	This function set the register of GC0329.
 *
 * PARAMETERS
 *	addr : the register index of GC0329
 *  para : setting parameter of the specified register of GC0329
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0329_write_reg(kal_uint32 addr, kal_uint32 para)
{
	GC0329_write_cmos_sensor(addr, para);
} /* GC0329_write_reg() */


/*************************************************************************
 * FUNCTION
 *	GC0329_read_cmos_sensor
 *
 * DESCRIPTION
 *	This function read parameter of specified register from GC0329.
 *
 * PARAMETERS
 *	addr : the register index of GC0329
 *
 * RETURNS
 *	the data that read from GC0329
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint32 GC0329_read_reg(kal_uint32 addr)
{
	return GC0329_read_cmos_sensor(addr);
} /* OV7670_read_reg() */


/*************************************************************************
* FUNCTION
*	GC0329_awb_enable
*
* DESCRIPTION
*	This function enable or disable the awb (Auto White Balance).
*
* PARAMETERS
*	1. kal_bool : KAL_TRUE - enable awb, KAL_FALSE - disable awb.
*
* RETURNS
*	kal_bool : It means set awb right or not.
*
*************************************************************************/
static void GC0329_awb_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AWB_reg = 0;

	temp_AWB_reg = GC0329_read_cmos_sensor(0x42);
	
	if (enalbe)
	{
		GC0329_write_cmos_sensor(0x42, (temp_AWB_reg |0x02));
	}
	else
	{
		GC0329_write_cmos_sensor(0x42, (temp_AWB_reg & (~0x02)));
	}

}


/*************************************************************************
 * FUNCTION
 *	GC0329_config_window
 *
 * DESCRIPTION
 *	This function config the hardware window of GC0329 for getting specified
 *  data of that window.
 *
 * PARAMETERS
 *	start_x : start column of the interested window
 *  start_y : start row of the interested window
 *  width  : column widht of the itnerested window
 *  height : row depth of the itnerested window
 *
 * RETURNS
 *	the data that read from GC0329
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0329_config_window(kal_uint16 startx, kal_uint16 starty, kal_uint16 width, kal_uint16 height)
{
} /* GC0329_config_window */


/*************************************************************************
 * FUNCTION
 *	GC0329_SetGain
 *
 * DESCRIPTION
 *	This function is to set global gain to sensor.
 *
 * PARAMETERS
 *   iGain : sensor global gain(base: 0x40)
 *
 * RETURNS
 *	the actually gain set to sensor.
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint16 GC0329_SetGain(kal_uint16 iGain)
{
	return iGain;
}


/*************************************************************************
* FUNCTION
*	GC0329_GAMMA_Select
*
* DESCRIPTION
*	This function is served for FAE to select the appropriate GAMMA curve.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/


void GC0329GammaSelect(kal_uint32 GammaLvl)
{
	switch(GammaLvl)
	{
		case GC0329_RGB_Gamma_m1:						//smallest gamma curve
			GC0329_write_cmos_sensor(0xfe, 0x00);
			GC0329_write_cmos_sensor(0xbf, 0x06);
			GC0329_write_cmos_sensor(0xc0, 0x12);
			GC0329_write_cmos_sensor(0xc1, 0x22);
			GC0329_write_cmos_sensor(0xc2, 0x35);
			GC0329_write_cmos_sensor(0xc3, 0x4b);
			GC0329_write_cmos_sensor(0xc4, 0x5f);
			GC0329_write_cmos_sensor(0xc5, 0x72);
			GC0329_write_cmos_sensor(0xc6, 0x8d);
			GC0329_write_cmos_sensor(0xc7, 0xa4);
			GC0329_write_cmos_sensor(0xc8, 0xb8);
			GC0329_write_cmos_sensor(0xc9, 0xc8);
			GC0329_write_cmos_sensor(0xca, 0xd4);
			GC0329_write_cmos_sensor(0xcb, 0xde);
			GC0329_write_cmos_sensor(0xcc, 0xe6);
			GC0329_write_cmos_sensor(0xcd, 0xf1);
			GC0329_write_cmos_sensor(0xce, 0xf8);
			GC0329_write_cmos_sensor(0xcf, 0xfd);
			break;
		case GC0329_RGB_Gamma_m2:
			GC0329_write_cmos_sensor(0xBF, 0x08);
			GC0329_write_cmos_sensor(0xc0, 0x0F);
			GC0329_write_cmos_sensor(0xc1, 0x21);
			GC0329_write_cmos_sensor(0xc2, 0x32);
			GC0329_write_cmos_sensor(0xc3, 0x43);
			GC0329_write_cmos_sensor(0xc4, 0x50);
			GC0329_write_cmos_sensor(0xc5, 0x5E);
			GC0329_write_cmos_sensor(0xc6, 0x78);
			GC0329_write_cmos_sensor(0xc7, 0x90);
			GC0329_write_cmos_sensor(0xc8, 0xA6);
			GC0329_write_cmos_sensor(0xc9, 0xB9);
			GC0329_write_cmos_sensor(0xcA, 0xC9);
			GC0329_write_cmos_sensor(0xcB, 0xD6);
			GC0329_write_cmos_sensor(0xcC, 0xE0);
			GC0329_write_cmos_sensor(0xcD, 0xEE);
			GC0329_write_cmos_sensor(0xcE, 0xF8);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0329_RGB_Gamma_m3:			
			GC0329_write_cmos_sensor(0xfe , 0x00);
			GC0329_write_cmos_sensor(0xbf , 0x0b);
			GC0329_write_cmos_sensor(0xc0 , 0x1d);
			GC0329_write_cmos_sensor(0xc1 , 0x33);
			GC0329_write_cmos_sensor(0xc2 , 0x49);
			GC0329_write_cmos_sensor(0xc3 , 0x5d);
			GC0329_write_cmos_sensor(0xc4 , 0x6e);
			GC0329_write_cmos_sensor(0xc5 , 0x7c);
			GC0329_write_cmos_sensor(0xc6 , 0x99);
			GC0329_write_cmos_sensor(0xc7 , 0xaf);
			GC0329_write_cmos_sensor(0xc8 , 0xc2);
			GC0329_write_cmos_sensor(0xc9 , 0xd0);
			GC0329_write_cmos_sensor(0xca , 0xda);
			GC0329_write_cmos_sensor(0xcb , 0xe2);
			GC0329_write_cmos_sensor(0xcc , 0xe7);
			GC0329_write_cmos_sensor(0xcd , 0xf0);
			GC0329_write_cmos_sensor(0xce , 0xf7);
			GC0329_write_cmos_sensor(0xcf , 0xff);
			break;
			
		case GC0329_RGB_Gamma_m4:
			GC0329_write_cmos_sensor(0xBF, 0x0E);
			GC0329_write_cmos_sensor(0xc0, 0x1C);
			GC0329_write_cmos_sensor(0xc1, 0x34);
			GC0329_write_cmos_sensor(0xc2, 0x48);
			GC0329_write_cmos_sensor(0xc3, 0x5A);
			GC0329_write_cmos_sensor(0xc4, 0x6B);
			GC0329_write_cmos_sensor(0xc5, 0x7B);
			GC0329_write_cmos_sensor(0xc6, 0x95);
			GC0329_write_cmos_sensor(0xc7, 0xAB);
			GC0329_write_cmos_sensor(0xc8, 0xBF);
			GC0329_write_cmos_sensor(0xc9, 0xCE);
			GC0329_write_cmos_sensor(0xcA, 0xD9);
			GC0329_write_cmos_sensor(0xcB, 0xE4);
			GC0329_write_cmos_sensor(0xcC, 0xEC);
			GC0329_write_cmos_sensor(0xcD, 0xF7);
			GC0329_write_cmos_sensor(0xcE, 0xFD);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0329_RGB_Gamma_m5:
			GC0329_write_cmos_sensor(0xBF, 0x10);
			GC0329_write_cmos_sensor(0xc0, 0x20);
			GC0329_write_cmos_sensor(0xc1, 0x38);
			GC0329_write_cmos_sensor(0xc2, 0x4E);
			GC0329_write_cmos_sensor(0xc3, 0x63);
			GC0329_write_cmos_sensor(0xc4, 0x76);
			GC0329_write_cmos_sensor(0xc5, 0x87);
			GC0329_write_cmos_sensor(0xc6, 0xA2);
			GC0329_write_cmos_sensor(0xc7, 0xB8);
			GC0329_write_cmos_sensor(0xc8, 0xCA);
			GC0329_write_cmos_sensor(0xc9, 0xD8);
			GC0329_write_cmos_sensor(0xcA, 0xE3);
			GC0329_write_cmos_sensor(0xcB, 0xEB);
			GC0329_write_cmos_sensor(0xcC, 0xF0);
			GC0329_write_cmos_sensor(0xcD, 0xF8);
			GC0329_write_cmos_sensor(0xcE, 0xFD);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
			
		case GC0329_RGB_Gamma_m6:										// largest gamma curve
			GC0329_write_cmos_sensor(0xBF, 0x14);
			GC0329_write_cmos_sensor(0xc0, 0x28);
			GC0329_write_cmos_sensor(0xc1, 0x44);
			GC0329_write_cmos_sensor(0xc2, 0x5D);
			GC0329_write_cmos_sensor(0xc3, 0x72);
			GC0329_write_cmos_sensor(0xc4, 0x86);
			GC0329_write_cmos_sensor(0xc5, 0x95);
			GC0329_write_cmos_sensor(0xc6, 0xB1);
			GC0329_write_cmos_sensor(0xc7, 0xC6);
			GC0329_write_cmos_sensor(0xc8, 0xD5);
			GC0329_write_cmos_sensor(0xc9, 0xE1);
			GC0329_write_cmos_sensor(0xcA, 0xEA);
			GC0329_write_cmos_sensor(0xcB, 0xF1);
			GC0329_write_cmos_sensor(0xcC, 0xF5);
			GC0329_write_cmos_sensor(0xcD, 0xFB);
			GC0329_write_cmos_sensor(0xcE, 0xFE);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
		case GC0329_RGB_Gamma_night:									//Gamma for night mode
			GC0329_write_cmos_sensor(0xBF, 0x0B);
			GC0329_write_cmos_sensor(0xc0, 0x16);
			GC0329_write_cmos_sensor(0xc1, 0x29);
			GC0329_write_cmos_sensor(0xc2, 0x3C);
			GC0329_write_cmos_sensor(0xc3, 0x4F);
			GC0329_write_cmos_sensor(0xc4, 0x5F);
			GC0329_write_cmos_sensor(0xc5, 0x6F);
			GC0329_write_cmos_sensor(0xc6, 0x8A);
			GC0329_write_cmos_sensor(0xc7, 0x9F);
			GC0329_write_cmos_sensor(0xc8, 0xB4);
			GC0329_write_cmos_sensor(0xc9, 0xC6);
			GC0329_write_cmos_sensor(0xcA, 0xD3);
			GC0329_write_cmos_sensor(0xcB, 0xDD);
			GC0329_write_cmos_sensor(0xcC, 0xE5);
			GC0329_write_cmos_sensor(0xcD, 0xF1);
			GC0329_write_cmos_sensor(0xcE, 0xFA);
			GC0329_write_cmos_sensor(0xcF, 0xFF);
			break;
		default:
			//GC0329_RGB_Gamma_m3
			GC0329_write_cmos_sensor(0xfe , 0x00);
			GC0329_write_cmos_sensor(0xbf , 0x0b);
			GC0329_write_cmos_sensor(0xc0 , 0x1d);
			GC0329_write_cmos_sensor(0xc1 , 0x33);
			GC0329_write_cmos_sensor(0xc2 , 0x49);
			GC0329_write_cmos_sensor(0xc3 , 0x5d);
			GC0329_write_cmos_sensor(0xc4 , 0x6e);
			GC0329_write_cmos_sensor(0xc5 , 0x7c);
			GC0329_write_cmos_sensor(0xc6 , 0x99);
			GC0329_write_cmos_sensor(0xc7 , 0xaf);
			GC0329_write_cmos_sensor(0xc8 , 0xc2);
			GC0329_write_cmos_sensor(0xc9 , 0xd0);
			GC0329_write_cmos_sensor(0xca , 0xda);
			GC0329_write_cmos_sensor(0xcb , 0xe2);
			GC0329_write_cmos_sensor(0xcc , 0xe7);
			GC0329_write_cmos_sensor(0xcd , 0xf0);
			GC0329_write_cmos_sensor(0xce , 0xf7);
			GC0329_write_cmos_sensor(0xcf , 0xff);
			break;
	}
}


/*************************************************************************
 * FUNCTION
 *	GC0329_NightMode
 *
 * DESCRIPTION
 *	This function night mode of GC0329.
 *
 * PARAMETERS
 *	bEnable: KAL_TRUE -> enable night mode, otherwise, disable night mode
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void GC0329NightMode(kal_bool bEnable)
{
	if (bEnable)
	{	
		GC0329_write_cmos_sensor(0xfe, 0x01);
		if(GC0329_MPEG4_encode_mode == KAL_TRUE)
			GC0329_write_cmos_sensor(0x33, 0x30);
		else
			GC0329_write_cmos_sensor(0x33, 0x30);
		GC0329_write_cmos_sensor(0xfe, 0x00);
		GC0329GammaSelect(GC0329_RGB_Gamma_night);
		GC0329_NIGHT_MODE = KAL_TRUE;
	}
	else 
	{
		GC0329_write_cmos_sensor(0xfe, 0x01);
		if(GC0329_MPEG4_encode_mode == KAL_TRUE)
			GC0329_write_cmos_sensor(0x33, 0x20);
		else
			GC0329_write_cmos_sensor(0x33, 0x20);
		GC0329_write_cmos_sensor(0xfe, 0x00);
		GC0329GammaSelect(GC0329_RGB_Gamma_m3);
		GC0329_NIGHT_MODE = KAL_FALSE;
	}
} /* GC0329_NightMode */

/*************************************************************************
* FUNCTION
*	GC0329_Sensor_Init
*
* DESCRIPTION
*	This function apply all of the initial setting to sensor.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
*************************************************************************/
void GC0329_Sensor_Init(void)
{
	GC0329_write_cmos_sensor(0xfe , 0x80);
	GC0329_write_cmos_sensor(0xfc , 0x16);
	GC0329_write_cmos_sensor(0xfc , 0x16);
	GC0329_write_cmos_sensor(0xfe , 0x00);

	GC0329_write_cmos_sensor(0x70 , 0x48);
	GC0329_write_cmos_sensor(0x73 , 0x90);
	GC0329_write_cmos_sensor(0x74 , 0x80);
	GC0329_write_cmos_sensor(0x75 , 0x80);
	GC0329_write_cmos_sensor(0x76 , 0x94); //80 jambo 
	GC0329_write_cmos_sensor(0x77 , 0x62);
	GC0329_write_cmos_sensor(0x78 , 0x47);
	GC0329_write_cmos_sensor(0x79 , 0x40);

	GC0329_write_cmos_sensor(0x03 , 0x02);
	GC0329_write_cmos_sensor(0x04 , 0x40);


	////////////////////analog////////////////////
	GC0329_write_cmos_sensor(0xfc , 0x16);
	GC0329_write_cmos_sensor(0x09 , 0x00);
	GC0329_write_cmos_sensor(0x0a , 0x02);
	GC0329_write_cmos_sensor(0x0b , 0x00);
	GC0329_write_cmos_sensor(0x0c , 0x02);
	GC0329_write_cmos_sensor(0x17 , 0x14);
	GC0329_write_cmos_sensor(0x19 , 0x05);
	GC0329_write_cmos_sensor(0x1b , 0x24);
	GC0329_write_cmos_sensor(0x1c , 0x04);
	GC0329_write_cmos_sensor(0x1e , 0x08);
	GC0329_write_cmos_sensor(0x1f , 0x08); //C8 
	GC0329_write_cmos_sensor(0x20 , 0x01);
	GC0329_write_cmos_sensor(0x21 , 0x48);
	GC0329_write_cmos_sensor(0x22 , 0xba);
	GC0329_write_cmos_sensor(0x23 , 0x22);
	GC0329_write_cmos_sensor(0x24 , 0x16);


	////////////////////blk////////////////////
	GC0329_write_cmos_sensor(0x26 , 0xf7); //BLK
	GC0329_write_cmos_sensor(0x28 , 0x7f); //BLK limit
	GC0329_write_cmos_sensor(0x29 , 0x00);
	GC0329_write_cmos_sensor(0x32 , 0x00); //04 darkc
	GC0329_write_cmos_sensor(0x33 , 0x20); //blk ratio
	GC0329_write_cmos_sensor(0x34 , 0x20);
	GC0329_write_cmos_sensor(0x35 , 0x20);
	GC0329_write_cmos_sensor(0x36 , 0x20);

	GC0329_write_cmos_sensor(0x3b , 0x04); //manual offset
	GC0329_write_cmos_sensor(0x3c , 0x04);
	GC0329_write_cmos_sensor(0x3d , 0x04);
	GC0329_write_cmos_sensor(0x3e , 0x04);

	////////////////////ISP BLOCK ENABLE////////////////////
	GC0329_write_cmos_sensor(0x40 , 0xff);
	GC0329_write_cmos_sensor(0x41 , 0x24);//[5]skin detection
	GC0329_write_cmos_sensor(0x42 , 0xfa);//disable ABS 
	GC0329_write_cmos_sensor(0x46 , 0x02);
	GC0329_write_cmos_sensor(0x4b , 0xca);
	GC0329_write_cmos_sensor(0x4d , 0x01);
	GC0329_write_cmos_sensor(0x4f , 0x01);
	GC0329_write_cmos_sensor(0x70 , 0x48);

	////////////////////DNDD////////////////////
	GC0329_write_cmos_sensor(0x80 , 0x07); // 0xe7 20140915
	GC0329_write_cmos_sensor(0x81 , 0xc2); // 0x22 20140915
	GC0329_write_cmos_sensor(0x82 , 0x90); //DN auto DNDD DEC DNDD //0e //55 jambo
	GC0329_write_cmos_sensor(0x83 , 0x05);
	GC0329_write_cmos_sensor(0x87 , 0x40); // 0x4a  20140915 

	////////////////////INTPEE////////////////////
	GC0329_write_cmos_sensor(0x90 , 0x8c); //ac
	GC0329_write_cmos_sensor(0x92 , 0x05);
	GC0329_write_cmos_sensor(0x94 , 0x05);
	GC0329_write_cmos_sensor(0x95 , 0x45); //0x44
	GC0329_write_cmos_sensor(0x96 , 0x88); 

	////////////////////ASDE////////////////////
	GC0329_write_cmos_sensor(0xfe , 0x01);
	GC0329_write_cmos_sensor(0x18 , 0x22); 
	GC0329_write_cmos_sensor(0xfe , 0x00);
	GC0329_write_cmos_sensor(0x9c , 0x0a);
	GC0329_write_cmos_sensor(0xa0 , 0xaf);
	GC0329_write_cmos_sensor(0xa2 , 0xff); 
	GC0329_write_cmos_sensor(0xa4 , 0x30); //50 jambo 
	GC0329_write_cmos_sensor(0xa5 , 0x31); 
	GC0329_write_cmos_sensor(0xa7 , 0x35); 

	////////////////////RGB gamma////////////////////
	GC0329_write_cmos_sensor(0xfe , 0x00);
	GC0329_write_cmos_sensor(0xbf , 0x0b);
	GC0329_write_cmos_sensor(0xc0 , 0x1d);
	GC0329_write_cmos_sensor(0xc1 , 0x33);
	GC0329_write_cmos_sensor(0xc2 , 0x49);
	GC0329_write_cmos_sensor(0xc3 , 0x5d);
	GC0329_write_cmos_sensor(0xc4 , 0x6e);
	GC0329_write_cmos_sensor(0xc5 , 0x7c);
	GC0329_write_cmos_sensor(0xc6 , 0x99);
	GC0329_write_cmos_sensor(0xc7 , 0xaf);
	GC0329_write_cmos_sensor(0xc8 , 0xc2);
	GC0329_write_cmos_sensor(0xc9 , 0xd0);
	GC0329_write_cmos_sensor(0xca , 0xda);
	GC0329_write_cmos_sensor(0xcb , 0xe2);
	GC0329_write_cmos_sensor(0xcc , 0xe7);
	GC0329_write_cmos_sensor(0xcd , 0xf0);
	GC0329_write_cmos_sensor(0xce , 0xf7);
	GC0329_write_cmos_sensor(0xcf , 0xff);

	////////////////////Y gamma////////////////////
	GC0329_write_cmos_sensor(0xfe , 0x00);
	GC0329_write_cmos_sensor(0x63 , 0x00);
	GC0329_write_cmos_sensor(0x64 , 0x06);
	GC0329_write_cmos_sensor(0x65 , 0x0d);
	GC0329_write_cmos_sensor(0x66 , 0x1b);
	GC0329_write_cmos_sensor(0x67 , 0x2b);
	GC0329_write_cmos_sensor(0x68 , 0x3d);
	GC0329_write_cmos_sensor(0x69 , 0x50);
	GC0329_write_cmos_sensor(0x6a , 0x60);
	GC0329_write_cmos_sensor(0x6b , 0x80);
	GC0329_write_cmos_sensor(0x6c , 0xa0);
	GC0329_write_cmos_sensor(0x6d , 0xc0);
	GC0329_write_cmos_sensor(0x6e , 0xe0);
	GC0329_write_cmos_sensor(0x6f , 0xff);
	 
	 
	//////////////////CC///////////////////
#if 1 //main
	GC0329_write_cmos_sensor(0xfe , 0x00);
	GC0329_write_cmos_sensor(0xb3 , 0x44);
	GC0329_write_cmos_sensor(0xb4 , 0xfd);
	GC0329_write_cmos_sensor(0xb5 , 0x02);
	GC0329_write_cmos_sensor(0xb6 , 0xfa);
	GC0329_write_cmos_sensor(0xb7 , 0x48);
	GC0329_write_cmos_sensor(0xb8 , 0xf0);
#else //sub
	GC0329_write_cmos_sensor(0xfe , 0x00);
	GC0329_write_cmos_sensor(0xb3 , 0x42);//40
	GC0329_write_cmos_sensor(0xb4 , 0xff);//00
	GC0329_write_cmos_sensor(0xb5 , 0x06);//06
	GC0329_write_cmos_sensor(0xb6 , 0xf0);//00
	GC0329_write_cmos_sensor(0xb7 , 0x44);//40
	GC0329_write_cmos_sensor(0xb8 , 0xf0);//00
#endif

	// crop 
	GC0329_write_cmos_sensor(0x50 , 0x01);


	////////////////////YCP////////////////////
	GC0329_write_cmos_sensor(0xfe , 0x00);

	GC0329_write_cmos_sensor(0xd0 , 0x40);
	GC0329_write_cmos_sensor(0xd1 , 0x28);
	GC0329_write_cmos_sensor(0xd2 , 0x28);

	GC0329_write_cmos_sensor(0xd3 , 0x40); //cont 0x40
	GC0329_write_cmos_sensor(0xd5 , 0x00);

	GC0329_write_cmos_sensor(0xdd , 0x14); 
	GC0329_write_cmos_sensor(0xde , 0x34); 

	////////////////////AEC////////////////////
	GC0329_write_cmos_sensor(0xfe , 0x01);
	GC0329_write_cmos_sensor(0x10 , 0x40); // before Gamma 
	GC0329_write_cmos_sensor(0x11 , 0x21); //
	GC0329_write_cmos_sensor(0x12 , 0x13);	// center weight *2
	GC0329_write_cmos_sensor(0x13 , 0x50); //4 //4); // AE Target
	GC0329_write_cmos_sensor(0x17 , 0xa8);	//88, 08, c8, a8
	GC0329_write_cmos_sensor(0x1a , 0x21);
	GC0329_write_cmos_sensor(0x20 , 0x31);	//AEC stop margin
	GC0329_write_cmos_sensor(0x21 , 0xc0);
	GC0329_write_cmos_sensor(0x22 , 0x60); 
	GC0329_write_cmos_sensor(0x3c , 0x50);
	GC0329_write_cmos_sensor(0x3d , 0x40); 
	GC0329_write_cmos_sensor(0x3e , 0x45); //read 3f for status

	////////////////////AWB////////////////////
#if 1 //main
	GC0329_write_cmos_sensor(0xfe , 0x01);
	GC0329_write_cmos_sensor(0x06 , 0x12);
	GC0329_write_cmos_sensor(0x07 , 0x06);
	GC0329_write_cmos_sensor(0x08 , 0x9c);
	GC0329_write_cmos_sensor(0x09 , 0xee);
	GC0329_write_cmos_sensor(0x50 , 0xfc);
	GC0329_write_cmos_sensor(0x51 , 0x28);
	GC0329_write_cmos_sensor(0x52 , 0x10);
	GC0329_write_cmos_sensor(0x53 , 0x20);
	GC0329_write_cmos_sensor(0x54 , 0x12);
	GC0329_write_cmos_sensor(0x55 , 0x16);
	GC0329_write_cmos_sensor(0x56 , 0x30);
	GC0329_write_cmos_sensor(0x58 , 0x60);
	GC0329_write_cmos_sensor(0x59 , 0x08);
	GC0329_write_cmos_sensor(0x5a , 0x02);
	GC0329_write_cmos_sensor(0x5b , 0x63);
	GC0329_write_cmos_sensor(0x5c , 0x35);
	GC0329_write_cmos_sensor(0x5d , 0x72);
	GC0329_write_cmos_sensor(0x5e , 0x11);
	GC0329_write_cmos_sensor(0x5f , 0x40);
	GC0329_write_cmos_sensor(0x60 , 0x40);
	GC0329_write_cmos_sensor(0x61 , 0xc8);
	GC0329_write_cmos_sensor(0x62 , 0xa0);
	GC0329_write_cmos_sensor(0x63 , 0x40);
	GC0329_write_cmos_sensor(0x64 , 0x50);
	GC0329_write_cmos_sensor(0x65 , 0x98);
	GC0329_write_cmos_sensor(0x66 , 0xfa);
	GC0329_write_cmos_sensor(0x67 , 0x80);
	GC0329_write_cmos_sensor(0x68 , 0x60);
	GC0329_write_cmos_sensor(0x69 , 0x90);
	GC0329_write_cmos_sensor(0x6a , 0x40);
	GC0329_write_cmos_sensor(0x6b , 0x39);
	GC0329_write_cmos_sensor(0x6c , 0x30);
	GC0329_write_cmos_sensor(0x6d , 0x60);
	GC0329_write_cmos_sensor(0x6e , 0x41);
	GC0329_write_cmos_sensor(0x70 , 0x10);
	GC0329_write_cmos_sensor(0x71 , 0x00);
	GC0329_write_cmos_sensor(0x72 , 0x10);
	GC0329_write_cmos_sensor(0x73 , 0x40);
	GC0329_write_cmos_sensor(0x80 , 0x60);
	GC0329_write_cmos_sensor(0x81 , 0x50);
	GC0329_write_cmos_sensor(0x82 , 0x42);
	GC0329_write_cmos_sensor(0x83 , 0x40);
	GC0329_write_cmos_sensor(0x84 , 0x40);
	GC0329_write_cmos_sensor(0x85 , 0x40);
	GC0329_write_cmos_sensor(0x74 , 0x40);
	GC0329_write_cmos_sensor(0x75 , 0x58);
	GC0329_write_cmos_sensor(0x76 , 0x24);
	GC0329_write_cmos_sensor(0x77 , 0x40);
	GC0329_write_cmos_sensor(0x78 , 0x20);
	GC0329_write_cmos_sensor(0x79 , 0x60);
	GC0329_write_cmos_sensor(0x7a , 0x58);
	GC0329_write_cmos_sensor(0x7b , 0x20);
	GC0329_write_cmos_sensor(0x7c , 0x30);
	GC0329_write_cmos_sensor(0x7d , 0x35);
	GC0329_write_cmos_sensor(0x7e , 0x10);
	GC0329_write_cmos_sensor(0x7f , 0x08);
#else //sub
	GC0329_write_cmos_sensor(0xfe , 0x01);
	GC0329_write_cmos_sensor(0x06 , 0x16);
	GC0329_write_cmos_sensor(0x07 , 0x06);
	GC0329_write_cmos_sensor(0x08 , 0x98);
	GC0329_write_cmos_sensor(0x09 , 0xee);
	GC0329_write_cmos_sensor(0x50 , 0xfc);
	GC0329_write_cmos_sensor(0x51 , 0x20);
	GC0329_write_cmos_sensor(0x52 , 0x1b);//0b
	GC0329_write_cmos_sensor(0x53 , 0x10);//20
	GC0329_write_cmos_sensor(0x54 , 0x10);//40
	GC0329_write_cmos_sensor(0x55 , 0x10);
	GC0329_write_cmos_sensor(0x56 , 0x20);
	GC0329_write_cmos_sensor(0x57 , 0xa0);
	GC0329_write_cmos_sensor(0x58 , 0xa0);
	GC0329_write_cmos_sensor(0x59 , 0x28);
	GC0329_write_cmos_sensor(0x5a , 0x02);
	GC0329_write_cmos_sensor(0x5b , 0x63);
	GC0329_write_cmos_sensor(0x5c , 0x04);//34
	GC0329_write_cmos_sensor(0x5d , 0x73);
	GC0329_write_cmos_sensor(0x5e , 0x11);
	GC0329_write_cmos_sensor(0x5f , 0x40);
	GC0329_write_cmos_sensor(0x60 , 0x40);
	GC0329_write_cmos_sensor(0x61 , 0xc8);//d8 //c8
	GC0329_write_cmos_sensor(0x62 , 0xa0);///88 //A0
	GC0329_write_cmos_sensor(0x63 , 0x40);
	GC0329_write_cmos_sensor(0x64 , 0x40);//37
	GC0329_write_cmos_sensor(0x65 , 0xd0);
	GC0329_write_cmos_sensor(0x66 , 0xfa);
	GC0329_write_cmos_sensor(0x67 , 0x70);
	GC0329_write_cmos_sensor(0x68 , 0x58);
	GC0329_write_cmos_sensor(0x69 , 0xa5); //85 jaambo
	GC0329_write_cmos_sensor(0x6a , 0x40);
	GC0329_write_cmos_sensor(0x6b , 0x39);
	GC0329_write_cmos_sensor(0x6c , 0x18);
	GC0329_write_cmos_sensor(0x6d , 0x28);
	GC0329_write_cmos_sensor(0x6e , 0x41);
	GC0329_write_cmos_sensor(0x70 , 0x40);
	GC0329_write_cmos_sensor(0x71 , 0x00);
	GC0329_write_cmos_sensor(0x72 , 0x10);
	GC0329_write_cmos_sensor(0x73 , 0x30);//awb outdoor-th
	GC0329_write_cmos_sensor(0x80 , 0x60);
	GC0329_write_cmos_sensor(0x81 , 0x50);
	GC0329_write_cmos_sensor(0x82 , 0x42);
	GC0329_write_cmos_sensor(0x83 , 0x40);
	GC0329_write_cmos_sensor(0x84 , 0x40);
	GC0329_write_cmos_sensor(0x85 , 0x40);
	GC0329_write_cmos_sensor(0x86 , 0x40);
	GC0329_write_cmos_sensor(0x87 , 0x40);
	GC0329_write_cmos_sensor(0x88 , 0xfe);
	GC0329_write_cmos_sensor(0x89 , 0xa0);
	GC0329_write_cmos_sensor(0x74 , 0x40);
	GC0329_write_cmos_sensor(0x75 , 0x58);
	GC0329_write_cmos_sensor(0x76 , 0x24);
	GC0329_write_cmos_sensor(0x77 , 0x40);
	GC0329_write_cmos_sensor(0x78 , 0x20);
	GC0329_write_cmos_sensor(0x79 , 0x60);
	GC0329_write_cmos_sensor(0x7a , 0x58);
	GC0329_write_cmos_sensor(0x7b , 0x20);
	GC0329_write_cmos_sensor(0x7c , 0x30);
	GC0329_write_cmos_sensor(0x7d , 0x35);
	GC0329_write_cmos_sensor(0x7e , 0x10);
	GC0329_write_cmos_sensor(0x7f , 0x08);
#endif

	///////////////////ABS///////////////////////
	GC0329_write_cmos_sensor(0x9c , 0x00);
	GC0329_write_cmos_sensor(0x9e , 0xc0);
	GC0329_write_cmos_sensor(0x9f , 0x40);

	////////////////////CC-AWB////////////////////
	GC0329_write_cmos_sensor(0xd0 , 0x00);
	GC0329_write_cmos_sensor(0xd2 , 0x2c); 
	GC0329_write_cmos_sensor(0xd3 , 0x80);

	///////////////////LSC //////////////////////

	GC0329_write_cmos_sensor(0xfe , 0x01);
	GC0329_write_cmos_sensor(0xc0 , 0x0b);
	GC0329_write_cmos_sensor(0xc1 , 0x07);
	GC0329_write_cmos_sensor(0xc2 , 0x05);
	GC0329_write_cmos_sensor(0xc6 , 0x0b);
	GC0329_write_cmos_sensor(0xc7 , 0x07);
	GC0329_write_cmos_sensor(0xc8 , 0x05);
	GC0329_write_cmos_sensor(0xba , 0x39);
	GC0329_write_cmos_sensor(0xbb , 0x24);
	GC0329_write_cmos_sensor(0xbc , 0x23);
	GC0329_write_cmos_sensor(0xb4 , 0x39);
	GC0329_write_cmos_sensor(0xb5 , 0x24);
	GC0329_write_cmos_sensor(0xb6 , 0x23);
	GC0329_write_cmos_sensor(0xc3 , 0x00);
	GC0329_write_cmos_sensor(0xc4 , 0x00);
	GC0329_write_cmos_sensor(0xc5 , 0x00);
	GC0329_write_cmos_sensor(0xc9 , 0x00);
	GC0329_write_cmos_sensor(0xca , 0x00);
	GC0329_write_cmos_sensor(0xcb , 0x00);
	GC0329_write_cmos_sensor(0xbd , 0x2b);
	GC0329_write_cmos_sensor(0xbe , 0x00);
	GC0329_write_cmos_sensor(0xbf , 0x00);
	GC0329_write_cmos_sensor(0xb7 , 0x09);
	GC0329_write_cmos_sensor(0xb8 , 0x00);
	GC0329_write_cmos_sensor(0xb9 , 0x00);
	GC0329_write_cmos_sensor(0xa8 , 0x31);
	GC0329_write_cmos_sensor(0xa9 , 0x23);
	GC0329_write_cmos_sensor(0xaa , 0x20);
	GC0329_write_cmos_sensor(0xab , 0x31);
	GC0329_write_cmos_sensor(0xac , 0x23);
	GC0329_write_cmos_sensor(0xad , 0x20);
	GC0329_write_cmos_sensor(0xae , 0x31);
	GC0329_write_cmos_sensor(0xaf , 0x23);
	GC0329_write_cmos_sensor(0xb0 , 0x20);
	GC0329_write_cmos_sensor(0xb1 , 0x31);
	GC0329_write_cmos_sensor(0xb2 , 0x23);
	GC0329_write_cmos_sensor(0xb3 , 0x20);
	GC0329_write_cmos_sensor(0xa4 , 0x00);
	GC0329_write_cmos_sensor(0xa5 , 0x00);
	GC0329_write_cmos_sensor(0xa6 , 0x00);
	GC0329_write_cmos_sensor(0xa7 , 0x00);
	GC0329_write_cmos_sensor(0xa1 , 0x3c);
	GC0329_write_cmos_sensor(0xa2 , 0x50);
	GC0329_write_cmos_sensor(0xfe , 0x00);

	//////////////////// flicker ///////////////////
	GC0329_write_cmos_sensor(0x05 , 0x02); 	
	GC0329_write_cmos_sensor(0x06 , 0x2c); 
	GC0329_write_cmos_sensor(0x07 , 0x00);
	GC0329_write_cmos_sensor(0x08 , 0xb8);
	GC0329_write_cmos_sensor(0xfe , 0x01); 
	GC0329_write_cmos_sensor(0x29 , 0x00);  //anti-flicker step [11:8]
	GC0329_write_cmos_sensor(0x2a , 0x60);  //anti-flicker step [7:0]
	GC0329_write_cmos_sensor(0x2b , 0x02);  //exp level 0 , 0x14.28fps
	GC0329_write_cmos_sensor(0x2c , 0xa0); 
	GC0329_write_cmos_sensor(0x2d , 0x03);  //exp level 1 , 0x12.50fps
	GC0329_write_cmos_sensor(0x2e , 0x00); 
	GC0329_write_cmos_sensor(0x2f , 0x03);  //exp level 2 , 0x10.00fps
	GC0329_write_cmos_sensor(0x30 , 0xc0); 
	GC0329_write_cmos_sensor(0x31 , 0x05);  //exp level 3 , 0x7.14fps
	GC0329_write_cmos_sensor(0x32 , 0x40); 
	GC0329_write_cmos_sensor(0xfe , 0x00); 


	////////////////////out ///////////////////

	GC0329_write_cmos_sensor(0x44 , 0xa2);
	GC0329_write_cmos_sensor(0xf0 , 0x07);
	GC0329_write_cmos_sensor(0xf1 , 0x01);


}



UINT32 GC0329GetSensorID(UINT32 *sensorID)
{
    kal_uint16 sensor_id=0;
    int i;

    GC0329_write_cmos_sensor(0xfc, 0x16);
    Sleep(20);

    do
    {
        	// check if sensor ID correct
        	for(i = 0; i < 3; i++)
		{
	            	sensor_id = GC0329_read_cmos_sensor(0x00);
	            	printk("GC0329 Sensor id = %x\n", sensor_id);
	            	if (sensor_id == GC0329_SENSOR_ID)
			{
	               	break;
	            	}
        	}
        	mdelay(50);
    }while(0);

    if(sensor_id != GC0329_SENSOR_ID)
    {
        SENSORDB("GC0329 Sensor id read failed, ID = %x\n", sensor_id);
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    *sensorID = sensor_id;

    RETAILMSG(1, (TEXT("Sensor Read ID OK \r\n")));
	
    return ERROR_NONE;
}




/*************************************************************************
* FUNCTION
*	GC0329_Write_More_Registers
*
* DESCRIPTION
*	This function is served for FAE to modify the necessary Init Regs. Do not modify the regs
*     in init_GC0329() directly.
*
* PARAMETERS
*	None
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void GC0329_Write_More_Registers(void)
{
	///////////// for FAE to modify the necessary Init Regs. ////////////
}


/*************************************************************************
 * FUNCTION
 *	GC0329Open
 *
 * DESCRIPTION
 *	This function initialize the registers of CMOS sensor
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0329Open(void)
{
    kal_uint16 sensor_id=0;
    int i;

    GC0329_write_cmos_sensor(0xfc, 0x16);
    Sleep(20);

    do
    {
        	// check if sensor ID correct
        	for(i = 0; i < 3; i++)
		{
	            	sensor_id = GC0329_read_cmos_sensor(0x00);
	            	printk("GC0329 Sensor id = %x\n", sensor_id);
	            	if (sensor_id == GC0329_SENSOR_ID)
			{
	               	break;
	            	}
        	}
        	mdelay(50);
    }while(0);

    if(sensor_id != GC0329_SENSOR_ID)
    {
        SENSORDB("GC0329 Sensor id read failed, ID = %x\n", sensor_id);
        return ERROR_SENSOR_CONNECT_FAIL;
    }
	
     GC0329_MPEG4_encode_mode = KAL_FALSE;
    RETAILMSG(1, (TEXT("Sensor Read ID OK \r\n")));
    // initail sequence write in
    GC0329_Sensor_Init();
    GC0329_Write_More_Registers();

#ifdef DEBUG_SENSOR_GC0329  
		struct file *fp; 
		mm_segment_t fs; 
		loff_t pos = 0; 
		static char buf[60*1024] ;

		printk("open 0329 debug \n");
		printk("open 0329 debug \n");
		printk("open 0329 debug \n");	


		fp = filp_open("/mnt/sdcard/gc0329_sd.txt", O_RDONLY , 0); 

		if (IS_ERR(fp)) 
		{ 

			fromsd = 0;   
			printk("open 0329 file error\n");
			printk("open 0329 file error\n");
			printk("open 0329 file error\n");		


		} 
		else 
		{
			fromsd = 1;
			printk("open 0329 file ok\n");
			printk("open 0329 file ok\n");
			printk("open 0329 file ok\n");

			//gc0329_Initialize_from_T_Flash();
			
			filp_close(fp, NULL); 
			set_fs(fs);
		}

		if(fromsd == 1)
		{
			printk("________________0329 from t!\n");
			printk("________________0329 from t!\n");
			printk("________________0329 from t!\n");		
			GC0329_Initialize_from_T_Flash();
			printk("______after_____0329 from t!\n");	
			//GC0329_Sensor_Init(); 	
		}
		else
		{
			//GC0329_MPEG4_encode_mode = KAL_FALSE;
			printk("________________0329 not from t!\n");	
			printk("________________0329 not from t!\n");
			printk("________________0329 not from t!\n");		
			RETAILMSG(1, (TEXT("Sensor Read ID OK \r\n"))); 
		}

#endif
	
    return ERROR_NONE;
} /* GC0329Open */


/*************************************************************************
 * FUNCTION
 *	GC0329Close
 *
 * DESCRIPTION
 *	This function is to turn off sensor module power.
 *
 * PARAMETERS
 *	None
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0329Close(void)
{
    return ERROR_NONE;
} /* GC0329Close */


/*************************************************************************
 * FUNCTION
 * GC0329Preview
 *
 * DESCRIPTION
 *	This function start the sensor preview.
 *
 * PARAMETERS
 *	*image_window : address pointer of pixel numbers in one period of HSYNC
 *  *sensor_config_data : address pointer of line numbers in one period of VSYNC
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0329Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    kal_uint32 iTemp;
    kal_uint16 iStartX = 0, iStartY = 1;

    if(sensor_config_data->SensorOperationMode == MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
        RETAILMSG(1, (TEXT("Camera Video preview\r\n")));
        GC0329_MPEG4_encode_mode = KAL_TRUE;
       
    }
    else
    {
        RETAILMSG(1, (TEXT("Camera preview\r\n")));
        GC0329_MPEG4_encode_mode = KAL_FALSE;
    }

    image_window->GrabStartX= IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY= IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth = IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight =IMAGE_SENSOR_PV_HEIGHT;

    // copy sensor_config_data
    memcpy(&GC0329SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* GC0329Preview */


/*************************************************************************
 * FUNCTION
 *	GC0329Capture
 *
 * DESCRIPTION
 *	This function setup the CMOS sensor in capture MY_OUTPUT mode
 *
 * PARAMETERS
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
UINT32 GC0329Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    GC0329_MODE_CAPTURE=KAL_TRUE;

    image_window->GrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth= IMAGE_SENSOR_FULL_WIDTH;
    image_window->ExposureWindowHeight = IMAGE_SENSOR_FULL_HEIGHT;

    // copy sensor_config_data
    memcpy(&GC0329SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* GC0329_Capture() */



UINT32 GC0329GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT;
    pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT;
    pSensorResolution->SensorVideoWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorVideoHeight=IMAGE_SENSOR_PV_HEIGHT;
    return ERROR_NONE;
} /* GC0329GetResolution() */


UINT32 GC0329GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
        MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    pSensorInfo->SensorPreviewResolutionX=IMAGE_SENSOR_PV_WIDTH;
    pSensorInfo->SensorPreviewResolutionY=IMAGE_SENSOR_PV_HEIGHT;
    pSensorInfo->SensorFullResolutionX=IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY=IMAGE_SENSOR_FULL_HEIGHT;

    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=10;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=1;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;
    pSensorInfo->CaptureDelayFrame = 1;
    pSensorInfo->PreviewDelayFrame = 0;
    pSensorInfo->VideoDelayFrame = 4;
    pSensorInfo->YUVAwbDelayFrame = 2;  // add by lanking
    pSensorInfo->YUVEffectDelayFrame = 2;  // add by lanking
    pSensorInfo->SensorMasterClockSwitch = 0;
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_2MA;

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount=	3;
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;

        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
        break;
    default:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
        pSensorInfo->SensorGrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
        break;
    }
    GC0329PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &GC0329SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* GC0329GetInfo() */


UINT32 GC0329Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
       // GC0329Capture(pImageWindow, pSensorConfigData);
         GC0329Preview(pImageWindow, pSensorConfigData);
        break;
    }


    return TRUE;
}	/* GC0329Control() */

BOOL GC0329_set_param_wb(UINT16 para)
{

	switch (para)
	{
		case AWB_MODE_OFF:

		break;
		
		case AWB_MODE_AUTO:
			GC0329_awb_enable(KAL_TRUE);
		break;
		
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x8c); //WB_manual_gain 
			GC0329_write_cmos_sensor(0x78, 0x50);
			GC0329_write_cmos_sensor(0x79, 0x40);
		break;
		
		case AWB_MODE_DAYLIGHT: //sunny
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x74); 
			GC0329_write_cmos_sensor(0x78, 0x52);
			GC0329_write_cmos_sensor(0x79, 0x40);			
		break;
		
		case AWB_MODE_INCANDESCENT: //office
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x48);
			GC0329_write_cmos_sensor(0x78, 0x40);
			GC0329_write_cmos_sensor(0x79, 0x5c);
		break;
		
		case AWB_MODE_TUNGSTEN: //home
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x40);
			GC0329_write_cmos_sensor(0x78, 0x54);
			GC0329_write_cmos_sensor(0x79, 0x70);
		break;
		
		case AWB_MODE_FLUORESCENT:
			GC0329_awb_enable(KAL_FALSE);
			GC0329_write_cmos_sensor(0x77, 0x40);
			GC0329_write_cmos_sensor(0x78, 0x42);
			GC0329_write_cmos_sensor(0x79, 0x50);
		break;
		
		default:
		return FALSE;
	}

	return TRUE;
} /* GC0329_set_param_wb */


BOOL GC0329_set_param_effect(UINT16 para)
{
	kal_uint32  ret = KAL_TRUE;

	switch (para)
	{
		case MEFFECT_OFF:
			GC0329_write_cmos_sensor(0x43 , 0x00);
		break;
		
		case MEFFECT_SEPIA:
			GC0329_write_cmos_sensor(0x43 , 0x02);
			GC0329_write_cmos_sensor(0xda , 0xd0);
			GC0329_write_cmos_sensor(0xdb , 0x28);
		break;
		
		case MEFFECT_NEGATIVE:
			GC0329_write_cmos_sensor(0x43 , 0x01);
		break;
		
		case MEFFECT_SEPIAGREEN:
			GC0329_write_cmos_sensor(0x43 , 0x02);
			GC0329_write_cmos_sensor(0xda , 0xc0);
			GC0329_write_cmos_sensor(0xdb , 0xc0);
		break;
		
		case MEFFECT_SEPIABLUE:
			GC0329_write_cmos_sensor(0x43 , 0x02);
			GC0329_write_cmos_sensor(0xda , 0x50);
			GC0329_write_cmos_sensor(0xdb , 0xe0);
		break;

		case MEFFECT_MONO:
			GC0329_write_cmos_sensor(0x43 , 0x02);
			GC0329_write_cmos_sensor(0xda , 0x00);
			GC0329_write_cmos_sensor(0xdb , 0x00);
		break;
		default:
			ret = FALSE;
	}

	return ret;

} /* GC0329_set_param_effect */


BOOL GC0329_set_param_banding(UINT16 para)
{
	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:
			GC0329_write_cmos_sensor(0x05, 0x02); 	
			GC0329_write_cmos_sensor(0x06, 0x2c); 
			GC0329_write_cmos_sensor(0x07, 0x00);
			GC0329_write_cmos_sensor(0x08, 0xb8);
			
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x29, 0x00);   //anti-flicker step [11:8]
			GC0329_write_cmos_sensor(0x2a, 0x60);   //anti-flicker step [7:0]
			
			GC0329_write_cmos_sensor(0x2b, 0x02);   //exp level 0  14.28fps
			GC0329_write_cmos_sensor(0x2c, 0xa0); 
			GC0329_write_cmos_sensor(0x2d, 0x03);   //exp level 1  12.50fps
			GC0329_write_cmos_sensor(0x2e, 0x00); 
			GC0329_write_cmos_sensor(0x2f, 0x03);   //exp level 2  10.00fps
			GC0329_write_cmos_sensor(0x30, 0xc0); 
			GC0329_write_cmos_sensor(0x31, 0x05);   //exp level 3  7.14fps
			GC0329_write_cmos_sensor(0x32, 0x40); 
			GC0329_SET_PAGE0;
			break;

		case AE_FLICKER_MODE_60HZ:
			GC0329_write_cmos_sensor(0x05, 0x02); 	
			GC0329_write_cmos_sensor(0x06, 0x4c); 
			GC0329_write_cmos_sensor(0x07, 0x00);
			GC0329_write_cmos_sensor(0x08, 0x88);
			
			GC0329_SET_PAGE1;
			GC0329_write_cmos_sensor(0x29, 0x00);   //anti-flicker step [11:8]
			GC0329_write_cmos_sensor(0x2a, 0x4e);   //anti-flicker step [7:0]
			
			GC0329_write_cmos_sensor(0x2b, 0x02);   //exp level 0  15.00fps
			GC0329_write_cmos_sensor(0x2c, 0x70); 
			GC0329_write_cmos_sensor(0x2d, 0x03);   //exp level 0  12.00fps
			GC0329_write_cmos_sensor(0x2e, 0x0c); 
			GC0329_write_cmos_sensor(0x2f, 0x03);   //exp level 0  10.00fps
			GC0329_write_cmos_sensor(0x30, 0xa8); 
			GC0329_write_cmos_sensor(0x31, 0x05);   //exp level 0  7.05fps
			GC0329_write_cmos_sensor(0x32, 0x2e); 
			GC0329_SET_PAGE0;
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* GC0329_set_param_banding */


BOOL GC0329_set_param_exposure(UINT16 para)
{
	kal_uint8 value_luma, value_Y;
	value_luma = (GC0329_NIGHT_MODE?0x2b:0x00);
	value_Y = (GC0329_NIGHT_MODE?0x68:0x50);

	switch (para)
	{
		case AE_EV_COMP_n13:
			GC0329_write_cmos_sensor(0xd5, value_luma - 0x48);
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0x13, value_Y - 0x28);
			GC0329_write_cmos_sensor(0xfe, 0x00);
		break;
		
		case AE_EV_COMP_n10:
			GC0329_write_cmos_sensor(0xd5, value_luma - 0x30);
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0x13, value_Y - 0x18);
			GC0329_write_cmos_sensor(0xfe, 0x00);
		break;
		
		case AE_EV_COMP_n07:
			GC0329_write_cmos_sensor(0xd5, value_luma - 0x20);
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0x13, value_Y - 0x10);
			GC0329_write_cmos_sensor(0xfe, 0x00);
		break;
		
		case AE_EV_COMP_n03:
			GC0329_write_cmos_sensor(0xd5, value_luma - 0x10);
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0x13, value_Y - 0x08);
			GC0329_write_cmos_sensor(0xfe, 0x00);
		break;				
		
		case AE_EV_COMP_00:
			GC0329_write_cmos_sensor(0xd5, value_luma);
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0x13, value_Y);
			GC0329_write_cmos_sensor(0xfe, 0x00);
		break;

		case AE_EV_COMP_03:
			GC0329_write_cmos_sensor(0xd5, value_luma + 0x10);
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0x13, value_Y + 0x10);
			GC0329_write_cmos_sensor(0xfe, 0x00);
		break;
		
		case AE_EV_COMP_07:
			GC0329_write_cmos_sensor(0xd5, value_luma + 0x20);
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0x13, value_Y + 0x20);
			GC0329_write_cmos_sensor(0xfe, 0x00);
		break;
		
		case AE_EV_COMP_10:
			GC0329_write_cmos_sensor(0xd5, value_luma + 0x30);
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0x13, value_Y + 0x30);
			GC0329_write_cmos_sensor(0xfe, 0x00);
		break;
		
		case AE_EV_COMP_13:
			GC0329_write_cmos_sensor(0xd5, value_luma + 0x48);
			GC0329_write_cmos_sensor(0xfe, 0x01);
			GC0329_write_cmos_sensor(0x13, value_Y + 0x48);
			GC0329_write_cmos_sensor(0xfe, 0x00);
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* GC0329_set_param_exposure */

UINT32 GC0329YUVSetVideoMode(UINT16 u2FrameRate)    // lanking add
{
  
        GC0329_MPEG4_encode_mode = KAL_TRUE;
     if (u2FrameRate == 30)
   	{
   	
   	    /*********video frame ************/
		
   	}
    else if (u2FrameRate == 15)       
    	{
    	
   	    /*********video frame ************/
		
    	}
    else
   	{
   	
            SENSORDB("Wrong Frame Rate"); 
			
   	}

      return TRUE;

}


UINT32 GC0329YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{

#ifdef DEBUG_SENSOR_GC0329
		printk("______%s______GC0329 YUV setting\n",__func__);
		return TRUE;
#endif

    switch (iCmd) {
    case FID_AWB_MODE:
        GC0329_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        GC0329_set_param_effect(iPara);
        break;
    case FID_AE_EV:
        GC0329_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
        GC0329_set_param_banding(iPara);
		break;
	case FID_SCENE_MODE:
		GC0329NightMode(iPara);
        break;
    default:
        break;
    }
    return TRUE;
} /* GC0329YUVSensorSetting */


UINT32 GC0329FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 GC0329SensorRegNumber;
    UINT32 i;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

    RETAILMSG(1, (_T("gaiyang GC0329FeatureControl FeatureId=%d\r\n"), FeatureId));

    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=IMAGE_SENSOR_FULL_WIDTH;
        *pFeatureReturnPara16=IMAGE_SENSOR_FULL_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        *pFeatureReturnPara16++=(VGA_PERIOD_PIXEL_NUMS)+GC0329_dummy_pixels;
        *pFeatureReturnPara16=(VGA_PERIOD_LINE_NUMS)+GC0329_dummy_lines;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        *pFeatureReturnPara32 = GC0329_g_fPV_PCLK;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        //GC0329NightMode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        GC0329_isp_master_clock=*pFeatureData32;
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        GC0329_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        pSensorRegData->RegData = GC0329_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        memcpy(pSensorConfigData, &GC0329SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
        *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
        break;
    case SENSOR_FEATURE_SET_CCT_REGISTER:
    case SENSOR_FEATURE_GET_CCT_REGISTER:
    case SENSOR_FEATURE_SET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_ENG_REGISTER:
    case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
    case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
    case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
    case SENSOR_FEATURE_GET_GROUP_COUNT:
    case SENSOR_FEATURE_GET_GROUP_INFO:
    case SENSOR_FEATURE_GET_ITEM_INFO:
    case SENSOR_FEATURE_SET_ITEM_INFO:
    case SENSOR_FEATURE_GET_ENG_INFO:
        break;
    case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
        // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
        // if EEPROM does not exist in camera module.
        *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_YUV_CMD:
        GC0329YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;
    case SENSOR_FEATURE_SET_VIDEO_MODE:    //  lanking
	 GC0329YUVSetVideoMode(*pFeatureData16);
	 break;
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
	GC0329GetSensorID(pFeatureData32);
	break;
    default:
        break;
	}
return ERROR_NONE;
}	/* GC0329FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncGC0329YUV=
{
	GC0329Open,
	GC0329GetInfo,
	GC0329GetResolution,
	GC0329FeatureControl,
	GC0329Control,
	GC0329Close
};


UINT32 GC0329_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncGC0329YUV;
	return ERROR_NONE;
} /* SensorInit() */
