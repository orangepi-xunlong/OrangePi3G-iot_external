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
/*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.c
 *
 * Project:
 * --------
 *  
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 *   lanking zhou
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * [HM0357YUV V1.0.0]
 * 10.30.2012 Lanking.zhou
 * .First Release
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by GalaxyCoreinc. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
//#include <windows.h>
//#include <memory.h>
//#include <nkintr.h>
//#include <ceddk.h>
//#include <ceddk_exp.h>

//#include "kal_release.h"
//#include "i2c_exp.h"
//#include "gpio_exp.h"
//#include "msdk_exp.h"
//#include "msdk_sensor_exp.h"
//#include "msdk_isp_exp.h"
//#include "base_regs.h"
//#include "Sensor.h"
//#include "camera_sensor_para.h"
//#include "CameraCustomized.h"

#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
//#include <mach/mt6516_pll.h>

#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"
#include "kd_camera_feature.h"

#include "hm0357yuv_Sensor.h"
#include "hm0357yuv_Camera_Sensor_para.h"
#include "hm0357yuv_CameraCustomized.h"

#define HM0357YUV_DEBUG
#ifdef HM0357YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif


//#define scaler_preview

extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
extern int getSensorIdx();
#define HM0357_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para ,1,HM0357_WRITE_ID)
#define HM0357_write_cmos_sensor_2(addr, para, bytes) iWriteReg((u16) addr , (u32) para ,bytes,HM0357_WRITE_ID)
kal_uint16 HM0357_read_cmos_sensor(kal_uint32 addr)
{
kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,HM0357_WRITE_ID);
    return get_byte;
}


extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
/*************************************************************************
* FUNCTION
*    HM0357_write_cmos_sensor
*
* DESCRIPTION
*    This function wirte data to CMOS sensor through I2C
*
* PARAMETERS
*    addr: the 16bit address of register
*    para: the 8bit value of register
*
* RETURNS
*    None
*
* LOCAL AFFECTED
*
*************************************************************************/
/*
static void HM0357_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
kal_uint8 out_buff[2];

    out_buff[0] = addr;
    out_buff[1] = para;

    iWriteRegI2C((u8*)out_buff , (u16)sizeof(out_buff), HM0357_WRITE_ID); 

#if (defined(__HM0357_DEBUG_TRACE__))
  if (sizeof(out_buff) != rt) printk("I2C write %x, %x error\n", addr, para);
#endif
}
*/

/*************************************************************************
* FUNCTION
*    HM0357_read_cmos_sensor
*
* DESCRIPTION
*    This function read data from CMOS sensor through I2C.
*
* PARAMETERS
*    addr: the 16bit address of register
*
* RETURNS
*    8bit data read through I2C
*
* LOCAL AFFECTED
*
*************************************************************************/
/*
static kal_uint8 HM0357_read_cmos_sensor(kal_uint8 addr)
{
  kal_uint8 in_buff[1] = {0xFF};
  kal_uint8 out_buff[1];
  
  out_buff[0] = addr;

    if (0 != iReadRegI2C((u8*)out_buff , (u16) sizeof(out_buff), (u8*)in_buff, (u16) sizeof(in_buff), 0x49)) {
        SENSORDB("ERROR: HM0357_read_cmos_sensor \n");
    }

#if (defined(__HM0357_DEBUG_TRACE__))
  if (size != rt) printk("I2C read %x error\n", addr);
#endif

  return in_buff[0];
}
*/

/*******************************************************************************
* // Adapter for Winmo typedef 
********************************************************************************/
#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT


/*******************************************************************************
* // End Adapter for Winmo typedef 
********************************************************************************/
/* Global Valuable */

static kal_uint32 zoom_factor = 0; 

static kal_bool HM0357_VEDIO_encode_mode = KAL_FALSE; //Picture(Jpeg) or Video(Mpeg4)
static kal_bool HM0357_sensor_cap_state = KAL_FALSE; //Preview or Capture

static kal_uint16 HM0357_exposure_lines=0, HM0357_extra_exposure_lines = 0;

static kal_uint16 HM0357_Capture_Shutter=0;
static kal_uint16 HM0357_Capture_Extra_Lines=0;

kal_uint32 HM0357_PV_dummy_pixels = 0,  HM0357_PV_dummy_lines = 0 ,HM0357_CAP_dummy_pixels = 0, HM0357_isp_master_clock=0;//DUMMY = 1000;

static kal_uint32  HM0357_sensor_pclk=390;

static kal_int8 HM0357_DELAY_AFTER_PREVIEW = -1;

static kal_uint32 Preview_Shutter = 0;
static kal_uint32 Capture_Shutter = 0;



MSDK_SENSOR_CONFIG_STRUCT HM0357SensorConfigData;

kal_uint16 HM0357_read_shutter(void)
{
	return  (HM0357_read_cmos_sensor(0x03) << 8)|HM0357_read_cmos_sensor(0x04) ;
} /* HM0357 read_shutter */



static void HM0357_write_shutter(kal_uint32 shutter)
{

	if(shutter < 1)	
 	return;

	//HM0357_write_cmos_sensor(0x03, (shutter >> 8) & 0xff);
	//HM0357_write_cmos_sensor(0x04, shutter & 0xff);
}    /* HM0357_write_shutter */




static void HM0357_set_mirror_flip(kal_uint8 image_mirror)
{
	kal_uint8 HM0357_HV_Mirror;

	switch (image_mirror) 
	{
		case IMAGE_NORMAL:
			//HM0357_HV_Mirror = 0x00; 
		    break;
		case IMAGE_H_MIRROR:
			//HM0357_HV_Mirror = 0x01;
		    break;
		case IMAGE_V_MIRROR:
			//HM0357_HV_Mirror = 0x02; 
		    break;
		case IMAGE_HV_MIRROR:
			//HM0357_HV_Mirror = 0x03;
		    break;
		default:
		    break;
	}
	//HM0357_write_cmos_sensor(0x06, HM0357_HV_Mirror);
}

static void HM0357_set_AE_mode(kal_bool AE_enable)
{
	printk("%s\n", __func__);
	
	kal_uint16 temp_AE_reg = 0;
	//temp_AE_reg = HM0357_read_cmos_sensor(0x0380);
	
	if (AE_enable)	  
	{
		//HM0357_write_cmos_sensor(0x0380, (temp_AE_reg| 0x01));	/* Turn ON AEC/AGC*/
	}
	else
	{
		//HM0357_write_cmos_sensor(0x0380, (temp_AE_reg&(~0x01))); /* Turn OFF AEC/AGC*/
	}

	//HM0357_write_cmos_sensor(0x0000,0xFF);	
	//HM0357_write_cmos_sensor(0x0100,0xFF);
	//HM0357_write_cmos_sensor(0x0101,0xFF);

	return KAL_TRUE;	
}
 

static void HM0357_set_AWB_mode(kal_bool AWB_enable)
{
	printk("%s\n", __func__);
    kal_uint8 temp_AWB_reg = 0;
    temp_AWB_reg = HM0357_read_cmos_sensor(0x0380);
    //return ;

    if (AWB_enable == KAL_TRUE)
    {
        //enable Auto WB
		//HM0357_write_cmos_sensor(0x0380, (temp_AWB_reg | 0x02));
	  }
	else
		{
		//HM0357_write_cmos_sensor(0x0380, (temp_AWB_reg& ~0x02));		
	 }

		//HM0357_write_cmos_sensor(0x0000,0xFF);	
		//HM0357_write_cmos_sensor(0x0100,0xFF);
		//HM0357_write_cmos_sensor(0x0101,0xFF);       
    return KAL_TRUE;
}
/*
static void HM0357_set_BLK_mode(kal_bool BLK_enable)
{

	HM0357_write_cmos_sensor(0xfe, 0x00);
	if (BLK_enable == KAL_TRUE)
	{
		//enable BLK
		HM0357_write_cmos_sensor(0x40, 0x77);
	}
	else
	{
		//turn off BLK
		HM0357_write_cmos_sensor(0x40, 0x76);
	}
}
*/

/*************************************************************************
* FUNCTION
*	HM0357_night_mode
*
* DESCRIPTION
*	This function night mode of HM0357.
*
* PARAMETERS
*	none
*
* RETURNS
*	None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void HM0357_night_mode(kal_bool enable)
{
	printk("%s\n", __func__);
	if (enable) 		/* Night Mode */
	{
		/* HM0357 night mode enable. */
		if (HM0357_VEDIO_encode_mode == KAL_TRUE)	/* Video */
		{
			//HM0357_write_cmos_sensor(0x038F,0x0A);	//Min 4 FPS.
			//HM0357_write_cmos_sensor(0x0390,0x00);
		}
		else 										/* Camera */
		{
			//HM0357_write_cmos_sensor(0x038F,0x0A);	//Min 4 FPS.
			//HM0357_write_cmos_sensor(0x0390,0x00);
		}			
    
		//HM0357_write_cmos_sensor(0x02E0,0x00);	// 00 for Night Mode, By Brandon/20110208
		//HM0357_write_cmos_sensor(0x0481,0x06);	// 06 for Night Mode, By Brandon/20110208
		//HM0357_write_cmos_sensor(0x04B1,0x88);	// 88 for Night Mode, By Brandon/20110208
		//HM0357_write_cmos_sensor(0x04B4,0x20);	// 20 for Night Mode, By Brandon/20110208
		//HM0357_write_cmos_sensor(0x0000,0xFF);
		//HM0357_write_cmos_sensor(0x0100,0xFF);  
	}
	else  				/* Normal Mode */
	{
		/* HM0357 night mode disable. */
		if (HM0357_VEDIO_encode_mode == KAL_TRUE)	/* Video */
		{
			//HM0357_write_cmos_sensor(0x038F,0x05);	//Min 8 FPS.
			//HM0357_write_cmos_sensor(0x0390,0x04);
		}
		else										/* Camera */
		{
    	//HM0357_write_cmos_sensor(0x038F,0x05);	//Min 8 FPS.
			//HM0357_write_cmos_sensor(0x0390,0x04);
		}		
			//HM0357_write_cmos_sensor(0x02E0,0x02);	//06->02, By Brandon/20110129
			//HM0357_write_cmos_sensor(0x0481,0x08);	//06->08, By Brandon/20110129
			//HM0357_write_cmos_sensor(0x04B1,0x00);
			//HM0357_write_cmos_sensor(0x04B4,0x00);
			//HM0357_write_cmos_sensor(0x0000,0xFF);
			//HM0357_write_cmos_sensor(0x0100,0xFF);
	}
}	/* HM0357_night_mode */



/*************************************************************************
* FUNCTION
*	HM0357_GetSensorID
*
* DESCRIPTION
*	This function get the sensor ID
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

static kal_uint32 HM0357_GetSensorID(kal_uint32 *sensorID)

{
    int  retry = 3; 
    // check if sensor ID correct
    SENSORDB("HM0357 start GetSensorID\n"); 
    do {
        *sensorID=((HM0357_read_cmos_sensor(0x0001)<< 8)|HM0357_read_cmos_sensor(0x0002));
        if (*sensorID == HM0357_YUV_SENSOR_ID)
            break; 
        SENSORDB("HM0357 Read Sensor ID Fail = 0x%04x\n", *sensorID); 
        retry--; 
    } while (retry > 0);

    if (*sensorID != HM0357_YUV_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;    
}   /* HM0357Open  */

static void HM0357_Sensor_Init(void)
{
	zoom_factor = 0; 
	SENSORDB("HM0357_Sensor_Init");  		
		HM0357_write_cmos_sensor(0x0022,0x00);
		HM0357_write_cmos_sensor(0x0020,0x40);
		HM0357_write_cmos_sensor(0x0023,0x5C);//58
		HM0357_write_cmos_sensor(0x008F,0x00);
		HM0357_write_cmos_sensor(0x0004,0x11);
		HM0357_write_cmos_sensor(0x0006,0x03);//by jink 20140626		
		HM0357_write_cmos_sensor(0x000F,0x00);
		HM0357_write_cmos_sensor(0x0012,0x09);
		HM0357_write_cmos_sensor(0x0013,0x03);
		HM0357_write_cmos_sensor(0x0015,0x02);
		HM0357_write_cmos_sensor(0x0016,0x01);
		HM0357_write_cmos_sensor(0x0025,0x00);
		HM0357_write_cmos_sensor(0x0027,0x38);//10->30->38 by jink 20140626
		HM0357_write_cmos_sensor(0x0040,0x0F);
		HM0357_write_cmos_sensor(0x0053,0x0F);
		HM0357_write_cmos_sensor(0x0075,0x40);
		HM0357_write_cmos_sensor(0x0041,0x02);
		HM0357_write_cmos_sensor(0x0045,0xCA);
		HM0357_write_cmos_sensor(0x0046,0x4F);
		HM0357_write_cmos_sensor(0x004A,0x0A);
		HM0357_write_cmos_sensor(0x004B,0x72);
		HM0357_write_cmos_sensor(0x004D,0xBF);
		HM0357_write_cmos_sensor(0x004E,0x30);
		HM0357_write_cmos_sensor(0x0055,0x10);
		HM0357_write_cmos_sensor(0x0053,0x00);
		HM0357_write_cmos_sensor(0x0070,0x44);
		HM0357_write_cmos_sensor(0x0071,0xB9);
		HM0357_write_cmos_sensor(0x0072,0x55);
		HM0357_write_cmos_sensor(0x0073,0x10);
		HM0357_write_cmos_sensor(0x0081,0xD2);
		HM0357_write_cmos_sensor(0x0082,0xA6);
		HM0357_write_cmos_sensor(0x0083,0x70);
		HM0357_write_cmos_sensor(0x0085,0x11);
		HM0357_write_cmos_sensor(0x0086,0xA7);
		HM0357_write_cmos_sensor(0x0088,0xE1);
		HM0357_write_cmos_sensor(0x008A,0x2D);
		HM0357_write_cmos_sensor(0x008D,0x20);
		HM0357_write_cmos_sensor(0x0090,0x00);
		HM0357_write_cmos_sensor(0x0091,0x10);
		HM0357_write_cmos_sensor(0x0092,0x11);
		HM0357_write_cmos_sensor(0x0093,0x12);
		HM0357_write_cmos_sensor(0x0094,0x13);
		HM0357_write_cmos_sensor(0x0095,0x17);
		HM0357_write_cmos_sensor(0x00A0,0xC0);
		HM0357_write_cmos_sensor(0x011F,0x44);
		HM0357_write_cmos_sensor(0x0120,0x36);
		HM0357_write_cmos_sensor(0x0121,0x80);
		HM0357_write_cmos_sensor(0x0122,0x6B);
		HM0357_write_cmos_sensor(0x0123,0xA5);
		HM0357_write_cmos_sensor(0x0124,0xD2);
		HM0357_write_cmos_sensor(0x0125,0xDE);
		HM0357_write_cmos_sensor(0x0126,0x71);
		HM0357_write_cmos_sensor(0x0140,0x14);
		HM0357_write_cmos_sensor(0x0141,0x0A);
		HM0357_write_cmos_sensor(0x0142,0x14);
		HM0357_write_cmos_sensor(0x0143,0x0A);
		HM0357_write_cmos_sensor(0x0144,0x0C);
		HM0357_write_cmos_sensor(0x0145,0x03);
		HM0357_write_cmos_sensor(0x0146,0x40);
		HM0357_write_cmos_sensor(0x0147,0x0A);
		HM0357_write_cmos_sensor(0x0148,0x70);
		HM0357_write_cmos_sensor(0x0149,0x0C);
		HM0357_write_cmos_sensor(0x014A,0x80);
		HM0357_write_cmos_sensor(0x014B,0x80);
		HM0357_write_cmos_sensor(0x014C,0x80);
		HM0357_write_cmos_sensor(0x014D,0x2E);
		HM0357_write_cmos_sensor(0x014E,0x05);
		HM0357_write_cmos_sensor(0x014F,0x05);
		HM0357_write_cmos_sensor(0x0150,0x0D);
		HM0357_write_cmos_sensor(0x0155,0x00);
		HM0357_write_cmos_sensor(0x0156,0x0A);
		HM0357_write_cmos_sensor(0x0157,0x0A);
		HM0357_write_cmos_sensor(0x0158,0x0A);
		HM0357_write_cmos_sensor(0x0159,0x0A);
		HM0357_write_cmos_sensor(0x0160,0x14);
		HM0357_write_cmos_sensor(0x0161,0x14);
		HM0357_write_cmos_sensor(0x0162,0x0A);
		HM0357_write_cmos_sensor(0x0163,0x0A);
		HM0357_write_cmos_sensor(0x01B0,0x33);
		HM0357_write_cmos_sensor(0x01B1,0x10);
		HM0357_write_cmos_sensor(0x01B2,0x10);
		HM0357_write_cmos_sensor(0x01B3,0x30);
		HM0357_write_cmos_sensor(0x01B4,0x18);
		HM0357_write_cmos_sensor(0x01D8,0x50);
		HM0357_write_cmos_sensor(0x01DE,0x70);
		HM0357_write_cmos_sensor(0x01E4,0x08);
		HM0357_write_cmos_sensor(0x01E5,0x08);
		HM0357_write_cmos_sensor(0x0220,0x00);
		HM0357_write_cmos_sensor(0x0221,0xBD);
		HM0357_write_cmos_sensor(0x0222,0x00);
		HM0357_write_cmos_sensor(0x0223,0x80);
		HM0357_write_cmos_sensor(0x0224,0x8A);
		HM0357_write_cmos_sensor(0x0225,0x00);
		HM0357_write_cmos_sensor(0x0226,0x80);
		HM0357_write_cmos_sensor(0x0227,0x8A);
		HM0357_write_cmos_sensor(0x0229,0x80);
		HM0357_write_cmos_sensor(0x022A,0x80);
		HM0357_write_cmos_sensor(0x022B,0x00);
		HM0357_write_cmos_sensor(0x022C,0x80);
		HM0357_write_cmos_sensor(0x022D,0x11);
		HM0357_write_cmos_sensor(0x022E,0x10);
		HM0357_write_cmos_sensor(0x022F,0x11);
		HM0357_write_cmos_sensor(0x0230,0x10);
		HM0357_write_cmos_sensor(0x0231,0x11);
		HM0357_write_cmos_sensor(0x0233,0x11);
		HM0357_write_cmos_sensor(0x0234,0x10);
		HM0357_write_cmos_sensor(0x0235,0x46);
		HM0357_write_cmos_sensor(0x0236,0x01);
		HM0357_write_cmos_sensor(0x0237,0x46);
		HM0357_write_cmos_sensor(0x0238,0x01);
		HM0357_write_cmos_sensor(0x023B,0x46);
		HM0357_write_cmos_sensor(0x023C,0x01);
		HM0357_write_cmos_sensor(0x023D,0xF8);
		HM0357_write_cmos_sensor(0x023E,0x00);
		HM0357_write_cmos_sensor(0x023F,0xF8);
		HM0357_write_cmos_sensor(0x0240,0x00);
		HM0357_write_cmos_sensor(0x0243,0xF8);
		HM0357_write_cmos_sensor(0x0244,0x00);
		HM0357_write_cmos_sensor(0x0250,0x03);
		HM0357_write_cmos_sensor(0x0251,0x0D);
		HM0357_write_cmos_sensor(0x0252,0x08);
		HM0357_write_cmos_sensor(0x0280,0x0B);
		HM0357_write_cmos_sensor(0x0282,0x15);
		HM0357_write_cmos_sensor(0x0284,0x2A);
		HM0357_write_cmos_sensor(0x0286,0x4C);
		HM0357_write_cmos_sensor(0x0288,0x5A);
		HM0357_write_cmos_sensor(0x028A,0x67);
		HM0357_write_cmos_sensor(0x028C,0x73);
		HM0357_write_cmos_sensor(0x028E,0x7D);
		HM0357_write_cmos_sensor(0x0290,0x86);
		HM0357_write_cmos_sensor(0x0292,0x8E);
		HM0357_write_cmos_sensor(0x0294,0x9E);
		HM0357_write_cmos_sensor(0x0296,0xAC);
		HM0357_write_cmos_sensor(0x0298,0xC0);
		HM0357_write_cmos_sensor(0x029A,0xD2);
		HM0357_write_cmos_sensor(0x029C,0xE2);
		HM0357_write_cmos_sensor(0x029E,0x27);
		HM0357_write_cmos_sensor(0x02A0,0x04);
		HM0357_write_cmos_sensor(0x02C0,0xCC);
		HM0357_write_cmos_sensor(0x02C1,0x01);
		HM0357_write_cmos_sensor(0x02C2,0xBB);
		HM0357_write_cmos_sensor(0x02C3,0x04);
		HM0357_write_cmos_sensor(0x02C4,0x11);
		HM0357_write_cmos_sensor(0x02C5,0x04);
		HM0357_write_cmos_sensor(0x02C6,0x2E);
		HM0357_write_cmos_sensor(0x02C7,0x04);
		HM0357_write_cmos_sensor(0x02C8,0x6C);
		HM0357_write_cmos_sensor(0x02C9,0x01);
		HM0357_write_cmos_sensor(0x02CA,0x3E);
		HM0357_write_cmos_sensor(0x02CB,0x04);
		HM0357_write_cmos_sensor(0x02CC,0x04);
		HM0357_write_cmos_sensor(0x02CD,0x04);
		HM0357_write_cmos_sensor(0x02CE,0xBE);
		HM0357_write_cmos_sensor(0x02CF,0x04);
		HM0357_write_cmos_sensor(0x02D0,0xC2);
		HM0357_write_cmos_sensor(0x02D1,0x01);
		HM0357_write_cmos_sensor(0x02F0,0xA3);
		HM0357_write_cmos_sensor(0x02F1,0x04);
		HM0357_write_cmos_sensor(0x02F2,0xE9);
		HM0357_write_cmos_sensor(0x02F3,0x00);
		HM0357_write_cmos_sensor(0x02F4,0x45);
		HM0357_write_cmos_sensor(0x02F5,0x04);
		HM0357_write_cmos_sensor(0x02F6,0x1D);
		HM0357_write_cmos_sensor(0x02F7,0x04);
		HM0357_write_cmos_sensor(0x02F8,0x45);
		HM0357_write_cmos_sensor(0x02F9,0x04);
		HM0357_write_cmos_sensor(0x02FA,0x63);
		HM0357_write_cmos_sensor(0x02FB,0x00);
		HM0357_write_cmos_sensor(0x02FC,0x3E);
		HM0357_write_cmos_sensor(0x02FD,0x04);
		HM0357_write_cmos_sensor(0x02FE,0x39);
		HM0357_write_cmos_sensor(0x02FF,0x04);
		HM0357_write_cmos_sensor(0x0300,0x79);
		HM0357_write_cmos_sensor(0x0301,0x00);
		HM0357_write_cmos_sensor(0x0328,0x00);
		HM0357_write_cmos_sensor(0x0329,0x04);
		HM0357_write_cmos_sensor(0x032D,0x66);
		HM0357_write_cmos_sensor(0x032E,0x01);
		HM0357_write_cmos_sensor(0x032F,0x00);
		HM0357_write_cmos_sensor(0x0330,0x01);
		HM0357_write_cmos_sensor(0x0331,0x66);
		HM0357_write_cmos_sensor(0x0332,0x01);
		HM0357_write_cmos_sensor(0x0333,0x00);
		HM0357_write_cmos_sensor(0x0334,0x00);
		HM0357_write_cmos_sensor(0x0335,0x00);
		HM0357_write_cmos_sensor(0x033E,0x00);
		HM0357_write_cmos_sensor(0x033F,0x00);
		HM0357_write_cmos_sensor(0x0340,0x38);
		HM0357_write_cmos_sensor(0x0341,0x41);
		HM0357_write_cmos_sensor(0x0342,0x04);
		HM0357_write_cmos_sensor(0x0343,0x2C);
		HM0357_write_cmos_sensor(0x0344,0x80);
		HM0357_write_cmos_sensor(0x0345,0x37);
		HM0357_write_cmos_sensor(0x0346,0x80);
		HM0357_write_cmos_sensor(0x0347,0x3D);
		HM0357_write_cmos_sensor(0x0348,0x76);
		HM0357_write_cmos_sensor(0x0349,0x42);
		HM0357_write_cmos_sensor(0x034A,0x6B);
		HM0357_write_cmos_sensor(0x034B,0x68);
		HM0357_write_cmos_sensor(0x034C,0x58);
		HM0357_write_cmos_sensor(0x0350,0x90);
		HM0357_write_cmos_sensor(0x0351,0x90);
		HM0357_write_cmos_sensor(0x0352,0x18);
		HM0357_write_cmos_sensor(0x0353,0x18);
		HM0357_write_cmos_sensor(0x0354,0x73);
		HM0357_write_cmos_sensor(0x0355,0x45);
		HM0357_write_cmos_sensor(0x0356,0x78);
		HM0357_write_cmos_sensor(0x0357,0xD0);
		HM0357_write_cmos_sensor(0x0358,0x05);
		HM0357_write_cmos_sensor(0x035A,0x05);
		HM0357_write_cmos_sensor(0x035B,0xA0);
		HM0357_write_cmos_sensor(0x0381,0x5c);
		HM0357_write_cmos_sensor(0x0382,0x44);
		HM0357_write_cmos_sensor(0x0383,0x20);
		HM0357_write_cmos_sensor(0x038A,0x80);
		HM0357_write_cmos_sensor(0x038B,0x10);
		HM0357_write_cmos_sensor(0x038C,0xD1);
		HM0357_write_cmos_sensor(0x038E,0x50);
		HM0357_write_cmos_sensor(0x038F,0x05);
		HM0357_write_cmos_sensor(0x0390,0xa0);
		HM0357_write_cmos_sensor(0x0391,0x7C);
		HM0357_write_cmos_sensor(0x0393,0x80);
		HM0357_write_cmos_sensor(0x0395,0x12);
		HM0357_write_cmos_sensor(0x0398,0x01);
		HM0357_write_cmos_sensor(0x0399,0xF0);
		HM0357_write_cmos_sensor(0x039A,0x03);
		HM0357_write_cmos_sensor(0x039B,0x00);
		HM0357_write_cmos_sensor(0x039C,0x04);
		HM0357_write_cmos_sensor(0x039D,0x00);
		HM0357_write_cmos_sensor(0x039E,0x06);
		HM0357_write_cmos_sensor(0x039F,0x00);
		HM0357_write_cmos_sensor(0x03A0,0x09);
		HM0357_write_cmos_sensor(0x03A1,0x50);
		HM0357_write_cmos_sensor(0x03A6,0x10);
		HM0357_write_cmos_sensor(0x03A7,0x10);
		HM0357_write_cmos_sensor(0x03A8,0x36);
		HM0357_write_cmos_sensor(0x03A9,0x40);
		HM0357_write_cmos_sensor(0x03AE,0x26);
		HM0357_write_cmos_sensor(0x03AF,0x22);
		HM0357_write_cmos_sensor(0x03B0,0x0A);
		HM0357_write_cmos_sensor(0x03B1,0x08);
		HM0357_write_cmos_sensor(0x03B3,0x00);
		HM0357_write_cmos_sensor(0x03B5,0x08);
		HM0357_write_cmos_sensor(0x03B7,0xA0);
		HM0357_write_cmos_sensor(0x03B9,0xD0);
		HM0357_write_cmos_sensor(0x03BB,0x5F);
		HM0357_write_cmos_sensor(0x03BE,0x00);
		HM0357_write_cmos_sensor(0x03BF,0x1D);
		HM0357_write_cmos_sensor(0x03C0,0x2E);
		HM0357_write_cmos_sensor(0x03C3,0x0F);
		HM0357_write_cmos_sensor(0x03D0,0xE0);
		HM0357_write_cmos_sensor(0x0420,0x86);
		HM0357_write_cmos_sensor(0x0421,0x00);
		HM0357_write_cmos_sensor(0x0422,0x00);
		HM0357_write_cmos_sensor(0x0423,0x00);
		HM0357_write_cmos_sensor(0x0430,0x00);
		HM0357_write_cmos_sensor(0x0431,0x60);
		HM0357_write_cmos_sensor(0x0432,0x30);
		HM0357_write_cmos_sensor(0x0433,0x30);
		HM0357_write_cmos_sensor(0x0434,0x00);
		HM0357_write_cmos_sensor(0x0435,0x40);
		HM0357_write_cmos_sensor(0x0436,0x00);
		HM0357_write_cmos_sensor(0x0450,0xFF);
		HM0357_write_cmos_sensor(0x0451,0xFF);
		HM0357_write_cmos_sensor(0x0452,0xB0);
		HM0357_write_cmos_sensor(0x0453,0x70);
		HM0357_write_cmos_sensor(0x0454,0x00);
		HM0357_write_cmos_sensor(0x045A,0x00);
		HM0357_write_cmos_sensor(0x045B,0x10);
		HM0357_write_cmos_sensor(0x045C,0x00);
		HM0357_write_cmos_sensor(0x045D,0xA0);
		HM0357_write_cmos_sensor(0x0465,0x02);
		HM0357_write_cmos_sensor(0x0466,0x04);
		HM0357_write_cmos_sensor(0x047A,0x20);
		HM0357_write_cmos_sensor(0x047B,0x20);
		HM0357_write_cmos_sensor(0x0480,0x40);//saturation
		HM0357_write_cmos_sensor(0x0481,0x06);
		HM0357_write_cmos_sensor(0x04B0,0x46);//contrast 
		HM0357_write_cmos_sensor(0x04B1,0x00);
		HM0357_write_cmos_sensor(0x04B3,0x7F);
		HM0357_write_cmos_sensor(0x04B4,0x00);
		HM0357_write_cmos_sensor(0x04B6,0x20);
		HM0357_write_cmos_sensor(0x04B9,0x40);
		HM0357_write_cmos_sensor(0x0540,0x00);
		HM0357_write_cmos_sensor(0x0541,0x78);
		HM0357_write_cmos_sensor(0x0542,0x00);
		HM0357_write_cmos_sensor(0x0543,0x90);
		HM0357_write_cmos_sensor(0x0580,0x08);//denoise
		HM0357_write_cmos_sensor(0x0581,0x15);//low light denoise
		HM0357_write_cmos_sensor(0x0590,0x20);
		HM0357_write_cmos_sensor(0x0591,0x30);
		HM0357_write_cmos_sensor(0x0594,0x02);
		HM0357_write_cmos_sensor(0x0595,0x08);
		HM0357_write_cmos_sensor(0x05A0,0x08);
		HM0357_write_cmos_sensor(0x05A1,0x0C);
		HM0357_write_cmos_sensor(0x05A2,0x50);
		HM0357_write_cmos_sensor(0x05A3,0x60);
		HM0357_write_cmos_sensor(0x05B0,0x10);//sharpness 18->28->20 by jink 20140630
		HM0357_write_cmos_sensor(0x05B1,0x06);
		HM0357_write_cmos_sensor(0x05D0,0x2D);
		HM0357_write_cmos_sensor(0x05D1,0x07);
		HM0357_write_cmos_sensor(0x05E4,0x04);
		HM0357_write_cmos_sensor(0x05E5,0x00);
		HM0357_write_cmos_sensor(0x05E6,0x83);
		HM0357_write_cmos_sensor(0x05E7,0x02);
		HM0357_write_cmos_sensor(0x05E8,0x04);
		HM0357_write_cmos_sensor(0x05E9,0x00);
		HM0357_write_cmos_sensor(0x05EA,0xE3);
		HM0357_write_cmos_sensor(0x05EB,0x01);

		/* add by jst for r6372 alx lca project begin */
		if (getSensorIdx()==0)
		{
			//switch direction
			HM0357_write_cmos_sensor(0x06, 0x03);
		}
		else
		{
			//switch direction
			HM0357_write_cmos_sensor(0x06, 0x00);
		}
		/* add by jst for r6372 alx lca project end */
		
		HM0357_write_cmos_sensor(0x0000,0x01);
		HM0357_write_cmos_sensor(0x0100,0x01);
		HM0357_write_cmos_sensor(0x0101,0x01);
		HM0357_write_cmos_sensor(0x0005,0x01);

}
  
static void HM0357_Preview_setting(void)
{
	SENSORDB("HM0357_Preview_setting");

}

static void HM0357_Capture_setting(void)
{
	kal_uint8 temp = 0;
	SENSORDB("HM0357_Capture_setting");
}


static void HM0357_Write_More(void)
{
  //////////////For FAE ////////////////

}

/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/**************************************************** *************************/
/*************************************************************************
* FUNCTION
*	HM0357Open
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
UINT32 HM0357Open(void)
{
	volatile signed char i;
	kal_uint16 sensor_id=0;

	zoom_factor = 0; 
	Sleep(10);
	HM0357_write_cmos_sensor(0x0022,0x01);// Reset sensor
	mDELAY(10);

	//  Read sensor ID to adjust I2C is OK?
	for(i=0;i<3;i++)
	{
		sensor_id = (HM0357_read_cmos_sensor(0x0001) << 8) | HM0357_read_cmos_sensor(0x0002);
		if(sensor_id != HM0357_YUV_SENSOR_ID)  // HM0357_SENSOR_ID = 0x2035
		{
			SENSORDB("HM0357 Sensor Read ID ERR \r\n");
			return ERROR_SENSOR_CONNECT_FAIL;
		}
	}
	
	SENSORDB("HM0357 Sensor Read ID OK \r\n");
	HM0357_Sensor_Init();
	HM0357_Write_More();
	Preview_Shutter =HM0357_read_shutter();

	return ERROR_NONE;
}	/* HM0357Open() */

/*************************************************************************
* FUNCTION
*	HM0357Close
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
UINT32 HM0357Close(void)
{
//	CISModulePowerOn(FALSE);

	return ERROR_NONE;
}	/* HM0357Close() */

/*************************************************************************
* FUNCTION
*	HM0357Preview
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
UINT32 HM0357Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
	kal_uint8 iTemp, temp_AE_reg, temp_AWB_reg;
	kal_uint16 iStartX = 0, iStartY = 0;

	SENSORDB("HM0357Previe\n");

	HM0357_sensor_cap_state = KAL_FALSE;

	HM0357_Preview_setting();
	HM0357_set_mirror_flip(IMAGE_NORMAL);
	//HM0357_write_shutter(Preview_Shutter);
	HM0357_set_AE_mode(KAL_TRUE); 
	
	// copy sensor_config_data
	memcpy(&HM0357SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}	/* HM0357Preview() */




UINT32 HM0357Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
		volatile kal_uint32 shutter = 0;

    		SENSORDB("HM0357Capture\n");
			
    	if(HM0357_sensor_cap_state == KAL_FALSE)
	  {

		HM0357_set_AE_mode(KAL_FALSE); 

		//shutter = HM0357_read_shutter();

		//Preview_Shutter = shutter;

		HM0357_Capture_setting();  ////  2M_setting

    //shutter = shutter / 2;

	  //Capture_Shutter = shutter;

		//HM0357_write_shutter(Capture_Shutter);

		Sleep(100);
    	  }
 
		HM0357_sensor_cap_state = KAL_TRUE;  

		 /* 2M FULL Mode */
		SENSORDB("HM0357Capture 2M\n");
		image_window->GrabStartX=1;
		image_window->GrabStartY=1;
		image_window->ExposureWindowWidth=HM0357_IMAGE_SENSOR_FULL_WIDTH - image_window->GrabStartX - 2;
		image_window->ExposureWindowHeight=HM0357_IMAGE_SENSOR_FULL_HEIGHT -image_window->GrabStartY - 2;    	 
		HM0357_DELAY_AFTER_PREVIEW = 4;
		memcpy(&HM0357SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
		return ERROR_NONE;
}	/* HM0357Capture() */




UINT32 HM0357GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
	pSensorResolution->SensorFullWidth=HM0357_IMAGE_SENSOR_FULL_WIDTH - 2 * IMAGE_SENSOR_START_GRAB_X;
	pSensorResolution->SensorFullHeight=HM0357_IMAGE_SENSOR_FULL_HEIGHT - 2 * IMAGE_SENSOR_START_GRAB_Y;
	pSensorResolution->SensorPreviewWidth=HM0357_IMAGE_SENSOR_PV_WIDTH - 2 * IMAGE_SENSOR_START_GRAB_X;
	pSensorResolution->SensorPreviewHeight=HM0357_IMAGE_SENSOR_PV_HEIGHT - 2 * IMAGE_SENSOR_START_GRAB_Y;
	pSensorResolution->SensorVideoWidth=HM0357_IMAGE_SENSOR_PV_WIDTH - 2 * IMAGE_SENSOR_START_GRAB_X;
	pSensorResolution->SensorVideoHeight=HM0357_IMAGE_SENSOR_PV_HEIGHT - 2 * IMAGE_SENSOR_START_GRAB_Y;

	return ERROR_NONE;
}	/* HM0357GetResolution() */

UINT32 HM0357GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	pSensorInfo->SensorPreviewResolutionX=HM0357_IMAGE_SENSOR_PV_WIDTH;
	pSensorInfo->SensorPreviewResolutionY=HM0357_IMAGE_SENSOR_PV_HEIGHT;
	pSensorInfo->SensorFullResolutionX=HM0357_IMAGE_SENSOR_FULL_WIDTH;
	pSensorInfo->SensorFullResolutionY=HM0357_IMAGE_SENSOR_FULL_HEIGHT;

	pSensorInfo->SensorCameraPreviewFrameRate=30;
	pSensorInfo->SensorVideoFrameRate=30;
	pSensorInfo->SensorStillCaptureFrameRate=10;
	pSensorInfo->SensorWebCamCaptureFrameRate=15;
	pSensorInfo->SensorResetActiveHigh=FALSE;
	pSensorInfo->SensorResetDelayCount=1;
	pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
	pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	/*??? */
	pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
	pSensorInfo->SensorInterruptDelayLines = 1;
	pSensorInfo->CaptureDelayFrame = 4; 
	pSensorInfo->PreviewDelayFrame = 2; 
	pSensorInfo->VideoDelayFrame = 0; 
	pSensorInfo->SensorMasterClockSwitch = 0; 
	pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_6MA;
	pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;
	
/*	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=FALSE;

	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=TRUE;
	pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=FALSE;	*/

	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
	//	case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
      pSensorInfo->SensorGrabStartX = 2; 
      pSensorInfo->SensorGrabStartY = 2;
	
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
	//	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount= 0;
			pSensorInfo->SensorClockFallingCount= 2;
			pSensorInfo->SensorPixelClockCount= 3;
			pSensorInfo->SensorDataLatchCount= 2;
      pSensorInfo->SensorGrabStartX = 2; 
      pSensorInfo->SensorGrabStartY = 2;			
		break;
		default:
			pSensorInfo->SensorClockFreq=24;
			pSensorInfo->SensorClockDividCount=3;
			pSensorInfo->SensorClockRisingCount=0;
			pSensorInfo->SensorClockFallingCount=2;
			pSensorInfo->SensorPixelClockCount=3;
			pSensorInfo->SensorDataLatchCount=2;
      pSensorInfo->SensorGrabStartX = 2;   
      pSensorInfo->SensorGrabStartY = 2;             
			
		break;
	}
	memcpy(pSensorConfigData, &HM0357SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
	return ERROR_NONE;
}	/* HM0357GetInfo() */


UINT32 HM0357Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
	switch (ScenarioId)
	{
		case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
		case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
	//	case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
			HM0357Preview(pImageWindow, pSensorConfigData);
		break;
		case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
	//	case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
			HM0357Capture(pImageWindow, pSensorConfigData);
		break;
		default:
		    break; 
	}
	return TRUE;
}	/* HM0357Control() */

BOOL HM0357_set_param_wb(UINT16 para)
{

	switch (para)
	{
		case AWB_MODE_AUTO:
 		        HM0357_write_cmos_sensor(0x0380, 0xff);            
		        HM0357_write_cmos_sensor(0x0100, 0xff);
		break;
		case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy   
			HM0357_write_cmos_sensor(0x0380, 0xFD); 		   
			HM0357_write_cmos_sensor(0x032D, 0x20); 		   
			HM0357_write_cmos_sensor(0x032E, 0x01); 		   
			HM0357_write_cmos_sensor(0x032F, 0x00); 		   
			HM0357_write_cmos_sensor(0x0330, 0x01); 		   
			HM0357_write_cmos_sensor(0x0331, 0x00); 		   
			HM0357_write_cmos_sensor(0x0332, 0x01);
		       HM0357_write_cmos_sensor(0x0000,0x01);//																									  
			HM0357_write_cmos_sensor(0x0100,0x01);// 																									
			HM0357_write_cmos_sensor(0x0101,0x01);// 
			HM0357_write_cmos_sensor(0x0100, 0xFF);   

		break;
		case AWB_MODE_DAYLIGHT: //sunny    
			HM0357_write_cmos_sensor(0x0380, 0xfd); 		   
			HM0357_write_cmos_sensor(0x032D, 0x1e); 		   
			HM0357_write_cmos_sensor(0x032E, 0x01); 		   
			HM0357_write_cmos_sensor(0x032F, 0x00); 		   
			HM0357_write_cmos_sensor(0x0330, 0x01); 		   
			HM0357_write_cmos_sensor(0x0331, 0x01); 		   
			HM0357_write_cmos_sensor(0x0332, 0x01);
			HM0357_write_cmos_sensor(0x0000, 0x01);//																									   
			HM0357_write_cmos_sensor(0x0100, 0x01);//																									 
			HM0357_write_cmos_sensor(0x0101, 0x01);// 
			HM0357_write_cmos_sensor(0x0100, 0xFF); 					
			HM0357_write_cmos_sensor(0x0100, 0xff);   

		break;
		case AWB_MODE_INCANDESCENT: //office
			HM0357_write_cmos_sensor(0x0380, 0xfd); 		   
			HM0357_write_cmos_sensor(0x032D, 0x00); 		   
			HM0357_write_cmos_sensor(0x032E, 0x01); 		   
			HM0357_write_cmos_sensor(0x032F, 0x1b); 		   
			HM0357_write_cmos_sensor(0x0330, 0x01); 		   
			HM0357_write_cmos_sensor(0x0331, 0x40); 		   
			HM0357_write_cmos_sensor(0x0332, 0x02);
		       HM0357_write_cmos_sensor(0x0000,0x01);//																									  
			HM0357_write_cmos_sensor(0x0100,0x01);// 																									
			HM0357_write_cmos_sensor(0x0101,0x01);// 
			HM0357_write_cmos_sensor(0x0100, 0xFF); 					
			HM0357_write_cmos_sensor(0x0100, 0xff);   

		break;
		case AWB_MODE_TUNGSTEN: //home
			HM0357_write_cmos_sensor(0x0380, 0xfd); 		   
			HM0357_write_cmos_sensor(0x032D, 0x00); 		   
			HM0357_write_cmos_sensor(0x032E, 0x01); 		   
			HM0357_write_cmos_sensor(0x032F, 0x2b); 		   
			HM0357_write_cmos_sensor(0x0330, 0x01); 		   
			HM0357_write_cmos_sensor(0x0331, 0x40); 		   
			HM0357_write_cmos_sensor(0x0332, 0x02);
		       HM0357_write_cmos_sensor(0x0000,0x01);//																									  
			HM0357_write_cmos_sensor(0x0100,0x01);// 																									
			HM0357_write_cmos_sensor(0x0101,0x01);// 
			HM0357_write_cmos_sensor(0x0100, 0xFF); 					
			HM0357_write_cmos_sensor(0x0100, 0xff); 	

		break;
		case AWB_MODE_FLUORESCENT:
			HM0357_write_cmos_sensor(0x0380, 0xfd); 		   
			HM0357_write_cmos_sensor(0x032D, 0x56); 		   
			HM0357_write_cmos_sensor(0x032E, 0x01); 		   
			HM0357_write_cmos_sensor(0x032F, 0x00); 		   
			HM0357_write_cmos_sensor(0x0330, 0x01); 		   
			HM0357_write_cmos_sensor(0x0331, 0x08); 		   
			HM0357_write_cmos_sensor(0x0332, 0x02);
			HM0357_write_cmos_sensor(0x0000,0x01);//																									  
			HM0357_write_cmos_sensor(0x0100,0x01);// 																									
			HM0357_write_cmos_sensor(0x0101,0x01);// 
			HM0357_write_cmos_sensor(0x0100, 0xFF); 					
			HM0357_write_cmos_sensor(0x0100, 0xff);    

		break;	
		default:
		return FALSE;
	}
	return TRUE;
} /* HM0357_set_param_wb */

BOOL HM0357_set_param_effect(UINT16 para)
{

	kal_uint32 ret = KAL_TRUE;
	switch (para)
	{
		case MEFFECT_OFF:
			HM0357_write_cmos_sensor(0x0488, 0x10);
			HM0357_write_cmos_sensor(0x0486, 0x00);
			HM0357_write_cmos_sensor(0x0487, 0xff);
			HM0357_write_cmos_sensor(0x0100, 0xff);	
		break;
		case MEFFECT_SEPIA:
			HM0357_write_cmos_sensor(0x0488, 0x11);
			HM0357_write_cmos_sensor(0x0486, 0x40);
			HM0357_write_cmos_sensor(0x0487, 0x90);
			HM0357_write_cmos_sensor(0x0100, 0xff);	
		break;  
		case MEFFECT_NEGATIVE:		
			HM0357_write_cmos_sensor(0x0488, 0x12);
			HM0357_write_cmos_sensor(0x0486, 0x00);
			HM0357_write_cmos_sensor(0x0487, 0xff);
			HM0357_write_cmos_sensor(0x0100, 0xff);	
		break; 
		case MEFFECT_SEPIAGREEN:		
			HM0357_write_cmos_sensor(0x0488, 0x11);
			HM0357_write_cmos_sensor(0x0486, 0x60);
			HM0357_write_cmos_sensor(0x0487, 0x60);
			HM0357_write_cmos_sensor(0x0100, 0xff);	
		break;
		case MEFFECT_SEPIABLUE:	
			HM0357_write_cmos_sensor(0x0488, 0x11);
			HM0357_write_cmos_sensor(0x0486, 0xb0);
			HM0357_write_cmos_sensor(0x0487, 0x80);
			HM0357_write_cmos_sensor(0x0100, 0xff);
		break;

		case MEFFECT_MONO:				
			HM0357_write_cmos_sensor(0x0488, 0x11);
			HM0357_write_cmos_sensor(0x0486, 0x80);
			HM0357_write_cmos_sensor(0x0487, 0x80);
			HM0357_write_cmos_sensor(0x0100, 0xff);
		break;

		default:
		return FALSE;
	}


	return ret;
} /* HM0357_set_param_effect */

BOOL HM0357_set_param_banding(UINT16 para)
{

    switch (para)
    {
        case AE_FLICKER_MODE_50HZ: 
			HM0357_write_cmos_sensor(0x0120  ,0x36);	
            break;

        case AE_FLICKER_MODE_60HZ:
			HM0357_write_cmos_sensor(0x0120  ,0x37);	
	
            break;
          default:
              return FALSE;
    }

    Sleep(50);
    return TRUE;
} /* HM0357_set_param_banding */

BOOL HM0357_set_param_exposure(UINT16 para)
{
    //HM0357_write_cmos_sensor(0x03,0x10);
		//HM0357_write_cmos_sensor(0x12,(HM0357_read_cmos_sensor(0x12)|0x10));//make sure the Yoffset control is opened.
	switch (para)
	{
    case AE_EV_COMP_n13:
		HM0357_write_cmos_sensor(0x04C0, 0xA0); 
		HM0357_write_cmos_sensor(0x038E, 0x30); 
		HM0357_write_cmos_sensor(0x0381, 0x40); 
		HM0357_write_cmos_sensor(0x0382, 0x20); 			
		HM0357_write_cmos_sensor(0x0100, 0xff);//-4
        break;
    case AE_EV_COMP_n10:
		HM0357_write_cmos_sensor(0x04C0, 0x98); 
		HM0357_write_cmos_sensor(0x038E, 0x38); 
		HM0357_write_cmos_sensor(0x0381, 0x48); 
		HM0357_write_cmos_sensor(0x0382, 0x38); 
		HM0357_write_cmos_sensor(0x0100, 0xff);//-3
        break;
    case AE_EV_COMP_n07:
		HM0357_write_cmos_sensor(0x04C0, 0x90); 
		HM0357_write_cmos_sensor(0x038E, 0x40); 
		HM0357_write_cmos_sensor(0x0381, 0x50); 
		HM0357_write_cmos_sensor(0x0382, 0x30); 
		HM0357_write_cmos_sensor(0x0100, 0xff); //-2
        break;
    case AE_EV_COMP_n03:
		HM0357_write_cmos_sensor(0x04C0, 0x88); 
		HM0357_write_cmos_sensor(0x038E, 0x48); 
		HM0357_write_cmos_sensor(0x0381, 0x58); 
		HM0357_write_cmos_sensor(0x0382, 0x38);
		HM0357_write_cmos_sensor(0x0100, 0xff);//-1
        break;
    case AE_EV_COMP_00:
		HM0357_write_cmos_sensor(0x04C0, 0x00); 
		HM0357_write_cmos_sensor(0x038E, 0x50); 
		HM0357_write_cmos_sensor(0x0381, 0x5c);
		HM0357_write_cmos_sensor(0x0382, 0x44); 
		HM0357_write_cmos_sensor(0x0100, 0xff);//-0
        break;
    case AE_EV_COMP_03:
		HM0357_write_cmos_sensor(0x04C0, 0x08); 
		HM0357_write_cmos_sensor(0x038E, 0x58); 
		HM0357_write_cmos_sensor(0x0381, 0x68); 
		HM0357_write_cmos_sensor(0x0382, 0x48); 
		HM0357_write_cmos_sensor(0x0100, 0xff);//  1
        break;
    case AE_EV_COMP_07:
		HM0357_write_cmos_sensor(0x04C0, 0x10); 
		HM0357_write_cmos_sensor(0x038E, 0x60); 
		HM0357_write_cmos_sensor(0x0381, 0x70); 
		HM0357_write_cmos_sensor(0x0382, 0x50); 
		HM0357_write_cmos_sensor(0x0100, 0xff);//  2
        break;
    case AE_EV_COMP_10:
		HM0357_write_cmos_sensor(0x04C0, 0x18); 
		HM0357_write_cmos_sensor(0x038E, 0x68); 
		HM0357_write_cmos_sensor(0x0381, 0x78); 
		HM0357_write_cmos_sensor(0x0382, 0x58); 
		HM0357_write_cmos_sensor(0x0100, 0xff);//  3
        break;
    case AE_EV_COMP_13:
		HM0357_write_cmos_sensor(0x04C0, 0x20); 
		HM0357_write_cmos_sensor(0x038E, 0x70); 
		HM0357_write_cmos_sensor(0x0381, 0x80); 
		HM0357_write_cmos_sensor(0x0382, 0x60); 
		HM0357_write_cmos_sensor(0x0100, 0xff);//  4
        break;
    default:
        return FALSE;
	}
	return TRUE;
} /* HM0357_set_param_exposure */

UINT32 HM0357YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
 //  if( HM0357_sensor_cap_state == KAL_TRUE)
	//   return TRUE;

	switch (iCmd) {
	case FID_SCENE_MODE:	    
	    if (iPara == SCENE_MODE_OFF)
	    {
	        HM0357_night_mode(0); 
	    }
	    else if (iPara == SCENE_MODE_NIGHTSCENE)
	    {
               HM0357_night_mode(1); 
	    }	    
	    break; 	    
	case FID_AWB_MODE:
           HM0357_set_param_wb(iPara);
	break;
	case FID_COLOR_EFFECT: 
           HM0357_set_param_effect(iPara);
	break;
	case FID_AE_EV:    	    
           HM0357_set_param_exposure(iPara);
	break;
	case FID_AE_FLICKER:    	    	    
           HM0357_set_param_banding(iPara);
	break;
        case FID_AE_SCENE_MODE: 
            if (iPara == AE_MODE_OFF) {
                HM0357_set_AE_mode(KAL_FALSE);
            }
            else {
                HM0357_set_AE_mode(KAL_TRUE);
	    }
            break; 
	case FID_ZOOM_FACTOR:
	    zoom_factor = iPara; 
        break; 
	default:
	break;
	}
	return TRUE;
}   /* HM0357YUVSensorSetting */

UINT32 HM0357YUVSetVideoMode(UINT16 u2FrameRate)
{
    kal_uint8 iTemp;
    if (u2FrameRate == 30)
    {
    }
    else if (u2FrameRate == 15)       
    {
    }
    else 
    {
        printk("Wrong frame rate setting \n");
    }
    HM0357_VEDIO_encode_mode = KAL_TRUE; 
        
    return TRUE;
}

UINT32 HM0357FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
	UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
	UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
	UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
	UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
	MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
	MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

	switch (FeatureId)
	{
		case SENSOR_FEATURE_GET_RESOLUTION:
			*pFeatureReturnPara16++=HM0357_IMAGE_SENSOR_FULL_WIDTH;
			*pFeatureReturnPara16=HM0357_IMAGE_SENSOR_FULL_HEIGHT;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PERIOD:
			*pFeatureReturnPara16++=HM0357_IMAGE_SENSOR_PV_WIDTH;
			*pFeatureReturnPara16=HM0357_IMAGE_SENSOR_PV_HEIGHT;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
			//*pFeatureReturnPara32 = HM0357_sensor_pclk/10;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_SET_ESHUTTER:
		break;
		case SENSOR_FEATURE_SET_NIGHTMODE:
			HM0357_night_mode((BOOL) *pFeatureData16);
		break;
		case SENSOR_FEATURE_SET_GAIN:
		case SENSOR_FEATURE_SET_FLASHLIGHT:
		break;
		case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
			HM0357_isp_master_clock=*pFeatureData32;
		break;
		case SENSOR_FEATURE_SET_REGISTER:
			HM0357_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
		break;
		case SENSOR_FEATURE_GET_REGISTER:
			pSensorRegData->RegData = HM0357_read_cmos_sensor(pSensorRegData->RegAddr);
		break;
		case SENSOR_FEATURE_GET_CONFIG_PARA:
			memcpy(pSensorConfigData, &HM0357SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
			*pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
		break;
		case SENSOR_FEATURE_SET_CCT_REGISTER:
		case SENSOR_FEATURE_GET_CCT_REGISTER:
		case SENSOR_FEATURE_SET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_ENG_REGISTER:
		case SENSOR_FEATURE_GET_REGISTER_DEFAULT:

		case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
		case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
		case SENSOR_FEATURE_GET_GROUP_INFO:
		case SENSOR_FEATURE_GET_ITEM_INFO:
		case SENSOR_FEATURE_SET_ITEM_INFO:
		case SENSOR_FEATURE_GET_ENG_INFO:
		break;
		case SENSOR_FEATURE_GET_GROUP_COUNT:
                        *pFeatureReturnPara32++=0;
                        *pFeatureParaLen=4;	    
		    break; 
		case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
			// get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
			// if EEPROM does not exist in camera module.
			*pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
			*pFeatureParaLen=4;
		break;
		case SENSOR_FEATURE_CHECK_SENSOR_ID:
			 HM0357_GetSensorID(pFeatureData32);
			 break;
		case SENSOR_FEATURE_SET_YUV_CMD:
		       //printk("HM0357 YUV sensor Setting:%d, %d \n", *pFeatureData32,  *(pFeatureData32+1));
			HM0357YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
		break;
		case SENSOR_FEATURE_SET_VIDEO_MODE:
		       HM0357YUVSetVideoMode(*pFeatureData16);
		       break; 
		default:
			break;			
	}
	return ERROR_NONE;
}	/* HM0357FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncHM0357=
{
	HM0357Open,
	HM0357GetInfo,
	HM0357GetResolution,
	HM0357FeatureControl,
	HM0357Control,
	HM0357Close
};

UINT32 HM0357_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncHM0357;

	return ERROR_NONE;
}	/* SensorInit() */
