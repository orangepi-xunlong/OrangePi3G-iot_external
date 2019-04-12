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
 *   BF3925yuv_Sensor.c
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
 * 2011/10/25 Firsty Released By Mormo(using "BF3925.set Revision1721" )
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

#include "bf3925_yuv_sensor.h"
#include "bf3925_yuv_camera_sensor_para.h"
#include "bf3925_yuv_cameracustomized.h"



#define BF3925YUV_DEBUG
#ifdef BF3925YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

#define BF3925_WRITE_ID 0xDC

//extern int iReadReg(u8 addr, u8 *buf, u8 i2cId);
//extern int iWriteReg(u8 addr, u8 buf, u32 size, u16 i2cId);

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);

static void BF3925_Write_Shutter(kal_uint16 shutter);

static int BF3925_PV_dummy_pixels;
static int BF3925_PV_dummy_lines;
static int BF3925_g_iInt_Step;

static int BF3925_g_iInt_Line;
static int BF3925_FULL_dummy_pixels;
static int BF3925_FULL_dummy_lines;

static void BF3925_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    char puSendCmd[2] = {(char)(addr & 0xFF), (char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd, 2,BF3925_WRITE_ID);
}

 kal_uint8 BF3925_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint8 get_byte = 0;
	
    char pSendCmd = (char)(addr & 0xFF);
    
   iReadRegI2C(&pSendCmd, 1, (u8*)&get_byte, 1, BF3925_WRITE_ID);

    return get_byte;
}


/*******************************************************************************
 * // Adapter for Winmo typedef
 ********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT

kal_bool   BF3925_MPEG4_encode_mode = KAL_FALSE;
kal_uint16 BF3925_dummy_pixels = 0, BF3925_dummy_lines = 0;
kal_bool   BF3925_MODE_CAPTURE = KAL_FALSE;
kal_bool   BF3925_CAM_BANDING_50HZ = KAL_FALSE;

kal_uint32 BF3925_isp_master_clock;
static kal_uint32 BF3925_g_fPV_PCLK = 24;

//kal_uint8 BF3925_sensor_write_I2C_address = BF3925_WRITE_ID;
//kal_uint8 BF3925_sensor_read_I2C_address = BF3925_READ_ID;

UINT8 BF3925PixelClockDivider=0;

MSDK_SENSOR_CONFIG_STRUCT BF3925SensorConfigData;


/*************************************************************************
 * FUNCTION
 *	BF3925_SetShutter
 *
 * DESCRIPTION
 *	This function set e-shutter of BF3925 to change exposure time.
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
void BF3925_Set_Shutter(kal_uint16 iShutter)
{
//	BF3925_exposure_lines = iShutter;
	BF3925_Write_Shutter(iShutter);
} /* Set_BF3925_Shutter */

void BF3925_Set_Dummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{

} /* BF3925_Set_Dummy */

kal_uint16 BF3925_read_sensor_INT_Step(void)
{

}


void BF3925_ConfigMinFrameRate(const kal_uint8 fFPS)
{

	
}   /*  ConfigMinFrameRate */

kal_uint32 BF3925_Computer_CLK(void)
{
	
	return 0;
}
/*************************************************************************
 * FUNCTION
 *	BF3925_read_Shutter
 *
 * DESCRIPTION
 *	This function read e-shutter of BF3925 .
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
kal_uint16 BF3925_Read_Shutter(void)
{

} /* BF3925_read_shutter */

static void BF3925_Write_Shutter(kal_uint16 shutter)
{

} /* BF3925_Write_Shutter */

/*************************************************************************
 * FUNCTION
 *	BF3925_write_reg
 *
 * DESCRIPTION
 *	This function set the register of BF3925.
 *
 * PARAMETERS
 *	addr : the register index of BF3925
 *  para : setting parameter of the specified register of BF3925
 *
 * RETURNS
 *	None
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void BF3925_write_reg(kal_uint32 addr, kal_uint32 para)
{
	BF3925_write_cmos_sensor(addr, para);
} /* BF3925_write_reg() */


/*************************************************************************
 * FUNCTION
 *	BF3925_read_cmos_sensor
 *
 * DESCRIPTION
 *	This function read parameter of specified register from BF3925.
 *
 * PARAMETERS
 *	addr : the register index of BF3925
 *
 * RETURNS
 *	the data that read from BF3925
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
kal_uint32 BF3925_read_reg(kal_uint32 addr)
{
	return BF3925_read_cmos_sensor(addr);
} /* BF3925_read_reg() */


/*************************************************************************
* FUNCTION
*	BF3925_awb_enable
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
static void BF3925_awb_enable(kal_bool enalbe)
{	 
	kal_uint16 temp_AWB_reg = 0;


}


/*************************************************************************
 * FUNCTION
 *	BF3925_config_window
 *
 * DESCRIPTION
 *	This function config the hardware window of BF3925 for getting specified
 *  data of that window.
 *
 * PARAMETERS
 *	start_x : start column of the interested window
 *  start_y : start row of the interested window
 *  width  : column widht of the itnerested window
 *  height : row depth of the itnerested window
 *
 * RETURNS
 *	the data that read from BF3925
 *
 * GLOBALS AFFECTED
 *
 *************************************************************************/
void BF3925_config_window(kal_uint16 startx, kal_uint16 starty, kal_uint16 width, kal_uint16 height)
{
} /* BF3925_config_window */


/*************************************************************************
 * FUNCTION
 *	BF3925_SetGain
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
kal_uint16 BF3925_SetGain(kal_uint16 iGain)
{
	return iGain;
}


/*************************************************************************
 * FUNCTION
 *	BF3925_NightMode
 *
 * DESCRIPTION
 *	This function night mode of BF3925.
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
void BF3925_night_mode(kal_bool bEnable)
{
    if (bEnable) {
        if(BF3925_MPEG4_encode_mode == KAL_TRUE) {
            RETAILMSG(1, (TEXT("BF3925 night mode true video normal mode preview\r\n")));
			BF3925_write_cmos_sensor(0xff, 0x01);
			BF3925_write_cmos_sensor(0x09, 0xa1);
       } else {
           // RETAILMSG(1, (TEXT("BF3925 night mode ture camera preview\r\n")));
			BF3925_write_cmos_sensor(0xff, 0x01);
			BF3925_write_cmos_sensor(0x09, 0xa1);
        }
    } else {
        if (BF3925_MPEG4_encode_mode == KAL_TRUE) {
            //RETAILMSG(1, (TEXT("BF3925 night mode false video normal mode preview\r\n")));
			BF3925_write_cmos_sensor(0xff, 0x01);			
			BF3925_write_cmos_sensor(0x09, 0xa1);
       } else {
            RETAILMSG(1, (TEXT("BF3925 night mode false camera preview\r\n")));
           // // this mode is used by camera  Normal mode
			BF3925_write_cmos_sensor(0xff, 0x01);
			BF3925_write_cmos_sensor(0x09, 0x8b);//9fps minumin

		   
       	}
    	}
} /* BF3925_NightMode */



UINT32 BF3925GetSensorID(UINT32 *sensorID)
{
    kal_uint16 sensor_id=0;
    int i;



    do
    {
      
        for(i = 0; i < 3; i++)
		{
	   
	      sensor_id = ((BF3925_read_cmos_sensor(0xfc) << 8) | BF3925_read_cmos_sensor(0xfd));
	      printk("JHY BF3925 Sensor id = %x\n", sensor_id);
	      if (sensor_id == BF3925_SENSOR_ID)
			{
	         break;
	        }
        }
        	mdelay(50);
    }while(0);

    if(sensor_id != BF3925_SENSOR_ID)
    {
         *sensorID = 0xFFFFFFFF;
        SENSORDB("BF3925 Sensor id read failed, ID = %x\n", sensor_id);
        return ERROR_SENSOR_CONNECT_FAIL;
    }

    *sensorID = sensor_id;

    RETAILMSG(1, (TEXT("Sensor Read ID OK \r\n")));
	
    return ERROR_NONE;
}


void BF3925_Sensor_Init(void)
{

	BF3925_write_cmos_sensor(0xff, 0x01);  //Bit[0]: select reg page 
	BF3925_write_cmos_sensor(0xf2, 0x01);  //Bit[0]: 1-soft reset     bit[1]:1-soft sleep mode
	BF3925_write_cmos_sensor(0xff, 0x01);  //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x50, 0x00);  //bit[4]: digital subsample  bit[2][0]: Data format selection
	BF3925_write_cmos_sensor(0x51, 0x00);  //YUV Sequence
	BF3925_write_cmos_sensor(0xe0, 0x00);
	BF3925_write_cmos_sensor(0xe2, 0x64);
	BF3925_write_cmos_sensor(0xe3, 0x48);
	BF3925_write_cmos_sensor(0xe4, 0x81);  //Drive capability 
	BF3925_write_cmos_sensor(0xe7, 0x80);
	//clock，dummy
	BF3925_write_cmos_sensor(0xff, 0x01);  //Bit[0]: select reg page   
	BF3925_write_cmos_sensor(0xe9, 0x08);  //PLL setting  0x09: 1倍频   0x1b: 5/4倍频   0x2b: 3/2倍频  0x08: 2倍频  0x1a: 5/2倍频  0x2a: 3倍频
	BF3925_write_cmos_sensor(0xff, 0x00);  //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x01, 0x00);
	BF3925_write_cmos_sensor(0x02, 0x00);  //Dummy Pixel Insert LSB 0x90
	BF3925_write_cmos_sensor(0x03, 0x02);
	BF3925_write_cmos_sensor(0x04, 0x00);  //Dummy line Insert LSB
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0xe5, 0x32);
	//resolution 1600*1200
	BF3925_write_cmos_sensor(0xff, 0x00);
	BF3925_write_cmos_sensor(0x05, 0xe0);
	BF3925_write_cmos_sensor(0x09, 0x00);
	BF3925_write_cmos_sensor(0x0a, 0x48);
	BF3925_write_cmos_sensor(0x0b, 0x60);
	BF3925_write_cmos_sensor(0x0c, 0x00);
	BF3925_write_cmos_sensor(0x0d, 0xb8);
	BF3925_write_cmos_sensor(0x0e, 0x40);
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x52, 0x02);  //Bit[1]: VSYNC option   Bit[0]: HSYNC option
	BF3925_write_cmos_sensor(0x5d, 0x02);
	BF3925_write_cmos_sensor(0x5a, 0x00);
	BF3925_write_cmos_sensor(0x5b, 0x00);
	BF3925_write_cmos_sensor(0x5c, 0x00);
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x53, 0x60); 
	BF3925_write_cmos_sensor(0x54, 0x40); 
	BF3925_write_cmos_sensor(0x55, 0x00);
	BF3925_write_cmos_sensor(0x56, 0x40); 
	BF3925_write_cmos_sensor(0x57, 0x00);
	BF3925_write_cmos_sensor(0x58, 0xb0);
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x50, 0x00);  //bit[4]: digital subsample  bit[2][0]: Data format selection
	//initial AWB and AE	
	BF3925_write_cmos_sensor(0xff, 0x00);  //Bit[0]: select reg page  
	BF3925_write_cmos_sensor(0xb2, 0x81);  //Manual AWB & AE 
	BF3925_write_cmos_sensor(0xb0, 0x16);  
	BF3925_write_cmos_sensor(0xb1, 0x1d);  
	BF3925_write_cmos_sensor(0xb2, 0x89);  
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x00, 0x00);
	BF3925_write_cmos_sensor(0x0e, 0x0a);
	BF3925_write_cmos_sensor(0x0f, 0x64);
	BF3925_write_cmos_sensor(0x10, 0x28);
	BF3925_write_cmos_sensor(0x00, 0x05);
	//black control                             
	BF3925_write_cmos_sensor(0xff, 0x00);
	BF3925_write_cmos_sensor(0x3c, 0x97);
	
	//black sun                             
	BF3925_write_cmos_sensor(0xff, 0x01);  //Bit[0]: select reg page     
	BF3925_write_cmos_sensor(0xe1, 0x28);  //bit[7:4]: Pixel bias current
	BF3925_write_cmos_sensor(0xff, 0x00);  //Bit[0]: select reg page     
	BF3925_write_cmos_sensor(0x00, 0x77);  //bit[6]: black sun control   bit[5]: mirror   bit[4]: vertical flip
	BF3925_write_cmos_sensor(0x18, 0x0c);  //PRST indoor
	BF3925_write_cmos_sensor(0x19, 0x1a);  //PRST outdoor
	//lens shading
	BF3925_write_cmos_sensor(0xff, 0x00);  //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x52, 0x13);
	BF3925_write_cmos_sensor(0x53, 0x5c);
	BF3925_write_cmos_sensor(0x54, 0x24);
	BF3925_write_cmos_sensor(0x55, 0x13);
	BF3925_write_cmos_sensor(0x56, 0x5c);
	BF3925_write_cmos_sensor(0x57, 0x24);
	BF3925_write_cmos_sensor(0x58, 0xd3);
	BF3925_write_cmos_sensor(0x59, 0x5c);
	BF3925_write_cmos_sensor(0x5a, 0x24);
	BF3925_write_cmos_sensor(0x5b, 0x46); //lens shading gain of R
	BF3925_write_cmos_sensor(0x5c, 0x43); //lens shading gain of G1
	BF3925_write_cmos_sensor(0x5d, 0x40); //lens shading gain of B
	BF3925_write_cmos_sensor(0x5e, 0x43); //lens shading gain of G0      
	//gamma default
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x60, 0x30);
	BF3925_write_cmos_sensor(0x61, 0x2a);
	BF3925_write_cmos_sensor(0x62, 0x24);
	BF3925_write_cmos_sensor(0x63, 0x1b);
	BF3925_write_cmos_sensor(0x64, 0x18);
	BF3925_write_cmos_sensor(0x65, 0x16);
	BF3925_write_cmos_sensor(0x66, 0x14);
	BF3925_write_cmos_sensor(0x67, 0x12);
	BF3925_write_cmos_sensor(0x68, 0x10);
	BF3925_write_cmos_sensor(0x69, 0x0e);
	BF3925_write_cmos_sensor(0x6a, 0x0d);
	BF3925_write_cmos_sensor(0x6b, 0x0c);
	BF3925_write_cmos_sensor(0x6c, 0x0a);
	BF3925_write_cmos_sensor(0x6d, 0x09);
	BF3925_write_cmos_sensor(0x6e, 0x09);
	BF3925_write_cmos_sensor(0x6f, 0xf0);
	BF3925_write_cmos_sensor(0x70, 0x20);
	BF3925_write_cmos_sensor(0x71, 0x60);
	BF3925_write_cmos_sensor(0x72,0x1a); //0x20
	BF3925_write_cmos_sensor(0x73,0x1a); //0x20
	/*                       
	//gamma 过曝过度好，高亮 	
	BF3925_write_cmos_sensor(0xff, 0x00);	 //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x60, 0x33);  
	BF3925_write_cmos_sensor(0x61, 0x2b);  
	BF3925_write_cmos_sensor(0x62, 0x27);  
	BF3925_write_cmos_sensor(0x63, 0x22);  
	BF3925_write_cmos_sensor(0x64, 0x1b);  
	BF3925_write_cmos_sensor(0x65, 0x17);  
	BF3925_write_cmos_sensor(0x66, 0x14);  
	BF3925_write_cmos_sensor(0x67, 0x11);  
	BF3925_write_cmos_sensor(0x68, 0x0e);  
	BF3925_write_cmos_sensor(0x69, 0x0c);  
	BF3925_write_cmos_sensor(0x6a, 0x0b);  
	BF3925_write_cmos_sensor(0x6b, 0x0a);  
	BF3925_write_cmos_sensor(0x6c, 0x09);  
	BF3925_write_cmos_sensor(0x6d, 0x08);  
	BF3925_write_cmos_sensor(0x6e, 0x07); 
	*/
	//gamma 清晰亮丽  	       
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x60, 0x28); 
	BF3925_write_cmos_sensor(0x61, 0x28);
	BF3925_write_cmos_sensor(0x62, 0x26);
	BF3925_write_cmos_sensor(0x63, 0x22);
	BF3925_write_cmos_sensor(0x64, 0x1f);
	BF3925_write_cmos_sensor(0x65, 0x1c);
	BF3925_write_cmos_sensor(0x66, 0x18);
	BF3925_write_cmos_sensor(0x67, 0x13);
	BF3925_write_cmos_sensor(0x68, 0x10);
	BF3925_write_cmos_sensor(0x69, 0x0d);
	BF3925_write_cmos_sensor(0x6a, 0x0a);//0c zhouji
	BF3925_write_cmos_sensor(0x6b, 0x08);//0a
	BF3925_write_cmos_sensor(0x6c, 0x04);//08
	BF3925_write_cmos_sensor(0x6d, 0x02);//07
	BF3925_write_cmos_sensor(0x6e, 0x01);//06
	/*
	//gamma low denoise
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x60, 0x24);
	BF3925_write_cmos_sensor(0x61, 0x30);	
	BF3925_write_cmos_sensor(0x62, 0x20);	
	BF3925_write_cmos_sensor(0x63, 0x1a);	
	BF3925_write_cmos_sensor(0x64, 0x16);	
	BF3925_write_cmos_sensor(0x65, 0x13);	
	BF3925_write_cmos_sensor(0x66, 0x11);	
	BF3925_write_cmos_sensor(0x67, 0x0e);	
	BF3925_write_cmos_sensor(0x68, 0x0d);	
	BF3925_write_cmos_sensor(0x69, 0x0c);	
	BF3925_write_cmos_sensor(0x6a, 0x0b);	
	BF3925_write_cmos_sensor(0x6b, 0x09);	
	BF3925_write_cmos_sensor(0x6c, 0x09);	
	BF3925_write_cmos_sensor(0x6d, 0x08);	
	BF3925_write_cmos_sensor(0x6e, 0x07);*/ 
/*
 //gamma more details under low light,light enough   
  BF3925_write_cmos_sensor(0x60, 0x50);  
  BF3925_write_cmos_sensor(0x61, 0x40);  
  BF3925_write_cmos_sensor(0x62, 0x28);  
  BF3925_write_cmos_sensor(0x63, 0x20);  
  BF3925_write_cmos_sensor(0x64, 0x18);  
  BF3925_write_cmos_sensor(0x65, 0x16);  
  BF3925_write_cmos_sensor(0x66, 0x12);  
  BF3925_write_cmos_sensor(0x67, 0x10);  
  BF3925_write_cmos_sensor(0x68, 0x0d);  
  BF3925_write_cmos_sensor(0x69, 0x0b);  
  BF3925_write_cmos_sensor(0x6a, 0x09);  
  BF3925_write_cmos_sensor(0x6b, 0x08);  
  BF3925_write_cmos_sensor(0x6c, 0x07);  
  BF3925_write_cmos_sensor(0x6d, 0x04);  
  BF3925_write_cmos_sensor(0x6e, 0x02); 
  BF3925_write_cmos_sensor(0x6a, 0x0a);
  BF3925_write_cmos_sensor(0x6b, 0x08);
  BF3925_write_cmos_sensor(0x6c, 0x04);
  BF3925_write_cmos_sensor(0x6d, 0x02);
  BF3925_write_cmos_sensor(0x6e, 0x01);
  */
  /*
  //new default gamma
  BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x60, 0x30);
	BF3925_write_cmos_sensor(0x61, 0x2a);
	BF3925_write_cmos_sensor(0x62, 0x24);
	BF3925_write_cmos_sensor(0x63, 0x1a);
	BF3925_write_cmos_sensor(0x64, 0x1a);
	BF3925_write_cmos_sensor(0x65, 0x18);
	BF3925_write_cmos_sensor(0x66, 0x16);
	BF3925_write_cmos_sensor(0x67, 0x14);
	BF3925_write_cmos_sensor(0x68, 0x10);
	BF3925_write_cmos_sensor(0x69, 0x0e);
	BF3925_write_cmos_sensor(0x6a, 0x0d);
	BF3925_write_cmos_sensor(0x6b, 0x0a);
	BF3925_write_cmos_sensor(0x6c, 0x06);
	BF3925_write_cmos_sensor(0x6d, 0x04);
	BF3925_write_cmos_sensor(0x6e, 0x02);	
  */
  BF3925_write_cmos_sensor(0x93, 0x0a);  //0x93[4:0]值越小，噪声越小，边缘越模糊0x0c
	
	//denoise and edge enhancement 
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x80, 0x0f);
	BF3925_write_cmos_sensor(0x81, 0x0c);//0x1c zhouji 1c
	BF3925_write_cmos_sensor(0x83, 0x45); //0x83[7:4]: The bigger, the smaller noise; 0x83[3:0]: The smaller, the smaller noise //27  zhouji    
	BF3925_write_cmos_sensor(0x84, 0xe6);
	BF3925_write_cmos_sensor(0x85, 0x88);
	BF3925_write_cmos_sensor(0x86, 0xfa);
	BF3925_write_cmos_sensor(0x87, 0x1a);
	BF3925_write_cmos_sensor(0x88, 0x02);  //bit[7]和[6]设置为0噪声小 0xa2
	BF3925_write_cmos_sensor(0x89, 0xca);
	BF3925_write_cmos_sensor(0x8b, 0x23); //0x8b[7:4]: Bright edge enhancement; 0x8b[3:0]: Dark edge enhancement //12 zhouji 11
	BF3925_write_cmos_sensor(0x91, 0x45);  //91写40，噪声大，清晰 
	//AWB
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0xa2, 0x06);  //the low limit of blue gain for indoor scene   
	BF3925_write_cmos_sensor(0xa3, 0x28);  //the upper limit of blue gain for indoor scene 
	BF3925_write_cmos_sensor(0xa4, 0x0a);  //the low limit of red gain for indoor scene    
	BF3925_write_cmos_sensor(0xa5, 0x2c);  //the upper limit of red gain for indoor scene  
	BF3925_write_cmos_sensor(0xa7, 0x1b);  //Base B gain不建议修改                         
	BF3925_write_cmos_sensor(0xa8, 0x14);  //Base R gain不建议修改                         
	BF3925_write_cmos_sensor(0xa9, 0x15);
	BF3925_write_cmos_sensor(0xaa, 0x18);
	BF3925_write_cmos_sensor(0xab, 0x26);
	BF3925_write_cmos_sensor(0xac, 0x5c);
	BF3925_write_cmos_sensor(0xae, 0x47);
	BF3925_write_cmos_sensor(0xb2, 0x89); 
	BF3925_write_cmos_sensor(0xb3, 0x66);  // green gain
	BF3925_write_cmos_sensor(0xb4, 0x03);  //the offset of F light
	BF3925_write_cmos_sensor(0xb5, 0x00);  //the offset of non-F light
	BF3925_write_cmos_sensor(0xb6, 0xd9);  //bit[7]: outdoor control
	BF3925_write_cmos_sensor(0xb8, 0xca);
	BF3925_write_cmos_sensor(0xbb, 0x0d);
	BF3925_write_cmos_sensor(0xbc, 0x15);
	BF3925_write_cmos_sensor(0xbd, 0x09);
	BF3925_write_cmos_sensor(0xbe, 0x24);
	BF3925_write_cmos_sensor(0xbf, 0x66);
   
	/*  
	// color default  
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page 
	BF3925_write_cmos_sensor(0xc0, 0x8a);
	BF3925_write_cmos_sensor(0xc1, 0x05);
	BF3925_write_cmos_sensor(0xc2, 0x84);
	BF3925_write_cmos_sensor(0xc3, 0x86);
	BF3925_write_cmos_sensor(0xc4, 0x03);
	BF3925_write_cmos_sensor(0xc5, 0x93);
                
  	//color 艳丽
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page    
	BF3925_write_cmos_sensor(0xc0, 0x83);
	BF3925_write_cmos_sensor(0xc1, 0x86);
	BF3925_write_cmos_sensor(0xc2, 0x82);
	BF3925_write_cmos_sensor(0xc3, 0x8a);
	BF3925_write_cmos_sensor(0xc4, 0x07);
	BF3925_write_cmos_sensor(0xc5, 0x9f);
    */
	//color 色彩淡  
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page   
	BF3925_write_cmos_sensor(0xc0, 0x83);
	BF3925_write_cmos_sensor(0xc1, 0x02); 
	BF3925_write_cmos_sensor(0xc2, 0x84); 
	BF3925_write_cmos_sensor(0xc3, 0x84); 
	BF3925_write_cmos_sensor(0xc4, 0x03); 
	BF3925_write_cmos_sensor(0xc5, 0x8d);    
  
	// A color  
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0xc6, 0x8a);
	BF3925_write_cmos_sensor(0xc7, 0x82);
	BF3925_write_cmos_sensor(0xc8, 0x8b);
	BF3925_write_cmos_sensor(0xc9, 0x87);
	BF3925_write_cmos_sensor(0xca, 0x83);
	BF3925_write_cmos_sensor(0xcb, 0x91);
	
	//Outdoor color  
	BF3925_write_cmos_sensor(0xff, 0x00); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0xd0, 0x90);
	BF3925_write_cmos_sensor(0xd1, 0x05);
	BF3925_write_cmos_sensor(0xd2, 0x82);
	BF3925_write_cmos_sensor(0xd3, 0x88);
	BF3925_write_cmos_sensor(0xd4, 0x03);
	BF3925_write_cmos_sensor(0xd5, 0x93);
	
	BF3925_write_cmos_sensor(0xcd, 0x30);
	BF3925_write_cmos_sensor(0xd6, 0x61);
 	//AE  
	BF3925_write_cmos_sensor(0xff, 0x01); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x00, 0x05);
	BF3925_write_cmos_sensor(0x01, 0x8a); // AE 窗口和权重
	BF3925_write_cmos_sensor(0x04, 0x48); // AE Target
	BF3925_write_cmos_sensor(0x05, 0x48); // Y target value1
	BF3925_write_cmos_sensor(0x07, 0x92); //Bit[3:2]: the bigger, Y_AVER_MODIFY is smaller
	BF3925_write_cmos_sensor(0x09, 0x8b); //Bit[5:0]: INT_MAX
	BF3925_write_cmos_sensor(0x0b, 0x82); //Bit[5:0]: INT_MIN 
	BF3925_write_cmos_sensor(0x0c, 0x81); //50hz banding//0xbe
	BF3925_write_cmos_sensor(0x0d, 0x6b); //60hz banding//0xa0
	BF3925_write_cmos_sensor(0x15, 0x42); //Bit[7:4]: Threshold for over exposure pixels, the smaller, the more over exposure pixels; Bit[3:0]: Control the start of AE
	BF3925_write_cmos_sensor(0x17, 0xb5);
	BF3925_write_cmos_sensor(0x18, 0x30);
	BF3925_write_cmos_sensor(0x19, 0x9a);
	BF3925_write_cmos_sensor(0x1b, 0x33); //minimum global gain
	BF3925_write_cmos_sensor(0x1c, 0x66);
	BF3925_write_cmos_sensor(0x1d, 0x55);
	BF3925_write_cmos_sensor(0x1e, 0x80);
	BF3925_write_cmos_sensor(0x1f, 0xc8); //maximum gain 0xc0
	// saturation  
	BF3925_write_cmos_sensor(0xff, 0x01); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x30, 0xe0);
	BF3925_write_cmos_sensor(0x31, 0x48);
	BF3925_write_cmos_sensor(0x32, 0xf0);
	BF3925_write_cmos_sensor(0x34, 0xb8);  //Cb Saturation Coefficient low 8 bit for NF light//da zhouji  0xa8
	BF3925_write_cmos_sensor(0x35, 0xb0);  //Cr Saturation Coefficient low 8 bit for NF light//ca zhouji 0x98
	BF3925_write_cmos_sensor(0x36, 0xff);  //Cb Saturation Coefficient low 8 bit for F light
	BF3925_write_cmos_sensor(0x37, 0xd0);  //Cr Saturation Coefficient low 8 bit for F light
	//skin
	BF3925_write_cmos_sensor(0xff, 0x01); //Bit[0]: select reg page
	BF3925_write_cmos_sensor(0x3b, 0x08);
	// auto contrast  
	BF3925_write_cmos_sensor(0xff, 0x01); //Bit[0]: select reg page
  BF3925_write_cmos_sensor(0x3e, 0x02); //不建议调整


  


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
} BF3925Status;
BF3925Status BF3925CurrentStatus;


static void BF3925InitialPara(void)
{
    BF3925CurrentStatus.iNightMode = 0xFFFF;
    BF3925CurrentStatus.iWB = AWB_MODE_AUTO;
    BF3925CurrentStatus.isoSpeed = AE_ISO_100;
    BF3925CurrentStatus.iEffect = MEFFECT_OFF;
    BF3925CurrentStatus.iBanding = AE_FLICKER_MODE_50HZ;
    BF3925CurrentStatus.iEV = AE_EV_COMP_n03;
    BF3925CurrentStatus.iMirror = IMAGE_NORMAL;
    BF3925CurrentStatus.iFrameRate = 25;
}

/*************************************************************************
 * FUNCTION
 *	BF3925Open
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
UINT32 BF3925Open(void)
{
    kal_uint16 sensor_id=0;


    Sleep(20);

    BF3925GetSensorID(&sensor_id);
    
    // initail sequence write in
    BF3925_Sensor_Init();
    printk("BF3925 Sensor id read OK, ID = %x\n", sensor_id);
    return ERROR_NONE;
} /* BF3925Open */

/*************************************************************************
 * FUNCTION
 *	BF3925Close
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
UINT32 BF3925Close(void)
{
    return ERROR_NONE;
} /* BF3925Close */


/*************************************************************************
 * FUNCTION
 * BF3925Preview
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
UINT32 BF3925Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
    kal_uint32 iTemp;
    BF3925_write_cmos_sensor(0xff, 0x01);
	kal_uint8 temp_reg = BF3925_read_cmos_sensor(0x00);	
    kal_uint16 iStartX = 0, iStartY = 11;

    kal_uint16 i;
    BF3925_g_fPV_PCLK = 24;
    BF3925_MODE_CAPTURE = KAL_FALSE;
    
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x50, 0x10);
	
    if(sensor_config_data->SensorOperationMode == MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
        RETAILMSG(1, (TEXT("Camera Video preview\r\n")));
         BF3925_MPEG4_encode_mode = KAL_TRUE;

    }
    else
    {
        RETAILMSG(1, (TEXT("Camera preview\r\n")));
        BF3925_MPEG4_encode_mode = KAL_FALSE;

    }

	BF3925_MODE_CAPTURE = KAL_FALSE;
	Sleep(140);

	//kal_prompt_trace(MOD_MMI,"bf3620 1600*1200 capture");
    BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x00, temp_reg|0x05);	// Turn  on AEC AGC
    // copy sensor_config_data
    memcpy(&BF3925SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* BF3925Preview */

/*************************************************************************
 * FUNCTION
 *	BF3925Capture
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
UINT32 BF3925Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
        MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)

{
	volatile kal_uint32 shutter;
	    BF3925_write_cmos_sensor(0xff, 0x01);
	kal_uint8 temp_reg = BF3925_read_cmos_sensor(0x00);	
	kal_uint16 iStartX = 0, iStartY = 11;
    

	BF3925_g_iInt_Step=BF3925_read_sensor_INT_Step();

	BF3925_MODE_CAPTURE=KAL_TRUE;

	//kal_prompt_trace(MOD_MMI,"bf3620 1600*1200 capture");
    BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x00, temp_reg&0xFA);	// Turn Off AEC AGC
	//shutter = shutter/2;
	//shutter = BF3925_Read_Shutter();
	BF3925_g_iInt_Line = shutter;
	//kal_prompt_trace(MOD_MMI, "---------->>>shutter=%d", shutter);


	//BF3925_FULL_dummy_pixels = 144;
	//BF3925_FULL_dummy_lines = 64;		


	//BF3925_Set_Dummy(BF3925_FULL_dummy_pixels, BF3925_FULL_dummy_lines);
	//BF3925_Write_Shutter(shutter);

	//resolution 1600*1200
	BF3925_write_cmos_sensor(0xff, 0x00);
	BF3925_write_cmos_sensor(0x05, 0xe0);
	BF3925_write_cmos_sensor(0x09, 0x00);
	BF3925_write_cmos_sensor(0x0a, 0x48);
	BF3925_write_cmos_sensor(0x0b, 0x60);
	BF3925_write_cmos_sensor(0x0c, 0x00);
	BF3925_write_cmos_sensor(0x0d, 0xb8);
	BF3925_write_cmos_sensor(0x0e, 0x40);
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x52, 0x02);  //Bit[1]: VSYNC option   Bit[0]: HSYNC option
	BF3925_write_cmos_sensor(0x5d, 0x02);
	BF3925_write_cmos_sensor(0x5a, 0x00);
	BF3925_write_cmos_sensor(0x5b, 0x00);
	BF3925_write_cmos_sensor(0x5c, 0x00);
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x53, 0x60); 
	BF3925_write_cmos_sensor(0x54, 0x40); 
	BF3925_write_cmos_sensor(0x55, 0x00);
	BF3925_write_cmos_sensor(0x56, 0x40); 
	BF3925_write_cmos_sensor(0x57, 0x00);
	BF3925_write_cmos_sensor(0x58, 0xb0);
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x50, 0x00);  //bit[4]: digital subsample  bit[2][0]: Data format selection

	image_window->GrabStartX = iStartX;
	image_window->GrabStartY = iStartY;
	image_window->ExposureWindowWidth = BF3925_IMAGE_SENSOR_FULL_WIDTH-2*iStartX;
	image_window->ExposureWindowHeight = BF3925_IMAGE_SENSOR_FULL_HEIGHT-2*iStartY;
	Sleep(140);
 //    copy sensor_config_data
    memcpy(&BF3925SensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;

    
} /* BF3925_Capture() */



UINT32 BF3925GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=BF3925_IMAGE_SENSOR_FULL_WIDTH;
    pSensorResolution->SensorFullHeight=BF3925_IMAGE_SENSOR_FULL_HEIGHT;
	
    pSensorResolution->SensorPreviewWidth=BF3925_IMAGE_SENSOR_PV_WIDTH;
    pSensorResolution->SensorPreviewHeight=BF3925_IMAGE_SENSOR_PV_HEIGHT;

    pSensorResolution->SensorVideoWidth     =  BF3925_IMAGE_SENSOR_PV_WIDTH; //add by JHY for video err 0416
    pSensorResolution->SensorVideoHeight    =  BF3925_IMAGE_SENSOR_PV_HEIGHT;   	
	
    return ERROR_NONE;
} /* BF3925GetResolution() */

UINT32 BF3925GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
        MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    pSensorInfo->SensorPreviewResolutionX=BF3925_IMAGE_SENSOR_PV_WIDTH;
    pSensorInfo->SensorPreviewResolutionY=BF3925_IMAGE_SENSOR_PV_HEIGHT;
    pSensorInfo->SensorFullResolutionX=BF3925_IMAGE_SENSOR_FULL_WIDTH;
    pSensorInfo->SensorFullResolutionY=BF3925_IMAGE_SENSOR_FULL_HEIGHT;

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
    BF3925PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &BF3925SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
    return ERROR_NONE;
} /* BF3925GetInfo() */


UINT32 BF3925Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
        MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
        BF3925Preview(pImageWindow, pSensorConfigData);
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
        BF3925Capture(pImageWindow, pSensorConfigData);
        break;
    }


    return ERROR_NONE;
}	/* BF3925Control() */

BOOL BF3925_set_param_wb(UINT16 para)
{
    kal_uint8  temp_reg;
   

    switch (para)
    {
    case AWB_MODE_OFF:
        break;
    case AWB_MODE_AUTO:
		 
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0xb2, 0x89);
        break;
    case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy

		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0xb2, 0x81); //CLOSE AWB
		BF3925_write_cmos_sensor(0xb3, 0x55); 
		BF3925_write_cmos_sensor(0xb0, 0x10);
		BF3925_write_cmos_sensor(0xb1, 0x20);
        break;
    case AWB_MODE_DAYLIGHT: //sunny
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0xb2, 0x81); 
		BF3925_write_cmos_sensor(0xb3, 0x55);
		BF3925_write_cmos_sensor(0xb0, 0x13);
		BF3925_write_cmos_sensor(0xb1, 0x19);
        break;
    case AWB_MODE_INCANDESCENT: //office
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0xb2, 0x81);
		BF3925_write_cmos_sensor(0xb3, 0x55);
		BF3925_write_cmos_sensor(0xb0, 0x14);
		BF3925_write_cmos_sensor(0xb1, 0x0d);	

        break;
    case AWB_MODE_TUNGSTEN: //home
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0xb2, 0x81); 
		BF3925_write_cmos_sensor(0xb3, 0x55); 
		BF3925_write_cmos_sensor(0xb0, 0x1a);
		BF3925_write_cmos_sensor(0xb1, 0x12);

        break;
    case AWB_MODE_FLUORESCENT:
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0xb2, 0x81); 
		BF3925_write_cmos_sensor(0xb3, 0x55); 
		BF3925_write_cmos_sensor(0xb0, 0x1f);
		BF3925_write_cmos_sensor(0xb1, 0x1a);

        break;
    default:
        return FALSE;
    }

    return TRUE;
	
} /* BF3925_set_param_wb */


BOOL BF3925_set_param_effect(UINT16 para)
{
    kal_uint32  ret = KAL_TRUE;

    switch (para)
    {
    case MEFFECT_OFF:

		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0x80, 0x0f);
		BF3925_write_cmos_sensor(0xd6, 0x61);
		BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x32, 0xd0);
		BF3925_write_cmos_sensor(0x50, 0x10);
		BF3925_write_cmos_sensor(0x52, 0x02);
		BF3925_write_cmos_sensor(0x5e, 0x80);
		BF3925_write_cmos_sensor(0x5f, 0x80);
        break;
    case MEFFECT_SEPIA:
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0x80, 0x0f);
		BF3925_write_cmos_sensor(0xd6, 0x61);
		BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x32, 0xd0);
		BF3925_write_cmos_sensor(0x50, 0x10);
		BF3925_write_cmos_sensor(0x52, 0x06);
		BF3925_write_cmos_sensor(0x5e, 0x60);
		BF3925_write_cmos_sensor(0x5f, 0xa0);
		BF3925_write_cmos_sensor(0x38, 0x40);
 
        break;
    case MEFFECT_NEGATIVE:
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0x80, 0x0f);
		BF3925_write_cmos_sensor(0xd6, 0x61);
		BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x32, 0xd0);
		BF3925_write_cmos_sensor(0x50, 0x18);
		BF3925_write_cmos_sensor(0x52, 0x02);
		BF3925_write_cmos_sensor(0x5e, 0x80);
		BF3925_write_cmos_sensor(0x5f, 0x80);
		BF3925_write_cmos_sensor(0x01, 0x05);
		BF3925_write_cmos_sensor(0x38, 0x40);


        break;
    case MEFFECT_SEPIAGREEN:
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0x80, 0x0f);
		BF3925_write_cmos_sensor(0xd6, 0x61);
		BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x32, 0xd0);
		BF3925_write_cmos_sensor(0x50, 0x10);
		BF3925_write_cmos_sensor(0x52, 0x06);
		BF3925_write_cmos_sensor(0x5e, 0x60);
		BF3925_write_cmos_sensor(0x5f, 0x70);
		BF3925_write_cmos_sensor(0x01, 0x05);
		BF3925_write_cmos_sensor(0x38, 0x40);

        break;
    case MEFFECT_SEPIABLUE:
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0x80, 0x0f);
		BF3925_write_cmos_sensor(0xd6, 0x61);
		BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x32, 0xd0);
		BF3925_write_cmos_sensor(0x50, 0x10);
		BF3925_write_cmos_sensor(0x52, 0x06);
		BF3925_write_cmos_sensor(0x5e, 0xe0);
		BF3925_write_cmos_sensor(0x5f, 0x60);
		BF3925_write_cmos_sensor(0x01, 0x05);
		BF3925_write_cmos_sensor(0x38, 0x40);

        break;
    case MEFFECT_MONO:
		BF3925_write_cmos_sensor(0xff, 0x00);
		BF3925_write_cmos_sensor(0x80, 0x0f);
		BF3925_write_cmos_sensor(0xd6, 0x61);
		BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x32, 0xd0);
		BF3925_write_cmos_sensor(0x50, 0x10);
		BF3925_write_cmos_sensor(0x52, 0x06);
		BF3925_write_cmos_sensor(0x5e, 0x80);
		BF3925_write_cmos_sensor(0x5f, 0x80);		
		BF3925_write_cmos_sensor(0x38, 0x40);
 
        break;
    default:
        ret = FALSE;
    }

    return ret;

} /* BF3925_set_param_effect */


BOOL BF3925_set_param_banding(UINT16 para)
{
 	

    switch (para)
    {
    case AE_FLICKER_MODE_50HZ:
		BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x07, 0x92); //Bit[3:2]: the bigger, Y_AVER_MODIFY is smaller,BIT[1]:banding filter selection
		BF3925_write_cmos_sensor(0x0c, 0x81); //50hz banding 0x78
	    //BF3925_write_cmos_sensor(0x0d, 0xa0); //60hz banding

        break;
    case AE_FLICKER_MODE_60HZ:

		BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x07, 0x90); //Bit[3:2]: the bigger, Y_AVER_MODIFY is smaller,BIT[1]:banding filter selection
		//BF3925_write_cmos_sensor(0x0c, 0xbe); //50hz banding
		BF3925_write_cmos_sensor(0x0d, 0x6b); //60hz banding 0x64
  
        break;
    default:
        return FALSE;
    }

    return TRUE;
} /* BF3925_set_param_banding */


BOOL BF3925_set_param_exposure(UINT16 para)
{ 
    switch (para)
    {
    case AE_EV_COMP_n13:
		//BF3925_write_cmos_sensor(0x56, 0x28);
		//BF3925_write_cmos_sensor(0x55, 0xF8);
        break;
    case AE_EV_COMP_n10:
        BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x04, 0x24); 
		BF3925_write_cmos_sensor(0x05, 0x24); 


        break;
    case AE_EV_COMP_n07:
        BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x04, 0x30); 
		BF3925_write_cmos_sensor(0x05, 0x30); 

        break;
    case AE_EV_COMP_n03:
        BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x04, 0x3c); 
		BF3925_write_cmos_sensor(0x05, 0x3c); 

        break;
    case AE_EV_COMP_00:
        BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x04, 0x48);//AE target for !F light 
		BF3925_write_cmos_sensor(0x05, 0x48);//AE target for F light 
        break;
    case AE_EV_COMP_03:
        BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x04, 0x54); 
		BF3925_write_cmos_sensor(0x05, 0x54); 

        break;
    case AE_EV_COMP_07:
        BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x04, 0x60); 
		BF3925_write_cmos_sensor(0x05, 0x60); 

        break;
    case AE_EV_COMP_10:
        BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x04, 0x6c); 
		BF3925_write_cmos_sensor(0x05, 0x6c); 

        break;
    case AE_EV_COMP_13:
        BF3925_write_cmos_sensor(0xff, 0x01);
		BF3925_write_cmos_sensor(0x04, 0x70); 
		BF3925_write_cmos_sensor(0x05, 0x70); 

        break;
    default:
        return FALSE;
    }

    return TRUE;
} /* BF3925_set_param_exposure */

BOOL BF3925_set_scene_mode(UINT16 Para)
{
	//SENSORDB("BF3925SetScenMode: %d\n",Para);
            printk("phl BF3925_set_scene_mode Para = %d\n",Para);

	switch (Para)
	{                   
    case SCENE_MODE_NIGHTSCENE:		
	 BF3925_write_cmos_sensor(0xff, 0x01);
	 BF3925_write_cmos_sensor(0x09, 0xa1);
      	break;
    case SCENE_MODE_OFF://SCENE_MODE_AUTO: 	
	BF3925_write_cmos_sensor(0xff, 0x01);
	BF3925_write_cmos_sensor(0x09, 0x8b);

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
} /* BF3925SetSceneMode */

UINT32 BF3925YUVSensorSetting(FEATURE_ID iCmd, UINT16 iPara)
{
    switch (iCmd) {
    case FID_SCENE_MODE:
	  BF3925_set_scene_mode(iPara);
            break; 
    case FID_AWB_MODE:
        BF3925_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        BF3925_set_param_effect(iPara);
        break;
    case FID_AE_EV:
        BF3925_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:
        BF3925_set_param_banding(iPara);
        break;
    default:
        break;
    }
    return TRUE;
} /* BF3925YUVSensorSetting */

void BF3925GetExifInfo(UINT32 exifAddr)
{
    SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
    pExifInfo->FNumber = 28;
    pExifInfo->AEISOSpeed = BF3925CurrentStatus.isoSpeed;
    pExifInfo->AWBMode = BF3925CurrentStatus.iWB;
    pExifInfo->CapExposureTime = 0;
    pExifInfo->FlashLightTimeus = 0;
    pExifInfo->RealISOValue = BF3925CurrentStatus.isoSpeed;
}


UINT32 BF3925FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
        UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 BF3925SensorRegNumber;
    UINT32 i;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;

    RETAILMSG(1, (_T("gaiyang BF3925FeatureControl FeatureId=%d\r\n"), FeatureId));

    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=BF3925_IMAGE_SENSOR_FULL_WIDTH;
        *pFeatureReturnPara16=BF3925_IMAGE_SENSOR_FULL_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        *pFeatureReturnPara16++=(694)+BF3925_dummy_pixels;
        *pFeatureReturnPara16=(488)+BF3925_dummy_lines;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        *pFeatureReturnPara32 = BF3925_g_fPV_PCLK;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:
        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        BF3925_night_mode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        BF3925_isp_master_clock=*pFeatureData32;
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        BF3925_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        pSensorRegData->RegData = BF3925_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        memcpy(pSensorConfigData, &BF3925SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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
        BF3925YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;
	case SENSOR_FEATURE_GET_EXIF_INFO:
             SENSORDB("SENSOR_FEATURE_GET_EXIF_INFO\n");
             SENSORDB("EXIF addr = 0x%x\n",*pFeatureData32);
             BF3925GetExifInfo(*pFeatureData32);
             break;	
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
	BF3925GetSensorID(pFeatureData32);
	break;
    default:
        break;
	}
return ERROR_NONE;
}	/* BF3925FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncBF3925YUV=
{
	BF3925Open,
	BF3925GetInfo,
	BF3925GetResolution,
	BF3925FeatureControl,
	BF3925Control,
	BF3925Close
};

UINT32 BF3925_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
	/* To Do : Check Sensor status here */
	if (pfFunc!=NULL)
		*pfFunc=&SensorFuncBF3925YUV;
	return ERROR_NONE;
} /* SensorInit() */
