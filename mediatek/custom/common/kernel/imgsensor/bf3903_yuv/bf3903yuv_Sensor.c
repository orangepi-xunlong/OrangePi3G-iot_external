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
 *   BF3903yuv_Sensor.c
 *
 * Project:
 * --------
 *   MAUI
 *
 * Description:
 * ------------
 *   Image sensor driver function
 *   V1.0.0
 *
 * Author:
 * -------
 *   Mormo
 *
 *=============================================================
 *             HISTORY
 * Below this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Log$
 * 2011/10/25 Firsty Released By Mormo(using "BF3903.set Revision1721" )
 *   
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by PVCS VM. DO NOT MODIFY!!
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

#include "bf3903yuv_Sensor.h"
#include "bf3903yuv_Camera_Sensor_para.h"
#include "bf3903yuv_CameraCustomized.h"



#define BF3903YUV_DEBUG
#ifdef BF3903YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

#define BF3903_WRITE_ID 0xDC

//extern int iReadReg(u8 addr, u8 *buf, u8 i2cId);
//extern int iWriteReg(u8 addr, u8 buf, u32 size, u16 i2cId);

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);



static void BF3903_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    char puSendCmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd, 2,BF3903_WRITE_ID);
}

 kal_uint8 BF3903_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint8 get_byte = 0;
	
    char pSendCmd = (char)(addr & 0xFF);
    
   iReadRegI2C(&pSendCmd, 1, (u8*)&get_byte, 1, BF3903_WRITE_ID);

    return get_byte;
}


/*******************************************************************************
 * // Adapter for Winmo typedef
 ********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT

kal_bool   BF3903_MPEG4_encode_mode = KAL_FALSE;
kal_uint16 BF3903_dummy_pixels = 0, BF3903_dummy_lines = 0;
kal_bool   BF3903_MODE_CAPTURE = KAL_FALSE;
kal_bool   BF3903_CAM_BANDING_50HZ = KAL_FALSE;

kal_uint32 BF3903_isp_master_clock;
static kal_uint32 BF3903_g_fPV_PCLK = 24;

//kal_uint8 BF3903_sensor_write_I2C_address = BF3903_WRITE_ID;
//kal_uint8 BF3903_sensor_read_I2C_address = BF3903_READ_ID;

UINT8 BF3903PixelClockDivider=0;

MSDK_SENSOR_CONFIG_STRUCT BF3903SensorConfigData;


/*************************************************************************
 * FUNCTION
 *	BF3903_SetShutter
 *
 * DESCRIPTION
 *	This function set e-shutter of BF3903 to change exposure time.
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
void BF3903_Set_Shutter(kal_uint16 iShutter)
{
} /* Set_BF3903_Shutter */


/*************************************************************************
 * FUNCTION
 *	BF3903_read_Shutter
 *
 * DESCRIPTION
 *	This function read e-shutter of BF3903 .
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
kal_uint16 BF3903_Read_Shutter(void)
{
    	kal_uint8 temp_reg1, temp_reg2;
	kal_uint16 shutter;

	temp_reg1 = BF3903_read_cmos_sensor(0x8c);
	temp_reg2 = BF3903_read_cmos_sensor(0x8d);

	shutter = (temp_reg1 & 0xFF) | (temp_reg2 << 8);

	return shutter;
} /* BF3903_read_shutter */


/*************************************************************************
 * FUNCTION
 *	BF3903_write_reg
 *
 * DESCRIPTION
 *	This function set the register of BF3903.
 *
 * PARAMETERS
 *	addr : the register index of BF3903
 *  para : setting parameter of the specified register of BF3903
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void BF3903_write_reg(kal_uint32 addr, kal_uint32 para)
{
	BF3903_write_cmos_sensor(addr, para);
} /* BF3903_write_reg() */


/*************************************************************************
 * FUNCTION
 *	BF3903_read_cmos_sensor
 *
 * DESCRIPTION
 *	This function read parameter of specified register from BF3903.
 *
 * PARAMETERS
 *	addr : the register index of BF3903
 *
 * RETURNS
 *	the data that read from BF3903
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint32 BF3903_read_reg(kal_uint32 addr)
{
	return BF3903_read_cmos_sensor(addr);
} /* OV7670_read_reg() */


/*************************************************************************
* FUNCTION
*	BF3903_awb_enable
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
static void BF3903_awb_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AWB_reg = 0;

	temp_AWB_reg = BF3903_read_cmos_sensor(0x13);
	

	if (enalbe)
	{
		BF3903_write_cmos_sensor(0x13, (temp_AWB_reg |0x02));
	}
	else
	{
		BF3903_write_cmos_sensor(0x13, (temp_AWB_reg & (~0x02)));
	}

}


/*************************************************************************
 * FUNCTION
 *	BF3903_config_window
 *
 * DESCRIPTION
 *	This function config the hardware window of BF3903 for getting specified
 *  data of that window.
 *
 * PARAMETERS
 *	start_x : start column of the interested window
 *  start_y : start row of the interested window
 *  width  : column widht of the itnerested window
 *  height : row depth of the itnerested window
 *
 * RETURNS
 *	the data that read from BF3903
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void BF3903_config_window(kal_uint16 startx, kal_uint16 starty, kal_uint16 width, kal_uint16 height)
{
} /* BF3903_config_window */


/*************************************************************************
 * FUNCTION
 *	BF3903_SetGain
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
kal_uint16 BF3903_SetGain(kal_uint16 iGain)
{
	return iGain;
}


/*************************************************************************
 * FUNCTION
 *	BF3903_NightMode
 *
 * DESCRIPTION
 *	This function night mode of BF3903.
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
void BF3903_night_mode(kal_bool bEnable)
{
        printk("phl BF3903_night_mode bEnable = %d\n",bEnable);
	if (bEnable)
	{
           	if(BF3903_MPEG4_encode_mode == KAL_TRUE) 
    		{
        		 BF3903_write_cmos_sensor(0x89, 0xa5);				
           	 }
        	else 
        	{        
        		 BF3903_write_cmos_sensor(0x89, 0xa5); // 79 分频	F2 不分频		    		
        	}
	}
	else 
	{
        	if (BF3903_MPEG4_encode_mode == KAL_TRUE) 
		{
        		 BF3903_write_cmos_sensor(0x89, 0x7d);
			
        	}
		else
		{
			
        		 BF3903_write_cmos_sensor(0x89, 0x7d);

    		}
	}
} /* BF3903_NightMode */




UINT32 BF3903GetSensorID(UINT32 *sensorID)
{
    kal_uint16 sensor_id=0;
    int i;



    do
    {
      
        for(i = 0; i < 3; i++)
		{
	   
	      sensor_id = ((BF3903_read_cmos_sensor(0xfc) << 8) | BF3903_read_cmos_sensor(0xfd));
	      printk("JHY BF3903 Sensor id = %x\n", sensor_id);
	      if (sensor_id == BF3903_SENSOR_ID)
			{
	         break;
	        }
        }
        	mdelay(50);
    }while(0);

    if(sensor_id != BF3903_SENSOR_ID)
    {
         *sensorID = 0xFFFFFFFF;
        SENSORDB("BF3903 Sensor id read failed, ID = %x\n", sensor_id);
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    *sensorID = sensor_id;

    RETAILMSG(1, (TEXT("Sensor Read ID OK \r\n")));
	
    return ERROR_NONE;
}


void BF3903_Sensor_init(void)
{
	BF3903_write_cmos_sensor(0x11, 0x80);
	BF3903_write_cmos_sensor(0x09, 0x01);
	BF3903_write_cmos_sensor(0x15, 0x00);
	BF3903_write_cmos_sensor(0x3a, 0x00);

	//initial awb and ae setting	
	BF3903_write_cmos_sensor(0x13, 0x00);
	BF3903_write_cmos_sensor(0x01, 0x11);
	BF3903_write_cmos_sensor(0x02, 0x22);
	BF3903_write_cmos_sensor(0x8c, 0x02);
	BF3903_write_cmos_sensor(0x8d, 0x64);
	BF3903_write_cmos_sensor(0x9d, 0x2f);   //manual global gain
	BF3903_write_cmos_sensor(0x13, 0x07); 

	// analog setting
	BF3903_write_cmos_sensor(0x20, 0x89);//BYD FAE 20111118
	BF3903_write_cmos_sensor(0x2f, 0xc5);//BYD FAE 20111118
	BF3903_write_cmos_sensor(0x06, 0x68);
	BF3903_write_cmos_sensor(0x08, 0x10);

	//lens shading   
	BF3903_write_cmos_sensor(0x35, 0x46);
	BF3903_write_cmos_sensor(0x65, 0x46);
	BF3903_write_cmos_sensor(0x66, 0x46);
	BF3903_write_cmos_sensor(0xbe, 0x44);
	BF3903_write_cmos_sensor(0xbd, 0xf4);
	BF3903_write_cmos_sensor(0xbc, 0x0d);
	BF3903_write_cmos_sensor(0x9c, 0x44);
	BF3903_write_cmos_sensor(0x9b, 0xf4);
	BF3903_write_cmos_sensor(0x36, 0x45);
	BF3903_write_cmos_sensor(0x37, 0xf4);
	BF3903_write_cmos_sensor(0x38, 0x44);
	BF3903_write_cmos_sensor(0x6e, 0x10);

	//denoise
	BF3903_write_cmos_sensor(0x72, 0x2f);// 0x72[7:4] denoise:the bigger,the smaller noise 
	BF3903_write_cmos_sensor(0x73, 0x2f);// 0x73[7:4] denoise:the bigger,the smaller noise 
	BF3903_write_cmos_sensor(0x74, 0x27);// 0x74[3:0] denoise:the smaller,the smaller noise 

	//edge enhancement
	BF3903_write_cmos_sensor(0x75, 0x24);//0x75[6:4]:bright edge enhancement.0x75[2:0]:dark edge enhancement.

	//denoise in low light
	BF3903_write_cmos_sensor(0x79, 0x2d);
	BF3903_write_cmos_sensor(0x7a, 0x2d);//和0x86中对应的数字gain 一样
	BF3903_write_cmos_sensor(0x7e, 0x1a);

	//denoise for outdoor
	BF3903_write_cmos_sensor(0x7c, 0x88);
	BF3903_write_cmos_sensor(0x7d, 0xba);

	//color fringe correction
	BF3903_write_cmos_sensor(0x5b, 0xc2);
	BF3903_write_cmos_sensor(0x76, 0x90);
	BF3903_write_cmos_sensor(0x7b, 0x55);

	//ae
	BF3903_write_cmos_sensor(0x25, 0x88); 
	BF3903_write_cmos_sensor(0x80, 0x86); 
	BF3903_write_cmos_sensor(0x81, 0xa0);//ae speed
	BF3903_write_cmos_sensor(0x82, 0x2d);//minimum global gain
	BF3903_write_cmos_sensor(0x83, 0x5a);//BYD FAE 20111118	  
	BF3903_write_cmos_sensor(0x84, 0x30);//BYD FAE 20111118
	BF3903_write_cmos_sensor(0x85, 0x40);//BYD FAE 20111118
	BF3903_write_cmos_sensor(0x86, 0x50);//maximum global gain //BYD FAE 20111118
	BF3903_write_cmos_sensor(0x89, 0xa3); 
	BF3903_write_cmos_sensor(0x8a, 0x99); //50HZ
	BF3903_write_cmos_sensor(0x8b, 0x7f); //60HZ
	BF3903_write_cmos_sensor(0x8e, 0x2c);		  
	BF3903_write_cmos_sensor(0x8f, 0x82);
	BF3903_write_cmos_sensor(0x97, 0x78);
	BF3903_write_cmos_sensor(0x98, 0x12);//ae window
	BF3903_write_cmos_sensor(0x24, 0xb0);//ae target

	//used to modify Y_aver
	BF3903_write_cmos_sensor(0x95, 0x80);//the smaller value, the bigger brightness in gray region. 推荐0x80以上。
	BF3903_write_cmos_sensor(0x9a, 0xa0);

	//outdoor detection
	BF3903_write_cmos_sensor(0x9e, 0x50);
	BF3903_write_cmos_sensor(0x9f, 0x50);
	BF3903_write_cmos_sensor(0xd2, 0x78);

	//contrast
	BF3903_write_cmos_sensor(0x56, 0x40);

	//auto offset in high light scence
	BF3903_write_cmos_sensor(0x90, 0x20);
	BF3903_write_cmos_sensor(0x91, 0x1c); 

	//auto offset in low light scence
	BF3903_write_cmos_sensor(0x3b, 0x60); 
	BF3903_write_cmos_sensor(0x3c, 0x10); //the bigger, the brighter in dark scence.

	//gamma offset
	BF3903_write_cmos_sensor(0x39, 0xa0); //偶列offset,与0x3f的值要写一样
	BF3903_write_cmos_sensor(0x3f, 0xa0);// 奇列offset，与0x39的值要写成一样。

	//gamma1 过曝过渡好
	BF3903_write_cmos_sensor(0x40, 0x80); 
	BF3903_write_cmos_sensor(0x41, 0x80); 
	BF3903_write_cmos_sensor(0x42, 0x68); 
	BF3903_write_cmos_sensor(0x43, 0x53); 
	BF3903_write_cmos_sensor(0x44, 0x48); 
	BF3903_write_cmos_sensor(0x45, 0x41); 
	BF3903_write_cmos_sensor(0x46, 0x3b); 
	BF3903_write_cmos_sensor(0x47, 0x36); 
	BF3903_write_cmos_sensor(0x48, 0x32); 
	BF3903_write_cmos_sensor(0x49, 0x2f); 
	BF3903_write_cmos_sensor(0x4b, 0x2c); 
	BF3903_write_cmos_sensor(0x4c, 0x2a); 
	BF3903_write_cmos_sensor(0x4e, 0x25); 
	BF3903_write_cmos_sensor(0x4f, 0x22); 
	BF3903_write_cmos_sensor(0x50, 0x20); 

	/*//gamma2 噪声稍小
	BF3903_write_cmos_sensor(0x40, 0x68); 
	BF3903_write_cmos_sensor(0x41, 0x60); 
	BF3903_write_cmos_sensor(0x42, 0x54); 
	BF3903_write_cmos_sensor(0x43, 0x46); 
	BF3903_write_cmos_sensor(0x44, 0x3e); 
	BF3903_write_cmos_sensor(0x45, 0x39); 
	BF3903_write_cmos_sensor(0x46, 0x35); 
	BF3903_write_cmos_sensor(0x47, 0x31); 
	BF3903_write_cmos_sensor(0x48, 0x2e); 
	BF3903_write_cmos_sensor(0x49, 0x2b); 
	BF3903_write_cmos_sensor(0x4b, 0x29); 
	BF3903_write_cmos_sensor(0x4c, 0x27); 
	BF3903_write_cmos_sensor(0x4e, 0x24); 
	BF3903_write_cmos_sensor(0x4f, 0x22); 
	BF3903_write_cmos_sensor(0x50, 0x20); 

	//gamma3 噪声小
	BF3903_write_cmos_sensor(0x40, 0x64); 
	BF3903_write_cmos_sensor(0x41, 0x5c); 
	BF3903_write_cmos_sensor(0x42, 0x50); 
	BF3903_write_cmos_sensor(0x43, 0x42); 
	BF3903_write_cmos_sensor(0x44, 0x3a); 
	BF3903_write_cmos_sensor(0x45, 0x35); 
	BF3903_write_cmos_sensor(0x46, 0x31); 
	BF3903_write_cmos_sensor(0x47, 0x2d); 
	BF3903_write_cmos_sensor(0x48, 0x2a); 
	BF3903_write_cmos_sensor(0x49, 0x28); 
	BF3903_write_cmos_sensor(0x4b, 0x26); 
	BF3903_write_cmos_sensor(0x4c, 0x25); 
	BF3903_write_cmos_sensor(0x4e, 0x23); 
	BF3903_write_cmos_sensor(0x4f, 0x21); 
	BF3903_write_cmos_sensor(0x50, 0x20); 

	//gamma4 清晰亮丽
	BF3903_write_cmos_sensor(0x40, 0x78); 
	BF3903_write_cmos_sensor(0x41, 0x70); 
	BF3903_write_cmos_sensor(0x42, 0x68); 
	BF3903_write_cmos_sensor(0x43, 0x55); 
	BF3903_write_cmos_sensor(0x44, 0x4A); 
	BF3903_write_cmos_sensor(0x45, 0x43); 
	BF3903_write_cmos_sensor(0x46, 0x3C); 
	BF3903_write_cmos_sensor(0x47, 0x37); 
	BF3903_write_cmos_sensor(0x48, 0x33); 
	BF3903_write_cmos_sensor(0x49, 0x2F); 
	BF3903_write_cmos_sensor(0x4b, 0x2C); 
	BF3903_write_cmos_sensor(0x4c, 0x29); 
	BF3903_write_cmos_sensor(0x4e, 0x25); 
	BF3903_write_cmos_sensor(0x4f, 0x22); 
	BF3903_write_cmos_sensor(0x50, 0x20); */

	//color coefficient for outdoor scene 	   
	BF3903_write_cmos_sensor(0x5a, 0x16);
	BF3903_write_cmos_sensor(0x51, 0x08);
	BF3903_write_cmos_sensor(0x52, 0x1a);
	BF3903_write_cmos_sensor(0x53, 0xb4);
	BF3903_write_cmos_sensor(0x54, 0x8d);
	BF3903_write_cmos_sensor(0x57, 0xab);
	BF3903_write_cmos_sensor(0x58, 0x40);

	//color coefficient 1 for indoor scene	
	BF3903_write_cmos_sensor(0x5a, 0x96);		
	BF3903_write_cmos_sensor(0x51, 0x05);
	BF3903_write_cmos_sensor(0x52, 0x05);
	BF3903_write_cmos_sensor(0x53, 0x30);
	BF3903_write_cmos_sensor(0x54, 0x18);
	BF3903_write_cmos_sensor(0x57, 0x0f);
	BF3903_write_cmos_sensor(0x58, 0x04);

	/*//肤色好
	BF3903_write_cmos_sensor(0x5a, 0x96);
	BF3903_write_cmos_sensor(0x51, 0x03);
	BF3903_write_cmos_sensor(0x52, 0x0d);
	BF3903_write_cmos_sensor(0x53, 0x77);
	BF3903_write_cmos_sensor(0x54, 0x66);
	BF3903_write_cmos_sensor(0x57, 0x8b);
	BF3903_write_cmos_sensor(0x58, 0x37);

	//色彩艳丽
	BF3903_write_cmos_sensor(0x5a, 0x96);
	BF3903_write_cmos_sensor(0x51, 0x1f);
	BF3903_write_cmos_sensor(0x52, 0x13);
	BF3903_write_cmos_sensor(0x53, 0xab);
	BF3903_write_cmos_sensor(0x54, 0x92);
	BF3903_write_cmos_sensor(0x57, 0xbb);
	BF3903_write_cmos_sensor(0x58, 0x3b);*/

	//color saturation
	BF3903_write_cmos_sensor(0x5c, 0x28);//the smaller, the smaller color noise in low light scene; 
	BF3903_write_cmos_sensor(0xb0, 0xe0);//when more than 0x80, the smaller, the smaller color noise in low light scene.              
	BF3903_write_cmos_sensor(0xb1, 0xd0);//blue coefficience for saturation 
	BF3903_write_cmos_sensor(0xb2, 0xc8);//red coefficience for saturation
	BF3903_write_cmos_sensor(0xb3, 0x7c);
	BF3903_write_cmos_sensor(0xb4, 0x60); //灰色区域降彩色噪声。0xb4=0x20,噪声最小。

	//awb
	BF3903_write_cmos_sensor(0x6a, 0x81);
	BF3903_write_cmos_sensor(0x23, 0x66);
	BF3903_write_cmos_sensor(0xa0, 0xd0);
	BF3903_write_cmos_sensor(0xa1, 0x31);
	BF3903_write_cmos_sensor(0xa2, 0x0b); //the low limit of blue gain		
	BF3903_write_cmos_sensor(0xa3, 0x26); //the upper limit of blue gain
	BF3903_write_cmos_sensor(0xa4, 0x09); //the low limit of red gain
	BF3903_write_cmos_sensor(0xa5, 0x2c); //the upper limit of red gain
	BF3903_write_cmos_sensor(0xa6, 0x06);
	BF3903_write_cmos_sensor(0xa7, 0x97);
	BF3903_write_cmos_sensor(0xa8, 0x16);
	BF3903_write_cmos_sensor(0xa9, 0x50);    	
	BF3903_write_cmos_sensor(0xaa, 0x50);    
	BF3903_write_cmos_sensor(0xab, 0x1a);
	BF3903_write_cmos_sensor(0xac, 0x3c);    
	BF3903_write_cmos_sensor(0xad, 0xf0);                              
	BF3903_write_cmos_sensor(0xae, 0x57);
	BF3903_write_cmos_sensor(0xc5, 0xaa);
	BF3903_write_cmos_sensor(0xc6, 0x77);//pure color detection	
	BF3903_write_cmos_sensor(0xd0, 0x00);
	
	/*****************************************************
	0xd1[7:4]: 值大于8，值越大，蓝色增益越大；值小于8，值越大，蓝色增益越小；
	0xd1[3:0]: 值大于8，值越大，红色增益越大；值小于8，值越大，红色增益越小；
	*******************************************************/
	BF3903_write_cmos_sensor(0xd1, 0xaa);

	
	BF3903_write_cmos_sensor(0xc8, 0x0d);
	BF3903_write_cmos_sensor(0xc9, 0x0f);
	BF3903_write_cmos_sensor(0xd3, 0x09);               
	BF3903_write_cmos_sensor(0xd4, 0x24);

	BF3903_write_cmos_sensor(0x69, 0x00);//0x69写0x1e改善水波纹	

	//black sun  correction  
	BF3903_write_cmos_sensor(0x61, 0xc8);
	BF3903_write_cmos_sensor(0x74, 0x27);
	BF3903_write_cmos_sensor(0x78, 0xc7);
	BF3903_write_cmos_sensor(0x16, 0xa1);
	
	BF3903_write_cmos_sensor(0x6f, 0x5f);

	//switch direction
	BF3903_write_cmos_sensor(0x1e,0x00);//00:normal  10:IMAGE_V_MIRROR   20:IMAGE_H_MIRROR  30:IMAGE_HV_MIRROR

	
}


typedef struct
{
    UINT16  iSensorVersion;
    UINT16  iNightMode;
    UINT16  iWB;
    UINT16  isoSpeed;
    UINT16  iEffect;
    UINT16  iEV;
    UINT16  iBanding;
    UINT16  iMirror;
    UINT16  iFrameRate;
} BF3903Status;
BF3903Status BF3903CurrentStatus;


static void BF3903InitialPara(void)
{
    BF3903CurrentStatus.iNightMode = 0xFFFF;
    BF3903CurrentStatus.iWB = AWB_MODE_AUTO;
    BF3903CurrentStatus.isoSpeed = AE_ISO_100;
    BF3903CurrentStatus.iEffect = MEFFECT_OFF;
    BF3903CurrentStatus.iBanding = AE_FLICKER_MODE_50HZ;
    BF3903CurrentStatus.iEV = AE_EV_COMP_n03;
    BF3903CurrentStatus.iMirror = IMAGE_NORMAL;
    BF3903CurrentStatus.iFrameRate = 25;
}

/*************************************************************************
 * FUNCTION
 *	BF3903Open
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
UINT32 BF3903Open(void)
{
kal_uint16 sensor_id=0;

 printk("BF3903 Open 11111111 JHY\n");
 
 BF3903GetSensorID(&sensor_id);
 //BF3903InitialPara();
 BF3903_Sensor_init();

 printk("BF3903 Sensor id read OK, ID = %x\n", sensor_id);

 return ERROR_NONE;

} /* BF3903Open */


/*************************************************************************
 * FUNCTION
 *	BF3903Close
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
UINT32 BF3903Close(void)
{
    return ERROR_NONE;
} /* BF3903Close */


/*************************************************************************
 * FUNCTION
 * BF3903Preview
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
UINT32 BF3903Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    kal_uint32 iTemp;
    kal_uint16 iStartX = 0, iStartY = 1;

    if(sensor_config_data->SensorOperationMode == MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
        RETAILMSG(1, (TEXT("Camera Video preview\r\n")));
        BF3903_MPEG4_encode_mode = KAL_TRUE;
       
    }
    else
    {
        RETAILMSG(1, (TEXT("Camera preview\r\n")));
        BF3903_MPEG4_encode_mode = KAL_FALSE;
    }

    image_window->GrabStartX= 0; //IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY= 1; //IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth = IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight =IMAGE_SENSOR_PV_HEIGHT;

    // copy sensor_config_data
    memcpy(&BF3903SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* BF3903Preview */


/*************************************************************************
 * FUNCTION
 *	BF3903Capture
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
UINT32 BF3903Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    BF3903_MODE_CAPTURE=KAL_TRUE;

    image_window->GrabStartX = 0; //IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY = 1; //IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth= IMAGE_SENSOR_FULL_WIDTH;
    image_window->ExposureWindowHeight = IMAGE_SENSOR_FULL_HEIGHT;

    // copy sensor_config_data
    memcpy(&BF3903SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* BF3903_Capture() */



UINT32 BF3903GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT;
	
    pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT;

    pSensorResolution->SensorVideoWidth     =  IMAGE_SENSOR_PV_WIDTH; //add by JHY for video err 0416
    pSensorResolution->SensorVideoHeight    =  IMAGE_SENSOR_PV_HEIGHT;   	
	
    return ERROR_NONE;
} /* BF3903GetResolution() */

UINT32 BF3903GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
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

/*  
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

    */
    
    pSensorInfo->CaptureDelayFrame = 1;
    pSensorInfo->PreviewDelayFrame = 0;
    pSensorInfo->VideoDelayFrame = 4;
    pSensorInfo->SensorMasterClockSwitch = 0;
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_2MA;

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    //scase MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount=	3;
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = 0;
        pSensorInfo->SensorGrabStartY = 1;

        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = 0;
        pSensorInfo->SensorGrabStartY = 1;
        break;
    default:
        pSensorInfo->SensorClockFreq=24;
        pSensorInfo->SensorClockDividCount= 3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = 0;
        pSensorInfo->SensorGrabStartY = 1;
        break;
    }
    BF3903PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &BF3903SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* BF3903GetInfo() */


UINT32 BF3903Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
        BF3903Preview(pImageWindow, pSensorConfigData);
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
        BF3903Capture(pImageWindow, pSensorConfigData);
        break;
    }


    return ERROR_NONE;
}	/* BF3903Control() */

BOOL BF3903_set_param_wb(UINT16 para)
{

	switch (para)
	{
		case AWB_MODE_OFF:

		break;
		
		case AWB_MODE_AUTO:
			BF3903_awb_enable(KAL_TRUE);
		break;
		
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			BF3903_awb_enable(KAL_FALSE);
		BF3903_write_cmos_sensor(0x01, 0x10);
		BF3903_write_cmos_sensor(0x02, 0x28);		
		break;
		
		case AWB_MODE_DAYLIGHT: //sunny
			BF3903_awb_enable(KAL_FALSE);
		BF3903_write_cmos_sensor(0x01, 0x13);
		BF3903_write_cmos_sensor(0x02, 0x26);			
		break;
		
		case AWB_MODE_INCANDESCENT: //office
			BF3903_awb_enable(KAL_FALSE);
		BF3903_write_cmos_sensor(0x01, 0x1f);
		BF3903_write_cmos_sensor(0x02, 0x15);		
		break;
		
		case AWB_MODE_TUNGSTEN: //home
			BF3903_awb_enable(KAL_FALSE);
		BF3903_write_cmos_sensor(0x01, 0x1a);
		BF3903_write_cmos_sensor(0x02, 0x0d);		
		break;
		
		case AWB_MODE_FLUORESCENT:
			BF3903_awb_enable(KAL_FALSE);
		BF3903_write_cmos_sensor(0x01, 0x1a);
		BF3903_write_cmos_sensor(0x02, 0x1e);	
		break;
		
		default:
		return FALSE;
	}

	return TRUE;
} /* BF3903_set_param_wb */


BOOL BF3903_set_param_effect(UINT16 para)
{
	kal_uint32  ret = KAL_TRUE;

	switch (para)
	{
		case MEFFECT_OFF:
		BF3903_write_cmos_sensor(0x70, 0x0f);		
		BF3903_write_cmos_sensor(0x69, 0x00);
		BF3903_write_cmos_sensor(0x67, 0x80);		
		BF3903_write_cmos_sensor(0x68, 0x80);	
		BF3903_write_cmos_sensor(0x98, 0x12);		
		BF3903_write_cmos_sensor(0x56, 0x40);
		break;
		
		case MEFFECT_SEPIA:
		BF3903_write_cmos_sensor(0x70, 0x0f);		
		BF3903_write_cmos_sensor(0x69, 0x20);
		BF3903_write_cmos_sensor(0x67, 0x60);		
		BF3903_write_cmos_sensor(0x68, 0xa0);	
		BF3903_write_cmos_sensor(0x98, 0x92);		
		BF3903_write_cmos_sensor(0x56, 0x40);	
		break;
		
		case MEFFECT_NEGATIVE:
		BF3903_write_cmos_sensor(0x70, 0x0f);		
		BF3903_write_cmos_sensor(0x69, 0x01);
		BF3903_write_cmos_sensor(0x67, 0x80);		
		BF3903_write_cmos_sensor(0x68, 0x80);	
		BF3903_write_cmos_sensor(0x98, 0x92);		
		BF3903_write_cmos_sensor(0x56, 0x40);	
                break;
		
		case MEFFECT_SEPIAGREEN:
		BF3903_write_cmos_sensor(0x70, 0x0f);		
		BF3903_write_cmos_sensor(0x69, 0x20);
		BF3903_write_cmos_sensor(0x67, 0x60);		
		BF3903_write_cmos_sensor(0x68, 0x70);	
		BF3903_write_cmos_sensor(0x98, 0x92);		
		BF3903_write_cmos_sensor(0x56, 0x40);
		break;
		
		case MEFFECT_SEPIABLUE:
		BF3903_write_cmos_sensor(0x70, 0x0f);		
		BF3903_write_cmos_sensor(0x69, 0x20);
		BF3903_write_cmos_sensor(0x67, 0xe0);		
		BF3903_write_cmos_sensor(0x68, 0x60);		
		BF3903_write_cmos_sensor(0x98, 0x92);		
		BF3903_write_cmos_sensor(0x56, 0x40);	
		break;

		case MEFFECT_MONO:
		BF3903_write_cmos_sensor(0x70, 0x0f);		
		BF3903_write_cmos_sensor(0x69, 0x20);
		BF3903_write_cmos_sensor(0x67, 0x80);		
		BF3903_write_cmos_sensor(0x68, 0x80);	
		BF3903_write_cmos_sensor(0x98, 0x92);		
		BF3903_write_cmos_sensor(0x56, 0x40);
                break;
		default:
			ret = FALSE;
	}

	return ret;

} /* BF3903_set_param_effect */


BOOL BF3903_set_param_banding(UINT16 para)
{
  kal_uint8 temp_Reg_80 = BF3903_read_cmos_sensor(0x80);

	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:

    BF3903_write_cmos_sensor(0x80,temp_Reg_80|0x02);
    BF3903_write_cmos_sensor(0x8a,0x99);//99
                 
			BF3903_CAM_BANDING_50HZ = KAL_TRUE;

			break;

		case AE_FLICKER_MODE_60HZ:
      BF3903_write_cmos_sensor(0x80,temp_Reg_80&0xFd);
      BF3903_write_cmos_sensor(0x8b,0x7f);//7f
			BF3903_CAM_BANDING_50HZ = KAL_FALSE;
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* BF3903_set_param_banding */


BOOL BF3903_set_param_exposure(UINT16 para)
{
	switch (para)
	{
		case AE_EV_COMP_n13:
			BF3903_write_cmos_sensor(0x55, 0xc0);		

			
		break;
		
		case AE_EV_COMP_n10:
			BF3903_write_cmos_sensor(0x55, 0xb0);		

			
		break;
		
		case AE_EV_COMP_n07:
			BF3903_write_cmos_sensor(0x55, 0xa0);		

			
		break;
		
		case AE_EV_COMP_n03:
			BF3903_write_cmos_sensor(0x55, 0x90);		

		break;
		
		case AE_EV_COMP_00:
			BF3903_write_cmos_sensor(0x55, 0x00);		
		break;

		case AE_EV_COMP_03:
			BF3903_write_cmos_sensor(0x55, 0x10);		

			
		break;
		
		case AE_EV_COMP_07:
			BF3903_write_cmos_sensor(0x55, 0x30);		

			
		break;
		
		case AE_EV_COMP_10:
			BF3903_write_cmos_sensor(0x55, 0x50);		

			
		break;
		
		case AE_EV_COMP_13:
			BF3903_write_cmos_sensor(0x55, 0x70);		

			
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* BF3903_set_param_exposure */

BOOL BF3903_set_scene_mode(UINT16 Para)
{
	//SENSORDB("BF3903SetScenMode: %d\n",Para);
            printk("phl BF3903_set_scene_mode Para = %d\n",Para);

	switch (Para)
	{                   
    case SCENE_MODE_NIGHTSCENE:		
		BF3903_write_cmos_sensor(0x89, 0xa5);
      	break;
    case SCENE_MODE_OFF://SCENE_MODE_AUTO: 	
	  BF3903_write_cmos_sensor(0x89, 0x7d); 
      	break;      
    case SCENE_MODE_PORTRAIT: 	
      	break;
    case SCENE_MODE_LANDSCAPE: 	
      	break;
    case SCENE_MODE_BEACH: 	
      	break;
    case SCENE_MODE_SUNSET:	
     	 break;
	case SCENE_MODE_SPORTS: 
		break;
	case SCENE_MODE_HDR: 
		break;
    default:
      return KAL_FALSE;
  }
  return KAL_TRUE;      
} /* BF3903SetSceneMode */
UINT32 BF3903YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
    switch (iCmd) {
    case FID_SCENE_MODE:
	  BF3903_set_scene_mode(iPara);
            break; 
    case FID_AWB_MODE:
        BF3903_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        BF3903_set_param_effect(iPara);
        break;
    case FID_AE_EV:
        BF3903_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
        BF3903_set_param_banding(iPara);
        break;
    default:
        break;
    }
    return TRUE;
} /* BF3903YUVSensorSetting */

void BF3903GetExifInfo(UINT32 exifAddr)
{
    SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
    pExifInfo->FNumber = 28;
    pExifInfo->AEISOSpeed = BF3903CurrentStatus.isoSpeed;
    pExifInfo->AWBMode = BF3903CurrentStatus.iWB;
    pExifInfo->CapExposureTime = 0;
    pExifInfo->FlashLightTimeus = 0;
    pExifInfo->RealISOValue = BF3903CurrentStatus.isoSpeed;
}


UINT32 BF3903FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 BF3903SensorRegNumber;
    UINT32 i;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

    RETAILMSG(1, (_T("gaiyang BF3903FeatureControl FeatureId=%d\r\n"), FeatureId));

    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=IMAGE_SENSOR_FULL_WIDTH;
        *pFeatureReturnPara16=IMAGE_SENSOR_FULL_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        *pFeatureReturnPara16++=(694)+BF3903_dummy_pixels;
        *pFeatureReturnPara16=(488)+BF3903_dummy_lines;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        *pFeatureReturnPara32 = BF3903_g_fPV_PCLK;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        BF3903_night_mode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        BF3903_isp_master_clock=*pFeatureData32;
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        BF3903_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        pSensorRegData->RegData = BF3903_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        memcpy(pSensorConfigData, &BF3903SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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
        BF3903YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;
	case SENSOR_FEATURE_GET_EXIF_INFO:
             SENSORDB("SENSOR_FEATURE_GET_EXIF_INFO\n");
             SENSORDB("EXIF addr = 0x%x\n",*pFeatureData32);
             BF3903GetExifInfo(*pFeatureData32);
             break;	
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
	BF3903GetSensorID(pFeatureData32);
	break;
    default:
        break;
	}
return ERROR_NONE;
}	/* BF3903FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncBF3903YUV=
{
	BF3903Open,
	BF3903GetInfo,
	BF3903GetResolution,
	BF3903FeatureControl,
	BF3903Control,
	BF3903Close
};

UINT32 BF3903_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncBF3903YUV;
	return ERROR_NONE;
} /* SensorInit() */
