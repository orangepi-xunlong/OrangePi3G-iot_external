
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
*****************************************************************************//*****************************************************************************
 *
 * Filename:
 * ---------
 *   sensor.c
 *
 * Project:
 * --------
 *   DUMA
 *
 * Description:
 * ------------
 *   Source code of Sensor driver
 *
 *
 * Author:
 * -------
 *   PC Huang (MTK02204)
 *
 *============================================================================
 *             HISTORY
 * Below this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *------------------------------------------------------------------------------
 * $Revision:$
 * $Modtime:$
 * $Log:$
 *
 * 07 11 2011 jun.pei
 * [ALPS00059464] hi706 sensor check in
 * .
 *
 *
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
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

#include "hi706yuv_Sensor.h"
#include "hi706yuv_Camera_Sensor_para.h"
#include "hi706yuv_CameraCustomized.h"

#define HI706YUV_DEBUG
#ifdef HI706YUV_DEBUG
#define SENSORDB printk
#else
#define SENSORDB(x,...)
#endif

#if 0
extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
static int sensor_id_fail = 0; 
#define HI706_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para ,1,HI706_WRITE_ID)
#define HI706_write_cmos_sensor_2(addr, para, bytes) iWriteReg((u16) addr , (u32) para ,bytes,HI706_WRITE_ID)
kal_uint16 HI706_read_cmos_sensor(kal_uint32 addr)
{
kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,HI706_WRITE_ID);
    return get_byte;
}

#endif
static DEFINE_SPINLOCK(hi706_yuv_drv_lock);
static int CAPTURE_FLAG = 0;
static int CAPTUREB_FLAG = 0;//Add By Paul

//extern void SubCameraDigtalPDNCtrl(u32 onoff);

extern int iReadRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u8 * a_pRecvData, u16 a_sizeRecvData, u16 i2cId);
extern int iWriteRegI2C(u8 *a_pSendData , u16 a_sizeSendData, u16 i2cId);
kal_uint16 HI706_write_cmos_sensor(kal_uint8 addr, kal_uint8 para)
{
    char puSendCmd[2] = {(char)(addr & 0xFF) , (char)(para & 0xFF)};
    iWriteRegI2C(puSendCmd , 2,HI706_WRITE_ID);
    return 0;
}
kal_uint16 HI706_read_cmos_sensor(kal_uint8 addr)
{
    kal_uint16 get_byte=0;
    char puSendCmd = { (char)(addr & 0xFF) };
    iReadRegI2C(&puSendCmd , 1, (u8*)&get_byte,1,HI706_WRITE_ID);
    return get_byte;
}


/*******************************************************************************
* // Adapter for Winmo typedef 
********************************************************************************/
#define WINMO_USE 0

#define Sleep(ms) mdelay(ms)
#define RETAILMSG(x,...)
#define TEXT


/*******************************************************************************
* follow is define by jun
********************************************************************************/
MSDK_SENSOR_CONFIG_STRUCT HI706SensorConfigData;

static struct HI706_sensor_STRUCT HI706_sensor;
static kal_uint32 HI706_zoom_factor = 0; 
static int sensor_id_fail = 0;	
const HI706_SENSOR_INIT_INFO HI706_Initial_Setting_Info[] =
{
  
    //PAGE 0
    //Image Size/Windowing/HSYNC/VSYNC[Type1]
    {0x03, 0x00},   //PAGEMODE(0x03)
    {0x01, 0xf1},
    {0x01, 0xf3},   //PWRCTL(0x01[P0])Bit[1]:Software Reset.
    {0x01, 0xf1},

    {0x11, 0x90},   //For No Fixed Framerate Bit[2]
    {0x12, 0x04},  //PCLK INV
        
    {0x20, 0x00},
    {0x21, 0x04},
    {0x22, 0x00},
    {0x23, 0x04},

    {0x24, 0x01},
    {0x25, 0xe0},
    {0x26, 0x02},
    {0x27, 0x80},

	{0x40, 0x01},   //HBLANK: 0x70 = 112
    {0x41, 0x58},
    {0x42, 0x00},   //VBLANK: 0x40 = 64
    {0x43, 0x13},   //0x04 -> 0x40: For Max Framerate = 30fps
            
    //BLC
    {0x80, 0x2e},
    {0x81, 0x7e},
    {0x82, 0x90},
    {0x83, 0x30},
    {0x84, 0x2c},
    {0x85, 0x4b},
    {0x89, 0x48},
        
    {0x90, 0x0a},
    {0x91, 0x0a},    
    {0x92, 0x48},
    {0x93, 0x40},
    {0x98, 0x38},
    {0x99, 0x40},
    {0xa0, 0x00},
    {0xa8, 0x40},
    
    //PAGE 2
    //Analog Circuit
    {0x03, 0x02},      
    {0x13, 0x40},
    {0x14, 0x04},
    {0x1a, 0x00},
    {0x1b, 0x08},
        
    {0x20, 0x33},
    {0x21, 0xaa},
    {0x22, 0xa7},
    {0x23, 0xb1},       //For Sun Pot
        
    {0x3b, 0x48},
        
    {0x50, 0x21},
    {0x52, 0xa2},
    {0x53, 0x0a},
    {0x54, 0x30},
    {0x55, 0x10},
    {0x56, 0x0c},
    {0x59, 0x0F},
        
    {0x60, 0x54},
    {0x61, 0x5d},
    {0x62, 0x56},
    {0x63, 0x5c},
    {0x64, 0x56},
    {0x65, 0x5c},
    {0x72, 0x57},
    {0x73, 0x5b},
    {0x74, 0x57},
    {0x75, 0x5b},
    {0x80, 0x02},
    {0x81, 0x46},
    {0x82, 0x07},
    {0x83, 0x10},
    {0x84, 0x07},
    {0x85, 0x10},
    {0x92, 0x24},
    {0x93, 0x30},
    {0x94, 0x24},
    {0x95, 0x30},
    {0xa0, 0x03},
    {0xa1, 0x45},
    {0xa4, 0x45},
    {0xa5, 0x03},
    {0xa8, 0x12},
    {0xa9, 0x20},
    {0xaa, 0x34},
    {0xab, 0x40},
    {0xb8, 0x55},
    {0xb9, 0x59},
    {0xbc, 0x05},
    {0xbd, 0x09},
    {0xc0, 0x5f},
    {0xc1, 0x67},
    {0xc2, 0x5f},
    {0xc3, 0x67},
    {0xc4, 0x60},
    {0xc5, 0x66},
    {0xc6, 0x60},
    {0xc7, 0x66},
    {0xc8, 0x61},
    {0xc9, 0x65},
    {0xca, 0x61},
    {0xcb, 0x65},
    {0xcc, 0x62},
    {0xcd, 0x64},
    {0xce, 0x62},
    {0xcf, 0x64},
    {0xd0, 0x53},
    {0xd1, 0x68},
     
    //PAGE 10
    //Image Format, Image Effect
    {0x03, 0x10},
    {0x10, 0x03},
    {0x11, 0x43},
    {0x12, 0x30},
    
        
	{0x40, 0x03},
	{0x41, 0x00},
    {0x48, 0x90},
        
    {0x50, 0x30},//AGBRT
           
    {0x60, 0x67},
    {0x61, 0x16},//bit4:5 over sat ratio
    {0x62, 0x80},//0xb0  //blue
    {0x63, 0x98},//0xa8// red
    {0x64, 0x30},//AGSAT
    {0x67, 0x67},//36
    {0x68, 0x80},//0x48
    
    //PAGE 11
    //Z-LPF
    {0x03, 0x11},
    {0x10, 0x21},   
    {0x11, 0x1f},   
        
    {0x20, 0x00},   
    {0x21, 0x38},   
    {0x23, 0x0a},
        
    {0x60, 0x08},   
    {0x61, 0x42},
    {0x62, 0x00},   
	{0x63, 0x85},
	{0x64, 0x85},
    {0x67, 0xf4},   
    {0x68, 0x40},   
    {0x69, 0x10},   
    
    //PAGE 12
    //2D
    {0x03, 0x12},
        
	{0x40, 0xeb},//0xe9,
	{0x41, 0x09},
        
	{0x50, 0x18},
	{0x51, 0x28},//0x24
        
    {0x70, 0x1f},
    {0x71, 0x00},
    {0x72, 0x00},
    {0x73, 0x00},
    {0x74, 0x10},
    {0x75, 0x10},
    {0x76, 0x20},
    {0x77, 0x80},
    {0x78, 0x88},
    {0x79, 0x18},
        
    {0xb0, 0x7d},

    //PAGE 13
    //Edge Enhancement
    {0x03, 0x13},
    {0x10, 0x01},   
    {0x11, 0x89},   
    {0x12, 0x14},   
    {0x13, 0x19},   
    {0x14, 0x08},
        
	{0x20, 0x07},
	{0x21, 0x05},
    {0x23, 0x30},
    {0x24, 0x33},
    {0x25, 0x08},
    {0x26, 0x18},
    {0x27, 0x00},
    {0x28, 0x08},
    {0x29, 0x50},
    {0x2a, 0xe0},
    {0x2b, 0x10},
    {0x2c, 0x28},
    {0x2d, 0x40},
    {0x2e, 0x00},
    {0x2f, 0x00},

    //PAGE 11
    {0x30, 0x11},
        
    {0x80, 0x03},
    {0x81, 0x07},
        
	{0x90, 0x05},
    {0x91, 0x03},
    {0x92, 0x00},
    {0x93, 0x20},
    {0x94, 0x42},
    {0x95, 0x60},
    
    //PAGE 14
    //Lens Shading Correction
    {0x03, 0x14},
    {0x10, 0x01},
        
    {0x20, 0x80},   //For Y decay
    {0x21, 0x80},   //For Y decay
    {0x22, 0x78},
    {0x23, 0x4d},
    {0x24, 0x46},
    
    //PAGE 15 
    //Color Correction
    {0x03, 0x15}, 
    {0x10, 0x03}, 
    {0x14, 0x3c},
    {0x16, 0x2c},
    {0x17, 0x2f},
          
    {0x30, 0xcf},//0xcb
    {0x31, 0x61},
    {0x32, 0x16},
    {0x33, 0x23},
    {0x34, 0xc1},//0xce
    {0x35, 0x2b},
    {0x36, 0x01},
    {0x37, 0x34},
    {0x38, 0x75},
           
    {0x40, 0x87},
    {0x41, 0x18},
    {0x42, 0x91},
    {0x43, 0x94},
    {0x44, 0x9f},
    {0x45, 0x33},
    {0x46, 0x00},
    {0x47, 0x94},
    {0x48, 0x14},
    
    //PAGE 16
    //Gamma Correction
    {0x03,  0x16},
        
    {0x30,  0x00},
	{0x31,  0x0a},
	{0x32,  0x1b},
	{0x33,  0x2e},
	{0x34,  0x5c},
	{0x35,  0x79},
	{0x36,  0x95},
	{0x37,  0xa4},
	{0x38,  0xb1},
	{0x39,  0xbd},
	{0x3a,  0xc8},
	{0x3b,  0xd9},
	{0x3c,  0xe8},
	{0x3d,  0xf5},
	{0x3e,  0xff},
    
    //PAGE 17 
    //Auto Flicker Cancellation 
    {0x03, 0x17},
        
    {0xc4, 0x3c},
    {0xc5, 0x32},
    
    //PAGE 20 
    //AE 
    {0x03, 0x20},
        
    {0x10, 0x0c},
    {0x11, 0x04},
           
    {0x20, 0x01},
    {0x28, 0x27},
    {0x29, 0xa1},   
    {0x2a, 0xf0},
    {0x2b, 0x34},
    {0x2c, 0x2b},
           
    {0x30, 0xf8},
    {0x39, 0x22},
    {0x3a, 0xde},
    {0x3b, 0x22},
    {0x3c, 0xde},
    
    {0x60, 0xc0},//0x95
    {0x68, 0x3c},
    {0x69, 0x64},
    {0x6A, 0x28},
    {0x6B, 0xc8},
    
	{0x70, 0x3a},
    {0x76, 0x22},
    {0x77, 0x02},   
    {0x78, 0x12},
    {0x79, 0x27},
    {0x7a, 0x23},  
    {0x7c, 0x1d},
    {0x7d, 0x22},
    
    {0x83, 0x00},//expTime:0x83,0x84,0x85
    {0x84, 0xbe},
    {0x85, 0x6e}, 
        
    {0x86, 0x00},//expMin is minimum time of expTime,
    {0x87, 0xfa},
        
    {0x88, 0x02},
    {0x89, 0x7a},
    {0x8a, 0xc4},    
        
    {0x8b, 0x3f},
    {0x8c, 0x7a},  
        
    {0x8d, 0x34},
    {0x8e, 0xbc},
    
    {0x91, 0x02},
    {0x92, 0xdc},
    {0x93, 0x6c},   
    {0x94, 0x01},
    {0x95, 0xb7},
    {0x96, 0x74},   
    {0x98, 0x8C},
    {0x99, 0x23},  
        
    {0x9c, 0x0b},   //For Y decay: Exposure Time
    {0x9d, 0xb8},   //For Y decay: Exposure Time
    {0x9e, 0x00},
    {0x9f, 0xfa},
    
    {0xb1, 0x14},
    {0xb2, 0x50},
    {0xb4, 0x14},
    {0xb5, 0x38},
    {0xb6, 0x26},
    {0xb7, 0x20},
    {0xb8, 0x1d},
    {0xb9, 0x1b},
    {0xba, 0x1a},
    {0xbb, 0x19},
    {0xbc, 0x19},
    {0xbd, 0x18},
    
    {0xc0, 0x16},   //0x1a->0x16
    {0xc3, 0x48},
    {0xc4, 0x48}, 
    
    //PAGE 22 
    //AWB
    {0x03, 0x22},
    {0x10, 0xe2},
    {0x11, 0x26},
        
    {0x21, 0x40},
           
    {0x30, 0x80},
    {0x31, 0x81},
    {0x38, 0x12},
    {0x39, 0x33},
        
    {0x40, 0xf0},
    {0x41, 0x33},//0x33
    {0x42, 0x33},
    {0x43, 0xf3},
    {0x44, 0x43},
    {0x45, 0x32},
    {0x46, 0x02},
           
    {0x80, 0x45},
    {0x81, 0x20},
    {0x82, 0x48},
    {0x83, 0x52},
    {0x84, 0x1b},
    {0x85, 0x50},
    {0x86, 0x25},
    {0x87, 0x4d},
    {0x88, 0x38},
    {0x89, 0x3e},
    {0x8a, 0x29},
    {0x8b, 0x02},
    {0x8d, 0x22},
    {0x8e, 0x71},  
    {0x8f, 0x63},
        
    {0x90, 0x60},
    {0x91, 0x5c},
    {0x92, 0x54},//0x56
    {0x93, 0x4b},//0x52
    {0x94, 0x48},//0x4c
    {0x95, 0x34},//0x36
    {0x96, 0x31},//0x31
    {0x97, 0x2d},//0x2e
    {0x98, 0x2a},//0x2a
    {0x99, 0x29},
    {0x9a, 0x26},
    {0x9b, 0x07},

    //PAGE 22
    {0x03, 0x22},
    {0x10, 0xfb},

    //PAGE 20
    {0x03, 0x20},
    {0x10, 0x9c},
    
    {0x01, 0xf0},

    //PAGE 0
    {0x03, 0x00},
    {0x01, 0x90},   //0xf1 ->0x41 : For Preview Green/Red Line.

    {0xff, 0xff}    //End of Initial Setting
};

static void HI706_Set_VGA_mode(void)
{
    HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)|0x01);   //Sleep: For Write Reg

    HI706_write_cmos_sensor(0x03, 0x00);
    HI706_write_cmos_sensor(0x10, 0x00);        //VGA Size

    HI706_write_cmos_sensor(0x20, 0x00);
    HI706_write_cmos_sensor(0x21, 0x04);

    HI706_write_cmos_sensor(0x40, 0x01);        //HBLANK: 0x70 = 112
    HI706_write_cmos_sensor(0x41, 0x58);
    HI706_write_cmos_sensor(0x42, 0x00);        //VBLANK: 0x04 = 4
    HI706_write_cmos_sensor(0x43, 0x13);

    HI706_write_cmos_sensor(0x03, 0x11);
    HI706_write_cmos_sensor(0x10, 0x21);  

    HI706_write_cmos_sensor(0x03, 0x20);

    HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)&0x7f);   //Close AE
    //HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)|0x08);   //Reset AE
	

    HI706_write_cmos_sensor(0x86, 0x00);
    HI706_write_cmos_sensor(0x87, 0xfa);

    HI706_write_cmos_sensor(0x8b, 0x3f);
    HI706_write_cmos_sensor(0x8c, 0x7a);
    HI706_write_cmos_sensor(0x8d, 0x34);
    HI706_write_cmos_sensor(0x8e, 0xbc);

    HI706_write_cmos_sensor(0x9c, 0x0b);
    HI706_write_cmos_sensor(0x9d, 0xb8);
    HI706_write_cmos_sensor(0x9e, 0x00);
    HI706_write_cmos_sensor(0x9f, 0xfa);

    HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);   //Exit Sleep: For Write Reg

    HI706_write_cmos_sensor(0x03, 0x20);
    HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);   //Open AE
    //HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);   //Reset AE

}

static void HI706_Initial_Setting(void)
{
	kal_uint32 iEcount;
	for(iEcount=0;(!((0xff==(HI706_Initial_Setting_Info[iEcount].address))&&(0xff==(HI706_Initial_Setting_Info[iEcount].data))));iEcount++)
	{	
		HI706_write_cmos_sensor(HI706_Initial_Setting_Info[iEcount].address, HI706_Initial_Setting_Info[iEcount].data);
	}
	
	HI706_Set_VGA_mode();
}

static void HI706_Init_Parameter(void)
{
    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.first_init = KAL_TRUE;
    HI706_sensor.pv_mode = KAL_TRUE;
    HI706_sensor.night_mode = KAL_FALSE;
    HI706_sensor.MPEG4_Video_mode = KAL_FALSE;

    HI706_sensor.cp_pclk = HI706_sensor.pv_pclk;

    HI706_sensor.pv_dummy_pixels = 0;
    HI706_sensor.pv_dummy_lines = 0;
    HI706_sensor.cp_dummy_pixels = 0;
    HI706_sensor.cp_dummy_lines = 0;

    HI706_sensor.wb = 0;
    HI706_sensor.exposure = 0;
    HI706_sensor.effect = 0;
    HI706_sensor.banding = AE_FLICKER_MODE_50HZ;

    HI706_sensor.pv_line_length = 640;
    HI706_sensor.pv_frame_height = 480;
    HI706_sensor.cp_line_length = 640;
    HI706_sensor.cp_frame_height = 480;
    spin_unlock(&hi706_yuv_drv_lock);    
}

static kal_uint8 HI706_power_on(void)
{
    kal_uint8 HI706_sensor_id = 0;
    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.pv_pclk = 13000000;
    spin_unlock(&hi706_yuv_drv_lock);
    //Software Reset
    HI706_write_cmos_sensor(0x01,0xf1);
    HI706_write_cmos_sensor(0x01,0xf3);
    HI706_write_cmos_sensor(0x01,0xf1);

    /* Read Sensor ID  */
    HI706_sensor_id = HI706_read_cmos_sensor(0x04);
    SENSORDB("[HI706YUV]:read Sensor ID:%x\n",HI706_sensor_id);	
    return HI706_sensor_id;
}


/*************************************************************************
* FUNCTION
*	HI706Open
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
UINT32 HI706Open(void)
{
    spin_lock(&hi706_yuv_drv_lock);
    sensor_id_fail = 0; 
    spin_unlock(&hi706_yuv_drv_lock);
    SENSORDB("[Enter]:HI706 Open func:");

    if (HI706_power_on() != HI706_SENSOR_ID) 
    {
        SENSORDB("[HI706]Error:read sensor ID fail\n");
        spin_lock(&hi706_yuv_drv_lock);
        sensor_id_fail = 1;
        spin_unlock(&hi706_yuv_drv_lock);
        return ERROR_SENSOR_CONNECT_FAIL;
    } else {
       // SubCameraDigtalPDNCtrl(1);
    }

    /* Apply sensor initail setting*/
    HI706_Initial_Setting();
    HI706_Init_Parameter(); 

    SENSORDB("[Exit]:HI706 Open func\n");     
    return ERROR_NONE;
}	/* HI706Open() */

/*************************************************************************
* FUNCTION
*	HI706_GetSensorID
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
static kal_uint32 HI706_GetSensorID(kal_uint32 *sensorID)
{
    SENSORDB("[Enter]:HI706 Open func ");
    *sensorID = HI706_power_on() ;

    if (*sensorID != HI706_SENSOR_ID) 
    {
        SENSORDB("[HI706]Error:read sensor ID fail\n");
        spin_lock(&hi706_yuv_drv_lock);
        sensor_id_fail = 1;
        spin_unlock(&hi706_yuv_drv_lock);
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    } else {
     //   SubCameraDigtalPDNCtrl(1);
    }
	//Jieve Liu
	//spin_lock(&hi706_yuv_drv_lock);
	//strcpy(HI706_sensor.vendor, "CAMERAKING");
	//spin_unlock(&hi706_yuv_drv_lock);

     return ERROR_NONE;    
}   /* HI706Open  */


/*************************************************************************
* FUNCTION
*	HI706Close
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
UINT32 HI706Close(void)
{
    if(sensor_id_fail == 0){
	    mt_set_gpio_mode(GPIO_CAMERA_CMPDN1_PIN,GPIO_MODE_00);
	    mt_set_gpio_dir(GPIO_CAMERA_CMPDN1_PIN,GPIO_DIR_OUT);
	    mt_set_gpio_out(GPIO_CAMERA_CMPDN1_PIN,GPIO_OUT_ZERO);
    }
	return ERROR_NONE;
}	/* HI706Close() */


static void HI706_Set_Mirror_Flip(kal_uint8 image_mirror)
{
    /********************************************************
    * Page Mode 0: Reg 0x0011 bit[1:0] = [Y Flip : X Flip]
    * 0: Off; 1: On.
    *********************************************************/ 
    kal_uint8 temp_data;   
    SENSORDB("[Enter]:HI706 set Mirror_flip func:image_mirror=%d\n",image_mirror);	
    HI706_write_cmos_sensor(0x03,0x00);     //Page 0	
    temp_data = (HI706_read_cmos_sensor(0x11) & 0xfc);
    spin_lock(&hi706_yuv_drv_lock);
    //HI706_sensor.mirror = (HI706_read_cmos_sensor(0x11) & 0xfc); 
    switch (image_mirror) 
    {
    case IMAGE_NORMAL:
        //HI706_sensor.mirror |= 0x00;
        temp_data |= 0x00;
        break;
    case IMAGE_H_MIRROR:
        //HI706_sensor.mirror |= 0x01;
        temp_data |= 0x01;
        break;
    case IMAGE_V_MIRROR:
        //HI706_sensor.mirror |= 0x02;
        temp_data |= 0x02;
        break;
    case IMAGE_HV_MIRROR:
        //HI706_sensor.mirror |= 0x03;
        temp_data |= 0x03;
        break;
    default:
        //HI706_sensor.mirror |= 0x00;
        temp_data |= 0x00;
    }
    HI706_sensor.mirror = temp_data;
    spin_unlock(&hi706_yuv_drv_lock);
    HI706_write_cmos_sensor(0x11, HI706_sensor.mirror);
    SENSORDB("[Exit]:HI706 set Mirror_flip func\n");
}

#if 0
static void HI706_set_dummy(kal_uint16 dummy_pixels,kal_uint16 dummy_lines)
{	
    HI706_write_cmos_sensor(0x03, 0x00);                        //Page 0
    HI706_write_cmos_sensor(0x40,((dummy_pixels & 0x0F00))>>8);       //HBLANK
    HI706_write_cmos_sensor(0x41,(dummy_pixels & 0xFF));
    HI706_write_cmos_sensor(0x42,((dummy_lines & 0xFF00)>>8));       //VBLANK ( Vsync Type 1)
    HI706_write_cmos_sensor(0x43,(dummy_lines & 0xFF));
}  
#endif

static void HI706_Cal_Min_Frame_Rate(kal_uint16 min_framerate)
{
    kal_uint32 HI706_expmax = 0;
    kal_uint32 HI706_expbanding = 0;
    kal_uint32 temp_data;
      
    SENSORDB("[HI706] HI706_Cal_Min_Frame_Rate:min_fps=%d\n",min_framerate);

    //No Fixed Framerate
    HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)|0x01);   //Sleep: For Write Reg
    HI706_write_cmos_sensor(0x03, 0x00);
    HI706_write_cmos_sensor(0x11, HI706_read_cmos_sensor(0x11)&0xfb);

    HI706_write_cmos_sensor(0x03, 0x20);
    HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)&0x7f);   //Close AE

    HI706_write_cmos_sensor(0x11, 0x04);
    HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)|0x08);   //Reset AE
    HI706_write_cmos_sensor(0x2a, 0xf0);
    HI706_write_cmos_sensor(0x2b, 0x34);

    HI706_write_cmos_sensor(0x03, 0x00);
    temp_data = ((HI706_read_cmos_sensor(0x40)<<8)|HI706_read_cmos_sensor(0x41));
    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.pv_dummy_pixels = temp_data;
    HI706_sensor.pv_line_length = HI706_VGA_DEFAULT_PIXEL_NUMS+ HI706_sensor.pv_dummy_pixels ;
    spin_unlock(&hi706_yuv_drv_lock);

    if(HI706_sensor.banding == AE_FLICKER_MODE_50HZ)
    {
        HI706_expbanding = (HI706_sensor.pv_pclk/HI706_sensor.pv_line_length/100)*HI706_sensor.pv_line_length/8;
        HI706_expmax = HI706_expbanding*10*(100/min_framerate) ;
    }
    else if(HI706_sensor.banding == AE_FLICKER_MODE_60HZ)
    {
        HI706_expbanding = (HI706_sensor.pv_pclk/HI706_sensor.pv_line_length/120)*HI706_sensor.pv_line_length/8;
        HI706_expmax = HI706_expbanding*10*(120/min_framerate) ;
    }
    else//default 5oHZ
    {
        //SENSORDB("[HI706][Error] Wrong Banding Setting!!!...");
        HI706_expbanding = (HI706_sensor.pv_pclk/HI706_sensor.pv_line_length/100)*HI706_sensor.pv_line_length/8;
        HI706_expmax = HI706_expbanding*10*(100/min_framerate) ;

	}
        
    HI706_write_cmos_sensor(0x03, 0x20);
    HI706_write_cmos_sensor(0x88, (HI706_expmax>>16)&0xff);
    HI706_write_cmos_sensor(0x89, (HI706_expmax>>8)&0xff);
    HI706_write_cmos_sensor(0x8a, (HI706_expmax>>0)&0xff);

    HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);   //Exit Sleep: For Write Reg

    HI706_write_cmos_sensor(0x03, 0x20);
    HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);   //Open AE
    HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);   //Reset AE
}


static void HI706_Fix_Video_Frame_Rate(kal_uint16 fix_framerate)
{
    kal_uint32 HI706_expfix;
    kal_uint32 HI706_expfix_temp;
    kal_uint32 HI706_expmax = 0;
    kal_uint32 HI706_expbanding = 0;
    kal_uint32 temp_data1,temp_data2;
      
    SENSORDB("[Enter]HI706 Fix_video_frame_rate func: fix_fps=%d\n",fix_framerate);

    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.video_current_frame_rate = fix_framerate;
    spin_unlock(&hi706_yuv_drv_lock);
    // Fixed Framerate
    HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)|0x01);   //Sleep: For Write Reg

    HI706_write_cmos_sensor(0x03, 0x00);
    HI706_write_cmos_sensor(0x11, HI706_read_cmos_sensor(0x11)|0x04);

    HI706_write_cmos_sensor(0x03, 0x20);
    HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)&0x7f);   //Close AE

    HI706_write_cmos_sensor(0x11, 0x00);
    HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)|0x08);   //Reset AE
    HI706_write_cmos_sensor(0x2a, 0x00);
    HI706_write_cmos_sensor(0x2b, 0x35);

    HI706_write_cmos_sensor(0x03, 0x00);
    temp_data1 = ((HI706_read_cmos_sensor(0x40)<<8)|HI706_read_cmos_sensor(0x41));
    temp_data2 = ((HI706_read_cmos_sensor(0x42)<<8)|HI706_read_cmos_sensor(0x43));
    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.pv_dummy_pixels = temp_data1; 
    HI706_sensor.pv_line_length = HI706_VGA_DEFAULT_PIXEL_NUMS + HI706_sensor.pv_dummy_pixels ;   
    HI706_sensor.pv_dummy_lines = temp_data2;
    spin_unlock(&hi706_yuv_drv_lock);
        
    HI706_expfix_temp = ((HI706_sensor.pv_pclk*10/fix_framerate)-(HI706_sensor.pv_line_length*HI706_sensor.pv_dummy_lines))/8;    
    HI706_expfix = ((HI706_expfix_temp*8/HI706_sensor.pv_line_length)*HI706_sensor.pv_line_length)/8;
        
    HI706_write_cmos_sensor(0x03, 0x20);    
    //HI706_write_cmos_sensor(0x83, (HI706_expfix>>16)&0xff);
    //HI706_write_cmos_sensor(0x84, (HI706_expfix>>8)&0xff);
    //HI706_write_cmos_sensor(0x85, (HI706_expfix>>0)&0xff);    
    HI706_write_cmos_sensor(0x91, (HI706_expfix>>16)&0xff);
    HI706_write_cmos_sensor(0x92, (HI706_expfix>>8)&0xff);
    HI706_write_cmos_sensor(0x93, (HI706_expfix>>0)&0xff);

    if(HI706_sensor.banding == AE_FLICKER_MODE_50HZ)
    {
        HI706_expbanding = ((HI706_read_cmos_sensor(0x8b)<<8)|HI706_read_cmos_sensor(0x8c));
		
		HI706_expmax = ((HI706_expfix_temp-HI706_expbanding)/HI706_expbanding)*HI706_expbanding;	
		
		HI706_write_cmos_sensor(0x03, 0x20);
		HI706_write_cmos_sensor(0x88, (HI706_expmax>>16)&0xff);
		HI706_write_cmos_sensor(0x89, (HI706_expmax>>8)&0xff);
		HI706_write_cmos_sensor(0x8a, (HI706_expmax>>0)&0xff);
		
		HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);	//Exit Sleep: For Write Reg
		
		HI706_write_cmos_sensor(0x03, 0x20);
		HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);	//Open AE
		HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);	//Reset AE
    }
    else if(HI706_sensor.banding == AE_FLICKER_MODE_60HZ)
    {
        HI706_expbanding = ((HI706_read_cmos_sensor(0x8d)<<8)|HI706_read_cmos_sensor(0x8e));
		
		HI706_expmax = ((HI706_expfix_temp-HI706_expbanding)/HI706_expbanding)*HI706_expbanding;	
		
		HI706_write_cmos_sensor(0x03, 0x20);
		HI706_write_cmos_sensor(0x88, (HI706_expmax>>16)&0xff);
		HI706_write_cmos_sensor(0x89, (HI706_expmax>>8)&0xff);
		HI706_write_cmos_sensor(0x8a, (HI706_expmax>>0)&0xff);
		
		HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);	//Exit Sleep: For Write Reg
		
		HI706_write_cmos_sensor(0x03, 0x20);
		HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);	//Open AE
		HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);	//Reset AE
    }
    else//default 50HZ
    {
        HI706_expbanding = ((HI706_read_cmos_sensor(0x8b)<<8)|HI706_read_cmos_sensor(0x8c));
		
		HI706_expmax = ((HI706_expfix_temp-HI706_expbanding)/HI706_expbanding)*HI706_expbanding;	
		
		HI706_write_cmos_sensor(0x03, 0x20);
		HI706_write_cmos_sensor(0x88, (HI706_expmax>>16)&0xff);
		HI706_write_cmos_sensor(0x89, (HI706_expmax>>8)&0xff);
		HI706_write_cmos_sensor(0x8a, (HI706_expmax>>0)&0xff);
		
		HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);	//Exit Sleep: For Write Reg
		
		HI706_write_cmos_sensor(0x03, 0x20);
		HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);	//Open AE
		HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);	//Reset AE
    }
}

#if 0
// 320 * 240
static void HI706_Set_QVGA_mode(void)
{
    HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)|0x01);   //Sleep: For Write Reg
    
    HI706_write_cmos_sensor(0x03, 0x00);
    HI706_write_cmos_sensor(0x10, 0x01);        //QVGA Size: 0x10 -> 0x01

    HI706_write_cmos_sensor(0x20, 0x00);
    HI706_write_cmos_sensor(0x21, 0x02);

    HI706_write_cmos_sensor(0x40, 0x01);        //HBLANK:  0x0158 = 344
    HI706_write_cmos_sensor(0x41, 0x58);
    HI706_write_cmos_sensor(0x42, 0x00);        //VBLANK:  0x14 = 20
    HI706_write_cmos_sensor(0x43, 0x14);

    HI706_write_cmos_sensor(0x03, 0x11);        //QVGA Fixframerate
    HI706_write_cmos_sensor(0x10, 0x21);  

    HI706_write_cmos_sensor(0x03, 0x20);
    HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)&0x7f);   //Close AE
    HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)|0x08);   //Reset AE

    HI706_write_cmos_sensor(0x83, 0x00);
    HI706_write_cmos_sensor(0x84, 0xaf);
    HI706_write_cmos_sensor(0x85, 0xc8);
    HI706_write_cmos_sensor(0x86, 0x00);
    HI706_write_cmos_sensor(0x87, 0xfa);

    HI706_write_cmos_sensor(0x8b, 0x3a);
    HI706_write_cmos_sensor(0x8c, 0x98);
    HI706_write_cmos_sensor(0x8d, 0x30);
    HI706_write_cmos_sensor(0x8e, 0xd4);

    HI706_write_cmos_sensor(0x9c, 0x0b);
    HI706_write_cmos_sensor(0x9d, 0x3b);
    HI706_write_cmos_sensor(0x9e, 0x00);
    HI706_write_cmos_sensor(0x9f, 0xfa);

    HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);   //Exit Sleep: For Write Reg

    HI706_write_cmos_sensor(0x03, 0x20);
    HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);   //Open AE
    HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);   //Reset AE

}
#endif
BOOL HI706_set_param_wb(UINT16 para);
void HI706_night_mode(kal_bool enable)
{
    SENSORDB("[Enter]HI706 night mode func:enable = %d\n",enable);
    SENSORDB("HI706_sensor.video_mode = %d\n",HI706_sensor.MPEG4_Video_mode); 
    SENSORDB("HI706_sensor.night_mode = %d\n",HI706_sensor.night_mode);
    SENSORDB("HI706_sensor.banding = %d\n",HI706_sensor.banding);
    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.night_mode = enable;
    spin_unlock(&hi706_yuv_drv_lock);

    if(HI706_sensor.MPEG4_Video_mode == KAL_TRUE)
        return;

    if(enable)
    {
        HI706_set_param_wb(AWB_MODE_AUTO);//Lock auto AWB in night mode.
        //HI706_Cal_Min_Frame_Rate(HI706_MIN_FRAMERATE_5);                            
        if(HI706_sensor.banding == AE_FLICKER_MODE_50HZ)
        {
            HI706_write_cmos_sensor(0x03, 0x20);

            HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)&0x7f);   //Close AE
            HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)|0x08);   //Reset AE

            HI706_write_cmos_sensor(0x83, 0x04);
            HI706_write_cmos_sensor(0x84, 0xf5);
            HI706_write_cmos_sensor(0x85, 0x88);
            HI706_write_cmos_sensor(0x86, 0x00);
            HI706_write_cmos_sensor(0x87, 0xfa);

            HI706_write_cmos_sensor(0x88, 0x04);
            HI706_write_cmos_sensor(0x89, 0xf5);
            HI706_write_cmos_sensor(0x8a, 0x88);

            HI706_write_cmos_sensor(0x8b, 0x3f);
            HI706_write_cmos_sensor(0x8c, 0x7a);
            HI706_write_cmos_sensor(0x8d, 0x34);
            HI706_write_cmos_sensor(0x8e, 0xbc);

            HI706_write_cmos_sensor(0x9c, 0x0b);
            HI706_write_cmos_sensor(0x9d, 0xd6);
            HI706_write_cmos_sensor(0x9e, 0x00);
            HI706_write_cmos_sensor(0x9f, 0xfa);

            HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);   //Exit Sleep: For Write Reg

            HI706_write_cmos_sensor(0x03, 0x20);
            HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);   //Open AE
            HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);   //Reset AE
        }
       else 
       {
            HI706_write_cmos_sensor(0x03, 0x20);

            HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)&0x7f);   //Close AE
            HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)|0x08);   //Reset AE

            HI706_write_cmos_sensor(0x83, 0x04);
            HI706_write_cmos_sensor(0x84, 0xf1);
            HI706_write_cmos_sensor(0x85, 0xa0);
            HI706_write_cmos_sensor(0x86, 0x00);
            HI706_write_cmos_sensor(0x87, 0xfa);

            HI706_write_cmos_sensor(0x88, 0x04);
            HI706_write_cmos_sensor(0x89, 0xf1);
            HI706_write_cmos_sensor(0x8a, 0xa0);

            HI706_write_cmos_sensor(0x8b, 0x3f);
            HI706_write_cmos_sensor(0x8c, 0x7a);
            HI706_write_cmos_sensor(0x8d, 0x34);
            HI706_write_cmos_sensor(0x8e, 0xbc);

            HI706_write_cmos_sensor(0x9c, 0x06);
            HI706_write_cmos_sensor(0x9d, 0xd6);
            HI706_write_cmos_sensor(0x9e, 0x00);
            HI706_write_cmos_sensor(0x9f, 0xfa);

            HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);   //Exit Sleep: For Write Reg

            HI706_write_cmos_sensor(0x03, 0x20);
            HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);   //Open AE
            HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);   //Reset AE
        }
    }
    else
    {
        //HI706_Cal_Min_Frame_Rate(HI706_MIN_FRAMERATE_10);
        if(HI706_sensor.banding == AE_FLICKER_MODE_50HZ)
        {
            HI706_write_cmos_sensor(0x03, 0x20);

            HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)&0x7f);   //Close AE
            HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)|0x08);   //Reset AE

            HI706_write_cmos_sensor(0x83, 0x02);
            HI706_write_cmos_sensor(0x84, 0xf9);
            HI706_write_cmos_sensor(0x85, 0xb8);
            HI706_write_cmos_sensor(0x86, 0x00);
            HI706_write_cmos_sensor(0x87, 0xfa);

            HI706_write_cmos_sensor(0x88, 0x03);
            HI706_write_cmos_sensor(0x89, 0xf7);
            HI706_write_cmos_sensor(0x8a, 0xa0);


            HI706_write_cmos_sensor(0x8b, 0x3f);
            HI706_write_cmos_sensor(0x8c, 0x7a);
            HI706_write_cmos_sensor(0x8d, 0x34);
            HI706_write_cmos_sensor(0x8e, 0xbc);

            HI706_write_cmos_sensor(0x9c, 0x06);
            HI706_write_cmos_sensor(0x9d, 0xd6);
            HI706_write_cmos_sensor(0x9e, 0x00);
            HI706_write_cmos_sensor(0x9f, 0xfa);

            HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);   //Exit Sleep: For Write Reg

            HI706_write_cmos_sensor(0x03, 0x20);
            HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);   //Open AE
            HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);   //Reset AE
        }
        else 
        {
            HI706_write_cmos_sensor(0x03, 0x20);
            HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)&0x7f);	//Close AE
            HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)|0x08);	//Reset AE

            HI706_write_cmos_sensor(0x83, 0x03);
            HI706_write_cmos_sensor(0x84, 0x17);
            HI706_write_cmos_sensor(0x85, 0x04);
            HI706_write_cmos_sensor(0x86, 0x00);
            HI706_write_cmos_sensor(0x87, 0xfa);

            HI706_write_cmos_sensor(0x88, 0x04);
            HI706_write_cmos_sensor(0x89, 0x1e);
            HI706_write_cmos_sensor(0x8a, 0xb0);

            HI706_write_cmos_sensor(0x8b, 0x3f);
            HI706_write_cmos_sensor(0x8c, 0x7a);
            HI706_write_cmos_sensor(0x8d, 0x34);
            HI706_write_cmos_sensor(0x8e, 0xbc);

            HI706_write_cmos_sensor(0x9c, 0x06);
            HI706_write_cmos_sensor(0x9d, 0xd6);
            HI706_write_cmos_sensor(0x9e, 0x00);
            HI706_write_cmos_sensor(0x9f, 0xfa);

            HI706_write_cmos_sensor(0x01, HI706_read_cmos_sensor(0x01)&0xfe);	//Exit Sleep: For Write Reg

            HI706_write_cmos_sensor(0x03, 0x20);
            HI706_write_cmos_sensor(0x10, HI706_read_cmos_sensor(0x10)|0x80);	//Open AE
            HI706_write_cmos_sensor(0x18, HI706_read_cmos_sensor(0x18)&0xf7);	//Reset AE
        }
    }
}

/*************************************************************************
* FUNCTION
*	HI706Preview
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
static UINT32 HI706Preview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{       
    
    spin_lock(&hi706_yuv_drv_lock);
    sensor_config_data->SensorImageMirror = IMAGE_HV_MIRROR; 
    if(HI706_sensor.first_init == KAL_TRUE)
    {
        HI706_sensor.MPEG4_Video_mode = HI706_sensor.MPEG4_Video_mode;
    }
    else
    {
        HI706_sensor.MPEG4_Video_mode = !HI706_sensor.MPEG4_Video_mode;
    }
    spin_unlock(&hi706_yuv_drv_lock);

    SENSORDB("[Enter]:HI706 preview func:");		
    SENSORDB("HI706_sensor.video_mode = %d\n",HI706_sensor.MPEG4_Video_mode); 

    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.first_init = KAL_FALSE;	
    HI706_sensor.pv_mode = KAL_TRUE;		
    spin_unlock(&hi706_yuv_drv_lock);

    //{   
    //    SENSORDB("[HI706]preview set_VGA_mode\n");
    //
    //}

   // HI706_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);
   // HI706_Set_Mirror_Flip(IMAGE_V_MIRROR);

    SENSORDB("[Exit]:HI706 preview func\n");
   
    HI706_Set_VGA_mode();
    //HI706_night_mode(HI706_sensor.night_mode);
    return TRUE; 
}	/* HI706_Preview */


UINT32 HI706Capture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
					  MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    SENSORDB("[HI706][Enter]HI706_capture_func\n");
    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.pv_mode = KAL_FALSE;	
    spin_unlock(&hi706_yuv_drv_lock);
    CAPTURE_FLAG = 1;
    CAPTUREB_FLAG = 1;
    return ERROR_NONE;
}	/* HM3451Capture() */


UINT32 HI706GetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    SENSORDB("[Enter]:HI706 get Resolution func\n");

    pSensorResolution->SensorFullWidth=HI706_IMAGE_SENSOR_FULL_WIDTH ;  
    pSensorResolution->SensorFullHeight=HI706_IMAGE_SENSOR_FULL_HEIGHT ;
    pSensorResolution->SensorPreviewWidth=HI706_IMAGE_SENSOR_PV_WIDTH ;
    pSensorResolution->SensorPreviewHeight=HI706_IMAGE_SENSOR_PV_HEIGHT ;
    pSensorResolution->SensorVideoWidth=HI706_IMAGE_SENSOR_PV_WIDTH ;
    pSensorResolution->SensorVideoHeight=HI706_IMAGE_SENSOR_PV_HEIGHT ;
    pSensorResolution->Sensor3DFullWidth=HI706_IMAGE_SENSOR_FULL_WIDTH ;  
    pSensorResolution->Sensor3DFullHeight=HI706_IMAGE_SENSOR_FULL_HEIGHT ;
    pSensorResolution->Sensor3DPreviewWidth=HI706_IMAGE_SENSOR_PV_WIDTH ;
    pSensorResolution->Sensor3DPreviewHeight=HI706_IMAGE_SENSOR_PV_HEIGHT ;
    pSensorResolution->Sensor3DVideoWidth=HI706_IMAGE_SENSOR_PV_WIDTH ;
    pSensorResolution->Sensor3DVideoHeight=HI706_IMAGE_SENSOR_PV_HEIGHT ;

    SENSORDB("[Exit]:HI706 get Resolution func\n");	
    return ERROR_NONE;
}	/* HI706GetResolution() */

UINT32 HI706GetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
					  MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{

	switch(ScenarioId)
		{
		 
			case MSDK_SCENARIO_ID_CAMERA_ZSD:
				 pSensorInfo->SensorPreviewResolutionX=HI706_IMAGE_SENSOR_PV_WIDTH;
				 pSensorInfo->SensorPreviewResolutionY=HI706_IMAGE_SENSOR_PV_HEIGHT;
				 pSensorInfo->SensorFullResolutionX=HI706_IMAGE_SENSOR_FULL_WIDTH;
				 pSensorInfo->SensorFullResolutionY=HI706_IMAGE_SENSOR_FULL_HEIGHT;			 
				 pSensorInfo->SensorCameraPreviewFrameRate=15;
				 break;
	
			case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
			case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
				 pSensorInfo->SensorPreviewResolutionX=HI706_IMAGE_SENSOR_PV_WIDTH;
				 pSensorInfo->SensorPreviewResolutionY=HI706_IMAGE_SENSOR_PV_HEIGHT;
				 pSensorInfo->SensorFullResolutionX=HI706_IMAGE_SENSOR_FULL_WIDTH;
				 pSensorInfo->SensorFullResolutionY=HI706_IMAGE_SENSOR_FULL_HEIGHT;				 
				 pSensorInfo->SensorCameraPreviewFrameRate=30;
				 break;
			case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
				 pSensorInfo->SensorPreviewResolutionX=HI706_IMAGE_SENSOR_PV_WIDTH;
				 pSensorInfo->SensorPreviewResolutionY=HI706_IMAGE_SENSOR_PV_HEIGHT;
				 pSensorInfo->SensorFullResolutionX=HI706_IMAGE_SENSOR_FULL_WIDTH;
				 pSensorInfo->SensorFullResolutionY=HI706_IMAGE_SENSOR_FULL_HEIGHT;				 
				 pSensorInfo->SensorCameraPreviewFrameRate=30;			
				break;
			case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
			case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
			case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
				 pSensorInfo->SensorPreviewResolutionX=HI706_IMAGE_SENSOR_PV_WIDTH;
				 pSensorInfo->SensorPreviewResolutionY=HI706_IMAGE_SENSOR_PV_HEIGHT;
				 pSensorInfo->SensorFullResolutionX=HI706_IMAGE_SENSOR_FULL_WIDTH;
				 pSensorInfo->SensorFullResolutionY=HI706_IMAGE_SENSOR_FULL_HEIGHT; 			 
				 pSensorInfo->SensorCameraPreviewFrameRate=30;			
				break;
			default:
	
				 pSensorInfo->SensorPreviewResolutionX=HI706_IMAGE_SENSOR_PV_WIDTH;
				 pSensorInfo->SensorPreviewResolutionY=HI706_IMAGE_SENSOR_PV_HEIGHT;
				 pSensorInfo->SensorFullResolutionX=HI706_IMAGE_SENSOR_FULL_WIDTH;
				 pSensorInfo->SensorFullResolutionY=HI706_IMAGE_SENSOR_FULL_HEIGHT;				 
				 pSensorInfo->SensorCameraPreviewFrameRate=30;
				 break;
				 
			}
	


    SENSORDB("[Enter]:HI706 getInfo func:ScenarioId = %d\n",ScenarioId);

  //  pSensorInfo->SensorPreviewResolutionX=HI706_IMAGE_SENSOR_PV_WIDTH;
  //  pSensorInfo->SensorPreviewResolutionY=HI706_IMAGE_SENSOR_PV_HEIGHT;
 //   pSensorInfo->SensorFullResolutionX=HI706_IMAGE_SENSOR_FULL_WIDTH;
 //   pSensorInfo->SensorFullResolutionY=HI706_IMAGE_SENSOR_FULL_HEIGHT;

    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=30;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;//low is to reset 
    pSensorInfo->SensorResetDelayCount=4;  //4ms 
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV; //SENSOR_OUTPUT_FORMAT_YVYU;
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW;	
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorInterruptDelayLines = 1; 
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_PARALLEL;


    pSensorInfo->CaptureDelayFrame = 4; 
    pSensorInfo->PreviewDelayFrame = 4;//10; 
    pSensorInfo->VideoDelayFrame = 0; 
    pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_8MA;   	

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
    case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:		
        pSensorInfo->SensorClockFreq=26;
        pSensorInfo->SensorClockDividCount=	3;
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = 4; 
        pSensorInfo->SensorGrabStartY = 2;  	
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
        pSensorInfo->SensorClockFreq=26;
        pSensorInfo->SensorClockDividCount=	3;
        pSensorInfo->SensorClockRisingCount= 0;
        pSensorInfo->SensorClockFallingCount= 2;
        pSensorInfo->SensorPixelClockCount= 3;
        pSensorInfo->SensorDataLatchCount= 2;
        pSensorInfo->SensorGrabStartX = 4; 
        pSensorInfo->SensorGrabStartY = 2;//4;     			
        break;
    default:
        pSensorInfo->SensorClockFreq=26;
        pSensorInfo->SensorClockDividCount=3;
        pSensorInfo->SensorClockRisingCount=0;
        pSensorInfo->SensorClockFallingCount=2;
        pSensorInfo->SensorPixelClockCount=3;
        pSensorInfo->SensorDataLatchCount=2;
        pSensorInfo->SensorGrabStartX = 4; 
        pSensorInfo->SensorGrabStartY = 2;//4;     			
        break;
    }
    //	HI706_PixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &HI706SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    SENSORDB("[Exit]:HI706 getInfo func\n");	
    return ERROR_NONE;
}	/* HI706GetInfo() */


UINT32 HI706Control(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
					  MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    SENSORDB("[Enter]:HI706 Control func:ScenarioId = %d\n",ScenarioId);

    switch (ScenarioId)
    {
    case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
    case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
    //case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW:
    //case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:		
        HI706Preview(pImageWindow, pSensorConfigData); 
        break;
    case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
    case MSDK_SCENARIO_ID_CAMERA_ZSD:
    //case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE:
        HI706Capture(pImageWindow, pSensorConfigData); 
        break;
    //case MSDK_SCENARIO_ID_CAMERA_3D_PREVIEW: //added
    //case MSDK_SCENARIO_ID_CAMERA_3D_VIDEO:
    //case MSDK_SCENARIO_ID_CAMERA_3D_CAPTURE: //added   
    //   HI706Preview(pImageWindow, pSensorConfigData); 
    //   break;
    //case MSDK_SCENARIO_ID_CAMERA_ZSD:
    //   HI706Capture(pImageWindow, pSensorConfigData); 
    //   break;		
    default:
        HI706Preview(pImageWindow, pSensorConfigData); 
        break; 
    }

    SENSORDB("[Exit]:HI706 Control func\n");	
    return TRUE;
}	/* HI706Control() */


/*************************************************************************
* FUNCTION
*	HI706_set_param_wb
*
* DESCRIPTION
*	wb setting.
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
BOOL HI706_set_param_wb(UINT16 para)
{
    //This sensor need more time to balance AWB, 
    //we suggest higher fps or drop some frame to avoid garbage color when preview initial
    SENSORDB("[Enter]HI706 set_param_wb func:para = %d\n",para);
    SENSORDB("[Enter]HI706 set_param_wb func:HI706_sensor.wb = %d\n",HI706_sensor.wb);

    //if(HI706_sensor.wb == para) return KAL_TRUE;	

    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.wb = para;
    spin_unlock(&hi706_yuv_drv_lock);
    
    switch (para)
    {            
    case AWB_MODE_AUTO:
        {
        HI706_write_cmos_sensor(0x03, 0x22);			
        HI706_write_cmos_sensor(0x11, 0x2e);				
        HI706_write_cmos_sensor(0x80, 0x38);
        HI706_write_cmos_sensor(0x82, 0x38);				
        HI706_write_cmos_sensor(0x83, 0x5e);//0x5e
        HI706_write_cmos_sensor(0x84, 0x24);//0x24
        HI706_write_cmos_sensor(0x85, 0x5e);//0x5e
        HI706_write_cmos_sensor(0x86, 0x24);//0x24				
        HI706_write_cmos_sensor(0x10, 0xfb);				
        }                
        break;
    case AWB_MODE_CLOUDY_DAYLIGHT:
    {
        HI706_write_cmos_sensor(0x03, 0x22);
        HI706_write_cmos_sensor(0x11, 0x28);
        HI706_write_cmos_sensor(0x80, 0x71);
        HI706_write_cmos_sensor(0x82, 0x2b);
        HI706_write_cmos_sensor(0x83, 0x72);
        HI706_write_cmos_sensor(0x84, 0x70);
        HI706_write_cmos_sensor(0x85, 0x2b);
        HI706_write_cmos_sensor(0x86, 0x28);
        HI706_write_cmos_sensor(0x10, 0xfb);
        }			   
        break;
    case AWB_MODE_DAYLIGHT:
        {
        HI706_write_cmos_sensor(0x03, 0x22);
        HI706_write_cmos_sensor(0x11, 0x28);          
        HI706_write_cmos_sensor(0x80, 0x59);
        HI706_write_cmos_sensor(0x82, 0x29);
        HI706_write_cmos_sensor(0x83, 0x60);
        HI706_write_cmos_sensor(0x84, 0x50);
        HI706_write_cmos_sensor(0x85, 0x2f);
        HI706_write_cmos_sensor(0x86, 0x23);
        HI706_write_cmos_sensor(0x10, 0xfb);
        }      
        break;
    case AWB_MODE_INCANDESCENT:	
        {
        HI706_write_cmos_sensor(0x03, 0x22);
        HI706_write_cmos_sensor(0x11, 0x28);          
        HI706_write_cmos_sensor(0x80, 0x29);
        HI706_write_cmos_sensor(0x82, 0x54);
        HI706_write_cmos_sensor(0x83, 0x2e);
        HI706_write_cmos_sensor(0x84, 0x23);
        HI706_write_cmos_sensor(0x85, 0x58);
        HI706_write_cmos_sensor(0x86, 0x4f);
        HI706_write_cmos_sensor(0x10, 0xfb);
        }		
        break;  
    case AWB_MODE_FLUORESCENT:
        {
        HI706_write_cmos_sensor(0x03, 0x22);
        HI706_write_cmos_sensor(0x11, 0x28);
        HI706_write_cmos_sensor(0x80, 0x41);
        HI706_write_cmos_sensor(0x82, 0x42);
        HI706_write_cmos_sensor(0x83, 0x44);
        HI706_write_cmos_sensor(0x84, 0x34);
        HI706_write_cmos_sensor(0x85, 0x46);
        HI706_write_cmos_sensor(0x86, 0x3a);
        HI706_write_cmos_sensor(0x10, 0xfb);
        }	
        break;  
    case AWB_MODE_TUNGSTEN:
        {
        HI706_write_cmos_sensor(0x03, 0x22);
        HI706_write_cmos_sensor(0x80, 0x24);
        HI706_write_cmos_sensor(0x81, 0x20);
        HI706_write_cmos_sensor(0x82, 0x58);
        HI706_write_cmos_sensor(0x83, 0x27);
        HI706_write_cmos_sensor(0x84, 0x22);
        HI706_write_cmos_sensor(0x85, 0x58);
        HI706_write_cmos_sensor(0x86, 0x52);
        HI706_write_cmos_sensor(0x10, 0xfb);
        }
        break;
    case AWB_MODE_OFF:
        {
        SENSORDB("HI706 AWB OFF");
        HI706_write_cmos_sensor(0x03, 0x22);
        HI706_write_cmos_sensor(0x10, 0xe2);
        }
        break;
    default:
        return FALSE;
    }

    return TRUE;	
} /* HI706_set_param_wb */

/*************************************************************************
* FUNCTION
*	HI706_set_param_effect
*
* DESCRIPTION
*	effect setting.
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
BOOL HI706_set_param_effect(UINT16 para)
{
   SENSORDB("[Enter]HI706 set_param_effect func:para = %d\n",para);
   
    //if(HI706_sensor.effect == para) return KAL_TRUE;

    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.effect = para;
    spin_unlock(&hi706_yuv_drv_lock);
    
    switch (para)
    {
    case MEFFECT_OFF:
        {
        HI706_write_cmos_sensor(0x03, 0x10);
        HI706_write_cmos_sensor(0x11, 0x43);
        HI706_write_cmos_sensor(0x12, 0x30);

        HI706_write_cmos_sensor(0x44, 0x80);
        HI706_write_cmos_sensor(0x45, 0x98);

        HI706_write_cmos_sensor(0x47, 0x7f);
        HI706_write_cmos_sensor(0x03, 0x13);
        HI706_write_cmos_sensor(0x20, 0x07);
        HI706_write_cmos_sensor(0x21, 0x07);
        }
        break;
    case MEFFECT_SEPIA:
        {
        HI706_write_cmos_sensor(0x03, 0x10);
        HI706_write_cmos_sensor(0x11, 0x03);
        HI706_write_cmos_sensor(0x12, 0x23);
        HI706_write_cmos_sensor(0x13, 0x00);
        HI706_write_cmos_sensor(0x44, 0x70);
        HI706_write_cmos_sensor(0x45, 0x98);

        HI706_write_cmos_sensor(0x47, 0x7f);
        HI706_write_cmos_sensor(0x03, 0x13);
        HI706_write_cmos_sensor(0x20, 0x07);
        HI706_write_cmos_sensor(0x21, 0x07);
        }	
        break;  
    case MEFFECT_NEGATIVE:
        {
        HI706_write_cmos_sensor(0x03, 0x10);
        HI706_write_cmos_sensor(0x11, 0x03);
        HI706_write_cmos_sensor(0x12, 0x08);
        HI706_write_cmos_sensor(0x13, 0x00);
        HI706_write_cmos_sensor(0x14, 0x00);
        }
        break; 
    case MEFFECT_SEPIAGREEN:		
        {
        HI706_write_cmos_sensor(0x03, 0x10);
        HI706_write_cmos_sensor(0x11, 0x03);
        HI706_write_cmos_sensor(0x12, 0x03);
        //HI706_write_cmos_sensor(0x40, 0x00);
        HI706_write_cmos_sensor(0x13, 0x00);
        HI706_write_cmos_sensor(0x44, 0x30);
        HI706_write_cmos_sensor(0x45, 0x50);
        }	
        break;
    case MEFFECT_SEPIABLUE:
        {
        HI706_write_cmos_sensor(0x03, 0x10);
        HI706_write_cmos_sensor(0x11, 0x03);
        HI706_write_cmos_sensor(0x12, 0x03);
        //HI706_write_cmos_sensor(0x40, 0x00);
        HI706_write_cmos_sensor(0x13, 0x00);
        HI706_write_cmos_sensor(0x44, 0xb0);
        HI706_write_cmos_sensor(0x45, 0x40);
        }     
        break;        
    case MEFFECT_MONO:			
        {
        HI706_write_cmos_sensor(0x03, 0x10);
        HI706_write_cmos_sensor(0x11, 0x03);
        HI706_write_cmos_sensor(0x12, 0x03);
        //HI706_write_cmos_sensor(0x40, 0x00);
        HI706_write_cmos_sensor(0x44, 0x80);
        HI706_write_cmos_sensor(0x45, 0x80);
        }
        break;
    default:
        return KAL_FALSE;
    }

    return KAL_TRUE;
} /* HI706_set_param_effect */

/*************************************************************************
* FUNCTION
*	HI706_set_param_banding
*
* DESCRIPTION
*	banding setting.
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
BOOL HI706_set_param_banding(UINT16 para)
{
    SENSORDB("[Enter]HI706 set_param_banding func:para = %d\n",para);

    //if(HI706_sensor.banding == para) return KAL_TRUE;

    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.banding = para;
    spin_unlock(&hi706_yuv_drv_lock);

    switch (para)
    {
    case AE_FLICKER_MODE_50HZ:
        {
		HI706_write_cmos_sensor(0x03,0x20);
		HI706_write_cmos_sensor(0x10,0x1c);
		HI706_write_cmos_sensor(0x18,0x38);
		HI706_write_cmos_sensor(0x83,0x03);
		HI706_write_cmos_sensor(0x84,0xf7);
		HI706_write_cmos_sensor(0x85,0xa0);
		HI706_write_cmos_sensor(0x88,0x03);
		HI706_write_cmos_sensor(0x89,0xf7);
		HI706_write_cmos_sensor(0x8a,0xa0);
		HI706_write_cmos_sensor(0x10,0x9c);
		HI706_write_cmos_sensor(0x18,0x30);
        }
        break;
    case AE_FLICKER_MODE_60HZ:
        {
		HI706_write_cmos_sensor(0x03,0x20);
		HI706_write_cmos_sensor(0x10,0x0c);
		HI706_write_cmos_sensor(0x18,0x38);
		HI706_write_cmos_sensor(0x83,0x02);
		HI706_write_cmos_sensor(0x84,0x78);
		HI706_write_cmos_sensor(0x85,0xd0);
		HI706_write_cmos_sensor(0x88,0x04);
		HI706_write_cmos_sensor(0x89,0x1e);
		HI706_write_cmos_sensor(0x8a,0xb0);
		HI706_write_cmos_sensor(0x10,0x8c);
		HI706_write_cmos_sensor(0x18,0x30);
        }
        break;
    default:
        return KAL_FALSE;
    }
    
    return KAL_TRUE;
} /* HI706_set_param_banding */




/*************************************************************************
* FUNCTION
*	HI706_set_param_exposure
*
* DESCRIPTION
*	exposure setting.
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
BOOL HI706_set_param_exposure(UINT16 para)
{
    SENSORDB("[Enter]HI706 set_param_exposure func:para = %d\n",para);

    //if(HI706_sensor.exposure == para) return KAL_TRUE;

    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.exposure = para;
    spin_unlock(&hi706_yuv_drv_lock);

    HI706_write_cmos_sensor(0x03,0x10);
    HI706_write_cmos_sensor(0x12,HI706_read_cmos_sensor(0x12)|0x10);
    switch (para)
    {
    case AE_EV_COMP_13:  //+4 EV
        HI706_write_cmos_sensor(0x40,0x60);
        break;  
    case AE_EV_COMP_10:  //+3 EV
        HI706_write_cmos_sensor(0x40,0x48);
        break;    
    case AE_EV_COMP_07:  //+2 EV
        HI706_write_cmos_sensor(0x40,0x30);
        break;    
    case AE_EV_COMP_03:	 //	+1 EV	
        HI706_write_cmos_sensor(0x40,0x18);	
        break;    
    case AE_EV_COMP_00:  // +0 EV
        HI706_write_cmos_sensor(0x40,0x84);
        break;    
    case AE_EV_COMP_n03:  // -1 EV
        HI706_write_cmos_sensor(0x40,0x98);
        break;    
    case AE_EV_COMP_n07:	// -2 EV		
        HI706_write_cmos_sensor(0x40,0xb0);	
        break;    
    case AE_EV_COMP_n10:   //-3 EV
        HI706_write_cmos_sensor(0x40,0xc8);
        break;
    case AE_EV_COMP_n13:  // -4 EV
        HI706_write_cmos_sensor(0x40,0xe0);
        break;
    default:
        return FALSE;
    }

    return TRUE;	
} /* HI706_set_param_exposure */

void HI706_set_AE_mode(UINT32 iPara)
{
    UINT8 temp_AE_reg = 0;
    SENSORDB("HI706_set_AE_mode = %d E \n",iPara);
    HI706_write_cmos_sensor(0x03,0x20);
    temp_AE_reg = HI706_read_cmos_sensor(0x10);

    if (AE_MODE_OFF == iPara)
    {
        // turn off AEC/AGC
        HI706_write_cmos_sensor(0x10,temp_AE_reg &~ 0x10);
    }	
    else
    {
        HI706_write_cmos_sensor(0x10,temp_AE_reg | 0x10);
    }
}
UINT32 HI706YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
    SENSORDB("[Enter]HI706YUVSensorSetting func:cmd = %d\n",iCmd);

    switch (iCmd) 
    {
    case FID_SCENE_MODE:	    //auto mode or night mode
        if (iPara == SCENE_MODE_OFF)//auto mode
        {	  	if( CAPTURE_FLAG == 0)
             HI706_night_mode(FALSE); 

	  else
	   CAPTURE_FLAG = 0;

        }
        else if (iPara == SCENE_MODE_NIGHTSCENE)//night mode
        {	  	if( CAPTURE_FLAG == 0)
            HI706_night_mode(TRUE); 
	   else
	   CAPTURE_FLAG = 0;

        }	
        break; 	    
    case FID_AWB_MODE:
        HI706_set_param_wb(iPara);
        break;
    case FID_COLOR_EFFECT:
        HI706_set_param_effect(iPara);
        break;
    case FID_AE_EV:	    	    
        HI706_set_param_exposure(iPara);
        break;
    case FID_AE_FLICKER:	
			if( CAPTUREB_FLAG == 0)
	        HI706_set_param_banding(iPara);

	else
	CAPTUREB_FLAG = 0;

        break;
    case FID_ZOOM_FACTOR:
        spin_lock(&hi706_yuv_drv_lock);
        HI706_zoom_factor = iPara; 
        spin_unlock(&hi706_yuv_drv_lock);
        break; 
    case FID_AE_SCENE_MODE: 
        HI706_set_AE_mode(iPara);
        break; 
    default:
        break;
    }
    return TRUE;
}   /* HI706YUVSensorSetting */

UINT32 HI706YUVSetVideoMode(UINT16 u2FrameRate)
{
    spin_lock(&hi706_yuv_drv_lock);
    HI706_sensor.MPEG4_Video_mode = KAL_TRUE;
    spin_unlock(&hi706_yuv_drv_lock);
    SENSORDB("[Enter]HI706 Set Video Mode:FrameRate= %d\n",u2FrameRate);
    SENSORDB("HI706_sensor.video_mode = %d\n",HI706_sensor.MPEG4_Video_mode);
/*
    if(u2FrameRate >= 30)
		u2FrameRate = 20;
	else if(u2FrameRate>0)
		u2FrameRate = 15;
	else 
    {
        SENSORDB("Wrong Frame Rate"); 
		
		return FALSE;
    }
*/

    spin_lock(&hi706_yuv_drv_lock);
    //HI706_sensor.fix_framerate = u2FrameRate * 10;
    spin_unlock(&hi706_yuv_drv_lock);
   
    //HI706_Fix_Video_Frame_Rate(HI706_sensor.fix_framerate);

    return TRUE;
}

void HI706GetAFMaxNumFocusAreas(UINT32 *pFeatureReturnPara32)
{	
    *pFeatureReturnPara32 = 0;    
    SENSORDB("HI706GetAFMaxNumFocusAreas *pFeatureReturnPara32 = %d\n",  *pFeatureReturnPara32);
}

void HI706GetAEMaxNumMeteringAreas(UINT32 *pFeatureReturnPara32)
{     
    *pFeatureReturnPara32 = 0;    
    SENSORDB("HI706GetAEMaxNumMeteringAreas *pFeatureReturnPara32 = %d\n",  *pFeatureReturnPara32);	
}

void HI706GetExifInfo(UINT32 exifAddr)
{
    SENSOR_EXIF_INFO_STRUCT* pExifInfo = (SENSOR_EXIF_INFO_STRUCT*)exifAddr;
    pExifInfo->FNumber = 28;
    pExifInfo->AEISOSpeed = AE_ISO_100;
    pExifInfo->AWBMode = HI706_sensor.wb;
    pExifInfo->CapExposureTime = 0;
    pExifInfo->FlashLightTimeus = 0;
    pExifInfo->RealISOValue = AE_ISO_100;
}

UINT32 HI706FeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
							 UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{
    //UINT16 u2Temp = 0; 
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    SENSORDB("[Enter]:HI706 Feature Control func:FeatureId = %d\n",FeatureId);

    switch (FeatureId)
    {
    case SENSOR_FEATURE_GET_RESOLUTION:
        *pFeatureReturnPara16++=HI706_IMAGE_SENSOR_FULL_WIDTH;
        *pFeatureReturnPara16=HI706_IMAGE_SENSOR_FULL_HEIGHT;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PERIOD:
        *pFeatureReturnPara16++=HI706_IMAGE_SENSOR_PV_WIDTH;//+HI706_sensor.pv_dummy_pixels;
        *pFeatureReturnPara16=HI706_IMAGE_SENSOR_PV_HEIGHT;//+HI706_sensor.pv_dummy_lines;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
        //*pFeatureReturnPara32 = HI706_sensor_pclk/10;
        *pFeatureParaLen=4;
        break;
    case SENSOR_FEATURE_SET_ESHUTTER:

        break;
    case SENSOR_FEATURE_SET_NIGHTMODE:
        HI706_night_mode((BOOL) *pFeatureData16);
        break;
    case SENSOR_FEATURE_SET_GAIN:
        break; 
    case SENSOR_FEATURE_SET_FLASHLIGHT:
        break;
    case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
        break;
    case SENSOR_FEATURE_SET_REGISTER:
        HI706_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
        break;
    case SENSOR_FEATURE_GET_REGISTER:
        pSensorRegData->RegData = HI706_read_cmos_sensor(pSensorRegData->RegAddr);
        break;
    case SENSOR_FEATURE_GET_CONFIG_PARA:
        memcpy(pSensorConfigData, &HI706SensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
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
    case SENSOR_FEATURE_SET_YUV_CMD:
        HI706YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));
        break;	
    case SENSOR_FEATURE_SET_VIDEO_MODE:
        HI706YUVSetVideoMode(*pFeatureData16);
        break; 
    case SENSOR_FEATURE_CHECK_SENSOR_ID:
        HI706_GetSensorID(pFeatureData32); 
        break; 
    case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
        HI706GetAFMaxNumFocusAreas(pFeatureReturnPara32);            
        *pFeatureParaLen=4;
        break;        
    case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
        HI706GetAEMaxNumMeteringAreas(pFeatureReturnPara32);            
        *pFeatureParaLen=4;
        break;   
    case SENSOR_FEATURE_GET_EXIF_INFO:
        SENSORDB("SENSOR_FEATURE_GET_EXIF_INFO\n");
        SENSORDB("EXIF addr = 0x%x\n",*pFeatureData32);          
        HI706GetExifInfo(*pFeatureData32);
        break;        
	default:
        break;			
    }
    return ERROR_NONE;
}	/* HI706FeatureControl() */


SENSOR_FUNCTION_STRUCT	SensorFuncHI706=
{
    HI706Open,
    HI706GetInfo,
    HI706GetResolution,
    HI706FeatureControl,
    HI706Control,
    HI706Close
};

UINT32 HI706_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncHI706;

    return ERROR_NONE;
}	/* SensorInit() */



