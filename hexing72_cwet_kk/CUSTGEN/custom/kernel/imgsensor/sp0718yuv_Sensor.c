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
 *   sp0718yuv_Sensor.c
 *
 * Project:
 * --------
 *   MAUI
 *
 * Description:

      9.20 朱争华 上传sp0718 mtk6515平台的代码，该代码在鼎维尔调试验证过.
 * ------------
 *   Image sensor driver function
 *   V1.2.3
 *
 * Author:
 * -------
 *   Leo
 *
 *=============================================================
 *             HISTORY
 * Below this line, this part is controlled by GCoreinc. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Log$
 * 2012.02.29  kill bugs
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

#include "sp0718yuv_Sensor.h"
#include "sp0718yuv_Camera_Sensor_para.h"
#include "sp0718yuv_CameraCustomized.h"

//#include <mt6575_gpio.h>
#define SP0718_NORMAL_Y0ffset  0x20
#define SP0718_LOWLIGHT_Y0ffset  0x25
//AE target
#define  SP0718_P1_0xeb  0x78
#define  SP0718_P1_0xec  0x6c
//HEQ
#define  SP0718_P1_0x10  0x00//outdoor
#define  SP0718_P1_0x14  0x20
#define  SP0718_P1_0x11  0x00//nr
#define  SP0718_P1_0x15  0x18
#define  SP0718_P1_0x12  0x00//dummy
#define  SP0718_P1_0x16  0x10
#define  SP0718_P1_0x13  0x00//low
#define  SP0718_P1_0x17  0x00
#define SP0718_YUV_DEBUG
#ifdef SP0718_YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

kal_uint16 SP0718_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
    char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};
	
	iWriteRegI2C(puSendCmd , 2, SP0718_WRITE_ID);

}
kal_uint16 SP0718_read_cmos_sensor(kal_uint8 addr)
{
	kal_uint16 get_byte=0;
    char puSendCmd = { (char)(addr & 0xFF) };
	iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte, 1, SP0718_WRITE_ID);
	
    return get_byte;
}


/*******************************************************************************
 * // Adapter for Winmo typedef
 ********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT

kal_bool   SP0718_MPEG4_encode_mode = KAL_FALSE;
kal_uint16 SP0718_dummy_pixels = 0, SP0718_dummy_lines = 0;
kal_bool   SP0718_MODE_CAPTURE = KAL_FALSE;
kal_bool   SP0718_NIGHT_MODE = KAL_FALSE;

kal_uint32 SP0718_isp_master_clock;
static kal_uint32 SP0718_g_fPV_PCLK = 24;

kal_uint8 SP0718_sensor_write_I2C_address = SP0718_WRITE_ID;
kal_uint8 SP0718_sensor_read_I2C_address = SP0718_READ_ID;

UINT8 SP0718_PixelClockDivider=0;

kal_uint8 sp0718_isBanding = 0; // 0: 50hz  1:60hz

MSDK_SENSOR_CONFIG_STRUCT SP0718_SensorConfigData;



/*************************************************************************
 * FUNCTION
 *	SP0718_SetShutter
 *
 * DESCRIPTION
 *	This function set e-shutter of SP0718_ to change exposure time.
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
void SP0718_Set_Shutter(kal_uint16 iShutter)
{
} /* Set_SP0718_Shutter */


/*************************************************************************
 * FUNCTION
 *	SP0718_read_Shutter
 *
 * DESCRIPTION
 *	This function read e-shutter of SP0718_ .
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
kal_uint16 SP0718_Read_Shutter(void)
{
} /* SP0718_read_shutter */


/*************************************************************************
* FUNCTION
*	SP0718_awb_enable
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
static void SP0718_awb_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AWB_reg = 0;
#if 0
	temp_AWB_reg = SP0718_read_cmos_sensor(0x42);
	
	if (enalbe)
	{
		SP0718_write_cmos_sensor(0x42, (temp_AWB_reg |0x02));
	}
	else
	{
		SP0718_write_cmos_sensor(0x42, (temp_AWB_reg & (~0x02)));
	}
#endif
}


/*************************************************************************
 * FUNCTION
 *	SP0718_config_window
 *
 * DESCRIPTION
 *	This function config the hardware window of SP0718_ for getting specified
 *  data of that window.
 *
 * PARAMETERS
 *	start_x : start column of the interested window
 *  start_y : start row of the interested window
 *  width  : column widht of the itnerested window
 *  height : row depth of the itnerested window
 *
 * RETURNS
 *	the data that read from SP0718_
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void SP0718_config_window(kal_uint16 startx, kal_uint16 starty, kal_uint16 width, kal_uint16 height)
{
} /* SP0718_config_window */


/*************************************************************************
 * FUNCTION
 *	SP0718_SetGain
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
kal_uint16 SP0718_SetGain(kal_uint16 iGain)
{
	return iGain;
}

/*************************************************************************
 * FUNCTION
 *	SP0718_NightMode
 *
 * DESCRIPTION
 *	This function night mode of SP0718_.
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
void SP0718_NightMode(kal_bool bEnable)
{

	if(bEnable)//night mode
	{ 
		SP0718_NIGHT_MODE = KAL_TRUE;			
		SP0718_write_cmos_sensor(0xfd,0x01);
		SP0718_write_cmos_sensor(0xcd,SP0718_LOWLIGHT_Y0ffset);
		SP0718_write_cmos_sensor(0xce,0x1f);
	   if(SP0718_MPEG4_encode_mode == KAL_TRUE)
		{
				if(sp0718_isBanding== 0)
				{
					//SI15_SP0718 24M 50Hz 10-10fps AE_Parameters_20121210104002.txt
					//ae setting
					  SP0718_write_cmos_sensor(0xfd,0x00);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x04,0x32);
					  SP0718_write_cmos_sensor(0x06,0x00);
					  SP0718_write_cmos_sensor(0x09,0x05);
					  SP0718_write_cmos_sensor(0x0a,0xdf);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0xef,0x33);
					  SP0718_write_cmos_sensor(0xf0,0x00);
					  SP0718_write_cmos_sensor(0x02,0x0a);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x06,0x2d);
					  SP0718_write_cmos_sensor(0x07,0x00);
					  SP0718_write_cmos_sensor(0x08,0x01);
					  SP0718_write_cmos_sensor(0x09,0x00);
					//Status
					  SP0718_write_cmos_sensor(0xfd,0x02);
					  SP0718_write_cmos_sensor(0xbe,0xfe);
					  SP0718_write_cmos_sensor(0xbf,0x01);
					  SP0718_write_cmos_sensor(0xd0,0xfe);
					  SP0718_write_cmos_sensor(0xd1,0x01);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x5b,0x01);
					  SP0718_write_cmos_sensor(0x5c,0xfe);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x32,0x15);
				
				       SENSORDB(" video 50Hz night\r\n");
				}
				else if(sp0718_isBanding == 1)
				{
					//SI15_SP0718 24M 60Hz 10.1176-10fps AE_Parameters_20121210104008.txt
					//ae setting
					  SP0718_write_cmos_sensor(0xfd,0x00);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x04,0x02);
					  SP0718_write_cmos_sensor(0x06,0x00);
					  SP0718_write_cmos_sensor(0x09,0x05);
					  SP0718_write_cmos_sensor(0x0a,0xc4);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0xef,0x2b);
					  SP0718_write_cmos_sensor(0xf0,0x00);
					  SP0718_write_cmos_sensor(0x02,0x0c);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x06,0x25);
					  SP0718_write_cmos_sensor(0x07,0x00);
					  SP0718_write_cmos_sensor(0x08,0x01);
					  SP0718_write_cmos_sensor(0x09,0x00);
					//Status
					  SP0718_write_cmos_sensor(0xfd,0x02);
					  SP0718_write_cmos_sensor(0xbe,0x04);
					  SP0718_write_cmos_sensor(0xbf,0x02);
					  SP0718_write_cmos_sensor(0xd0,0x04);
					  SP0718_write_cmos_sensor(0xd1,0x02);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x5b,0x02);
					  SP0718_write_cmos_sensor(0x5c,0x04);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x32,0x15);

				       SENSORDB(" video 60Hz night\r\n");
				}
	  	}	
	    else 
	   {  
				
			       if(sp0718_isBanding== 0)
				{
					//SI15_SP0718 24M 50Hz 15-6fps AE_Parameters_20121210101250.txt
					//ae setting
					  SP0718_write_cmos_sensor(0xfd,0x00);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x04,0xce);
					  SP0718_write_cmos_sensor(0x06,0x00);
					  SP0718_write_cmos_sensor(0x09,0x02);
					  SP0718_write_cmos_sensor(0x0a,0xc4);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0xef,0x4d);
					  SP0718_write_cmos_sensor(0xf0,0x00);
					  SP0718_write_cmos_sensor(0x02,0x10);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x06,0x47);
					  SP0718_write_cmos_sensor(0x07,0x00);
					  SP0718_write_cmos_sensor(0x08,0x01);
					  SP0718_write_cmos_sensor(0x09,0x00);
					//Status                            
					  SP0718_write_cmos_sensor(0xfd,0x02);
					  SP0718_write_cmos_sensor(0xbe,0xd0);
					  SP0718_write_cmos_sensor(0xbf,0x04);
					  SP0718_write_cmos_sensor(0xd0,0xd0);
					  SP0718_write_cmos_sensor(0xd1,0x04);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x5b,0x04);
					  SP0718_write_cmos_sensor(0x5c,0xd0);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x32,0x15);
			
				       SENSORDB(" priview 50Hz night\r\n");	
				}  
				else if(sp0718_isBanding== 1)
				{
					//SI15_SP0718 24M 60Hz 15-6fps AE_Parameters_20121210101304.txt
					//ae setting
					  SP0718_write_cmos_sensor(0xfd,0x00);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x04,0x80);
					  SP0718_write_cmos_sensor(0x06,0x00);
					  SP0718_write_cmos_sensor(0x09,0x02);
					  SP0718_write_cmos_sensor(0x0a,0xc9);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0xef,0x40);
					  SP0718_write_cmos_sensor(0xf0,0x00);
					  SP0718_write_cmos_sensor(0x02,0x14);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x06,0x3a);
					  SP0718_write_cmos_sensor(0x07,0x00);
					  SP0718_write_cmos_sensor(0x08,0x01);
					  SP0718_write_cmos_sensor(0x09,0x00);
					//Status                            
					  SP0718_write_cmos_sensor(0xfd,0x02);
					  SP0718_write_cmos_sensor(0xbe,0x00);
					  SP0718_write_cmos_sensor(0xbf,0x05);
					  SP0718_write_cmos_sensor(0xd0,0x00);
					  SP0718_write_cmos_sensor(0xd1,0x05);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x5b,0x05);
					  SP0718_write_cmos_sensor(0x5c,0x00);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x32,0x15);

				       SENSORDB(" priview 60Hz night\r\n");	
				}
       } 		
	}
	else    // daylight mode
	{
		
		SP0718_NIGHT_MODE = KAL_FALSE;			
             SP0718_write_cmos_sensor(0xfd,0x01);
		SP0718_write_cmos_sensor(0xcd,SP0718_NORMAL_Y0ffset);
		SP0718_write_cmos_sensor(0xce,0x1f);
	         if(SP0718_MPEG4_encode_mode == KAL_TRUE)
	          {
				
				if(sp0718_isBanding== 0)
				{
					//SI15_SP0718 24M 50Hz 20-20fps AE_Parameters_20121210103954.txt
					//ae setting
					  SP0718_write_cmos_sensor(0xfd,0x00);
					  SP0718_write_cmos_sensor(0x03,0x02);
					  SP0718_write_cmos_sensor(0x04,0x64);
					  SP0718_write_cmos_sensor(0x06,0x00);
					  SP0718_write_cmos_sensor(0x09,0x01);
					  SP0718_write_cmos_sensor(0x0a,0x46);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0xef,0x66);
					  SP0718_write_cmos_sensor(0xf0,0x00);
					  SP0718_write_cmos_sensor(0x02,0x05);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x06,0x60);
					  SP0718_write_cmos_sensor(0x07,0x00);
					  SP0718_write_cmos_sensor(0x08,0x01);
					  SP0718_write_cmos_sensor(0x09,0x00);
					//Status
					  SP0718_write_cmos_sensor(0xfd,0x02);
					  SP0718_write_cmos_sensor(0xbe,0xfe);
					  SP0718_write_cmos_sensor(0xbf,0x01);
					  SP0718_write_cmos_sensor(0xd0,0xfe);
					  SP0718_write_cmos_sensor(0xd1,0x01);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x5b,0x01);
					  SP0718_write_cmos_sensor(0x5c,0xfe);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x32,0x15);
				
				       SENSORDB(" video 50Hz normal\r\n");				
				}
				else if(sp0718_isBanding == 1)
				{
					//SI15_SP0718 24M 60Hz 20-20fps AE_Parameters_20121210103936.txt
					//ae setting
					  SP0718_write_cmos_sensor(0xfd,0x00);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x04,0xfe);
					  SP0718_write_cmos_sensor(0x06,0x00);
					  SP0718_write_cmos_sensor(0x09,0x01);
					  SP0718_write_cmos_sensor(0x0a,0x46);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0xef,0x55);
					  SP0718_write_cmos_sensor(0xf0,0x00);
					  SP0718_write_cmos_sensor(0x02,0x06);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x06,0x4f);
					  SP0718_write_cmos_sensor(0x07,0x00);
					  SP0718_write_cmos_sensor(0x08,0x01);
					  SP0718_write_cmos_sensor(0x09,0x00);
					//Status
					  SP0718_write_cmos_sensor(0xfd,0x02);
					  SP0718_write_cmos_sensor(0xbe,0xfe);
					  SP0718_write_cmos_sensor(0xbf,0x01);
					  SP0718_write_cmos_sensor(0xd0,0xfe);
					  SP0718_write_cmos_sensor(0xd1,0x01);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x5b,0x01);
					  SP0718_write_cmos_sensor(0x5c,0xfe);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x32,0x15);

				       SENSORDB(" video 60Hz normal \n");	
				}
			   }
		else 
			{
			       if(sp0718_isBanding== 0)
				{
					//SI15_SP0718 24M 50Hz 15-8fps AE_Parameters_20121208114739.txt
					//ae setting
					   SP0718_write_cmos_sensor(0xfd,0x00);
					   SP0718_write_cmos_sensor(0x03,0x01);
					   SP0718_write_cmos_sensor(0x04,0xce);
					   SP0718_write_cmos_sensor(0x06,0x00);
					   SP0718_write_cmos_sensor(0x09,0x02);
					   SP0718_write_cmos_sensor(0x0a,0xc4);
					   SP0718_write_cmos_sensor(0xfd,0x01);
					   SP0718_write_cmos_sensor(0xef,0x4d);
					   SP0718_write_cmos_sensor(0xf0,0x00);
					   SP0718_write_cmos_sensor(0x02,0x0c);
					   SP0718_write_cmos_sensor(0x03,0x01);
					   SP0718_write_cmos_sensor(0x06,0x47);
					   SP0718_write_cmos_sensor(0x07,0x00);
					   SP0718_write_cmos_sensor(0x08,0x01);
					   SP0718_write_cmos_sensor(0x09,0x00);
					//Status                             
					   SP0718_write_cmos_sensor(0xfd,0x02);
					   SP0718_write_cmos_sensor(0xbe,0x9c);
					   SP0718_write_cmos_sensor(0xbf,0x03);
					   SP0718_write_cmos_sensor(0xd0,0x9c);
					   SP0718_write_cmos_sensor(0xd1,0x03);
					   SP0718_write_cmos_sensor(0xfd,0x01);
					   SP0718_write_cmos_sensor(0x5b,0x03);
					   SP0718_write_cmos_sensor(0x5c,0x9c);
					   SP0718_write_cmos_sensor(0xfd,0x01);
					   SP0718_write_cmos_sensor(0x32,0x15);
				
				       SENSORDB(" priview 50Hz normal\r\n");
				}
				else if(sp0718_isBanding== 1)
				{
					//SI15_SP0718 24M 60Hz 15-8fps AE_Parameters_20121210101304.txt
					//ae setting
					  SP0718_write_cmos_sensor(0xfd,0x00);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x04,0x80);
					  SP0718_write_cmos_sensor(0x06,0x00);
					  SP0718_write_cmos_sensor(0x09,0x02);
					  SP0718_write_cmos_sensor(0x0a,0xc9);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0xef,0x40);
					  SP0718_write_cmos_sensor(0xf0,0x00);
					  SP0718_write_cmos_sensor(0x02,0x0f);
					  SP0718_write_cmos_sensor(0x03,0x01);
					  SP0718_write_cmos_sensor(0x06,0x3a);
					  SP0718_write_cmos_sensor(0x07,0x00);
					  SP0718_write_cmos_sensor(0x08,0x01);
					  SP0718_write_cmos_sensor(0x09,0x00);
					//Status                            
					  SP0718_write_cmos_sensor(0xfd,0x02);
					  SP0718_write_cmos_sensor(0xbe,0xc0);
					  SP0718_write_cmos_sensor(0xbf,0x03);
					  SP0718_write_cmos_sensor(0xd0,0xc0);
					  SP0718_write_cmos_sensor(0xd1,0x03);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x5b,0x03);
					  SP0718_write_cmos_sensor(0x5c,0xc0);
					  SP0718_write_cmos_sensor(0xfd,0x01);
					  SP0718_write_cmos_sensor(0x32,0x15);
					  
					  SENSORDB(" priview 60Hz normal\r\n");
				}
       }
	   
	}  
}

/*************************************************************************
* FUNCTION
*	SP0718_Sensor_Init
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
void SP0718_Sensor_Init(void)
{
  SP0718_write_cmos_sensor(0xfd,0x00);
  SP0718_write_cmos_sensor(0x1C,0x00);
  SP0718_write_cmos_sensor(0x31,0x00);
  SP0718_write_cmos_sensor(0x27,0xb3);//0xb3	//2x gain
  SP0718_write_cmos_sensor(0x1b,0x17);
  SP0718_write_cmos_sensor(0x26,0xaa);
  SP0718_write_cmos_sensor(0x37,0x02);
  SP0718_write_cmos_sensor(0x28,0x8f);
  SP0718_write_cmos_sensor(0x1a,0x73);
  SP0718_write_cmos_sensor(0x1e,0x1b);
  SP0718_write_cmos_sensor(0x21,0x06);  //blackout voltage
  SP0718_write_cmos_sensor(0x22,0x2a);  //colbias
  SP0718_write_cmos_sensor(0x0f,0x3f);
  SP0718_write_cmos_sensor(0x10,0x3e);
  SP0718_write_cmos_sensor(0x11,0x00);
  SP0718_write_cmos_sensor(0x12,0x01);
  SP0718_write_cmos_sensor(0x13,0x3f);
  SP0718_write_cmos_sensor(0x14,0x04);
  SP0718_write_cmos_sensor(0x15,0x30);
  SP0718_write_cmos_sensor(0x16,0x31);
  SP0718_write_cmos_sensor(0x17,0x01);
  SP0718_write_cmos_sensor(0x69,0x31);
  SP0718_write_cmos_sensor(0x6a,0x2a);
  SP0718_write_cmos_sensor(0x6b,0x33);
  SP0718_write_cmos_sensor(0x6c,0x1a);
  SP0718_write_cmos_sensor(0x6d,0x32);
  SP0718_write_cmos_sensor(0x6e,0x28);
  SP0718_write_cmos_sensor(0x6f,0x29);
  SP0718_write_cmos_sensor(0x70,0x34);
  SP0718_write_cmos_sensor(0x71,0x18);
SP0718_write_cmos_sensor(0x36,0x00);//02 delete badframe
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0x5d,0x51);//position
SP0718_write_cmos_sensor(0xf2,0x19);

//Blacklevel
SP0718_write_cmos_sensor(0x1f,0x10);
SP0718_write_cmos_sensor(0x20,0x1f);
//pregain 
SP0718_write_cmos_sensor(0xfd,0x02);
SP0718_write_cmos_sensor(0x00,0x88);
SP0718_write_cmos_sensor(0x01,0x88);
//SI15_SP0718 24M 50Hz 15-8fps 
//ae setting
SP0718_write_cmos_sensor(0xfd,0x00);
SP0718_write_cmos_sensor(0x03,0x01);
SP0718_write_cmos_sensor(0x04,0xce);
SP0718_write_cmos_sensor(0x06,0x00);
SP0718_write_cmos_sensor(0x09,0x02);
SP0718_write_cmos_sensor(0x0a,0xc4);
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0xef,0x4d);
SP0718_write_cmos_sensor(0xf0,0x00);
SP0718_write_cmos_sensor(0x02,0x0c);
SP0718_write_cmos_sensor(0x03,0x01);
SP0718_write_cmos_sensor(0x06,0x47);
SP0718_write_cmos_sensor(0x07,0x00);
SP0718_write_cmos_sensor(0x08,0x01);
SP0718_write_cmos_sensor(0x09,0x00);
//Status   
SP0718_write_cmos_sensor(0xfd,0x02);
SP0718_write_cmos_sensor(0xbe,0x9c);
SP0718_write_cmos_sensor(0xbf,0x03);
SP0718_write_cmos_sensor(0xd0,0x9c);
SP0718_write_cmos_sensor(0xd1,0x03);
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0x5b,0x03);
SP0718_write_cmos_sensor(0x5c,0x9c);

//rpc
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0xe0,0x40);////24//4c//48//4c//44//4c//3e//3c//3a//38//rpc_1base_max
SP0718_write_cmos_sensor(0xe1,0x30);////24//3c//38//3c//36//3c//30//2e//2c//2a//rpc_2base_max
SP0718_write_cmos_sensor(0xe2,0x2e);////24//34//30//34//2e//34//2a//28//26//26//rpc_3base_max
SP0718_write_cmos_sensor(0xe3,0x2a);////24//2a//2e//2c//2e//2a//2e//26//24//22//rpc_4base_max
SP0718_write_cmos_sensor(0xe4,0x2a);////24//2a//2e//2c//2e//2a//2e//26//24//22//rpc_5base_max
SP0718_write_cmos_sensor(0xe5,0x28);////24//2c//2a//2c//28//2c//24//22//20//rpc_6base_max
SP0718_write_cmos_sensor(0xe6,0x28);////24//2c//2a//2c//28//2c//24//22//20//rpc_7base_max
SP0718_write_cmos_sensor(0xe7,0x26);////24//2a//28//2a//26//2a//22//20//20//1e//rpc_8base_max
SP0718_write_cmos_sensor(0xe8,0x26);////24//2a//28//2a//26//2a//22//20//20//1e//rpc_9base_max
SP0718_write_cmos_sensor(0xe9,0x26);////24//2a//28//2a//26//2a//22//20//20//1e//rpc_10base_max
SP0718_write_cmos_sensor(0xea,0x26);////24//28//26//28//24//28//20//1f//1e//1d//rpc_11base_max
SP0718_write_cmos_sensor(0xf3,0x26);////24//28//26//28//24//28//20//1f//1e//1d//rpc_12base_max
SP0718_write_cmos_sensor(0xf4,0x26);////24//28//26//28//24//28//20//1f//1e//1d//rpc_13base_max
//ae gain &status
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0x04,0xe0);//rpc_max_indr
SP0718_write_cmos_sensor(0x05,0x26);//1e//rpc_min_indr 
SP0718_write_cmos_sensor(0x0a,0xa0);//rpc_max_outdr
SP0718_write_cmos_sensor(0x0b,0x26);//rpc_min_outdr
SP0718_write_cmos_sensor(0x5a,0x40);//dp rpc   
SP0718_write_cmos_sensor(0xfd,0x02); 
SP0718_write_cmos_sensor(0xbc,0xa0);//rpc_heq_low
SP0718_write_cmos_sensor(0xbd,0x80);//rpc_heq_dummy
SP0718_write_cmos_sensor(0xb8,0x80);//mean_normal_dummy
SP0718_write_cmos_sensor(0xb9,0x90);//mean_dummy_normal

//ae target
SP0718_write_cmos_sensor(0xfd,0x01); 
SP0718_write_cmos_sensor(0xeb,SP0718_P1_0xeb);//78 
SP0718_write_cmos_sensor(0xec,SP0718_P1_0xec);//78
SP0718_write_cmos_sensor(0xed,0x0a);	
SP0718_write_cmos_sensor(0xee,0x10);

//lsc       
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0x26,0x30);
SP0718_write_cmos_sensor(0x27,0x2c);
SP0718_write_cmos_sensor(0x28,0x07);
SP0718_write_cmos_sensor(0x29,0x08);
SP0718_write_cmos_sensor(0x2a,0x40);
SP0718_write_cmos_sensor(0x2b,0x03);
SP0718_write_cmos_sensor(0x2c,0x00);
SP0718_write_cmos_sensor(0x2d,0x00);
      
SP0718_write_cmos_sensor(0xa1,0x24);
SP0718_write_cmos_sensor(0xa2,0x27);
SP0718_write_cmos_sensor(0xa3,0x27);
SP0718_write_cmos_sensor(0xa4,0x2b);
SP0718_write_cmos_sensor(0xa5,0x1c);
SP0718_write_cmos_sensor(0xa6,0x1a);
SP0718_write_cmos_sensor(0xa7,0x1a);
SP0718_write_cmos_sensor(0xa8,0x1a);
SP0718_write_cmos_sensor(0xa9,0x18);
SP0718_write_cmos_sensor(0xaa,0x1c);
SP0718_write_cmos_sensor(0xab,0x17);
SP0718_write_cmos_sensor(0xac,0x17);
SP0718_write_cmos_sensor(0xad,0x08);
SP0718_write_cmos_sensor(0xae,0x08);
SP0718_write_cmos_sensor(0xaf,0x08);
SP0718_write_cmos_sensor(0xb0,0x00);
SP0718_write_cmos_sensor(0xb1,0x00);
SP0718_write_cmos_sensor(0xb2,0x00);
SP0718_write_cmos_sensor(0xb3,0x00);
SP0718_write_cmos_sensor(0xb4,0x00);
SP0718_write_cmos_sensor(0xb5,0x02);
SP0718_write_cmos_sensor(0xb6,0x06);
SP0718_write_cmos_sensor(0xb7,0x00);
SP0718_write_cmos_sensor(0xb8,0x00);
       
       
//DP       
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0x48,0x09);
SP0718_write_cmos_sensor(0x49,0x99);
        
//awb       
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0x32,0x05);
SP0718_write_cmos_sensor(0xfd,0x00);
SP0718_write_cmos_sensor(0xe7,0x03);
SP0718_write_cmos_sensor(0xfd,0x02);
SP0718_write_cmos_sensor(0x26,0xc8);
SP0718_write_cmos_sensor(0x27,0xb6);
SP0718_write_cmos_sensor(0xfd,0x00);
SP0718_write_cmos_sensor(0xe7,0x00);
SP0718_write_cmos_sensor(0xfd,0x02);
SP0718_write_cmos_sensor(0x1b,0x80);
SP0718_write_cmos_sensor(0x1a,0x80);
SP0718_write_cmos_sensor(0x18,0x26);
SP0718_write_cmos_sensor(0x19,0x28);
SP0718_write_cmos_sensor(0xfd,0x02);
SP0718_write_cmos_sensor(0x2a,0x00);
SP0718_write_cmos_sensor(0x2b,0x08);
SP0718_write_cmos_sensor(0x28,0xef);//0xa0//f8
SP0718_write_cmos_sensor(0x29,0x20);

//d65 90  e2 93
SP0718_write_cmos_sensor(0x66,0x42);//0x59//0x60////0x58//4e//0x48
SP0718_write_cmos_sensor(0x67,0x62);//0x74//0x70//0x78//6b//0x69
SP0718_write_cmos_sensor(0x68,0xee);//0xd6//0xe3//0xd5//cb//0xaa
SP0718_write_cmos_sensor(0x69,0x18);//0xf4//0xf3//0xf8//ed
SP0718_write_cmos_sensor(0x6a,0xa6);//0xa5
//indoor 91
SP0718_write_cmos_sensor(0x7c,0x3b);//0x45//30//41//0x2f//0x44
SP0718_write_cmos_sensor(0x7d,0x5b);//0x70//60//55//0x4b//0x6f
SP0718_write_cmos_sensor(0x7e,0x15);//0a//0xed
SP0718_write_cmos_sensor(0x7f,0x39);//23//0x28
SP0718_write_cmos_sensor(0x80,0xaa);//0xa6
//cwf   92 
SP0718_write_cmos_sensor(0x70,0x3e);//0x38//41//0x3b
SP0718_write_cmos_sensor(0x71,0x59);//0x5b//5f//0x55
SP0718_write_cmos_sensor(0x72,0x31);//0x30//22//0x28
SP0718_write_cmos_sensor(0x73,0x4f);//0x54//44//0x45
SP0718_write_cmos_sensor(0x74,0xaa);
//tl84  93 
SP0718_write_cmos_sensor(0x6b,0x1b);//0x18//11
SP0718_write_cmos_sensor(0x6c,0x3a);//0x3c//25//0x2f
SP0718_write_cmos_sensor(0x6d,0x3e);//0x3a//35
SP0718_write_cmos_sensor(0x6e,0x59);//0x5c//46//0x52
SP0718_write_cmos_sensor(0x6f,0xaa);
//f    94
SP0718_write_cmos_sensor(0x61,0xea);//0x03//0x00//f4//0xed
SP0718_write_cmos_sensor(0x62,0x03);//0x1a//0x25//0f//0f
SP0718_write_cmos_sensor(0x63,0x6a);//0x62//0x60//52//0x5d
SP0718_write_cmos_sensor(0x64,0x8a);//0x7d//0x85//70//0x75//0x8f
SP0718_write_cmos_sensor(0x65,0x6a);//0xaa//6a
      
SP0718_write_cmos_sensor(0x75,0x80);
SP0718_write_cmos_sensor(0x76,0x20);
SP0718_write_cmos_sensor(0x77,0x00);
SP0718_write_cmos_sensor(0x24,0x25);

//针对室内调偏不过灯箱测试使用//针对人脸调偏
SP0718_write_cmos_sensor(0x20,0xd8);
SP0718_write_cmos_sensor(0x21,0xa3);//82//a8偏暗照度还有调偏
SP0718_write_cmos_sensor(0x22,0xd0);//e3//bc
SP0718_write_cmos_sensor(0x23,0x86);

//outdoor r\b range
SP0718_write_cmos_sensor(0x78,0xc3);//d8
SP0718_write_cmos_sensor(0x79,0xba);//82
SP0718_write_cmos_sensor(0x7a,0xa6);//e3
SP0718_write_cmos_sensor(0x7b,0x99);//86


//skin 
SP0718_write_cmos_sensor(0x08,0x15);//
SP0718_write_cmos_sensor(0x09,0x04);//
SP0718_write_cmos_sensor(0x0a,0x20);//
SP0718_write_cmos_sensor(0x0b,0x12);//
SP0718_write_cmos_sensor(0x0c,0x27);//
SP0718_write_cmos_sensor(0x0d,0x06);//
SP0718_write_cmos_sensor(0x0e,0x63);//

//wt th
SP0718_write_cmos_sensor(0x3b,0x10);
//gw
SP0718_write_cmos_sensor(0x31,0x60);
SP0718_write_cmos_sensor(0x32,0x60);
SP0718_write_cmos_sensor(0x33,0xc0);
SP0718_write_cmos_sensor(0x35,0x6f);

// sharp
SP0718_write_cmos_sensor(0xfd,0x02);
SP0718_write_cmos_sensor(0xde,0x0f);
SP0718_write_cmos_sensor(0xd2,0x02);//6//控制黑白边；0-边粗，f-变细
SP0718_write_cmos_sensor(0xd3,0x06);
SP0718_write_cmos_sensor(0xd4,0x06);
SP0718_write_cmos_sensor(0xd5,0x06);
SP0718_write_cmos_sensor(0xd7,0x20);//10//2x根据增益判断轮廓阈值
SP0718_write_cmos_sensor(0xd8,0x30);//24//1A//4x
SP0718_write_cmos_sensor(0xd9,0x38);//28//8x
SP0718_write_cmos_sensor(0xda,0x38);//16x
SP0718_write_cmos_sensor(0xdb,0x08);//
SP0718_write_cmos_sensor(0xe8,0x58);//48//轮廓强度
SP0718_write_cmos_sensor(0xe9,0x48);
SP0718_write_cmos_sensor(0xea,0x30);
SP0718_write_cmos_sensor(0xeb,0x20);
SP0718_write_cmos_sensor(0xec,0x48);//60//80
SP0718_write_cmos_sensor(0xed,0x48);//50//60
SP0718_write_cmos_sensor(0xee,0x30);
SP0718_write_cmos_sensor(0xef,0x20);
//平坦区域锐化力度
SP0718_write_cmos_sensor(0xf3,0x50);
SP0718_write_cmos_sensor(0xf4,0x10);
SP0718_write_cmos_sensor(0xf5,0x10);
SP0718_write_cmos_sensor(0xf6,0x10);
//dns       
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0x64,0x44);//沿方向边缘平滑力度  //0-最强，8-最弱
SP0718_write_cmos_sensor(0x65,0x22);
SP0718_write_cmos_sensor(0x6d,0x04);//8//强平滑（平坦）区域平滑阈值
SP0718_write_cmos_sensor(0x6e,0x06);//8
SP0718_write_cmos_sensor(0x6f,0x10);
SP0718_write_cmos_sensor(0x70,0x10);
SP0718_write_cmos_sensor(0x71,0x08);//0d//弱平滑（非平坦）区域平滑阈值	
SP0718_write_cmos_sensor(0x72,0x12);//1b
SP0718_write_cmos_sensor(0x73,0x1c);//20
SP0718_write_cmos_sensor(0x74,0x24);
SP0718_write_cmos_sensor(0x75,0x44);//[7:4]平坦区域强度，[3:0]非平坦区域强度；0-最强，8-最弱；
SP0718_write_cmos_sensor(0x76,0x02);//46
SP0718_write_cmos_sensor(0x77,0x02);//33
SP0718_write_cmos_sensor(0x78,0x02);
SP0718_write_cmos_sensor(0x81,0x10);//18//2x//根据增益判定区域阈值，低于这个做强平滑、大于这个做弱平滑；
SP0718_write_cmos_sensor(0x82,0x20);//30//4x
SP0718_write_cmos_sensor(0x83,0x30);//40//8x
SP0718_write_cmos_sensor(0x84,0x48);//50//16x
SP0718_write_cmos_sensor(0x85,0x0c);//12/8+reg0x81 第二阈值，在平坦和非平坦区域做连接
SP0718_write_cmos_sensor(0xfd,0x02);
SP0718_write_cmos_sensor(0xdc,0x0f);
       
//gamma    
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0x8b,0x00);//00//00     
SP0718_write_cmos_sensor(0x8c,0x0a);//0c//09     
SP0718_write_cmos_sensor(0x8d,0x16);//19//17     
SP0718_write_cmos_sensor(0x8e,0x1f);//25//24     
SP0718_write_cmos_sensor(0x8f,0x2a);//30//33     
SP0718_write_cmos_sensor(0x90,0x3c);//44//47     
SP0718_write_cmos_sensor(0x91,0x4e);//54//58     
SP0718_write_cmos_sensor(0x92,0x5f);//61//64     
SP0718_write_cmos_sensor(0x93,0x6c);//6d//70     
SP0718_write_cmos_sensor(0x94,0x82);//80//81     
SP0718_write_cmos_sensor(0x95,0x94);//92//8f     
SP0718_write_cmos_sensor(0x96,0xa6);//a1//9b     
SP0718_write_cmos_sensor(0x97,0xb2);//ad//a5     
SP0718_write_cmos_sensor(0x98,0xbf);//ba//b0     
SP0718_write_cmos_sensor(0x99,0xc9);//c4//ba     
SP0718_write_cmos_sensor(0x9a,0xd1);//cf//c4     
SP0718_write_cmos_sensor(0x9b,0xd8);//d7//ce     
SP0718_write_cmos_sensor(0x9c,0xe0);//e0//d7     
SP0718_write_cmos_sensor(0x9d,0xe8);//e8//e1     
SP0718_write_cmos_sensor(0x9e,0xef);//ef//ea     
SP0718_write_cmos_sensor(0x9f,0xf8);//f7//f5     
SP0718_write_cmos_sensor(0xa0,0xff);//ff//ff     
//CCM      
SP0718_write_cmos_sensor(0xfd,0x02);
SP0718_write_cmos_sensor(0x15,0xd0);//b>th
SP0718_write_cmos_sensor(0x16,0x95);//r<th
//gc镜头照人脸偏黄
//!F        
SP0718_write_cmos_sensor(0xa0,0x80);//80
SP0718_write_cmos_sensor(0xa1,0x00);//00
SP0718_write_cmos_sensor(0xa2,0x00);//00
SP0718_write_cmos_sensor(0xa3,0x00);//06
SP0718_write_cmos_sensor(0xa4,0x8c);//8c
SP0718_write_cmos_sensor(0xa5,0xf4);//ed
SP0718_write_cmos_sensor(0xa6,0x0c);//0c
SP0718_write_cmos_sensor(0xa7,0xf4);//f4
SP0718_write_cmos_sensor(0xa8,0x80);//80
SP0718_write_cmos_sensor(0xa9,0x00);//00
SP0718_write_cmos_sensor(0xaa,0x30);//30
SP0718_write_cmos_sensor(0xab,0x0c);//0c 
//F        
SP0718_write_cmos_sensor(0xac,0x8c);
SP0718_write_cmos_sensor(0xad,0xf4);
SP0718_write_cmos_sensor(0xae,0x00);
SP0718_write_cmos_sensor(0xaf,0xed);
SP0718_write_cmos_sensor(0xb0,0x8c);
SP0718_write_cmos_sensor(0xb1,0x06);
SP0718_write_cmos_sensor(0xb2,0xf4);
SP0718_write_cmos_sensor(0xb3,0xf4);
SP0718_write_cmos_sensor(0xb4,0x99);
SP0718_write_cmos_sensor(0xb5,0x0c);
SP0718_write_cmos_sensor(0xb6,0x03);
SP0718_write_cmos_sensor(0xb7,0x0f);
        
//sat u     
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0xd3,0x9c);//0x88//50
SP0718_write_cmos_sensor(0xd4,0x98);//0x88//50
SP0718_write_cmos_sensor(0xd5,0x8c);//50
SP0718_write_cmos_sensor(0xd6,0x84);//50
//sat v   
SP0718_write_cmos_sensor(0xd7,0x9c);//0x88//50
SP0718_write_cmos_sensor(0xd8,0x98);//0x88//50
SP0718_write_cmos_sensor(0xd9,0x8c);//50
SP0718_write_cmos_sensor(0xda,0x84);//50
//auto_sat  
SP0718_write_cmos_sensor(0xdd,0x30);
SP0718_write_cmos_sensor(0xde,0x10);
SP0718_write_cmos_sensor(0xd2,0x01);//autosa_en
SP0718_write_cmos_sensor(0xdf,0xff);//a0//y_mean_th
        
//uv_th     
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0xc2,0xaa);
SP0718_write_cmos_sensor(0xc3,0xaa);
SP0718_write_cmos_sensor(0xc4,0x66);
SP0718_write_cmos_sensor(0xc5,0x66); 

//heq
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0x0f,0xff);
SP0718_write_cmos_sensor(0x10,SP0718_P1_0x10); //out
SP0718_write_cmos_sensor(0x14,SP0718_P1_0x14); 
SP0718_write_cmos_sensor(0x11,SP0718_P1_0x11); //nr
SP0718_write_cmos_sensor(0x15,SP0718_P1_0x15);  
SP0718_write_cmos_sensor(0x12,SP0718_P1_0x12); //dummy
SP0718_write_cmos_sensor(0x16,SP0718_P1_0x16); 
SP0718_write_cmos_sensor(0x13,SP0718_P1_0x13); //low 	
SP0718_write_cmos_sensor(0x17,SP0718_P1_0x17);   	

SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0xcd,0x20);
SP0718_write_cmos_sensor(0xce,0x1f);
SP0718_write_cmos_sensor(0xcf,0x20);
SP0718_write_cmos_sensor(0xd0,0x55);  
//auto 
SP0718_write_cmos_sensor(0xfd,0x01);
SP0718_write_cmos_sensor(0xfb,0x33);
//SP0718_write_cmos_sensor(0x32,0x15);
  SP0718_write_cmos_sensor(0x33,0xff);
  SP0718_write_cmos_sensor(0x34,0xe7);
  

  
}



UINT32 SP0718_GetSensorID(UINT32 *sensorID)
{
    kal_uint16 sensor_id=0;
    int retry=3;

     SENSORDB("SP0718_GetSensorID \n");	

       // check if sensor ID correct
	do {
		
		SP0718_write_cmos_sensor(0xfd,0x00);
	    sensor_id=SP0718_read_cmos_sensor(0x02);
    	    if (sensor_id == SP0718_SENSOR_ID) {
                 break; 
    	    }
         SENSORDB("Read Sensor ID Fail = 0x%x\n", sensor_id); 
    	    
    	    retry--; 
	}while (retry > 0); 
	
	if (sensor_id != SP0718_SENSOR_ID) {
	    return ERROR_SENSOR_CONNECT_FAIL;
	}

      *sensorID = sensor_id;
       RETAILMSG(1, (TEXT("Sensor Read ID OK \r\n")));
	
    return ERROR_NONE;
}


/*************************************************************************
 * FUNCTION
 *	SP0718_Open
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
UINT32 SP0718_Open(void)
{
    kal_uint16 sensor_id=0;
    int retry = 3; 

     SENSORDB("SP0718_Open \n");

     do {
		
		SP0718_write_cmos_sensor(0xfd,0x00);
	    sensor_id=SP0718_read_cmos_sensor(0x02);
    	    if (sensor_id == SP0718_SENSOR_ID) {
                 break; 
    	    }
         SENSORDB("Read Sensor ID Fail = 0x%x\n", sensor_id); 
    	    
    	    retry--; 
	}while (retry > 0); 
	
	if (sensor_id != SP0718_SENSOR_ID) 
	   {
	      return ERROR_SENSOR_CONNECT_FAIL;
	   }
         SENSORDB("SP0718_ Sensor id read OK, ID = %x\n", sensor_id);

     Sleep(10);
    SP0718_Sensor_Init();		
    SENSORDB("SP0718_Open end \n");
    return ERROR_NONE;
} /* SP0718_Open */


/*************************************************************************
 * FUNCTION
 *	SP0718_Close
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
UINT32 SP0718_Close(void)
{
     SENSORDB("SP0718_Close\n");
    return ERROR_NONE;
} /* SP0718_Close */


/*************************************************************************
 * FUNCTION
 * SP0718_Preview
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
UINT32 SP0718_Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    kal_uint32 iTemp;
    kal_uint16 iStartX = 0, iStartY = 1;

    if(sensor_config_data->SensorOperationMode == MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
        RETAILMSG(1, (TEXT("Camera Video preview\r\n")));
        SP0718_MPEG4_encode_mode = KAL_TRUE;
       
    }
    else
    {
        RETAILMSG(1, (TEXT("Camera preview\r\n")));
        SP0718_MPEG4_encode_mode = KAL_FALSE;
    }

    image_window->GrabStartX= IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY= IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth = IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight =IMAGE_SENSOR_PV_HEIGHT;

    // copy sensor_config_data
    memcpy(&SP0718_SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* SP0718_Preview */


/*************************************************************************
 * FUNCTION
 *	SP0718_Capture
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
UINT32 SP0718_Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    SP0718_MODE_CAPTURE=KAL_TRUE;

    image_window->GrabStartX = IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY = IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth= IMAGE_SENSOR_FULL_WIDTH;
    image_window->ExposureWindowHeight = IMAGE_SENSOR_FULL_HEIGHT;

    // copy sensor_config_data
    memcpy(&SP0718_SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* SP0718_Capture() */



UINT32 SP0718_GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT;
    pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT;
	pSensorResolution->SensorVideoWidth=IMAGE_SENSOR_FULL_WIDTH;
	pSensorResolution->SensorVideoHeight=IMAGE_SENSOR_FULL_HEIGHT;

    return ERROR_NONE;
} /* SP0718_GetResolution() */


UINT32 SP0718_GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
        MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    pSensorInfo->SensorPreviewResolutionX=IMAGE_SENSOR_PV_WIDTH;
    pSensorInfo->SensorPreviewResolutionY=IMAGE_SENSOR_PV_HEIGHT;
    pSensorInfo->SensorFullResolutionX=IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY=IMAGE_SENSOR_FULL_WIDTH;

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
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;
    pSensorInfo->SensorInterruptDelayLines = 1;
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;
#if 0
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_5M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_5M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_1M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=FALSE;
#endif
    pSensorInfo->CaptureDelayFrame = 1;
    pSensorInfo->PreviewDelayFrame = 0;
    pSensorInfo->VideoDelayFrame = 4;
    pSensorInfo->SensorMasterClockSwitch = 0;
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
 //   case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
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
  //  case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
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
    SP0718_PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &SP0718_SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* SP0718_GetInfo() */


UINT32 SP0718_Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
//    case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
        SP0718_Preview(pImageWindow, pSensorConfigData);
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
//    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
        SP0718_Capture(pImageWindow, pSensorConfigData);
        break;
    }


    return TRUE;
}	/* SP0718_Control() */

BOOL SP0718_set_param_wb(UINT16 para)
{

	switch (para)
	{
		case AWB_MODE_OFF:
			// SP0718_write_cmos_sensor(0xfd,0x00);				   
			// SP0718_write_cmos_sensor(0x32,0x05);	  
		break;
		
		case AWB_MODE_AUTO:
			
			 SP0718_write_cmos_sensor(0xfd,0x02);	
			 SP0718_write_cmos_sensor(0x26,0xc8);	
			 SP0718_write_cmos_sensor(0x27,0xb6);	
			 SP0718_write_cmos_sensor(0xfd,0x01);	
			 SP0718_write_cmos_sensor(0x32,0x15);	
			 SP0718_write_cmos_sensor(0xfd,0x00);	
		break;
		
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			 SP0718_write_cmos_sensor(0xfd,0x01);	
			 SP0718_write_cmos_sensor(0x32,0x05);	
			 SP0718_write_cmos_sensor(0xfd,0x02);	
			 SP0718_write_cmos_sensor(0x26,0xdc);	
			 SP0718_write_cmos_sensor(0x27,0x75);	
			 SP0718_write_cmos_sensor(0xfd,0x00);	
		break;
		
		case AWB_MODE_DAYLIGHT: //sunny
			 SP0718_write_cmos_sensor(0xfd,0x01);	
			 SP0718_write_cmos_sensor(0x32,0x05);	
			 SP0718_write_cmos_sensor(0xfd,0x02);	
			 SP0718_write_cmos_sensor(0x26,0xc8);	
			 SP0718_write_cmos_sensor(0x27,0x89);	
			 SP0718_write_cmos_sensor(0xfd,0x00);	
		break;
		
		case AWB_MODE_INCANDESCENT: //office
			 SP0718_write_cmos_sensor(0xfd,0x01);	
			 SP0718_write_cmos_sensor(0x32,0x05);	
			 SP0718_write_cmos_sensor(0xfd,0x02);	
			 SP0718_write_cmos_sensor(0x26,0xaa);	
			 SP0718_write_cmos_sensor(0x27,0xce);	
			 SP0718_write_cmos_sensor(0xfd,0x00);	
		break;
		
		case AWB_MODE_TUNGSTEN: //home
			 SP0718_write_cmos_sensor(0xfd,0x01);	
			 SP0718_write_cmos_sensor(0x32,0x05);	
			 SP0718_write_cmos_sensor(0xfd,0x02);	
			 SP0718_write_cmos_sensor(0x26,0x75);	
			 SP0718_write_cmos_sensor(0x27,0xe2);	
			 SP0718_write_cmos_sensor(0xfd,0x00);	
		break;
		
		case AWB_MODE_FLUORESCENT:
			 SP0718_write_cmos_sensor(0xfd,0x01);	
			 SP0718_write_cmos_sensor(0x32,0x05);	
			 SP0718_write_cmos_sensor(0xfd,0x02);	
			 SP0718_write_cmos_sensor(0x26,0x91);	
			 SP0718_write_cmos_sensor(0x27,0xc8);	
			 SP0718_write_cmos_sensor(0xfd,0x00);	
		break;
		
		default:
		return FALSE;
	}

	return TRUE;
} /* SP0718_set_param_wb */


BOOL SP0718_set_param_effect(UINT16 para)
{
	kal_uint32  ret = KAL_TRUE;

	switch (para)
	{
		case MEFFECT_OFF:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0x66, 0x00);
			SP0718_write_cmos_sensor(0x67, 0x80);
			SP0718_write_cmos_sensor(0x68, 0x80);
			SP0718_write_cmos_sensor(0xfd, 0x00);
		break;
		
		case MEFFECT_SEPIA:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0x66, 0x10);
			SP0718_write_cmos_sensor(0x67, 0xc0);
			SP0718_write_cmos_sensor(0x68, 0x20);
			SP0718_write_cmos_sensor(0xfd, 0x00);
		break;
		
		case MEFFECT_NEGATIVE:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0x66, 0x08);
			SP0718_write_cmos_sensor(0x67, 0x80);
			SP0718_write_cmos_sensor(0x68, 0x80);
			SP0718_write_cmos_sensor(0xfd, 0x00);
		break;
		
		case MEFFECT_SEPIAGREEN:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0x66, 0x10);
			SP0718_write_cmos_sensor(0x67, 0x20);
			SP0718_write_cmos_sensor(0x68, 0x20);
			SP0718_write_cmos_sensor(0xfd, 0x00);
		break;
		
		case MEFFECT_SEPIABLUE:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0x66, 0x10);
			SP0718_write_cmos_sensor(0x67, 0x20);
			SP0718_write_cmos_sensor(0x68, 0xf0);
			SP0718_write_cmos_sensor(0xfd, 0x00);
		break;

		case MEFFECT_MONO:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0x66, 0x20);
			SP0718_write_cmos_sensor(0x67, 0x80);
			SP0718_write_cmos_sensor(0x68, 0x80);
			SP0718_write_cmos_sensor(0xfd, 0x00);
		break;
		default:
			ret = FALSE;
	}

	return ret;

} /* SP0718_set_param_effect */


BOOL SP0718_set_param_banding(UINT16 para)
{
	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:
			sp0718_isBanding = 0;
			break;

		case AE_FLICKER_MODE_60HZ:
			sp0718_isBanding = 1;
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* SP0718_set_param_banding */


BOOL SP0718_set_param_exposure(UINT16 para)
{
	switch (para)
	{
		case AE_EV_COMP_n13:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0xdb, 0xc0);
		break;
		
		case AE_EV_COMP_n10:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0xdb, 0xd0);
		break;
		
		case AE_EV_COMP_n07:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0xdb, 0xe0);
		break;
		
		case AE_EV_COMP_n03:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0xdb, 0xf0);
		break;				
		
		case AE_EV_COMP_00:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0xdb, 0x00);
		break;

		case AE_EV_COMP_03:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0xdb, 0x10);
		break;
		
		case AE_EV_COMP_07:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0xdb, 0x20);
		break;
		
		case AE_EV_COMP_10:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0xdb, 0x30);
		break;
		
		case AE_EV_COMP_13:
			SP0718_write_cmos_sensor(0xfd, 0x01);
			SP0718_write_cmos_sensor(0xdb, 0x40);
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* SP0718_set_param_exposure */


UINT32 SP0718_YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
    switch (iCmd) {

	  case FID_SCENE_MODE:	    
	    if (iPara == SCENE_MODE_OFF)
	    {
			SP0718_NightMode(0); 
	    }
	    else if (iPara == SCENE_MODE_NIGHTSCENE)
	    {
			SP0718_NightMode(1); 
	    }	    
	    break;   	    
    case FID_AWB_MODE:
        SP0718_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        SP0718_set_param_effect(iPara);
        break;
    case FID_AE_EV:
        SP0718_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
        SP0718_set_param_banding(iPara);
		    break;
	  //case FID_SCENE_MODE:
    //    SP0718_set_param_banding(iPara);
		//SP0718_NightMode(SP0718_NIGHT_MODE);
        break;
    default:
        break;
    }
    return TRUE;
} /* SP0718_YUVSensorSetting */


UINT32 SP0718_FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 SP0718_SensorRegNumber;
    UINT32 i;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

    RETAILMSG(1, (_T("gaiyang SP0718_FeatureControl FeatureId=%d\r\n"), FeatureId));

    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=IMAGE_SENSOR_FULL_WIDTH;
        *pFeatureReturnPara16=IMAGE_SENSOR_FULL_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        *pFeatureReturnPara16++=(VGA_PERIOD_PIXEL_NUMS)+SP0718_dummy_pixels;
        *pFeatureReturnPara16=(VGA_PERIOD_LINE_NUMS)+SP0718_dummy_lines;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        *pFeatureReturnPara32 = SP0718_g_fPV_PCLK;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        SP0718_NightMode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        SP0718_isp_master_clock=*pFeatureData32;
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        SP0718_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        pSensorRegData->RegData = SP0718_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        memcpy(pSensorConfigData, &SP0718_SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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
        SP0718_YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;
    case SENSOR_FEATURE_CHECK_SENSOR_ID:


	SP0718_GetSensorID(pFeatureData32);
	break;
    default:
        break;
	}
return ERROR_NONE;
}	/* SP0718_FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncSP0718_YUV=
{
	SP0718_Open,
	SP0718_GetInfo,
	SP0718_GetResolution,
	SP0718_FeatureControl,
	SP0718_Control,
	SP0718_Close
};


UINT32 SP0718_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncSP0718_YUV;
	return ERROR_NONE;
} /* SensorInit() */
