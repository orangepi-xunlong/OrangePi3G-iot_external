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
 *   BF3A03yuv_Sensor.c
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
 * 2011/10/25 Firsty Released By Mormo(using "BF3A03.set Revision1721" )
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

#include "bf3a03yuv_Sensor.h"
#include "bf3a03yuv_Camera_Sensor_para.h"
#include "bf3a03yuv_CameraCustomized.h"



#define BF3A03YUV_DEBUG
#ifdef BF3A03YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

#define BF3A03_WRITE_ID 0xDC

//extern int iReadReg(u8 addr, u8 *buf, u8 i2cId);
//extern int iWriteReg(u8 addr, u8 buf, u32 size, u16 i2cId);

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);



static void BF3A03_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    char puSendCmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd, 2,BF3A03_WRITE_ID);
}

 kal_uint8 BF3A03_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint8 get_byte = 0;
	
    char pSendCmd = (char)(addr & 0xFF);
    
   iReadRegI2C(&pSendCmd, 1, (u8*)&get_byte, 1, BF3A03_WRITE_ID);

    return get_byte;
}


/*******************************************************************************
 * // Adapter for Winmo typedef
 ********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT

kal_bool   BF3A03_MPEG4_encode_mode = KAL_FALSE;
kal_uint16 BF3A03_dummy_pixels = 0, BF3A03_dummy_lines = 0;
kal_bool   BF3A03_MODE_CAPTURE = KAL_FALSE;
kal_bool   BF3A03_CAM_BANDING_50HZ = KAL_FALSE;

kal_uint32 BF3A03_isp_master_clock;
static kal_uint32 BF3A03_g_fPV_PCLK = 24;

//kal_uint8 BF3A03_sensor_write_I2C_address = BF3A03_WRITE_ID;
//kal_uint8 BF3A03_sensor_read_I2C_address = BF3A03_READ_ID;

UINT8 BF3A03PixelClockDivider=0;

MSDK_SENSOR_CONFIG_STRUCT BF3A03SensorConfigData;


/*************************************************************************
 * FUNCTION
 *	BF3A03_SetShutter
 *
 * DESCRIPTION
 *	This function set e-shutter of BF3A03 to change exposure time.
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
void BF3A03_Set_Shutter(kal_uint16 iShutter)
{
} /* Set_BF3A03_Shutter */


/*************************************************************************
 * FUNCTION
 *	BF3A03_read_Shutter
 *
 * DESCRIPTION
 *	This function read e-shutter of BF3A03 .
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
kal_uint16 BF3A03_Read_Shutter(void)
{
    	kal_uint8 temp_reg1, temp_reg2;
	kal_uint16 shutter;

	temp_reg1 = BF3A03_read_cmos_sensor(0x8c);
	temp_reg2 = BF3A03_read_cmos_sensor(0x8d);

	shutter = (temp_reg1 & 0xFF) | (temp_reg2 << 8);

	return shutter;
} /* BF3A03_read_shutter */


/*************************************************************************
 * FUNCTION
 *	BF3A03_write_reg
 *
 * DESCRIPTION
 *	This function set the register of BF3A03.
 *
 * PARAMETERS
 *	addr : the register index of BF3A03
 *  para : setting parameter of the specified register of BF3A03
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void BF3A03_write_reg(kal_uint32 addr, kal_uint32 para)
{
	BF3A03_write_cmos_sensor(addr, para);
} /* BF3A03_write_reg() */


/*************************************************************************
 * FUNCTION
 *	BF3A03_read_cmos_sensor
 *
 * DESCRIPTION
 *	This function read parameter of specified register from BF3A03.
 *
 * PARAMETERS
 *	addr : the register index of BF3A03
 *
 * RETURNS
 *	the data that read from BF3A03
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint32 BF3A03_read_reg(kal_uint32 addr)
{
	return BF3A03_read_cmos_sensor(addr);
} /* OV7670_read_reg() */


/*************************************************************************
* FUNCTION
*	BF3A03_awb_enable
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
static void BF3A03_awb_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AWB_reg = 0;

	temp_AWB_reg = BF3A03_read_cmos_sensor(0x13);
	

	if (enalbe)
	{
		BF3A03_write_cmos_sensor(0x13, (temp_AWB_reg |0x02));
	}
	else
	{
		BF3A03_write_cmos_sensor(0x13, (temp_AWB_reg & (~0x02)));
	}

}


/*************************************************************************
 * FUNCTION
 *	BF3A03_config_window
 *
 * DESCRIPTION
 *	This function config the hardware window of BF3A03 for getting specified
 *  data of that window.
 *
 * PARAMETERS
 *	start_x : start column of the interested window
 *  start_y : start row of the interested window
 *  width  : column widht of the itnerested window
 *  height : row depth of the itnerested window
 *
 * RETURNS
 *	the data that read from BF3A03
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void BF3A03_config_window(kal_uint16 startx, kal_uint16 starty, kal_uint16 width, kal_uint16 height)
{
} /* BF3A03_config_window */


/*************************************************************************
 * FUNCTION
 *	BF3A03_SetGain
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
kal_uint16 BF3A03_SetGain(kal_uint16 iGain)
{
	return iGain;
}


/*************************************************************************
 * FUNCTION
 *	BF3A03_NightMode
 *
 * DESCRIPTION
 *	This function night mode of BF3A03.
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
void BF3A03_night_mode(kal_bool bEnable)
{
        printk("phl BF3A03_night_mode bEnable = %d\n",bEnable);
	if (bEnable)
	{
           	if(BF3A03_MPEG4_encode_mode == KAL_TRUE) 
    		{
        		 BF3A03_write_cmos_sensor(0x89, 0xa5);				
           	 }
        	else 
        	{        
        		 BF3A03_write_cmos_sensor(0x89, 0xa5); // 79 \B7\D6Ƶ	F2 \B2\BB\B7\D6Ƶ		    		
        	}
	}
	else 
	{
        	if (BF3A03_MPEG4_encode_mode == KAL_TRUE) 
		{
        		 BF3A03_write_cmos_sensor(0x89, 0x7d);
			
        	}
		else
		{
			
        		 BF3A03_write_cmos_sensor(0x89, 0x7d);

    		}
	}
} /* BF3A03_NightMode */




UINT32 BF3A03GetSensorID(UINT32 *sensorID)
{
    kal_uint16 sensor_id=0;
    int i;



    do
    {
      
        for(i = 0; i < 3; i++)
		{
	   
	      sensor_id = ((BF3A03_read_cmos_sensor(0xfc) << 8) | BF3A03_read_cmos_sensor(0xfd));
	      printk("JHY BF3A03 Sensor id = %x\n", sensor_id);
	      if (sensor_id == BF3A03_SENSOR_ID)
			{
	         break;
	        }
        }
        	mdelay(50);
    }while(0);

    if(sensor_id != BF3A03_SENSOR_ID)
    {
         *sensorID = 0xFFFFFFFF;
        SENSORDB("BF3A03 Sensor id read failed, ID = %x\n", sensor_id);
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    *sensorID = sensor_id;

    RETAILMSG(1, (TEXT("Sensor Read ID OK \r\n")));
	
    return ERROR_NONE;
}


void BF3A03_Sensor_init(void)
{
		BF3A03_write_cmos_sensor(0x09, 0x55);
		BF3A03_write_cmos_sensor(0x12, 0x00);
		BF3A03_write_cmos_sensor(0x15, 0x00);
		BF3A03_write_cmos_sensor(0x1b, 0x09);
		BF3A03_write_cmos_sensor(0x4a, 0x98);
		BF3A03_write_cmos_sensor(0x06, 0x78); //68   ryx 2013.08.15
		BF3A03_write_cmos_sensor(0x21, 0x00);
		BF3A03_write_cmos_sensor(0x90, 0x20);
		BF3A03_write_cmos_sensor(0x3e, 0x37);
		BF3A03_write_cmos_sensor(0x29, 0x2b); //29  ryx 2013.08.15
		BF3A03_write_cmos_sensor(0x27, 0x98);
		BF3A03_write_cmos_sensor(0x2f, 0x4e);
		BF3A03_write_cmos_sensor(0x3a, 0x00);
		BF3A03_write_cmos_sensor(0x1e, 0x70);
		BF3A03_write_cmos_sensor(0x13, 0x08);
		BF3A03_write_cmos_sensor(0x01, 0x14);
		BF3A03_write_cmos_sensor(0x02, 0x20);
		BF3A03_write_cmos_sensor(0x8c, 0x02);
		BF3A03_write_cmos_sensor(0x8d, 0x4c);
		BF3A03_write_cmos_sensor(0x87, 0x16);
		BF3A03_write_cmos_sensor(0x13, 0x07);
		BF3A03_write_cmos_sensor(0x35, 0x50);
		BF3A03_write_cmos_sensor(0x65, 0x46);
		BF3A03_write_cmos_sensor(0x66, 0x46);
		BF3A03_write_cmos_sensor(0x39, 0xc0);
		BF3A03_write_cmos_sensor(0x3f, 0xc0);
		BF3A03_write_cmos_sensor(0x5f, 0x01);
		BF3A03_write_cmos_sensor(0x40, 0x22);
		BF3A03_write_cmos_sensor(0x41, 0x23);
		BF3A03_write_cmos_sensor(0x42, 0x28);
		BF3A03_write_cmos_sensor(0x43, 0x25);
		BF3A03_write_cmos_sensor(0x44, 0x1d);
		BF3A03_write_cmos_sensor(0x45, 0x17);
		BF3A03_write_cmos_sensor(0x46, 0x13);
		BF3A03_write_cmos_sensor(0x47, 0x12);
		BF3A03_write_cmos_sensor(0x48, 0x10);
		BF3A03_write_cmos_sensor(0x49, 0x0d);
		BF3A03_write_cmos_sensor(0x4b, 0x0b);
		BF3A03_write_cmos_sensor(0x4c, 0x0b);
		BF3A03_write_cmos_sensor(0x4e, 0x09);
		BF3A03_write_cmos_sensor(0x4f, 0x07);
		BF3A03_write_cmos_sensor(0x50, 0x06);
		BF3A03_write_cmos_sensor(0x70, 0x0f);
		BF3A03_write_cmos_sensor(0x3b, 0x00);
		BF3A03_write_cmos_sensor(0x71, 0x0c);
		BF3A03_write_cmos_sensor(0x72, 0x4c);
		BF3A03_write_cmos_sensor(0x73, 0x27);
		BF3A03_write_cmos_sensor(0x75, 0x88);
		BF3A03_write_cmos_sensor(0x76, 0xd8);
		BF3A03_write_cmos_sensor(0x77, 0x0a);
		BF3A03_write_cmos_sensor(0x78, 0xff);
		BF3A03_write_cmos_sensor(0x79, 0x14);
		BF3A03_write_cmos_sensor(0x7a, 0x12);
		BF3A03_write_cmos_sensor(0x9e, 0xc4);
		BF3A03_write_cmos_sensor(0x7d, 0x2a);
		BF3A03_write_cmos_sensor(0x24, 0x50);
		BF3A03_write_cmos_sensor(0x97, 0x40);
		BF3A03_write_cmos_sensor(0x25, 0x88);
		BF3A03_write_cmos_sensor(0x81, 0x00);
		BF3A03_write_cmos_sensor(0x82, 0x18);
		BF3A03_write_cmos_sensor(0x83, 0x30);
		BF3A03_write_cmos_sensor(0x84, 0x20);
		BF3A03_write_cmos_sensor(0x85, 0x38);
		BF3A03_write_cmos_sensor(0x86, 0x55);
		BF3A03_write_cmos_sensor(0x94, 0x62);
		BF3A03_write_cmos_sensor(0x80, 0x92);
		BF3A03_write_cmos_sensor(0x89, 0x7d);
		BF3A03_write_cmos_sensor(0x2b, 0x20);
		BF3A03_write_cmos_sensor(0x8a, 0x93);
		BF3A03_write_cmos_sensor(0x8e, 0x2c);
		BF3A03_write_cmos_sensor(0x8f, 0x86);
		BF3A03_write_cmos_sensor(0xb0, 0xa0);
		BF3A03_write_cmos_sensor(0xb1, 0xe6);
		BF3A03_write_cmos_sensor(0xb2, 0xec);
		BF3A03_write_cmos_sensor(0xb4, 0xe1);
		BF3A03_write_cmos_sensor(0x5a, 0x78);
		BF3A03_write_cmos_sensor(0x51, 0x80);
		BF3A03_write_cmos_sensor(0x52, 0x04);
		BF3A03_write_cmos_sensor(0x53, 0x8d);
		BF3A03_write_cmos_sensor(0x54, 0x88);
		BF3A03_write_cmos_sensor(0x57, 0x82);
		BF3A03_write_cmos_sensor(0x58, 0x8d);
		BF3A03_write_cmos_sensor(0xb0, 0x30);
		BF3A03_write_cmos_sensor(0xb1, 0xc0);
		BF3A03_write_cmos_sensor(0xb2, 0xa0);
		BF3A03_write_cmos_sensor(0xb4, 0xf1);
		BF3A03_write_cmos_sensor(0x5a, 0x6C);
		BF3A03_write_cmos_sensor(0x51, 0x90);
		BF3A03_write_cmos_sensor(0x52, 0x04);
		BF3A03_write_cmos_sensor(0x53, 0x8a);
		BF3A03_write_cmos_sensor(0x54, 0x88);
		BF3A03_write_cmos_sensor(0x57, 0x02);
		BF3A03_write_cmos_sensor(0x58, 0x8d);
		BF3A03_write_cmos_sensor(0x6a, 0x91);
		BF3A03_write_cmos_sensor(0x23, 0x44);
		BF3A03_write_cmos_sensor(0xa2, 0x0b);
		BF3A03_write_cmos_sensor(0xa3, 0x26);
		BF3A03_write_cmos_sensor(0xa4, 0x04);
		BF3A03_write_cmos_sensor(0xa5, 0x20);
		BF3A03_write_cmos_sensor(0xa6, 0x04);
		BF3A03_write_cmos_sensor(0xa7, 0x16);
		BF3A03_write_cmos_sensor(0xa8, 0x18);
		BF3A03_write_cmos_sensor(0xa9, 0x12);
		BF3A03_write_cmos_sensor(0xaa, 0x12);
		BF3A03_write_cmos_sensor(0xab, 0x16);
		BF3A03_write_cmos_sensor(0xac, 0x4c);
		BF3A03_write_cmos_sensor(0xad, 0xf0);
		BF3A03_write_cmos_sensor(0xae, 0x57);
		BF3A03_write_cmos_sensor(0xc7, 0x38);
		BF3A03_write_cmos_sensor(0xc9, 0x16);
		BF3A03_write_cmos_sensor(0xd4, 0x15);
		BF3A03_write_cmos_sensor(0xd0, 0x00);
		BF3A03_write_cmos_sensor(0xd1, 0x01);
		BF3A03_write_cmos_sensor(0xd2, 0x18);//58   ryx 2013.08.15
		BF3A03_write_cmos_sensor(0xd3, 0x09);
		BF3A03_write_cmos_sensor(0xd4, 0x24);
		BF3A03_write_cmos_sensor(0x56, 0x48);
		BF3A03_write_cmos_sensor(0x92, 0x6D);

		//  ryx   add   \D2\D4\CF¼Ĵ\E6\C6\F7\B8İ\E6оƬ\B1\D8\D0\EBҪд

		BF3A03_write_cmos_sensor(0x29, 0x2b);
		BF3A03_write_cmos_sensor(0x06, 0x78);
		BF3A03_write_cmos_sensor(0xd2, 0x18);
		BF3A03_write_cmos_sensor(0x20, 0x00);
		BF3A03_write_cmos_sensor(0x16, 0x25);

		//ryx  add   

		
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
} BF3A03Status;
BF3A03Status BF3A03CurrentStatus;


static void BF3A03InitialPara(void)
{
    BF3A03CurrentStatus.iNightMode = 0xFFFF;
    BF3A03CurrentStatus.iWB = AWB_MODE_AUTO;
    BF3A03CurrentStatus.isoSpeed = AE_ISO_100;
    BF3A03CurrentStatus.iEffect = MEFFECT_OFF;
    BF3A03CurrentStatus.iBanding = AE_FLICKER_MODE_50HZ;
    BF3A03CurrentStatus.iEV = AE_EV_COMP_n03;
    BF3A03CurrentStatus.iMirror = IMAGE_NORMAL;
    BF3A03CurrentStatus.iFrameRate = 25;
}

/*************************************************************************
 * FUNCTION
 *	BF3A03Open
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
UINT32 BF3A03Open(void)
{
kal_uint16 sensor_id=0;

 printk("BF3A03 Open 11111111 JHY\n");
 
 BF3A03GetSensorID(&sensor_id);
 //BF3A03InitialPara();
 BF3A03_Sensor_init();

 printk("BF3A03 Sensor id read OK, ID = %x\n", sensor_id);

 return ERROR_NONE;

} /* BF3A03Open */


/*************************************************************************
 * FUNCTION
 *	BF3A03Close
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
UINT32 BF3A03Close(void)
{
    return ERROR_NONE;
} /* BF3A03Close */


/*************************************************************************
 * FUNCTION
 * BF3A03Preview
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
UINT32 BF3A03Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    kal_uint32 iTemp;
    kal_uint16 iStartX = 0, iStartY = 1;

    if(sensor_config_data->SensorOperationMode == MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
        RETAILMSG(1, (TEXT("Camera Video preview\r\n")));
        BF3A03_MPEG4_encode_mode = KAL_TRUE;
       
    }
    else
    {
        RETAILMSG(1, (TEXT("Camera preview\r\n")));
        BF3A03_MPEG4_encode_mode = KAL_FALSE;
    }

    image_window->GrabStartX= 0; //IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY= 1; //IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth = IMAGE_SENSOR_PV_WIDTH;
    image_window->ExposureWindowHeight =IMAGE_SENSOR_PV_HEIGHT;

    // copy sensor_config_data
    memcpy(&BF3A03SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* BF3A03Preview */


/*************************************************************************
 * FUNCTION
 *	BF3A03Capture
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
UINT32 BF3A03Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    BF3A03_MODE_CAPTURE=KAL_TRUE;

    image_window->GrabStartX = 0; //IMAGE_SENSOR_VGA_GRAB_PIXELS;
    image_window->GrabStartY = 1; //IMAGE_SENSOR_VGA_GRAB_LINES;
    image_window->ExposureWindowWidth= IMAGE_SENSOR_FULL_WIDTH;
    image_window->ExposureWindowHeight = IMAGE_SENSOR_FULL_HEIGHT;

    // copy sensor_config_data
    memcpy(&BF3A03SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* BF3A03_Capture() */



UINT32 BF3A03GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT;
	
    pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT;

    pSensorResolution->SensorVideoWidth     =  IMAGE_SENSOR_PV_WIDTH; //add by JHY for video err 0416
    pSensorResolution->SensorVideoHeight    =  IMAGE_SENSOR_PV_HEIGHT;   	
	
    return ERROR_NONE;
} /* BF3A03GetResolution() */

UINT32 BF3A03GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
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
    BF3A03PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &BF3A03SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* BF3A03GetInfo() */


UINT32 BF3A03Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
        BF3A03Preview(pImageWindow, pSensorConfigData);
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
        BF3A03Capture(pImageWindow, pSensorConfigData);
        break;
    }


    return ERROR_NONE;
}	/* BF3A03Control() */

BOOL BF3A03_set_param_wb(UINT16 para)
{

	switch (para)
	{
		case AWB_MODE_OFF:

		break;
		
		case AWB_MODE_AUTO:
      BF3A03_write_cmos_sensor(0x01,0x13);
      BF3A03_write_cmos_sensor(0x02,0x25);
			BF3A03_awb_enable(KAL_TRUE);
		break;
		
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy
			BF3A03_awb_enable(KAL_FALSE);
      BF3A03_write_cmos_sensor(0x01,0x10);
      BF3A03_write_cmos_sensor(0x02,0x2c);
		break;
		
		case AWB_MODE_DAYLIGHT: //sunny
			BF3A03_awb_enable(KAL_FALSE);
      BF3A03_write_cmos_sensor(0x01,0x15);
      BF3A03_write_cmos_sensor(0x02,0x20);		
		break;
		
		case AWB_MODE_INCANDESCENT: //office
			BF3A03_awb_enable(KAL_FALSE);
      BF3A03_write_cmos_sensor(0x01,0x1f);
      BF3A03_write_cmos_sensor(0x02,0x20);
		break;
		
		case AWB_MODE_TUNGSTEN: //home
			BF3A03_awb_enable(KAL_FALSE);
      BF3A03_write_cmos_sensor(0x01,0x1a);
      BF3A03_write_cmos_sensor(0x02,0x0a);
		break;
		
		case AWB_MODE_FLUORESCENT:
			BF3A03_awb_enable(KAL_FALSE);
      BF3A03_write_cmos_sensor(0x01,0x1a);
      BF3A03_write_cmos_sensor(0x02,0x15);
		break;
		
		default:
		return FALSE;
	}

	return TRUE;
} /* BF3A03_set_param_wb */


BOOL BF3A03_set_param_effect(UINT16 para)
{
	kal_uint32  ret = KAL_TRUE;

	switch (para)
	{
		case MEFFECT_OFF:
			BF3A03_write_cmos_sensor(0x70,0x0f);
			BF3A03_write_cmos_sensor(0x69,0x00);
			BF3A03_write_cmos_sensor(0x67,0x80);
			BF3A03_write_cmos_sensor(0x68,0x80);
		break;
		
		case MEFFECT_SEPIA:
			BF3A03_write_cmos_sensor(0x70,0x0b);
			BF3A03_write_cmos_sensor(0x69,0x20);
			BF3A03_write_cmos_sensor(0x67,0x60);
			BF3A03_write_cmos_sensor(0x68,0xa0);
		break;
		
		case MEFFECT_NEGATIVE:
			BF3A03_write_cmos_sensor(0x70,0x0b);
			BF3A03_write_cmos_sensor(0x69,0x01);
			BF3A03_write_cmos_sensor(0x67,0x80);
			BF3A03_write_cmos_sensor(0x68,0x80);
                break;
		
		case MEFFECT_SEPIAGREEN:
			BF3A03_write_cmos_sensor(0x70,0x0b);
			BF3A03_write_cmos_sensor(0x69,0x20);
			BF3A03_write_cmos_sensor(0x67,0x60);
			BF3A03_write_cmos_sensor(0x68,0x70);
		break;
		
		case MEFFECT_SEPIABLUE:
			BF3A03_write_cmos_sensor(0x70,0x0b);
			BF3A03_write_cmos_sensor(0x69,0x20);
			BF3A03_write_cmos_sensor(0x67,0xe0);
			BF3A03_write_cmos_sensor(0x68,0x60);
		break;

		case MEFFECT_MONO:
			BF3A03_write_cmos_sensor(0x70,0x0b);
			BF3A03_write_cmos_sensor(0x69,0x20);
			BF3A03_write_cmos_sensor(0x67,0x80);
			BF3A03_write_cmos_sensor(0x68,0x80);
                break;
		default:
			ret = FALSE;
	}

	return ret;

} /* BF3A03_set_param_effect */


BOOL BF3A03_set_param_banding(UINT16 para)
{
  kal_uint8 temp_Reg_80 = BF3A03_read_cmos_sensor(0x80);

	switch (para)
	{
		case AE_FLICKER_MODE_50HZ:

    BF3A03_write_cmos_sensor(0x80,temp_Reg_80|0x02);
    BF3A03_write_cmos_sensor(0x8a,0x93);//99
                 
			BF3A03_CAM_BANDING_50HZ = KAL_TRUE;

			break;

		case AE_FLICKER_MODE_60HZ:
      BF3A03_write_cmos_sensor(0x80,temp_Reg_80&0xFd);
      BF3A03_write_cmos_sensor(0x8b,0x7a);//7f
			BF3A03_CAM_BANDING_50HZ = KAL_FALSE;
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* BF3A03_set_param_banding */


BOOL BF3A03_set_param_exposure(UINT16 para)
{
	switch (para)
	{
		case AE_EV_COMP_n13:
			BF3A03_write_cmos_sensor(0x55, 0xc0);		

			
		break;
		
		case AE_EV_COMP_n10:
			BF3A03_write_cmos_sensor(0x55, 0xb0);		

			
		break;
		
		case AE_EV_COMP_n07:
			BF3A03_write_cmos_sensor(0x55, 0xa0);		

			
		break;
		
		case AE_EV_COMP_n03:
			BF3A03_write_cmos_sensor(0x55, 0x90);		

		break;
		
		case AE_EV_COMP_00:
			BF3A03_write_cmos_sensor(0x55, 0x00);		
		break;

		case AE_EV_COMP_03:
			BF3A03_write_cmos_sensor(0x55, 0x10);		

			
		break;
		
		case AE_EV_COMP_07:
			BF3A03_write_cmos_sensor(0x55, 0x30);		

			
		break;
		
		case AE_EV_COMP_10:
			BF3A03_write_cmos_sensor(0x55, 0x50);		

			
		break;
		
		case AE_EV_COMP_13:
			BF3A03_write_cmos_sensor(0x55, 0x70);		

			
		break;
		default:
		return FALSE;
	}

	return TRUE;
} /* BF3A03_set_param_exposure */

BOOL BF3A03_set_scene_mode(UINT16 Para)
{
	//SENSORDB("BF3A03SetScenMode: %d\n",Para);
            printk("phl BF3A03_set_scene_mode Para = %d\n",Para);

	switch (Para)
	{                   
    case SCENE_MODE_NIGHTSCENE:		
		BF3A03_write_cmos_sensor(0x89, 0xa5);
      	break;
    case SCENE_MODE_OFF://SCENE_MODE_AUTO: 	
	  BF3A03_write_cmos_sensor(0x89, 0x7d); 
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
} /* BF3A03SetSceneMode */
UINT32 BF3A03YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
    switch (iCmd) {
    case FID_SCENE_MODE:
	  BF3A03_set_scene_mode(iPara);
            break; 
    case FID_AWB_MODE:
        BF3A03_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        BF3A03_set_param_effect(iPara);
        break;
    case FID_AE_EV:
        BF3A03_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
        BF3A03_set_param_banding(iPara);
        break;
    default:
        break;
    }
    return TRUE;
} /* BF3A03YUVSensorSetting */

void BF3A03GetExifInfo(UINT32 exifAddr)
{
    SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
    pExifInfo->FNumber = 28;
    pExifInfo->AEISOSpeed = BF3A03CurrentStatus.isoSpeed;
    pExifInfo->AWBMode = BF3A03CurrentStatus.iWB;
    pExifInfo->CapExposureTime = 0;
    pExifInfo->FlashLightTimeus = 0;
    pExifInfo->RealISOValue = BF3A03CurrentStatus.isoSpeed;
}


UINT32 BF3A03FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 BF3A03SensorRegNumber;
    UINT32 i;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

    RETAILMSG(1, (_T("gaiyang BF3A03FeatureControl FeatureId=%d\r\n"), FeatureId));

    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=IMAGE_SENSOR_FULL_WIDTH;
        *pFeatureReturnPara16=IMAGE_SENSOR_FULL_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        *pFeatureReturnPara16++=(694)+BF3A03_dummy_pixels;
        *pFeatureReturnPara16=(488)+BF3A03_dummy_lines;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        *pFeatureReturnPara32 = BF3A03_g_fPV_PCLK;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        BF3A03_night_mode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        BF3A03_isp_master_clock=*pFeatureData32;
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        BF3A03_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        pSensorRegData->RegData = BF3A03_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        memcpy(pSensorConfigData, &BF3A03SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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
        BF3A03YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;
	case SENSOR_FEATURE_GET_EXIF_INFO:
             SENSORDB("SENSOR_FEATURE_GET_EXIF_INFO\n");
             SENSORDB("EXIF addr = 0x%x\n",*pFeatureData32);
             BF3A03GetExifInfo(*pFeatureData32);
             break;	
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
	BF3A03GetSensorID(pFeatureData32);
	break;
    default:
        break;
	}
return ERROR_NONE;
}	/* BF3A03FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncBF3A03YUV=
{
	BF3A03Open,
	BF3A03GetInfo,
	BF3A03GetResolution,
	BF3A03FeatureControl,
	BF3A03Control,
	BF3A03Close
};

UINT32 BF3A03_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncBF3A03YUV;
	return ERROR_NONE;
} /* SensorInit() */
