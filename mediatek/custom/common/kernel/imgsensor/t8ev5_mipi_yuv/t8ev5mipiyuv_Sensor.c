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
 *------------------------------------------------------------------------------
 * Upper this line, this part is controlled by CC/CQ. DO NOT MODIFY!!
 *============================================================================
 ****************************************************************************/
#define WINMO_USE           0

 #if WINMO_USE
#include <windows.h>
#include <memory.h>
#include <nkintr.h>
#include <ceddk.h>
#include <ceddk_exp.h>

#include "CameraCustomized.h"

#include "kal_release.h"
#include "i2c_exp.h"
#include "gpio_exp.h"
#include "msdk_exp.h"
#include "msdk_sensor_exp.h"
#include "msdk_isp_exp.h"
#include "base_regs.h"
#include "Sensor.h"
#include "camera_sensor_para.h"
#else 
#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/slab.h>


#include "kd_camera_hw.h"
#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_imgsensor_errcode.h"

#include "t8ev5mipiyuv_Sensor.h"
#include "t8ev5mipiyuv_Camera_Sensor_para.h"
#include "t8ev5mipiyuv_CameraCustomized.h"

#include "kd_camera_feature.h"
#endif 

void T8EV5YUV_set_capture_shutter_gain(UINT16 para);
static struct T8EV5YUV_sensor_struct t8ev5yuv_stru;

static DEFINE_SPINLOCK(t8ev5yuv_drv_lock);

kal_bool  T8EV5YUV_MPEG4_encode_mode = KAL_FALSE;
/* MAX/MIN Explosure Lines Used By AE Algorithm */
kal_uint16 T8EV5YUV_MAX_EXPOSURE_LINES = T8EV5_PV_PERIOD_LINE_NUMS-4;
kal_uint8  T8EV5YUV_MIN_EXPOSURE_LINES = 2;
kal_uint32 T8EV5YUV_isp_master_clock;
kal_uint16 T8EV5YUV_CURRENT_FRAME_LINES = T8EV5_PV_PERIOD_LINE_NUMS;

static kal_uint16 T8EV5YUV_dummy_pixels=0, T8EV5YUV_dummy_lines=0;
kal_uint16 T8EV5YUV_PV_dummy_pixels=0,T8EV5YUV_PV_dummy_lines=0;

kal_uint8 T8EV5YUV_sensor_write_I2C_address = T8EV5_WRITE_ID;
kal_uint8 T8EV5YUV_sensor_read_I2C_address = T8EV5_READ_ID;

static kal_uint32 T8EV5YUV_zoom_factor = 0; 
static kal_uint16 is_first_preview=1;
static kal_uint16 is_flash_on=0;

//add by lingnan for af status
//static UINT8 STA_FOCUS = 0x8F; 

//static kal_uint32 AF_XS = 64;//version0.21, aug.2009
//static kal_uint32 AF_YS = 48;//version0.21, aug.2009

//static UINT8 ZONE[4] = {24, 18, 40, 30};////version0.21, aug.2009,center 4:3 window
static kal_uint32 m_zone_center_x=160;
static kal_uint32 m_zone_center_y=120;


#define LOG_TAG "[T8EV5Yuv]"
#define SENSORDB(fmt, arg...) printk( LOG_TAG  fmt, ##arg)
#define RETAILMSG(x,...)
#define TEXT

kal_uint16 T8EV5YUV_g_iDummyLines = 28; 

static kal_uint16 g_sensorAfMode=1;
static kal_uint16 returnAfMode=1;

#if WINMO_USE
HANDLE T8EV5YUVhDrvI2C;
I2C_TRANSACTION T8EV5YUVI2CConfig;
#endif 

UINT8 T8EV5YUVPixelClockDivider=0;

kal_uint16 T8EV5YUV_pv_exposure_lines=0x100,T8EV5YUV_g_iBackupExtraExp = 0,T8EV5YUV_extra_exposure_lines = 0;


MSDK_SENSOR_CONFIG_STRUCT T8EV5YUVSensorConfigData;

kal_uint32 T8EV5YUV_FAC_SENSOR_REG;
kal_uint16 T8EV5YUV_sensor_flip_value;

static void T8EV5_FOCUS_Init(void);

static bool AF_Power_OnOff = false;


/* FIXME: old factors and DIDNOT use now. s*/
SENSOR_REG_STRUCT T8EV5YUVSensorCCT[]=CAMERA_SENSOR_CCT_DEFAULT_VALUE;
SENSOR_REG_STRUCT T8EV5YUVSensorReg[ENGINEER_END]=CAMERA_SENSOR_REG_DEFAULT_VALUE;
/* FIXME: old factors and DIDNOT use now. e*/



////////////////////////////////////////////////////////////////
typedef enum
{
  T8EV5_720P,       //1M 1280x960
  T8EV5_1280_960,
  T8EV5_5M,     //5M 2592x1944
} T8EV5_RES_TYPE;
T8EV5_RES_TYPE T8EV5YUV_g_RES=T8EV5_1280_960;

/*
typedef enum
{
  T8EV5_MODE_PREVIEW,  //1M     1280x960
  T8EV5_MODE_CAPTURE   //5M    2592x1944
} T8EV5_MODE;
*/
//T8EV5_MODE g_iT8EV5YUV_Mode=T8EV5_MODE_PREVIEW;


extern int iReadReg(u16 a_u2Addr , u8 * a_puBuff , u16 i2cId);
extern int iWriteReg(u16 a_u2Addr , u32 a_u4Data , u32 a_u4Bytes , u16 i2cId);
extern int iBurstWriteReg(u8 *pData, u32 bytes, u16 i2cId); 
#define T8EV5YUV_write_cmos_sensor(addr, para) iWriteReg((u16) addr , (u32) para , 1, T8EV5_WRITE_ID)
#define T8EV5YUV_burst_write_cmos_sensor(pData, bytes)  iBurstWriteReg(pData, bytes, T8EV5_WRITE_ID)

static UINT32 g_sensorAfStatus = 0;

#define PROFILE 1

#if PROFILE 
static struct timeval T8EV5YUV_ktv1, T8EV5YUV_ktv2; 
inline void T8EV5YUV_imgSensorProfileStart(void)
{
    do_gettimeofday(&T8EV5YUV_ktv1);    
}

inline void T8EV5YUV_imgSensorProfileEnd(char *tag)
{
    unsigned long TimeIntervalUS;    
    do_gettimeofday(&T8EV5YUV_ktv2);

    TimeIntervalUS = (T8EV5YUV_ktv2.tv_sec - T8EV5YUV_ktv1.tv_sec) * 1000000 + (T8EV5YUV_ktv2.tv_usec - T8EV5YUV_ktv1.tv_usec); 
    SENSORDB("[%s]Profile = %lu\n",tag, TimeIntervalUS);
}
#else 
inline static void T8EV5YUV_imgSensorProfileStart() {}
inline static void T8EV5YUV_imgSensorProfileEnd(char *tag) {}
#endif 

#if 0
T8EV5YUV_write_cmos_sensor(addr, para) 
{
    u16 data = 0 ; 

    T8EV5YUV_imgSensorProfileStart();
    iWriteReg((u16) addr , (u32) para , 1, T8EV5_WRITE_ID);
    T8EV5YUV_imgSensorProfileEnd("T8EV5YUV_write_cmos_sensor");


    iReadReg((u16)addr, (u8*) &data, T8EV5_WRITE_ID); 

    SENSORDB("Write addr = %x , wdata = %x,  rdata= %x\n", addr, para, data); 
}
#endif 

kal_uint16 T8EV5YUV_read_cmos_sensor(kal_uint32 addr)
{
kal_uint16 get_byte=0;
    iReadReg((u16) addr ,(u8*)&get_byte,T8EV5_WRITE_ID);
    return get_byte;
}


#define Sleep(ms) mdelay(ms)

#if WINMO_USE
void T8EV5YUV_write_cmos_sensor(kal_uint32 addr, kal_uint32 para)
{
    T8EV5YUVI2CConfig.operation=I2C_OP_WRITE;
    T8EV5YUVI2CConfig.slaveAddr=T8EV5YUV_sensor_write_I2C_address>>1;
    T8EV5YUVI2CConfig.transfer_num=1;   /* TRANSAC_LEN */
    T8EV5YUVI2CConfig.transfer_len=3;
    T8EV5YUVI2CConfig.buffer[0]=(UINT8)(addr>>8);
    T8EV5YUVI2CConfig.buffer[1]=(UINT8)(addr&0xFF);
    T8EV5YUVI2CConfig.buffer[2]=(UINT8)para;
    DRV_I2CTransaction(T8EV5YUVhDrvI2C, &T8EV5YUVI2CConfig);
}    /* T8EV5YUV_write_cmos_sensor() */
#endif 

#if WINMO_USE
kal_uint32 T8EV5YUV_read_cmos_sensor(kal_uint32 addr)
{
    kal_uint16 get_byte=0;
    
    T8EV5YUVI2CConfig.operation=I2C_OP_WRITE;
    T8EV5YUVI2CConfig.slaveAddr=T8EV5YUV_sensor_write_I2C_address>>1;
    T8EV5YUVI2CConfig.transfer_num=1;   /* TRANSAC_LEN */
    T8EV5YUVI2CConfig.transfer_len=2;
    T8EV5YUVI2CConfig.buffer[0]=(UINT8)(addr>>8);
    T8EV5YUVI2CConfig.buffer[1]=(UINT8)(addr&0xFF);
    DRV_I2CTransaction(T8EV5YUVhDrvI2C, &T8EV5YUVI2CConfig);
    
    T8EV5YUVI2CConfig.operation=I2C_OP_READ;
    T8EV5YUVI2CConfig.slaveAddr=T8EV5YUV_sensor_read_I2C_address>>1;
    T8EV5YUVI2CConfig.transfer_num=1;   /* TRANSAC_LEN */
    T8EV5YUVI2CConfig.transfer_len=1;
    DRV_I2CTransaction(T8EV5YUVhDrvI2C, &T8EV5YUVI2CConfig);
    get_byte=T8EV5YUVI2CConfig.buffer[0];
    return get_byte;
}   /* T8EV5YUV_read_cmos_sensor() */
#endif 

void T8EV5YUV_write_shutter(kal_uint16 shutter)
{
    SENSORDB("[T8EV5YUV_write_shutter] X shutter = %d \n", shutter); 

    T8EV5YUV_write_cmos_sensor(0x0307, ((shutter>>8)&0xFF));//MES[15:8]
    T8EV5YUV_write_cmos_sensor(0x0308, (shutter&0xFF));//MES[7:0]
   
}   /* write_T8EV5_shutter */

static kal_uint16 T8EV5YUVReg2Gain(const kal_uint8 iReg)
{
    return iReg;
    
}

static kal_uint8 T8EV5YUVGain2Reg(const kal_uint16 iGain)
{
    return iGain;

}

/*************************************************************************
* FUNCTION
*    T8EV5YUV_SetGain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    gain : sensor global gain(base: 0x40)
*
* RETURNS
*    the actually gain set to sensor.
*
* GLOBALS AFFECTED
*
*************************************************************************/
//! Due to the T8EV5 set gain will happen race condition. 
//! It need to use a critical section to protect it. 
void T8EV5YUV_SetGain(UINT16 iGain)
{

    SENSORDB("[T8EV5YUV_SetGain] X Gain = %d \n", iGain); 

    T8EV5YUV_write_cmos_sensor(0x0309, ((iGain>>8)&0x0F));//ESLIMMODE/ROOMDET/-/-/MAG[11:8]
    T8EV5YUV_write_cmos_sensor(0x030A, (iGain&0xFF));//MAG[7:0]
}   /*  T8EV5YUV_SetGain  */


/*************************************************************************
* FUNCTION
*    read_T8EV5YUV_gain
*
* DESCRIPTION
*    This function is to set global gain to sensor.
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_uint16 read_T8EV5YUV_gain(void)
{
    kal_uint16 gain;
    
    gain=((T8EV5YUV_read_cmos_sensor(0x0361)&0xff)<<8)+T8EV5YUV_read_cmos_sensor(0x0362);   
    
    SENSORDB("[%s]gain =0x%x \n",__FUNCTION__,gain);
    return gain;
}  /* read_T8EV5YUV_gain */

void write_T8EV5YUV_gain(kal_uint16 gain)
{
    T8EV5YUV_SetGain(gain);
}
void T8EV5YUV_camera_para_to_sensor(void)
{
   /* kal_uint32    i;
    for(i=0; 0xFFFFFFFF!=T8EV5YUVSensorReg[i].Addr; i++)
    {
        T8EV5YUV_write_cmos_sensor(T8EV5YUVSensorReg[i].Addr, T8EV5YUVSensorReg[i].Para);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=T8EV5YUVSensorReg[i].Addr; i++)
    {
        T8EV5YUV_write_cmos_sensor(T8EV5YUVSensorReg[i].Addr, T8EV5YUVSensorReg[i].Para);
    }
    for(i=FACTORY_START_ADDR; i<FACTORY_END_ADDR; i++)
    {
        T8EV5YUV_write_cmos_sensor(T8EV5YUVSensorCCT[i].Addr, T8EV5YUVSensorCCT[i].Para);
    }*/
}


/*************************************************************************
* FUNCTION
*    T8EV5YUV_sensor_to_camera_para
*
* DESCRIPTION
*    // update camera_para from sensor register
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
void T8EV5YUV_sensor_to_camera_para(void)
{
    /*kal_uint32    i;
    for(i=0; 0xFFFFFFFF!=T8EV5YUVSensorReg[i].Addr; i++)
    {
        T8EV5YUVSensorReg[i].Para = T8EV5YUV_read_cmos_sensor(T8EV5YUVSensorReg[i].Addr);
    }
    for(i=ENGINEER_START_ADDR; 0xFFFFFFFF!=T8EV5YUVSensorReg[i].Addr; i++)
    {
        T8EV5YUVSensorReg[i].Para = T8EV5YUV_read_cmos_sensor(T8EV5YUVSensorReg[i].Addr);
    }*/
}


/*************************************************************************
* FUNCTION
*    T8EV5YUV_get_sensor_group_count
*
* DESCRIPTION
*    //
*
* PARAMETERS
*    None
*
* RETURNS
*    gain : sensor global gain(base: 0x40)
*
* GLOBALS AFFECTED
*
*************************************************************************/
kal_int32  T8EV5YUV_get_sensor_group_count(void)
{
    return GROUP_TOTAL_NUMS;
}

void T8EV5YUV_get_sensor_group_info(kal_uint16 group_idx, kal_int8* group_name_ptr, kal_int32* item_count_ptr)
{
   switch (group_idx)
   {
        case PRE_GAIN:
            sprintf((char *)group_name_ptr, "CCT");
            *item_count_ptr = 2;
            break;
        case CMMCLK_CURRENT:
            sprintf((char *)group_name_ptr, "CMMCLK Current");
            *item_count_ptr = 1;
            break;
        case FRAME_RATE_LIMITATION:
            sprintf((char *)group_name_ptr, "Frame Rate Limitation");
            *item_count_ptr = 2;
            break;
        case REGISTER_EDITOR:
            sprintf((char *)group_name_ptr, "Register Editor");
            *item_count_ptr = 2;
            break;
        default:
            ASSERT(0);
}
}

void T8EV5YUV_get_sensor_item_info(kal_uint16 group_idx,kal_uint16 item_idx, MSDK_SENSOR_ITEM_INFO_STRUCT* info_ptr)
{
    kal_int16 temp_reg=0;
    kal_uint16 temp_gain=0, temp_addr=0, temp_para=0;
    
    switch (group_idx)
    {
        case PRE_GAIN:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Pregain-Global");
                    temp_addr = PRE_GAIN_INDEX;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"GLOBAL_GAIN");
                    temp_addr = GLOBAL_GAIN_INDEX;
                    break;
                default:
                    ASSERT(0);
            }
            temp_para=T8EV5YUVSensorCCT[temp_addr].Para;
            temp_gain = T8EV5YUVReg2Gain(temp_para);

            temp_gain=(temp_gain*1000)/BASEGAIN;

            info_ptr->ItemValue=temp_gain;
            info_ptr->IsTrueFalse=KAL_FALSE;
            info_ptr->IsReadOnly=KAL_FALSE;
            info_ptr->IsNeedRestart=KAL_FALSE;
            info_ptr->Min=1000;
            info_ptr->Max=15875;
            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Drv Cur[2,4,6,8]mA");
                
                    //temp_reg=T8EV5YUVSensorReg[CMMCLK_CURRENT_INDEX].Para;
                    temp_reg = ISP_DRIVING_2MA;
                    if(temp_reg==ISP_DRIVING_2MA)
                    {
                        info_ptr->ItemValue=2;
                    }
                    else if(temp_reg==ISP_DRIVING_4MA)
                    {
                        info_ptr->ItemValue=4;
                    }
                    else if(temp_reg==ISP_DRIVING_6MA)
                    {
                        info_ptr->ItemValue=6;
                    }
                    else if(temp_reg==ISP_DRIVING_8MA)
                    {
                        info_ptr->ItemValue=8;
                    }
                
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_TRUE;
                    info_ptr->Min=2;
                    info_ptr->Max=8;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"Max Exposure Lines");
                    info_ptr->ItemValue=T8EV5YUV_MAX_EXPOSURE_LINES;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"Min Frame Rate");
                    info_ptr->ItemValue=12;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_TRUE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0;
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Addr.");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                case 1:
                    sprintf((char *)info_ptr->ItemNamePtr,"REG Value");
                    info_ptr->ItemValue=0;
                    info_ptr->IsTrueFalse=KAL_FALSE;
                    info_ptr->IsReadOnly=KAL_FALSE;
                    info_ptr->IsNeedRestart=KAL_FALSE;
                    info_ptr->Min=0;
                    info_ptr->Max=0xFFFF;
                    break;
                default:
                ASSERT(0);
            }
            break;
        default:
            ASSERT(0);
    }
}

//void T8EV5YUV_set_isp_driving_current(kal_uint8 current)
//{
//}

kal_bool T8EV5YUV_set_sensor_item_info(kal_uint16 group_idx, kal_uint16 item_idx, kal_int32 ItemValue)
{
//   kal_int16 temp_reg;
   kal_uint16 temp_addr=0, temp_para=0;

   switch (group_idx)
    {
        case PRE_GAIN:
            switch (item_idx)
            {
                case 0:
                    temp_addr = PRE_GAIN_INDEX;
                    break;
                case 1:
                    temp_addr = GLOBAL_GAIN_INDEX;
                    break;
                default:
                    ASSERT(0);
            }

            temp_para = T8EV5YUVGain2Reg(ItemValue);


            T8EV5YUVSensorCCT[temp_addr].Para = temp_para;
            T8EV5YUV_write_cmos_sensor(T8EV5YUVSensorCCT[temp_addr].Addr,temp_para);

            break;
        case CMMCLK_CURRENT:
            switch (item_idx)
            {
                case 0:
                    if(ItemValue==2)
                    {
                        T8EV5YUVSensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_2MA;
                        //T8EV5YUV_set_isp_driving_current(ISP_DRIVING_2MA);
                    }
                    else if(ItemValue==3 || ItemValue==4)
                    {
                        T8EV5YUVSensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_4MA;
                        //T8EV5YUV_set_isp_driving_current(ISP_DRIVING_4MA);
                    }
                    else if(ItemValue==5 || ItemValue==6)
                    {
                        T8EV5YUVSensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_6MA;
                        //T8EV5YUV_set_isp_driving_current(ISP_DRIVING_6MA);
                    }
                    else
                    {
                        T8EV5YUVSensorReg[CMMCLK_CURRENT_INDEX].Para = ISP_DRIVING_8MA;
                        //T8EV5YUV_set_isp_driving_current(ISP_DRIVING_8MA);
                    }
                    break;
                default:
                    ASSERT(0);
            }
            break;
        case FRAME_RATE_LIMITATION:
            ASSERT(0);
            break;
        case REGISTER_EDITOR:
            switch (item_idx)
            {
                case 0:
                    T8EV5YUV_FAC_SENSOR_REG=ItemValue;
                    break;
                case 1:
                    T8EV5YUV_write_cmos_sensor(T8EV5YUV_FAC_SENSOR_REG,ItemValue);
                    break;
                default:
                    ASSERT(0);
            }
            break;
        default:
            ASSERT(0);
    }
    return KAL_TRUE;
}

static void T8EV5YUV_SetDummy(const kal_uint16 iPixels, const kal_uint16 iLines)
{
    kal_uint16 LinesOneframe;
    kal_uint16 PixelsOneline = T8EV5_FULL_PERIOD_PIXEL_NUMS;
    if(T8EV5_720P == T8EV5YUV_g_RES)
    {
        PixelsOneline = (T8EV5_PV_PERIOD_PIXEL_NUMS_HTS + iPixels );
        LinesOneframe =iLines + T8EV5_PV_PERIOD_LINE_NUMS_VTS;
        if(T8EV5YUV_MPEG4_encode_mode == KAL_FALSE)//for Fix video framerate
            T8EV5YUV_CURRENT_FRAME_LINES = iLines + T8EV5_PV_PERIOD_LINE_NUMS_VTS;
    }
    else if (T8EV5_5M == T8EV5YUV_g_RES)
    {
        PixelsOneline = T8EV5_FULL_PERIOD_PIXEL_NUMS_HTS + iPixels;
     LinesOneframe =iLines + T8EV5_FULL_PERIOD_LINE_NUMS_VTS;

        T8EV5YUV_CURRENT_FRAME_LINES = iLines + T8EV5_FULL_PERIOD_LINE_NUMS_VTS;
    }
    if(iPixels)
    {
        //T8EV5YUV_write_cmos_sensor(0x380c, (PixelsOneline >> 8) & 0xFF);
        //T8EV5YUV_write_cmos_sensor(0x380d, PixelsOneline & 0xFF);
    }
    if(iLines)
    {
        //T8EV5YUV_write_cmos_sensor(0x380e, (LinesOneframe >> 8) & 0xFF);
        //T8EV5YUV_write_cmos_sensor(0x380f, LinesOneframe & 0xFF);
    }
         
}   /*  T8EV5YUV_SetDummy */

static void T8EV5YUV_set_AE_mode(kal_bool AE_enable)
{
    kal_uint8 temp_AE_reg = 0;

    if (AE_enable == KAL_TRUE)
    {
        // turn on AEC/AGC
       // temp_AE_reg = T8EV5YUV_read_cmos_sensor(0x3503);
       // T8EV5YUV_write_cmos_sensor(0x3503, temp_AE_reg&~0x07);
  
    }
    else
    {
        // turn off AEC/AGC
       // temp_AE_reg = T8EV5YUV_read_cmos_sensor(0x3503);
       // T8EV5YUV_write_cmos_sensor(0x3503, temp_AE_reg| 0x07);

    }
}


static void T8EV5YUV_set_AWB_mode(kal_bool AWB_enable)
{
    kal_uint8 temp_AWB_reg = 0;

    //return ;

    if (AWB_enable == KAL_TRUE)
    {
        //enable Auto WB
      //  temp_AWB_reg = T8EV5YUV_read_cmos_sensor(0x3406);
      //  T8EV5YUV_write_cmos_sensor(0x3406, temp_AWB_reg & ~0x01);        
    }
    else
    {
        //turn off AWB
      //  temp_AWB_reg = T8EV5YUV_read_cmos_sensor(0x3406);
      //  T8EV5YUV_write_cmos_sensor(0x3406, temp_AWB_reg | 0x01);        
    }
}

static void T8EV5_FOCUS_Check_MCU()
{
    kal_uint8 check[13] = {0x00};
    return; 
}


static void T8EV5YUV_Sensor_Init_setting(void)
{
    spin_lock(&t8ev5yuv_drv_lock);
    t8ev5yuv_stru.iShutter= 0;
    t8ev5yuv_stru.iGain= 0;
    t8ev5yuv_stru.d_gain=0;
    t8ev5yuv_stru.Camco_mode = T8EV5_MODE_PREVIEW;
    t8ev5yuv_stru.sceneMode = SCENE_MODE_OFF;
    spin_unlock(&t8ev5yuv_drv_lock);  

    T8EV5YUV_write_cmos_sensor(0x0004,0x00); //-/-/-/-/-/-/-/PISO_MSKN
    T8EV5YUV_write_cmos_sensor(0x0100,0x00); //-/-/-/-/-/-/-/MODSEL
    T8EV5YUV_write_cmos_sensor(0x0101,0x00); //-/-/-/-/-/-/VREVON/HREVON
    T8EV5YUV_write_cmos_sensor(0x0102,0x02); //-/-/-/-/-/-/VLAT_ON/GROUP_HOLD
    T8EV5YUV_write_cmos_sensor(0x0104,0x00); //-/-/-/-/-/-/PARALLEL_OUT_SW[1:0]
    T8EV5YUV_write_cmos_sensor(0x0105,0x01); //-/-/-/-/-/H_COUNT[10:8]
    T8EV5YUV_write_cmos_sensor(0x0106,0x10); //H_COUNT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0107,0x00); //-/-/-/-/-/V_COUNT[10:8]
    T8EV5YUV_write_cmos_sensor(0x0108,0x82); //V_COUNT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0109,0x00); //-/-/-/-/-/-/-/SCALE_M[8]
    T8EV5YUV_write_cmos_sensor(0x010A,0x20); //SCALE_M[7:0]
    T8EV5YUV_write_cmos_sensor(0x010B,0x11); //-/V_ANABIN[2:0]/-/-/-/H_ANABIN
    T8EV5YUV_write_cmos_sensor(0x010C,0x02); //-/-/HFILY[1:0]/HFILUV[1:0]/SCALING_MODE[1:0]
    T8EV5YUV_write_cmos_sensor(0x010D,0x05); //-/-/-/-/HOUTPIX[11:8]
    T8EV5YUV_write_cmos_sensor(0x010E,0x00); //HOUTPIX[7:0]
    T8EV5YUV_write_cmos_sensor(0x010F,0x03); //-/-/-/-/-/VOUTPIX[10:8]
    T8EV5YUV_write_cmos_sensor(0x0110,0xC0); //VOUTPIX[7:0]
    T8EV5YUV_write_cmos_sensor(0x0111,0x00); //-/-/-/-/-/-/HCROP[1:0]
    T8EV5YUV_write_cmos_sensor(0x0112,0x00); //-/-/-/-/-/-/VCROP[9:8]
    T8EV5YUV_write_cmos_sensor(0x0113,0x00); //VCROP[7:0]
    T8EV5YUV_write_cmos_sensor(0x0114,0x01); //-/-/-/-/OUTPUT_FORMAT[3:0]
    T8EV5YUV_write_cmos_sensor(0x0115,0x00); //-/-/-/-/PICEFF[3:0]
    T8EV5YUV_write_cmos_sensor(0x0116,0x00); //-/-/-/-/-/-/-/TH_RGBST
    T8EV5YUV_write_cmos_sensor(0x0120,0x00); //-/-/-/-/TEST_HC/VSYNC_PH/HSYNC_PH/ESYNC_SW
    T8EV5YUV_write_cmos_sensor(0x0121,0x00); //-/-/H_PRESET[13:8]
    T8EV5YUV_write_cmos_sensor(0x0122,0x00); //H_PRESET[7:0]
    T8EV5YUV_write_cmos_sensor(0x0123,0x00); //V_PRESET[15:8]
    T8EV5YUV_write_cmos_sensor(0x0124,0x00); //V_PRESET[7:0]
    T8EV5YUV_write_cmos_sensor(0x0125,0x01); //-/-/-/-/-/HREG_HRST_POS[10:8]
    T8EV5YUV_write_cmos_sensor(0x0126,0xBD); //HREG_HRST_POS[7:0]
    T8EV5YUV_write_cmos_sensor(0x0128,0x00); //-/-/-/-/-/-/TH_HOUT_MN[9:8]
    T8EV5YUV_write_cmos_sensor(0x0129,0x00); //TH_HOUT_MN[7:0]
    T8EV5YUV_write_cmos_sensor(0x012A,0x00); //-/-/-/-/TH_SIZE_SEL[3:0]
    T8EV5YUV_write_cmos_sensor(0x012B,0x00); //-/-/-/-/-/-/VCI[1:0]
    T8EV5YUV_write_cmos_sensor(0x012C,0x00); //-/-/-/-/-/-/-/TH_VOUT_MN[8]
    T8EV5YUV_write_cmos_sensor(0x012D,0x00); //TH_VOUT_MN[7:0]
    T8EV5YUV_write_cmos_sensor(0x012E,0x01); //-/-/-/-/-/-/-/ISPXSHUTDOWN
    T8EV5YUV_write_cmos_sensor(0x0130,0xFC); //DCLK_DRVUP/DOUT_DRVUP/SDA_DRVUP/MSDA_DRVUP/OCDI_DRVUP/PW
	T8EV5YUV_write_cmos_sensor(0x0131,0x01);//-/-/-/-/-/-/-/DCLK_POL
    T8EV5YUV_write_cmos_sensor(0x0132,0x00); //-/-/-/-/CKSP_SEL[1:0]/CKMR_SEL[1:0]
    T8EV5YUV_write_cmos_sensor(0x0133,0x00); //-/SLEEP_SW/VCO_STP_SW/PHY_PWRON_SW/-/SLEEP_MN/VCO_STP_
    T8EV5YUV_write_cmos_sensor(0x0134,0x00); //-/-/-/-/-/-/-/VCMTESTCK
    T8EV5YUV_write_cmos_sensor(0x0135,0x00); //-/-/-/-/-/-/CKUCDIV[1:0]
    T8EV5YUV_write_cmos_sensor(0x0136,0x02); //-/-/-/-/-/PCMODE/ICP_PCH/ICP_NCH
    T8EV5YUV_write_cmos_sensor(0x0137,0x01); //-/-/-/-/-/PRE_PLL_CNTL[2:0]
    T8EV5YUV_write_cmos_sensor(0x0138,0x00); //-/-/-/-/-/-/-/PLL_MULTI[8]
    T8EV5YUV_write_cmos_sensor(0x0139,0x3B); //PLL_MULTI[7:0]
	T8EV5YUV_write_cmos_sensor(0x013A,0x03);//-/-/-/-/VT_SYS_CNTL[3:0]
    T8EV5YUV_write_cmos_sensor(0x013B,0x01); //-/-/-/-/OP_SYS_CNTL[3:0]
    T8EV5YUV_write_cmos_sensor(0x013C,0x05); //-/-/-/-/VT_PIX_CNTL[3:0]
    T8EV5YUV_write_cmos_sensor(0x013D,0x00); //-/-/PLL_SNR_CNTL[1:0]/-/-/PLL_SYS_CNTL[1:0]
    T8EV5YUV_write_cmos_sensor(0x013E,0x27); //-/AD_CNTL[2:0]/-/ST_CNTL[2:0]
    T8EV5YUV_write_cmos_sensor(0x013F,0x77); //-/NREG_CNTL[2:0]/-/BST_CNTL[2:0]
    T8EV5YUV_write_cmos_sensor(0x0140,0x03); //-/-/-/-/-/-/VCO_EN/DIVRSTX
    T8EV5YUV_write_cmos_sensor(0x0141,0x00); //-/-/-/REGVD_SEL/-/-/AMON0_SEL[1:0]
    T8EV5YUV_write_cmos_sensor(0x0142,0x00); //-/AUTO_ICP_R_SEL/ICP_SEL[1:0]/TXEV_SEL[1:0]/LPFR_SEL[1:0]
    T8EV5YUV_write_cmos_sensor(0x0144,0x00); //-/-/-/-/-/VOUT_SEL[2:0]
    T8EV5YUV_write_cmos_sensor(0x0145,0x17); //-/-/CL_SEL[1:0]/-/LSTB/BIAS_SEL/CAMP_EN
    T8EV5YUV_write_cmos_sensor(0x0148,0x18); //EXTCLK_FRQ_MHZ[15:8]
    T8EV5YUV_write_cmos_sensor(0x0149,0x00); //EXTCLK_FRQ_MHZ[7:0]
    T8EV5YUV_write_cmos_sensor(0x014B,0x00); //SY_SPARE1[7:0]
    T8EV5YUV_write_cmos_sensor(0x014C,0x00); //SY_SPARE2[7:0]
    T8EV5YUV_write_cmos_sensor(0x0200,0xFF); //LMCC_SW/LM_BLK_SW/PGC_SW/UV_SW/YBLK_SW/ETI_SW/YNC_SW/
    T8EV5YUV_write_cmos_sensor(0x0201,0x3E); //ALS_SW/BLK_KILLER_SW/BRIGHT_SW/CNTLV_SW/CNT_SW/UVLPF_SW
    T8EV5YUV_write_cmos_sensor(0x0202,0x01); //-/-/-/-/-/-/LM_T0T1_SEL/FCS_SW
    T8EV5YUV_write_cmos_sensor(0x0203,0x11); //-/LCS_LI/LCS_CS/D_HCHARA/-/-/D_HLIMSW[1:0]
    T8EV5YUV_write_cmos_sensor(0x0204,0x5B); //LM_RMG[7:0]
    T8EV5YUV_write_cmos_sensor(0x0205,0x09); //LM_RMB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0206,0x1A); //LM_GMR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0207,0x1C); //LM_GMB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0208,0x04); //LM_BMR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0209,0x54); //LM_BMG[7:0]
    T8EV5YUV_write_cmos_sensor(0x020A,0x14); //-/GAM01P[6:0]
    T8EV5YUV_write_cmos_sensor(0x020B,0x18); //-/GAM02P[6:0]
    T8EV5YUV_write_cmos_sensor(0x020C,0x1B); //-/GAM03P[6:0]
    T8EV5YUV_write_cmos_sensor(0x020D,0x19); //-/GAM04P[6:0]
    T8EV5YUV_write_cmos_sensor(0x020E,0x2A); //-/GAM05P[6:0]
    T8EV5YUV_write_cmos_sensor(0x020F,0x2D); //-/GAM06P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0210,0x27); //-/GAM07P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0211,0x2C); //-/GAM08P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0212,0x48); //-/GAM09P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0213,0x46); //-/GAM10P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0214,0x30); //-/GAM11P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0215,0x21); //-/GAM12P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0216,0x21); //-/GAM13P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0217,0x1D); //-/GAM14P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0218,0x1A); //-/GAM15P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0219,0x19); //-/GAM16P[6:0]
    T8EV5YUV_write_cmos_sensor(0x021A,0x2F); //-/GAM17P[6:0]
    T8EV5YUV_write_cmos_sensor(0x021B,0x2A); //-/GAM18P[6:0]
    T8EV5YUV_write_cmos_sensor(0x021C,0x28); //-/GAM19P[6:0]
    T8EV5YUV_write_cmos_sensor(0x021D,0x28); //-/GAM20P[6:0]
    T8EV5YUV_write_cmos_sensor(0x021E,0x45); //-/GAM21P[6:0]
    T8EV5YUV_write_cmos_sensor(0x021F,0x3F); //-/GAM22P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0220,0x1F); //-/GAM23P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0221,0x0F); //-/GAM24P[6:0]
    T8EV5YUV_write_cmos_sensor(0x0222,0x00); //-/-/-/-/BLK_ADJ[3:0]
    T8EV5YUV_write_cmos_sensor(0x0223,0x57); //CbG_MAT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0224,0x83); //CbB_MAT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0225,0x80); //Cb_GAIN[7:0]
    T8EV5YUV_write_cmos_sensor(0x0226,0x83); //CrR_MAT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0227,0x6E); //CrG_MAT[7:0]
	T8EV5YUV_write_cmos_sensor(0x0228,0x80);//Cr_GAIN[7:0]
	T8EV5YUV_write_cmos_sensor(0x0229,0x2B);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
    T8EV5YUV_write_cmos_sensor(0x022A,0x03); //-/-/-/-/Cbr_MGAIN2[3:0]
	T8EV5YUV_write_cmos_sensor(0x022B,0x03);//EXTR_CRNG1[3:0]/EXTR_CRNG0[3:0]!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	T8EV5YUV_write_cmos_sensor(0x022C,0x00);//-/-/-/-/EXTR_CRNG2[3:0]
    T8EV5YUV_write_cmos_sensor(0x022D,0x88); //EXTR_PG1[3:0]/EXTR_PG0[3:0]
	T8EV5YUV_write_cmos_sensor(0x022E,0x00);//-/-/-/-/EXTR_PG2[3:0]
    T8EV5YUV_write_cmos_sensor(0x022F,0x88); //EXTR_MG1[3:0]/EXTR_MG0[3:0]
	T8EV5YUV_write_cmos_sensor(0x0230,0x00);//-/-/-/-/EXTR_MG2[3:0]
    T8EV5YUV_write_cmos_sensor(0x0231,0x55); //EXTR_LIM[7:0]
    T8EV5YUV_write_cmos_sensor(0x0232,0x00); //ETI_M[3:0]/ETI_P[3:0]
    T8EV5YUV_write_cmos_sensor(0x0233,0x00); //YNC_LIM1[3:0]/YNC_LIM0[3:0]
    T8EV5YUV_write_cmos_sensor(0x0234,0x00); //-/-/-/-/YNC_LIM2[3:0]
    T8EV5YUV_write_cmos_sensor(0x0235,0x00); //-/-/-/-/YNC_GAIN[3:0]
    T8EV5YUV_write_cmos_sensor(0x0236,0x00); //SHP_CRING1[3:0]/SHP_CRING0[3:0]
    T8EV5YUV_write_cmos_sensor(0x0237,0x00); //-/-/-/-/SHP_CRING2[3:0]
    T8EV5YUV_write_cmos_sensor(0x0238,0x00); //-/-/-/SHP_LIM1[4:0]
    T8EV5YUV_write_cmos_sensor(0x0239,0x38); //SHP_LIM2[7:0]
	T8EV5YUV_write_cmos_sensor(0x023A,0x08);//SHP_PGAIN1[3:0]/SHP_PGAIN0[3:0]
    T8EV5YUV_write_cmos_sensor(0x023B,0x00); //-/-/-/-/SHP_PGAIN2[3:0]
	T8EV5YUV_write_cmos_sensor(0x023C,0x08);//SHP_MGAIN1[3:0]/SHP_MGAIN0[3:0]
    T8EV5YUV_write_cmos_sensor(0x023D,0x00); //-/-/-/-/SHP_MGAIN2[3:0]
    T8EV5YUV_write_cmos_sensor(0x023E,0x80); //ALS_AG[7:0]
    T8EV5YUV_write_cmos_sensor(0x023F,0x60); //-/ALS_BLIM[6:0]
    T8EV5YUV_write_cmos_sensor(0x0240,0x60); //-/ALS_BLK[6:0]
    T8EV5YUV_write_cmos_sensor(0x0241,0x70); //ALS_WHT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0242,0xFC); //BLK_CHLV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0243,0xFC); //WHT_CHLV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0244,0xFF); //BLK_BLV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0245,0x00); //BLK_CNT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0246,0x08); //WHT_CNT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0247,0x00); //WHT_FLV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0248,0x18); //BRIGHT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0249,0x88); //CONT_LEV[7:0]
    T8EV5YUV_write_cmos_sensor(0x024A,0x03); //BLK_CONT1[3:0]/BLK_CONT0[3:0]
    T8EV5YUV_write_cmos_sensor(0x024B,0x00); //-/-/-/-/BLK_CONT2[3:0]
    T8EV5YUV_write_cmos_sensor(0x024C,0x00); //WHT_CONT1[3:0]/WHT_CONT0[3:0]
    T8EV5YUV_write_cmos_sensor(0x024D,0x00); //-/-/-/-/WHT_CONT2[3:0]
    T8EV5YUV_write_cmos_sensor(0x024E,0xFF); //UVK_GAIN1[1:0]/UVK_GAIN0[1:0]/LPF1[1:0]/LPF0[1:0]
    T8EV5YUV_write_cmos_sensor(0x024F,0x1F); //-/-/-/UVK_LIM0[4:0]
    T8EV5YUV_write_cmos_sensor(0x0250,0x1F); //-/-/-/UVK_LIM1[4:0]
    T8EV5YUV_write_cmos_sensor(0x0251,0x0F); //FCS_LV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0252,0x0F); //FCS_GAIN[7:0]
    T8EV5YUV_write_cmos_sensor(0x0253,0x07); //FCS_AG[7:0]
    T8EV5YUV_write_cmos_sensor(0x0255,0x82); //DTLGAIN[3:0]/EMBGAIN[3:0]
    T8EV5YUV_write_cmos_sensor(0x0256,0x6A); //SEPIAOFSU[7:0]
    T8EV5YUV_write_cmos_sensor(0x0257,0x93); //SEPIAOFSV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0258,0xC0); //UNICOFSU[7:0]           
    T8EV5YUV_write_cmos_sensor(0x0259,0x49); //UNICOFSV[7:0]
    T8EV5YUV_write_cmos_sensor(0x025A,0x02); //-/-/-/-/-/-/ANTQSFT[1:0]
    T8EV5YUV_write_cmos_sensor(0x025C,0x00); //-/-/-/-/-/-/TEST_IN_SW[1:0]
    T8EV5YUV_write_cmos_sensor(0x025D,0x00); //TEST_AG[7:0]
    T8EV5YUV_write_cmos_sensor(0x025E,0x00); //MP_SPARE[7:0]
    T8EV5YUV_write_cmos_sensor(0x0300,0x02); //-/-/-/-/-/-/ALCSW/ALCLOCK
    T8EV5YUV_write_cmos_sensor(0x0302,0x01); //-/-/-/-/-/-/ALCAIM[9:8]
    T8EV5YUV_write_cmos_sensor(0x0303,0x20); //ALCAIM[7:0]
    T8EV5YUV_write_cmos_sensor(0x0304,0x3E); //AGMIN[7:0]
	T8EV5YUV_write_cmos_sensor(0x0305,0x01);//-/-/-/-/AGMAX[11:8]
	T8EV5YUV_write_cmos_sensor(0x0306,0x4B);//AGMAX[7:0]
    T8EV5YUV_write_cmos_sensor(0x0307,0x0F); //MES[15:8]
    T8EV5YUV_write_cmos_sensor(0x0308,0xB0); //MES[7:0]
    T8EV5YUV_write_cmos_sensor(0x0309,0x40); //ESLIMMODE/ROOMDET/-/-/MAG[11:8]
    T8EV5YUV_write_cmos_sensor(0x030A,0xA0); //MAG[7:0]
    T8EV5YUV_write_cmos_sensor(0x030B,0x00); //MDG[7:0]
	T8EV5YUV_write_cmos_sensor(0x030C,0x1D);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
	T8EV5YUV_write_cmos_sensor(0x030D,0x1F);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
	T8EV5YUV_write_cmos_sensor(0x030E,0xD1);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
	T8EV5YUV_write_cmos_sensor(0x030F,0xD0);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
	T8EV5YUV_write_cmos_sensor(0x0310,0x1F);//UDMODE[1:0]/-/UPDNSPD[4:0]
	T8EV5YUV_write_cmos_sensor(0x0311,0x1F);//ALCOFS[2:0]/NEARSPD[4:0]
	T8EV5YUV_write_cmos_sensor(0x0312,0x08);//ALCFRZLV[7:0]
	T8EV5YUV_write_cmos_sensor(0x0313,0x01);//ALCFRZTIM[7:0]
	T8EV5YUV_write_cmos_sensor(0x0314,0xF0);//ALCSIGMAX[7:0]
    T8EV5YUV_write_cmos_sensor(0x0315,0x98); //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
    T8EV5YUV_write_cmos_sensor(0x0316,0x6C); //FLLONGON/FRMSPD[1:0]/FL600S[12:8]
    T8EV5YUV_write_cmos_sensor(0x0317,0x09); //FL600S[7:0]
    T8EV5YUV_write_cmos_sensor(0x0318,0xD6); //ACFDET/AC60M/FLMANU/ACDETDLY/MSKLINE[1:0]/ACDPWAIT[1:0]
    T8EV5YUV_write_cmos_sensor(0x0319,0x00); //FL600AT/TTL1V_ON/-/-/-/-/-/FLDETM[8]
    T8EV5YUV_write_cmos_sensor(0x031A,0x26); //FLDETM[7:0]
    T8EV5YUV_write_cmos_sensor(0x031B,0x02); //ACDET1LV[7:0]
    T8EV5YUV_write_cmos_sensor(0x031C,0x08); //ACDET2LV[7:0]
    T8EV5YUV_write_cmos_sensor(0x031D,0x0C); //SATGUP/SATFRZ/SAT1VDET/-/DETSEL[3:0]
    T8EV5YUV_write_cmos_sensor(0x031E,0xFF); //APLSAT[7:0]
    T8EV5YUV_write_cmos_sensor(0x031F,0x00); //PEAKALCON/-/-/-/PEAKDEC[3:0]
    T8EV5YUV_write_cmos_sensor(0x0320,0x81); //AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8]
    T8EV5YUV_write_cmos_sensor(0x0321,0x00); //WBMRG[7:0]
    T8EV5YUV_write_cmos_sensor(0x0322,0x3D); //CAREASEL[1:0]/AREAMODE[1:0]/HEXSW/YGATESW/WBMGG[9:8]
    T8EV5YUV_write_cmos_sensor(0x0323,0x00); //WBMGG[7:0]
    T8EV5YUV_write_cmos_sensor(0x0324,0x31); //SQ1SW/SQ1POL/SQ2SW/SQ2POL/SQ3SW/SQ3POL/WBMBG[9:8]
    T8EV5YUV_write_cmos_sensor(0x0325,0x00); //WBMBG[7:0]
    T8EV5YUV_write_cmos_sensor(0x0326,0x0D); //WBGRMAX[7:0]
    T8EV5YUV_write_cmos_sensor(0x0327,0x23); //WBGRMIN[7:0]
    T8EV5YUV_write_cmos_sensor(0x0328,0x2B); //WBGBMAX[7:0]
    T8EV5YUV_write_cmos_sensor(0x0329,0x0D); //WBGBMIN[7:0]
    T8EV5YUV_write_cmos_sensor(0x032A,0x0C); //RBCUT0H[7:0]
    T8EV5YUV_write_cmos_sensor(0x032B,0xEC); //RBCUT0L[7:0]
    T8EV5YUV_write_cmos_sensor(0x032C,0x28); //-/RYCUT0P[6:0]
    T8EV5YUV_write_cmos_sensor(0x032D,0x12); //-/RYCUT0N[6:0]
    T8EV5YUV_write_cmos_sensor(0x032E,0x0B); //-/BYCUT0P[6:0]
    T8EV5YUV_write_cmos_sensor(0x032F,0x29); //-/BYCUT0N[6:0]
    T8EV5YUV_write_cmos_sensor(0x0330,0x00); //RYCUT1H[7:0]
    T8EV5YUV_write_cmos_sensor(0x0331,0x00); //-/RYCUT1L[6:0]
    T8EV5YUV_write_cmos_sensor(0x0332,0x00); //BYCUT1H[7:0]
    T8EV5YUV_write_cmos_sensor(0x0333,0x00); //-/BYCUT1L[6:0]
    T8EV5YUV_write_cmos_sensor(0x0334,0xF0); //RYCUT2H[7:0]
    T8EV5YUV_write_cmos_sensor(0x0335,0x10); //-/RYCUT2L[6:0]
    T8EV5YUV_write_cmos_sensor(0x0336,0x15); //BYCUT2H[7:0]
    T8EV5YUV_write_cmos_sensor(0x0337,0x0F); //-/BYCUT2L[6:0]
    T8EV5YUV_write_cmos_sensor(0x0338,0x00); //RYCUT3H[7:0]
    T8EV5YUV_write_cmos_sensor(0x0339,0x00); //-/RYCUT3L[6:0]
    T8EV5YUV_write_cmos_sensor(0x033A,0x00); //BYCUT3H[7:0]
    T8EV5YUV_write_cmos_sensor(0x033B,0x00); //-/BYCUT3L[6:0]
    T8EV5YUV_write_cmos_sensor(0x033C,0xEB); //YGATEH[7:0]
    T8EV5YUV_write_cmos_sensor(0x033D,0x00); //YGATEL[7:0]
    T8EV5YUV_write_cmos_sensor(0x033E,0x45); //CGRANGE[1:0]/CGRANGE_EX/-/AWBSPD[3:0]
    T8EV5YUV_write_cmos_sensor(0x033F,0x02); //AWBHUECOR/-/-/AWBULV[4:0]
    T8EV5YUV_write_cmos_sensor(0x0340,0x02); //AWBFZTIM[2:0]/AWBVLV[4:0]
    T8EV5YUV_write_cmos_sensor(0x0341,0x00); //AWBSFTU[7:0]
    T8EV5YUV_write_cmos_sensor(0x0342,0x00); //AWBSFTV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0343,0x00); //AWBWAIT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0344,0x04); //-/-/-/-/CGCNGSLP[2:0]
    T8EV5YUV_write_cmos_sensor(0x0345,0x90); //CGCNGLV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0346,0x12); //-/RYCUTLITE[6:0]
    T8EV5YUV_write_cmos_sensor(0x0347,0x10); //-/BYCUTLITE[6:0]
    T8EV5YUV_write_cmos_sensor(0x0348,0x00); //SPLMKON/SPLMKBL/FAREAMK/CAREAMK/CGATEMK/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0349,0x03); //-/-/-/-/SPLADRH[11:8]
    T8EV5YUV_write_cmos_sensor(0x034A,0x28); //SPLADRH[7:0]
    T8EV5YUV_write_cmos_sensor(0x034B,0x20); //MKFLKON/MKFLKSPD[1:0]/-/-/SPLADRV[10:8]
    T8EV5YUV_write_cmos_sensor(0x034C,0x60); //SPLADRV[7:0]
    T8EV5YUV_write_cmos_sensor(0x034D,0x00); //MMES[15:8]
    T8EV5YUV_write_cmos_sensor(0x034E,0x60); //MMES[7:0]
    T8EV5YUV_write_cmos_sensor(0x034F,0x01); //-/-/-/-/-/-/ALCAIML[9:8]
    T8EV5YUV_write_cmos_sensor(0x0350,0x00); //ALCAIML[7:0]
    T8EV5YUV_write_cmos_sensor(0x0351,0xD0); //ALCSIGMIN[7:0]
    T8EV5YUV_write_cmos_sensor(0x0352,0x73); //PKDOT_CNTH[3:0]/PKDOT_CNTL[3:0]
    T8EV5YUV_write_cmos_sensor(0x0353,0xA0); //ALC_SUB[1:0]/ALC_ADD[1:0]/-/-/SELFRZLV/PKALCSW
    T8EV5YUV_write_cmos_sensor(0x0354,0x00); //-/-/-/-/-/-/PKALCESL[9:8]
    T8EV5YUV_write_cmos_sensor(0x0355,0x03); //PKALCESL[7:0]
    T8EV5YUV_write_cmos_sensor(0x0356,0x00); //-/-/ALC_SWSEL[1:0]/-/-/AWB_SWSEL[1:0]
    T8EV5YUV_write_cmos_sensor(0x0357,0xC0); //SATSET[7:0]
    T8EV5YUV_write_cmos_sensor(0x0358,0x00); //AU_SPARE_T0[7:0]
    T8EV5YUV_write_cmos_sensor(0x0400,0xFF); //HLNRSW/DANSASW[1:0]/LDNRSW/VNRSW/ABPCSW/WBPSW/BBPSW
    T8EV5YUV_write_cmos_sensor(0x0401,0x25); //-/PWB_T0T1_SEL/ANRSW/PEDAJSW/GDANSASW/LSSCSW/LSAG_SW/
    T8EV5YUV_write_cmos_sensor(0x0402,0x00); //-/-/-/-/-/TPATRGBSW[2:0]
    T8EV5YUV_write_cmos_sensor(0x0403,0x40); //BLKADJ[7:0]
    T8EV5YUV_write_cmos_sensor(0x0404,0x00); //BLKR[3:0]/BLKGR[3:0]
    T8EV5YUV_write_cmos_sensor(0x0405,0x00); //BLKGB[3:0]/BLKB[3:0]
    T8EV5YUV_write_cmos_sensor(0x0406,0x30); //-/-/OBWIDTH[5:0]
    T8EV5YUV_write_cmos_sensor(0x0407,0x00); //-/-/-/-/-/OBCLIP/BLLVSEL/DANSA2L
    T8EV5YUV_write_cmos_sensor(0x0408,0x03); //HNCLIM1[3:0]/HNCLIM0[3:0]
    T8EV5YUV_write_cmos_sensor(0x0409,0x0F); //-/-/-/-/HNCLIM2[3:0]
    T8EV5YUV_write_cmos_sensor(0x040A,0x03); //-/-/-/-/LDNRGA[3:0]
    T8EV5YUV_write_cmos_sensor(0x040B,0x73); //VNRLIM1[3:0]/VNRLIM0[3:0]
    T8EV5YUV_write_cmos_sensor(0x040C,0x0F); //-/-/-/-/VNRLIM2[3:0]
    T8EV5YUV_write_cmos_sensor(0x040D,0x08); //-/-/-/-/VNRG[3:0]
    T8EV5YUV_write_cmos_sensor(0x040E,0x0E); //BBPLV[7:0]
    T8EV5YUV_write_cmos_sensor(0x040F,0x10); //WBPLV0[7:0]
    T8EV5YUV_write_cmos_sensor(0x0410,0xC4); //WBPLV2[3:0]/WBPLV1[3:0]
    T8EV5YUV_write_cmos_sensor(0x0411,0x80); //WHTAG[7:0]
    T8EV5YUV_write_cmos_sensor(0x0412,0x68); //ANCNTLV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0413,0x55); //NSGAIN1[3:0]/NSGAIN0[3:0]
    T8EV5YUV_write_cmos_sensor(0x0414,0x00); //-/-/-/-/NSGAIN2[3:0]
    T8EV5YUV_write_cmos_sensor(0x0415,0x44); //EGCRNG1[3:0]/EGCRNG0[3:0]
    T8EV5YUV_write_cmos_sensor(0x0416,0x0F); //-/-/-/-/EGCRNG2[3:0]
    T8EV5YUV_write_cmos_sensor(0x0417,0x07); //-/-/-/-/EGGAIN[3:0]
    T8EV5YUV_write_cmos_sensor(0x0418,0x00); //-/-/EGLIMSG[1:0]/-/-/EGLIMS[1:0]
    T8EV5YUV_write_cmos_sensor(0x0419,0x04); //-/-/-/-/AJUST0[3:0]
    T8EV5YUV_write_cmos_sensor(0x041A,0x00); //GRPED0[3:0]/GBPED0[3:0]
    T8EV5YUV_write_cmos_sensor(0x041B,0x00); //RPED0[3:0]/BPED0[3:0]
    T8EV5YUV_write_cmos_sensor(0x041C,0x00); //GRPED1[3:0]/GBPED1[3:0]
    T8EV5YUV_write_cmos_sensor(0x041D,0x00); //RPED1[3:0]/BPED1[3:0]
    T8EV5YUV_write_cmos_sensor(0x041E,0x80); //GDANSALV[7:0]
    T8EV5YUV_write_cmos_sensor(0x041F,0x08); //-/-/-/-/GDANSAG[3:0]
    T8EV5YUV_write_cmos_sensor(0x0420,0x00); //PWBGAINGR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0421,0x00); //PWBGAINGB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0422,0x5A); //PWBGAINR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0423,0x3B); //PWBGAINB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0426,0x00); //-/-/-/-/-/-/LIPOL/CSPOL
    T8EV5YUV_write_cmos_sensor(0x0427,0x00); //-/LS4SIG[2:0]/-/LS1SIG[2:0]
    T8EV5YUV_write_cmos_sensor(0x0428,0x74); //LSHOFG[7:0]//80
    T8EV5YUV_write_cmos_sensor(0x0429,0x90); //LSHOFR[7:0]
    T8EV5YUV_write_cmos_sensor(0x042A,0xA8); //LSHOFB[7:0]
    T8EV5YUV_write_cmos_sensor(0x042B,0xAC); //LSVOFG[7:0]//B8
    T8EV5YUV_write_cmos_sensor(0x042C,0xA0); //LSVOFR[7:0]//B0
    T8EV5YUV_write_cmos_sensor(0x042D,0xA0); //LSVOFB[7:0]//A8
    T8EV5YUV_write_cmos_sensor(0x042E,0x60); //LSALUG[7:0] 1st left up
    T8EV5YUV_write_cmos_sensor(0x042F,0x50); //LSALUR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0430,0x50); //LSALUB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0431,0x80); //LSARUG[7:0] 1st right up
    T8EV5YUV_write_cmos_sensor(0x0432,0x50); //LSARUR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0433,0x40); //LSARUB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0434,0x58); //LSALDG[7:0] 1st left down
    T8EV5YUV_write_cmos_sensor(0x0435,0x69); //LSALDR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0436,0x56); //LSALDB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0437,0x38); //LSARDG[7:0]  1st right down
    T8EV5YUV_write_cmos_sensor(0x0438,0x42); //LSARDR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0439,0x1C); //LSARDB[7:0]
    T8EV5YUV_write_cmos_sensor(0x043A,0xC2); //LSBLG[7:0] 2nd left
    T8EV5YUV_write_cmos_sensor(0x043B,0x90); //LSBLR[7:0] //D8
    T8EV5YUV_write_cmos_sensor(0x043C,0x6D); //LSBLB[7:0]
    T8EV5YUV_write_cmos_sensor(0x043D,0xB5); //LSBRG[7:0]  2nd right
    T8EV5YUV_write_cmos_sensor(0x043E,0xB5); //LSBRR[7:0]//De
    T8EV5YUV_write_cmos_sensor(0x043F,0x62); //LSBRB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0440,0xE0); //LSCUG[7:0] 2nd up  
    T8EV5YUV_write_cmos_sensor(0x0441,0xC0); //LSCUR[7:0] //F7
    T8EV5YUV_write_cmos_sensor(0x0442,0x95); //LSCUB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0443,0x92); //LSCDG[7:0] 2nd down
    T8EV5YUV_write_cmos_sensor(0x0444,0x98); //LSCDR[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0445,0x89); //LSCDB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0446,0x00); //LSDLG[7:0] 4th left
    T8EV5YUV_write_cmos_sensor(0x0447,0x50); //LSDLR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0448,0x73); //LSDLB[7:0]
    T8EV5YUV_write_cmos_sensor(0x0449,0x30); //LSDRG[7:0] 4th right
    T8EV5YUV_write_cmos_sensor(0x044A,0x3C); //LSDRR[7:0]
    T8EV5YUV_write_cmos_sensor(0x044B,0x70); //LSDRB[7:0]
    T8EV5YUV_write_cmos_sensor(0x044C,0x40); //LSEUG[7:0] 4th up
    T8EV5YUV_write_cmos_sensor(0x044D,0x90); //LSEUR[7:0]
    T8EV5YUV_write_cmos_sensor(0x044E,0x90); //LSEUB[7:0]
    T8EV5YUV_write_cmos_sensor(0x044F,0xA0); //LSEDG[7:0] 4th down
    T8EV5YUV_write_cmos_sensor(0x0450,0xA0); //LSEDR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0451,0x90); //LSEDB[7:0]
    T8EV5YUV_write_cmos_sensor(0x045B,0xC0); //LSHCNT_MPY[7:0]
    T8EV5YUV_write_cmos_sensor(0x045C,0x01); //-/-/-/-/LSVCNT_MPY[11:8]
    T8EV5YUV_write_cmos_sensor(0x045D,0x7F); //LSVCNT_MPY[7:0]
    T8EV5YUV_write_cmos_sensor(0x045E,0x01); //-/-/-/-/-/-/LSMGSEL[1:0]
    T8EV5YUV_write_cmos_sensor(0x045F,0x00); //-/-/-/-/TESTPAT[3:0]
    T8EV5YUV_write_cmos_sensor(0x0460,0x00); //TPATSLPH[7:0]
    T8EV5YUV_write_cmos_sensor(0x0461,0x00); //TPATSLPV[7:0]
    T8EV5YUV_write_cmos_sensor(0x0462,0x00); //-/-/-/-/-/-/TDATARE[9:8]
    T8EV5YUV_write_cmos_sensor(0x0463,0x00); //TDATARE[7:0]
    T8EV5YUV_write_cmos_sensor(0x0464,0x00); //-/-/-/-/-/-/TDATAGR[9:8]
    T8EV5YUV_write_cmos_sensor(0x0465,0x00); //TDATAGR[7:0]
    T8EV5YUV_write_cmos_sensor(0x0466,0x00); //-/-/-/-/-/-/TDATABL[9:8]
    T8EV5YUV_write_cmos_sensor(0x0467,0x00); //TDATABL[7:0]
    T8EV5YUV_write_cmos_sensor(0x0468,0x00); //-/-/-/-/-/-/TDATAGB[9:8]
    T8EV5YUV_write_cmos_sensor(0x0469,0x00); //TDATAGB[7:0]
    T8EV5YUV_write_cmos_sensor(0x046B,0x00); //PP_SPARE[7:0]
    T8EV5YUV_write_cmos_sensor(0x0600,0x10); //-/-/-/BOOSTEN/POSLFIX/NEGLFIX/-/NEGLEAKCUT
    T8EV5YUV_write_cmos_sensor(0x0601,0x08); //BSTREADEV/-/-/-/NEGBSTCNT[3:0]
    T8EV5YUV_write_cmos_sensor(0x0602,0x06); //POSBSTSEL/-/-/-/-/POSBSTCNT[2:0]
    T8EV5YUV_write_cmos_sensor(0x0603,0x35); //-/POSBSTHG[2:0]/-/POSBSTGA[2:0]
    T8EV5YUV_write_cmos_sensor(0x0604,0x00); //-/-/-/-/RSTVDSEL/READVDSEL/LNOBMODE/GDMOSBGREN
    T8EV5YUV_write_cmos_sensor(0x0605,0x80); //KBIASSEL/-/-/-/-/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0606,0x07); //-/-/-/BSVBPSEL[4:0]
    T8EV5YUV_write_cmos_sensor(0x0607,0x31); //-/DRADRVLV[2:0]/-/-/DRADRVI[1:0]
    T8EV5YUV_write_cmos_sensor(0x0608,0x8E); //DRADRVPU[1:0]/-/VREFV[4:0]
    T8EV5YUV_write_cmos_sensor(0x0609,0xCC); //ADSW2WEAK/ADSW1WEAK/-/-/VREFAI[3:0]
    T8EV5YUV_write_cmos_sensor(0x060A,0x20); //ADCKSEL/-/ADCKDIV[1:0]/-/SENSEMODE[2:0]
    T8EV5YUV_write_cmos_sensor(0x060B,0x00); //-/-/SPARE[1:0]/ANAMON1_SEL[3:0]
    T8EV5YUV_write_cmos_sensor(0x060C,0x07); //HREG_TEST/-/-/-/-/BINVSIG/BINED/BINCMP
    T8EV5YUV_write_cmos_sensor(0x060E,0x00); //EXT_HCNT_MAX_ON/-/-/HCNT_MAX_MODE/-/-/-/MLT_SPL_MODE
    T8EV5YUV_write_cmos_sensor(0x060F,0x0A); //HCNT_MAX_FIXVAL[15:8]
    T8EV5YUV_write_cmos_sensor(0x0610,0xDC); //HCNT_MAX_FIXVAL[7:0]
    T8EV5YUV_write_cmos_sensor(0x0612,0x0C); //-/-/VREG_TEST[1:0]/ES_MODE/BIN_MODE/DIS_MODE/RODATA_U
    T8EV5YUV_write_cmos_sensor(0x0614,0x5C); //BSC_OFF/LIMITTER_BSC/-/DRESET_CONJ_U[4:0]
    T8EV5YUV_write_cmos_sensor(0x0615,0x04); //-/-/-/-/DRESET_CONJ_D[3:0]
    T8EV5YUV_write_cmos_sensor(0x0616,0x04); //FTLSNS_HIGH/-/FTLSNS_LBSC_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x0617,0x00); //-/-/-/-/-/FTLSNS_LBSC_D[2:0]
    T8EV5YUV_write_cmos_sensor(0x0618,0x18); //-/FTLSNS_CONJ_W[6:0]
    T8EV5YUV_write_cmos_sensor(0x0619,0x04); //-/-/-/-/FTLSNS_CONJ_D[3:0]
    T8EV5YUV_write_cmos_sensor(0x061A,0x08); //SADR_HIGH/-/SADR_LBSC_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x061B,0x00); //-/-/-/-/-/SADR_LBSC_D[2:0]
    T8EV5YUV_write_cmos_sensor(0x061C,0x2E); //SADR_CONJ_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x061D,0x00); //-/-/-/-/SADR_CONJ_D[3:0]
    T8EV5YUV_write_cmos_sensor(0x061F,0x03); //ESREAD_ALT_OFF/-/-/ELEC_INJ_MODE/-/-/AUTO_READ_W/AUT
    T8EV5YUV_write_cmos_sensor(0x0620,0x08); //-/-/-/-/FRAC_EXP_TIME[11:8]
    T8EV5YUV_write_cmos_sensor(0x0621,0x90); //FRAC_EXP_TIME[7:0]
    T8EV5YUV_write_cmos_sensor(0x0622,0x70); //ESREAD_1D[7:0]
    T8EV5YUV_write_cmos_sensor(0x0623,0x88); //ESREAD_1W[7:0]
    T8EV5YUV_write_cmos_sensor(0x0624,0x01); //-/-/-/-/-/-/ESREAD_2D[9:8]
    T8EV5YUV_write_cmos_sensor(0x0625,0x3D); //ESREAD_2D[7:0]
    T8EV5YUV_write_cmos_sensor(0x0626,0x88); //ESREAD_2W[7:0]
    T8EV5YUV_write_cmos_sensor(0x0627,0x24); //ESTGRESET_LOW/ESTGRESET_D[6:0]
    T8EV5YUV_write_cmos_sensor(0x0628,0x00); //ALLZEROSET_ON/EXTD_ROTGRESET/-/-/-/-/ROTGRESET_U[9:8]
    T8EV5YUV_write_cmos_sensor(0x0629,0xD8); //ROTGRESET_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x062B,0x00); //ROREAD_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x062C,0x88); //ROREAD_W[7:0]
    T8EV5YUV_write_cmos_sensor(0x062D,0x22); //ZEROSET_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x062E,0x60); //ZEROSET_W[7:0]
    T8EV5YUV_write_cmos_sensor(0x062F,0x00); //-/-/FIX_RSTDRAIN[1:0]/FIX_RSTDRAIN2[1:0]/FIX_RSTDRAIN3[1:0
    T8EV5YUV_write_cmos_sensor(0x0630,0x00); //-/RSTDRAIN_D[6:0]
    T8EV5YUV_write_cmos_sensor(0x0631,0x0F); //-/-/RSTDRAIN_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x0632,0x07); //-/-/-/-/RSTDRAIN2_D[3:0]
    T8EV5YUV_write_cmos_sensor(0x0633,0x07); //-/-/-/-/RSTDRAIN2_U[3:0]
    T8EV5YUV_write_cmos_sensor(0x0634,0x04); //-/-/-/-/RSTDRAIN3_D[3:0]
    T8EV5YUV_write_cmos_sensor(0x0635,0x0D); //-/-/RSTDRAIN3_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x0636,0x01); //-/-/DRCUT_SIGIN/DRCUT_HIGH/-/-/VSIGDR_MODE[1:0]
    T8EV5YUV_write_cmos_sensor(0x0637,0x01); //-/-/DRCUT_NML_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x0638,0x20); //-/-/DRCUT_CGR_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x0639,0x20); //-/-/DRCUT_CGR_D[5:0]
    T8EV5YUV_write_cmos_sensor(0x063A,0x30); //-/-/DRCUT_VDER_1U[5:0]
    T8EV5YUV_write_cmos_sensor(0x063B,0x04); //-/-/DRCUT_VDER_1D[5:0]
    T8EV5YUV_write_cmos_sensor(0x063C,0x30); //-/-/DRCUT_VDER_2U[5:0]
    T8EV5YUV_write_cmos_sensor(0x063D,0x04); //-/-/DRCUT_VDER_2D[5:0]
    T8EV5YUV_write_cmos_sensor(0x063E,0x00); //-/-/DRCUT_1ITV_MIN[1:0]/-/-/DRCUT_2ITV_MIN[1:0]
    T8EV5YUV_write_cmos_sensor(0x063F,0x2F); //GDMOSCNT_NML[3:0]/GDMOSCNT_VDER[3:0]
    T8EV5YUV_write_cmos_sensor(0x0640,0x01); //-/-/-/-/-/-/GDMOS_VDER_1U[1:0]
    T8EV5YUV_write_cmos_sensor(0x0641,0x04); //-/-/GDMOS_VDER_1D[5:0]
    T8EV5YUV_write_cmos_sensor(0x0642,0x01); //-/-/-/-/-/-/GDMOS_VDER_2U[1:0]
    T8EV5YUV_write_cmos_sensor(0x0643,0x04); //-/-/GDMOS_VDER_2D[5:0]
    T8EV5YUV_write_cmos_sensor(0x0644,0x00); //-/-/-/-/-/-/-/SIGIN_ON
    T8EV5YUV_write_cmos_sensor(0x0645,0x01); //-/-/-/-/-/-/GDMOSLT_VDER_1W[1:0]
    T8EV5YUV_write_cmos_sensor(0x0646,0x60); //-/GDMOSLT_VDER_1D[6:0]
    T8EV5YUV_write_cmos_sensor(0x0647,0x01); //-/-/-/-/-/-/GDMOSLT_VDER_2W[1:0]
    T8EV5YUV_write_cmos_sensor(0x0648,0x60); //-/GDMOSLT_VDER_2D[6:0]
    T8EV5YUV_write_cmos_sensor(0x0649,0x01); //-/-/-/-/-/-/-/VSIGPU_LOW
    T8EV5YUV_write_cmos_sensor(0x064A,0x0C); //VSIGPU_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x064B,0x14); //-/VSIGPU_W[6:0]
    T8EV5YUV_write_cmos_sensor(0x064C,0x00); //-/-/-/-/-/-/-/ROTGRESET_W[8]
    T8EV5YUV_write_cmos_sensor(0x064D,0x12); //ROTGRESET_W[7:0]
    T8EV5YUV_write_cmos_sensor(0x0650,0x0E); //ADSW1_D[7:0]
    T8EV5YUV_write_cmos_sensor(0x0651,0x00); //ADSW1_HIGH/-/ADSW_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x0652,0x07); //ADSW1DMX_LOW/-/-/ADSW1DMX_U[4:0]
    T8EV5YUV_write_cmos_sensor(0x0653,0x1C); //ADSW1LK_HIGH/ADSW1LK_D[6:0]
    T8EV5YUV_write_cmos_sensor(0x0654,0x1C); //ADSW1LKX_LOW/ADSW1LKX_U[6:0]
    T8EV5YUV_write_cmos_sensor(0x0655,0x80); //ADCMP1SRT_LOW/-/-/-/-/ADCMP1SRT_U[2:0]
    T8EV5YUV_write_cmos_sensor(0x0656,0x3D); //-/-/ADCMP1SRT_D[5:0]
    T8EV5YUV_write_cmos_sensor(0x0657,0x0E); //ADSW2_HIGH/-/ADSW2_D[5:0]
    T8EV5YUV_write_cmos_sensor(0x0658,0x07); //ADSW2DMX_LOW/-/-/ADSW2DMX_U[4:0]
    T8EV5YUV_write_cmos_sensor(0x0659,0x00); //FIX_ADENX[1:0]/ADENX_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x065A,0x01); //-/-/ADENX_D[5:0]
    T8EV5YUV_write_cmos_sensor(0x065B,0x00); //-/-/CMPI_CGR_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x065C,0x01); //-/-/CMPI_CGR_D[5:0]
    T8EV5YUV_write_cmos_sensor(0x065D,0xA3); //CMPI1_NML[3:0]/CMPI2_NML[3:0]
    T8EV5YUV_write_cmos_sensor(0x065E,0x33); //CMPI1_CGR[3:0]/CMPI2_CGR[3:0]
    T8EV5YUV_write_cmos_sensor(0x065F,0x00); //-/-/-/-/-/-/CGR_MODE/CDS_STOPBST
    T8EV5YUV_write_cmos_sensor(0x0660,0x04); //BSTCKLFIX_HIGH/-/-/BSTCKLFIX_U[4:0]
    T8EV5YUV_write_cmos_sensor(0x0661,0x04); //-/-/-/BSTCKLFIX_D[4:0]
    T8EV5YUV_write_cmos_sensor(0x0662,0x20); //-/-/ADENX_CGR_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x0663,0x20); //-/-/ADENX_CGR_D[5:0]
    T8EV5YUV_write_cmos_sensor(0x0664,0x00); //-/-/-/-/-/-/-/MS_ADV_INTVL[8]
    T8EV5YUV_write_cmos_sensor(0x0665,0xAE); //MS_ADV_INTVL[7:0]
    T8EV5YUV_write_cmos_sensor(0x0666,0x70); //MS_RSV_INTVL[7:0]
    T8EV5YUV_write_cmos_sensor(0x0667,0x03); //-/-/-/-/SINT_ZS_U[3:0]
    T8EV5YUV_write_cmos_sensor(0x0668,0x3B); //SINT_ZS_W[7:0]
    T8EV5YUV_write_cmos_sensor(0x0669,0x3B); //-/SINT_RS_U[6:0]
    T8EV5YUV_write_cmos_sensor(0x066A,0x59); //SINT_RS_W[7:0]
    T8EV5YUV_write_cmos_sensor(0x066B,0x1D); //SINT_FB_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x066C,0x3B); //-/SINT_FB_W[6:0]
    T8EV5YUV_write_cmos_sensor(0x066D,0x01); //-/-/-/-/-/-/-/SINT_AD_U[8]
    T8EV5YUV_write_cmos_sensor(0x066E,0x0B); //SINT_AD_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x066F,0x01); //-/-/-/-/-/-/-/SINT_AD_W[8]
    T8EV5YUV_write_cmos_sensor(0x0670,0x19); //SINT_AD_W[7:0]
    T8EV5YUV_write_cmos_sensor(0x0671,0x11); //-/-/SINTX_DSHIFT[1:0]/-/-/SINTX_USHIFT[1:0]
    T8EV5YUV_write_cmos_sensor(0x0672,0x1E); //SINT_MS_ZS_W[7:0]
    T8EV5YUV_write_cmos_sensor(0x0673,0x2B); //SINT_MS_RS_W[7:0]
    T8EV5YUV_write_cmos_sensor(0x0674,0x2E); //SINT_MS_RS_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x0675,0x01); //-/-/-/-/-/-/-/SINT_MS_AD1_U[8]
    T8EV5YUV_write_cmos_sensor(0x0676,0x23); //SINT_MS_AD1_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x0677,0x0F); //-/-/-/SINT_MS_FB_W[4:0]
    T8EV5YUV_write_cmos_sensor(0x0678,0x5F); //SINT_MS_AD_W[7:0]
    T8EV5YUV_write_cmos_sensor(0x0679,0x20); //-/SINT_MS_FB_D[6:0]
    T8EV5YUV_write_cmos_sensor(0x067A,0x6D); //SRST_RS_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x067B,0x0D); //-/-/SRST_RS_W[5:0]
    T8EV5YUV_write_cmos_sensor(0x067C,0x97); //SRST_ZS_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x067D,0x0D); //-/-/SRST_ZS_W[5:0]
    T8EV5YUV_write_cmos_sensor(0x067E,0x03); //-/-/-/-/SRST_AD_U[3:0]
    T8EV5YUV_write_cmos_sensor(0x067F,0x7D); //SRST_AD_D[7:0]
    T8EV5YUV_write_cmos_sensor(0x0680,0x60); //SRST_MS_AD4_D[7:0]
    T8EV5YUV_write_cmos_sensor(0x0681,0xBA); //SRST_MS_RS4_U[7:0]
    T8EV5YUV_write_cmos_sensor(0x0682,0x03); //-/-/-/-/SRST_MS_AD_U[3:0]
    T8EV5YUV_write_cmos_sensor(0x0683,0x0D); //-/-/SRST_MS_AD_W[5:0]
    T8EV5YUV_write_cmos_sensor(0x0684,0x03); //VREFSHBGR_LOW/-/-/-/VREFSHBGR_D[3:0]
    T8EV5YUV_write_cmos_sensor(0x0685,0x38); //-/-/VREFSHBGR_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x0686,0xBC); //EN_VREFC_ZERO/-/VREF_H_START_U[5:0]
    T8EV5YUV_write_cmos_sensor(0x0687,0x00); //ADCKEN_MASK[1:0]/-/-/-/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0688,0x0B); //-/ADCKEN_1U[6:0]
    T8EV5YUV_write_cmos_sensor(0x0689,0x0C); //-/-/-/ADCKEN_1D[4:0]
    T8EV5YUV_write_cmos_sensor(0x068A,0x0B); //-/ADCKEN_2U[6:0]
    T8EV5YUV_write_cmos_sensor(0x068B,0x0C); //-/-/-/ADCKEN_2D[4:0]
    T8EV5YUV_write_cmos_sensor(0x068C,0x09); //-/-/-/-/CNTRSTX_U[3:0]
    T8EV5YUV_write_cmos_sensor(0x068D,0x0C); //-/-/-/CNT0RSTX_1D[4:0]
    T8EV5YUV_write_cmos_sensor(0x068E,0x09); //-/-/-/-/CNT0RSTX_2U[3:0]
    T8EV5YUV_write_cmos_sensor(0x068F,0x0C); //-/-/-/CNT0RSTX_2D[4:0]
    T8EV5YUV_write_cmos_sensor(0x0690,0x08); //-/-/-/CNTINVX_START[4:0]
    T8EV5YUV_write_cmos_sensor(0x0691,0x14); //EDCONX_1D[7:0]
    T8EV5YUV_write_cmos_sensor(0x0692,0x00); //EDCONX_RS_HIGH/EDCONX_AD_HIGH/-/-/-/-/-/EDCONX_2D[8]
    T8EV5YUV_write_cmos_sensor(0x0693,0x28); //EDCONX_2D[7:0]
    T8EV5YUV_write_cmos_sensor(0x0694,0x00); //ADTESTCK_INTVL[3:0]/-/-/ADTESTCK_ON/COUNTER_TEST
    T8EV5YUV_write_cmos_sensor(0x0695,0x1F); //-/ADCKEN_MS_1U[6:0]
    T8EV5YUV_write_cmos_sensor(0x0696,0x1F); //-/ADCKEN_MS_2U[6:0]
    T8EV5YUV_write_cmos_sensor(0x0697,0x01); //EXT_VCD_ADJ_ON/MPS_AUTO_VCDADJ/-/AG_SEN_SHIFT/-/-/VCD_
    T8EV5YUV_write_cmos_sensor(0x0698,0x00); //VCD_COEF_FIXVAL[7:0]
    T8EV5YUV_write_cmos_sensor(0x0699,0x00); //-/-/VCD_INTC_FIXVAL[5:0]
    T8EV5YUV_write_cmos_sensor(0x069A,0x1B); //VREFAD_RNG1_SEL[1:0]/VREFAD_RNG2_SEL[1:0]/VREFAD_RNG3_SEL[1:0]
    T8EV5YUV_write_cmos_sensor(0x069B,0x00); //-/-/-/-/-/-/AGADJ1_VREFI_ZS[9:8]
    T8EV5YUV_write_cmos_sensor(0x069C,0x3C); //AGADJ1_VREFI_ZS[7:0]
    T8EV5YUV_write_cmos_sensor(0x069D,0x00); //-/-/-/-/-/-/AGADJ2_VREFI_ZS[9:8]
    T8EV5YUV_write_cmos_sensor(0x069E,0x1E); //AGADJ2_VREFI_ZS[7:0]
    T8EV5YUV_write_cmos_sensor(0x069F,0x00); //-/-/-/-/-/-/-/AGADJ1_VREFI_AD[8]
    T8EV5YUV_write_cmos_sensor(0x06A0,0x78); //T8EV5YUV_write_cmos_sensor(AGADJ1_VREFI_AD[7:0]
    T8EV5YUV_write_cmos_sensor(0x06A1,0x00); //-/-/-/-/-/-/-/AGADJ2_VREFI_AD[8]
    T8EV5YUV_write_cmos_sensor(0x06A2,0x3C); //AGADJ2_VREFI_AD[7:0]
    T8EV5YUV_write_cmos_sensor(0x06A3,0x06); //-/-/-/-/-/AGADJ_VREFC[2:0]
    T8EV5YUV_write_cmos_sensor(0x06A4,0x00); //EXT_VREFI_ZS_ON/-/-/-/-/-/VREFI_ZS_FIXVAL[9:8]
    T8EV5YUV_write_cmos_sensor(0x06A5,0x00); //VREFI_ZS_FIXVAL[7:0]
    T8EV5YUV_write_cmos_sensor(0x06A6,0x00); //EXT_VREFI_FB_ON/-/-/-/-/-/VREFI_FB_FIXVAL[9:8]
    T8EV5YUV_write_cmos_sensor(0x06A7,0x00); //VREFI_FB_FIXVAL[7:0]
    T8EV5YUV_write_cmos_sensor(0x06A9,0x02); //GDBSVDDSW_U[4:0]/NREGBSTEN/NREGBSTFNC[1:0]
    T8EV5YUV_write_cmos_sensor(0x06AA,0x00); //EXT_VREFC_ON/-/-/-/-/VREFC_FIXVAL[2:0]
    T8EV5YUV_write_cmos_sensor(0x06AB,0x04); //EXT_PLLFREQ_ON/-/-/-/PLLFREQ_FIXVAL[3:0]
    T8EV5YUV_write_cmos_sensor(0x06AC,0xBB); //PIXNREGBIASCNT[3:0]/CMPNREGBIASCNT[3:0]
    T8EV5YUV_write_cmos_sensor(0x06AD,0x40); //NREGBSTCNT[2:0]/PIXNREGEN/GDPXVDDEN/BSPXVDDEN/RGDBSVDDSRTX
    T8EV5YUV_write_cmos_sensor(0x06AE,0xC0); //BC_LTPTM[1:0]/-/ACT_TESTDAC/-/-/AG_TEST/TESTDACEN
    T8EV5YUV_write_cmos_sensor(0x06AF,0xFF); //TDAC_INT[7:0]
    T8EV5YUV_write_cmos_sensor(0x06B0,0x00); //TDAC_MIN[7:0]
    T8EV5YUV_write_cmos_sensor(0x06B1,0x10); //TDAC_STEP[3:0]/-/-/TDAC_SWD[1:0]
    T8EV5YUV_write_cmos_sensor(0x06B2,0x00); //PIXVREGFUNC[3:0]/CMPNREGEN/VREFNREGEN/RCVVDDSRTX/VSIGRCVDD
    T8EV5YUV_write_cmos_sensor(0x06B3,0x00); //DACS_INT[7:0]
    T8EV5YUV_write_cmos_sensor(0x06B4,0xFF); //DACS_MAX[7:0]
    T8EV5YUV_write_cmos_sensor(0x06B5,0x10); //DACS_STEP[3:0]/-/-/DACS_SWD[1:0]
    T8EV5YUV_write_cmos_sensor(0x06B6,0x80); //TESTDAC_RSVOL[7:0]
    T8EV5YUV_write_cmos_sensor(0x06B7,0x60); //TESTDAC_ADVOL[7:0]
    T8EV5YUV_write_cmos_sensor(0x06B8,0x62); //ZSV_EXEC_MODE[3:0]/-/AGADJ_EXEC_MODE[2:0]
    T8EV5YUV_write_cmos_sensor(0x06B9,0x02); //MPS_AGADJ_MODE/AGADJ_CALC_MODE/-/-/AGADJ_FIX_COEF[11:8]
    T8EV5YUV_write_cmos_sensor(0x06BA,0x06); //AGADJ_FIX_COEF[7:0]
    T8EV5YUV_write_cmos_sensor(0x06BB,0xF1); //ZSV_FORCE_END[3:0]/-/-/ZSV_SUSP_RANGE[1:0]
    T8EV5YUV_write_cmos_sensor(0x06BC,0x86); //ZSV_SUSP_CND/-/-/-/EN_PS_VREFI_ZS[3:0]
    T8EV5YUV_write_cmos_sensor(0x06BD,0x10); //VZS_MPS_STEP[7:0]
    T8EV5YUV_write_cmos_sensor(0x06BE,0xA0); //ZSV_LEVEL[7:0]
    T8EV5YUV_write_cmos_sensor(0x06BF,0x10); //-/-/ZSV_IN_RANGE[5:0]
    T8EV5YUV_write_cmos_sensor(0x06C0,0xC7); //PS_VZS_NML_COEF[7:0]
    T8EV5YUV_write_cmos_sensor(0x06C1,0x00); //-/PS_VZS_NML_INTC[6:0]
    T8EV5YUV_write_cmos_sensor(0x06C2,0x10); //VZS_NML_STEP[7:0]
    T8EV5YUV_write_cmos_sensor(0x06C3,0x44); //ZSV_STOP_CND[1:0]/-/-/ZSV_IN_LINES[3:0]
    T8EV5YUV_write_cmos_sensor(0x06C4,0xC7); //PS_VZS_MPS_COEF[7:0]
    T8EV5YUV_write_cmos_sensor(0x06C5,0x01); //-/PS_VZS_MPS_INTC[6:0]
    T8EV5YUV_write_cmos_sensor(0x06C6,0x18); //-/FBC_IN_RANGE[6:0]
    T8EV5YUV_write_cmos_sensor(0x06C7,0x44); //FBC_SUSP_RANGE[1:0]/-/FBC_IN_LINES[4:0]
    T8EV5YUV_write_cmos_sensor(0x06C8,0x44); //FBC_OUT_RANGE[1:0]/-/FBC_OUT_LINES[4:0]
    T8EV5YUV_write_cmos_sensor(0x06C9,0x20); //FBC_STOP_CND[2:0]/-/-/-/-/-
    T8EV5YUV_write_cmos_sensor(0x06CA,0x41); //FBC_START_CND[2:0]/-/VREFI_FB_STEP[3:0]
    T8EV5YUV_write_cmos_sensor(0x06CB,0x82); //FBC_SUSP_CND/-/-/-/EN_PS_VREFI_FB[3:0]
    T8EV5YUV_write_cmos_sensor(0x06CC,0xC0); //PS_VREFI_FB_AG/LIM_START_FBC/-/-/-/-/-/-
    T8EV5YUV_write_cmos_sensor(0x06CD,0x00); //-/-/-/ST_CKI[4:0]
    T8EV5YUV_write_cmos_sensor(0x06CE,0x30); //-/ST_BLACK_LEVEL[6:0]
    T8EV5YUV_write_cmos_sensor(0x06CF,0xF0); //ST_RSVD_REG[7:0]
    T8EV5YUV_write_cmos_sensor(0x06E5,0x22); //RORDDWSTMD/VDDRD28EN_1U[2:0]/-/VDDRD28EN_1D[2:0]
    T8EV5YUV_write_cmos_sensor(0x06E6,0x22); //-/BSTRDCUT_1U[2:0]/-/BSTRDCUT_1D[2:0]
    T8EV5YUV_write_cmos_sensor(0x06E7,0x22); //ESRDDWSTMD/VDDRD28EN_2U[2:0]/-/VDDRD28EN_2D[2:0]
    T8EV5YUV_write_cmos_sensor(0x06E8,0x22); //-/BSTRDCUT_2U[2:0]/-/BSTRDCUT_2D[2:0]
    T8EV5YUV_write_cmos_sensor(0x06E9,0x01); //-/-/GDMOS_CGR_D[5:0]
    T8EV5YUV_write_cmos_sensor(0x06EA,0x01); //-/-/-/-/GDMOSCNT_CGR[3:0]
    T8EV5YUV_write_cmos_sensor(0x06EB,0x02); //-/-/VOB_DISP/HOB_DISP/VENL_OFF/STP_VCNT_FRM[2:0]
    T8EV5YUV_write_cmos_sensor(0x06EC,0x00); //-/-/-/PS_VFB_NML_COEF[4:0]
    T8EV5YUV_write_cmos_sensor(0x06ED,0x00); //-/-/-/-/PS_VFB_NML_INTC[3:0]
    T8EV5YUV_write_cmos_sensor(0x06EE,0x00); //-/-/-/PS_VFB_MPS_COEF[4:0]
    T8EV5YUV_write_cmos_sensor(0x06EF,0x00); //-/-/-/-/PS_VFB_MPS_INTC[3:0]
    T8EV5YUV_write_cmos_sensor(0x06F0,0x00); //-/-/-/PS_VFB_NML1[4:0]
    T8EV5YUV_write_cmos_sensor(0x06F1,0x1E); //-/-/-/PS_VFB_NML2[4:0]
    T8EV5YUV_write_cmos_sensor(0x06F2,0x1C); //-/-/-/PS_VFB_NML3[4:0]
    T8EV5YUV_write_cmos_sensor(0x06F3,0x18); //-/-/-/PS_VFB_NML4[4:0]
    T8EV5YUV_write_cmos_sensor(0x06F4,0x1F); //-/-/-/PS_VFB_MPS1[4:0]
    T8EV5YUV_write_cmos_sensor(0x06F5,0x1D); //-/-/-/PS_VFB_MPS2[4:0]
    T8EV5YUV_write_cmos_sensor(0x06F6,0x1B); //-/-/-/PS_VFB_MPS3[4:0]
    T8EV5YUV_write_cmos_sensor(0x06F7,0x17); //-/-/-/PS_VFB_MPS4[4:0]
    T8EV5YUV_write_cmos_sensor(0x06F8,0x44); //BSC_ULMT_AGRNG2[7:0]
    T8EV5YUV_write_cmos_sensor(0x06F9,0x50); //BSC_ULMT_AGRNG1[7:0]
    T8EV5YUV_write_cmos_sensor(0x06FA,0x60); //BSC_ULMT_AGRNG0[7:0]
    T8EV5YUV_write_cmos_sensor(0x06FB,0x87); //KBIASCNT_RNG3[3:0]/KBIASCNT_RNG2[3:0]
    T8EV5YUV_write_cmos_sensor(0x06FC,0x64); //KBIASCNT_RNG1[3:0]/KBIASCNT_RNG0[3:0]
    T8EV5YUV_write_cmos_sensor(0x06FD,0x0F); //-/-/-/-/LIMIT_BSC_MODE[3:0]
    T8EV5YUV_write_cmos_sensor(0x0700,0x02); //SOCD[7:0]
    T8EV5YUV_write_cmos_sensor(0x0701,0x03); //SOEI[7:0]
    T8EV5YUV_write_cmos_sensor(0x0702,0x04); //EOEI[7:0]
    T8EV5YUV_write_cmos_sensor(0x0703,0xBC); //SOSI[7:0]
    T8EV5YUV_write_cmos_sensor(0x0704,0xBD); //EOSI[7:0]
    T8EV5YUV_write_cmos_sensor(0x0706,0x00); //-/-/-/SERI_ON_SW/-/-/-/SERI_ON_MN
    T8EV5YUV_write_cmos_sensor(0x0707,0x04); //-/-/-/-/JTIMES[3:0]
    T8EV5YUV_write_cmos_sensor(0x0710,0x68); //TCLK_POST[7:3]/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0711,0x28); //THS_PREPARE[7:3]/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0712,0x60); //THS_ZERO[7:3]/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0713,0x38); //THS_TRAIL[7:3]/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0714,0x38); //TCLK_TRAIL[7:3]/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0715,0x28); //TCLK_PREPARE[7:3]/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0716,0xC8); //TCLK_ZERO[7:3]/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0717,0x30); //TLPX[7:3]/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0718,0x03); //-/-/-/-/-/-/LNKBTWK_ON/LNKBT_ON
    T8EV5YUV_write_cmos_sensor(0x0719,0x02); //MSB_LBRATE[31:24]
    T8EV5YUV_write_cmos_sensor(0x071A,0xA8); //MSB_LBRATE[23:16]
    T8EV5YUV_write_cmos_sensor(0x0721,0x00); //-/-/-/-/-/-/CLKULPS/ESCREQ
    T8EV5YUV_write_cmos_sensor(0x0724,0x00); //-/-/-/-/-/MIPI_JPEG_ID[2:0]
	T8EV5YUV_write_cmos_sensor(0x0725,0x78);//ESCDATA[7:0]
    T8EV5YUV_write_cmos_sensor(0x0727,0x00); //LVDS_D1_DELAY[3:0]/LVDS_CLK_DELAY[3:0]
    T8EV5YUV_write_cmos_sensor(0x0728,0x40); //-/PHASE_ADJUST[2:0]/-/-/HS_SR_CNT/LP_SR_CNT
    T8EV5YUV_write_cmos_sensor(0x072A,0x00); //PN9/-/-/-/MIPI_TEST_MODE[3:0]
    T8EV5YUV_write_cmos_sensor(0x072B,0x00); //T_VALUE1[7:0]
    T8EV5YUV_write_cmos_sensor(0x072C,0x00); //T_VALUE2[7:0]
     T8EV5YUV_write_cmos_sensor(0x072D,0x00); //-/-/-/-/-/-/-/MIPI_CLK_MODE
    T8EV5YUV_write_cmos_sensor(0x072E,0x00); //-/-/-/-/LB_TEST_CLR/LB_TEST_EN/-/LB_MODE
    T8EV5YUV_write_cmos_sensor(0x0730,0x00); //-/-/-/-/-/-/FIFODLY[9:8]
    T8EV5YUV_write_cmos_sensor(0x0731,0x00); //FIFODLY[7:0]
    T8EV5YUV_write_cmos_sensor(0x0732,0xA7); //NUMWAKE[7:0]
    T8EV5YUV_write_cmos_sensor(0x0734,0x5F); //RV_MAT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0735,0x55); //GU_MAT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0736,0xB3); //GV_MAT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0737,0xBD); //BU_MAT[7:0]
    T8EV5YUV_write_cmos_sensor(0x0740,0x3E); //JPG_QTBLNUM[7:0]
    T8EV5YUV_write_cmos_sensor(0x0741,0x3C); //JPG_HTBLNUM[7:0]
    T8EV5YUV_write_cmos_sensor(0x0742,0x00); //JPG_DRIH[7:0]
    T8EV5YUV_write_cmos_sensor(0x0743,0x00); //JPG_DRIL[7:0]
    T8EV5YUV_write_cmos_sensor(0x0744,0x00); //-/-/-/JPG_PROC[1:0]/-/-/-
    T8EV5YUV_write_cmos_sensor(0x0745,0x00); //-/-/-/-/-/JPG_CTRL[2:0]
    T8EV5YUV_write_cmos_sensor(0x0746,0x01); //-/-/-/-/-/JPG_START[2:0]
    T8EV5YUV_write_cmos_sensor(0x074E,0x00); //JP_SPARE[7:0]
    T8EV5YUV_write_cmos_sensor(0x074F,0x00); //JP_SPARE[15:8]
    T8EV5YUV_write_cmos_sensor(0x0F00,0x00); //-/-/-/T_DACTEST/-/T_TMOSEL[2:0]
    T8EV5YUV_write_cmos_sensor(0x0F01,0x00); //-/-/-/T_OUTSEL[4:0]
    T8EV5YUV_write_cmos_sensor(0x0100,0x01); //-/-/-/-/-/-/-/MODSEL
    #if 0
	T8EV5YUV_write_cmos_sensor(0x030C,0x55);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
	T8EV5YUV_write_cmos_sensor(0x030D,0x5A);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
	T8EV5YUV_write_cmos_sensor(0x030E,0x95);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
	T8EV5YUV_write_cmos_sensor(0x030F,0x50);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
	#else
	T8EV5YUV_write_cmos_sensor(0x030C,0x0C);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
	T8EV5YUV_write_cmos_sensor(0x030D,0x1F);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
	T8EV5YUV_write_cmos_sensor(0x030E,0xD0);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
	T8EV5YUV_write_cmos_sensor(0x030F,0xC0);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-	
	#endif
}

static void T8EV5YUV_Sensor_AF_Patch(void)
{
    //AF  patch
    #if 1
    
    T8EV5YUV_write_cmos_sensor(0x2400,0x04);
    T8EV5YUV_write_cmos_sensor(0x2400,0x00);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x30);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x43);
    T8EV5YUV_write_cmos_sensor(0x240B,0x38);
    T8EV5YUV_write_cmos_sensor(0x240C,0x33);
    T8EV5YUV_write_cmos_sensor(0x240D,0x41);
    T8EV5YUV_write_cmos_sensor(0x240E,0x34);
    T8EV5YUV_write_cmos_sensor(0x240F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x30);
    T8EV5YUV_write_cmos_sensor(0x2412,0x38);
    T8EV5YUV_write_cmos_sensor(0x2413,0x30);
    T8EV5YUV_write_cmos_sensor(0x2414,0x45);
    T8EV5YUV_write_cmos_sensor(0x2415,0x41);
    T8EV5YUV_write_cmos_sensor(0x2416,0x36);
    T8EV5YUV_write_cmos_sensor(0x2417,0x46);
    T8EV5YUV_write_cmos_sensor(0x2418,0x30);
    T8EV5YUV_write_cmos_sensor(0x2419,0x30);
    T8EV5YUV_write_cmos_sensor(0x241A,0x43);
    T8EV5YUV_write_cmos_sensor(0x241B,0x30);
    T8EV5YUV_write_cmos_sensor(0x241C,0x44);
    T8EV5YUV_write_cmos_sensor(0x241D,0x42);
    T8EV5YUV_write_cmos_sensor(0x241E,0x31);
    T8EV5YUV_write_cmos_sensor(0x241F,0x31);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x41);
    T8EV5YUV_write_cmos_sensor(0x2422,0x30);
    T8EV5YUV_write_cmos_sensor(0x2423,0x46);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x30);
    T8EV5YUV_write_cmos_sensor(0x2426,0x45);
    T8EV5YUV_write_cmos_sensor(0x2427,0x32);
    T8EV5YUV_write_cmos_sensor(0x2428,0x34);
    T8EV5YUV_write_cmos_sensor(0x2429,0x30);
    T8EV5YUV_write_cmos_sensor(0x242A,0x43);
    T8EV5YUV_write_cmos_sensor(0x242B,0x34);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x31);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x30);
    T8EV5YUV_write_cmos_sensor(0x240B,0x41);
    T8EV5YUV_write_cmos_sensor(0x240C,0x30);
    T8EV5YUV_write_cmos_sensor(0x240D,0x46);
    T8EV5YUV_write_cmos_sensor(0x240E,0x38);
    T8EV5YUV_write_cmos_sensor(0x240F,0x30);
    T8EV5YUV_write_cmos_sensor(0x2410,0x46);
    T8EV5YUV_write_cmos_sensor(0x2411,0x32);
    T8EV5YUV_write_cmos_sensor(0x2412,0x37);
    T8EV5YUV_write_cmos_sensor(0x2413,0x38);
    T8EV5YUV_write_cmos_sensor(0x2414,0x33);
    T8EV5YUV_write_cmos_sensor(0x2415,0x32);
    T8EV5YUV_write_cmos_sensor(0x2416,0x45);
    T8EV5YUV_write_cmos_sensor(0x2417,0x41);
    T8EV5YUV_write_cmos_sensor(0x2418,0x36);
    T8EV5YUV_write_cmos_sensor(0x2419,0x46);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x30);
    T8EV5YUV_write_cmos_sensor(0x241C,0x43);
    T8EV5YUV_write_cmos_sensor(0x241D,0x30);
    T8EV5YUV_write_cmos_sensor(0x241E,0x44);
    T8EV5YUV_write_cmos_sensor(0x241F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2420,0x45);
    T8EV5YUV_write_cmos_sensor(0x2421,0x46);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x41);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x35);
    T8EV5YUV_write_cmos_sensor(0x2426,0x38);
    T8EV5YUV_write_cmos_sensor(0x2427,0x41);
    T8EV5YUV_write_cmos_sensor(0x2428,0x45);
    T8EV5YUV_write_cmos_sensor(0x2429,0x38);
    T8EV5YUV_write_cmos_sensor(0x242A,0x45);
    T8EV5YUV_write_cmos_sensor(0x242B,0x38);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x32);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x30);
    T8EV5YUV_write_cmos_sensor(0x240B,0x35);
    T8EV5YUV_write_cmos_sensor(0x240C,0x46);
    T8EV5YUV_write_cmos_sensor(0x240D,0x32);
    T8EV5YUV_write_cmos_sensor(0x240E,0x37);
    T8EV5YUV_write_cmos_sensor(0x240F,0x38);
    T8EV5YUV_write_cmos_sensor(0x2410,0x34);
    T8EV5YUV_write_cmos_sensor(0x2411,0x41);
    T8EV5YUV_write_cmos_sensor(0x2412,0x30);
    T8EV5YUV_write_cmos_sensor(0x2413,0x36);
    T8EV5YUV_write_cmos_sensor(0x2414,0x38);
    T8EV5YUV_write_cmos_sensor(0x2415,0x41);
    T8EV5YUV_write_cmos_sensor(0x2416,0x46);
    T8EV5YUV_write_cmos_sensor(0x2417,0x32);
    T8EV5YUV_write_cmos_sensor(0x2418,0x37);
    T8EV5YUV_write_cmos_sensor(0x2419,0x38);
    T8EV5YUV_write_cmos_sensor(0x241A,0x34);
    T8EV5YUV_write_cmos_sensor(0x241B,0x41);
    T8EV5YUV_write_cmos_sensor(0x241C,0x31);
    T8EV5YUV_write_cmos_sensor(0x241D,0x33);
    T8EV5YUV_write_cmos_sensor(0x241E,0x38);
    T8EV5YUV_write_cmos_sensor(0x241F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2420,0x46);
    T8EV5YUV_write_cmos_sensor(0x2421,0x32);
    T8EV5YUV_write_cmos_sensor(0x2422,0x37);
    T8EV5YUV_write_cmos_sensor(0x2423,0x38);
    T8EV5YUV_write_cmos_sensor(0x2424,0x34);
    T8EV5YUV_write_cmos_sensor(0x2425,0x41);
    T8EV5YUV_write_cmos_sensor(0x2426,0x41);
    T8EV5YUV_write_cmos_sensor(0x2427,0x38);
    T8EV5YUV_write_cmos_sensor(0x2428,0x38);
    T8EV5YUV_write_cmos_sensor(0x2429,0x46);
    T8EV5YUV_write_cmos_sensor(0x242A,0x32);
    T8EV5YUV_write_cmos_sensor(0x242B,0x42);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x33);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x46);
    T8EV5YUV_write_cmos_sensor(0x240B,0x32);
    T8EV5YUV_write_cmos_sensor(0x240C,0x37);
    T8EV5YUV_write_cmos_sensor(0x240D,0x38);
    T8EV5YUV_write_cmos_sensor(0x240E,0x34);
    T8EV5YUV_write_cmos_sensor(0x240F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2410,0x41);
    T8EV5YUV_write_cmos_sensor(0x2411,0x39);
    T8EV5YUV_write_cmos_sensor(0x2412,0x38);
    T8EV5YUV_write_cmos_sensor(0x2413,0x46);
    T8EV5YUV_write_cmos_sensor(0x2414,0x46);
    T8EV5YUV_write_cmos_sensor(0x2415,0x32);
    T8EV5YUV_write_cmos_sensor(0x2416,0x37);
    T8EV5YUV_write_cmos_sensor(0x2417,0x38);
    T8EV5YUV_write_cmos_sensor(0x2418,0x34);
    T8EV5YUV_write_cmos_sensor(0x2419,0x41);
    T8EV5YUV_write_cmos_sensor(0x241A,0x41);
    T8EV5YUV_write_cmos_sensor(0x241B,0x41);
    T8EV5YUV_write_cmos_sensor(0x241C,0x38);
    T8EV5YUV_write_cmos_sensor(0x241D,0x46);
    T8EV5YUV_write_cmos_sensor(0x241E,0x46);
    T8EV5YUV_write_cmos_sensor(0x241F,0x32);
    T8EV5YUV_write_cmos_sensor(0x2420,0x37);
    T8EV5YUV_write_cmos_sensor(0x2421,0x38);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x41);
    T8EV5YUV_write_cmos_sensor(0x2424,0x41);
    T8EV5YUV_write_cmos_sensor(0x2425,0x42);
    T8EV5YUV_write_cmos_sensor(0x2426,0x38);
    T8EV5YUV_write_cmos_sensor(0x2427,0x46);
    T8EV5YUV_write_cmos_sensor(0x2428,0x46);
    T8EV5YUV_write_cmos_sensor(0x2429,0x32);
    T8EV5YUV_write_cmos_sensor(0x242A,0x45);
    T8EV5YUV_write_cmos_sensor(0x242B,0x37);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x34);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x37);
    T8EV5YUV_write_cmos_sensor(0x240B,0x38);
    T8EV5YUV_write_cmos_sensor(0x240C,0x34);
    T8EV5YUV_write_cmos_sensor(0x240D,0x41);
    T8EV5YUV_write_cmos_sensor(0x240E,0x43);
    T8EV5YUV_write_cmos_sensor(0x240F,0x31);
    T8EV5YUV_write_cmos_sensor(0x2410,0x38);
    T8EV5YUV_write_cmos_sensor(0x2411,0x46);
    T8EV5YUV_write_cmos_sensor(0x2412,0x46);
    T8EV5YUV_write_cmos_sensor(0x2413,0x32);
    T8EV5YUV_write_cmos_sensor(0x2414,0x37);
    T8EV5YUV_write_cmos_sensor(0x2415,0x38);
    T8EV5YUV_write_cmos_sensor(0x2416,0x34);
    T8EV5YUV_write_cmos_sensor(0x2417,0x41);
    T8EV5YUV_write_cmos_sensor(0x2418,0x43);
    T8EV5YUV_write_cmos_sensor(0x2419,0x32);
    T8EV5YUV_write_cmos_sensor(0x241A,0x38);
    T8EV5YUV_write_cmos_sensor(0x241B,0x46);
    T8EV5YUV_write_cmos_sensor(0x241C,0x46);
    T8EV5YUV_write_cmos_sensor(0x241D,0x32);
    T8EV5YUV_write_cmos_sensor(0x241E,0x37);
    T8EV5YUV_write_cmos_sensor(0x241F,0x38);
    T8EV5YUV_write_cmos_sensor(0x2420,0x34);
    T8EV5YUV_write_cmos_sensor(0x2421,0x41);
    T8EV5YUV_write_cmos_sensor(0x2422,0x43);
    T8EV5YUV_write_cmos_sensor(0x2423,0x33);
    T8EV5YUV_write_cmos_sensor(0x2424,0x38);
    T8EV5YUV_write_cmos_sensor(0x2425,0x46);
    T8EV5YUV_write_cmos_sensor(0x2426,0x46);
    T8EV5YUV_write_cmos_sensor(0x2427,0x32);
    T8EV5YUV_write_cmos_sensor(0x2428,0x37);
    T8EV5YUV_write_cmos_sensor(0x2429,0x38);
    T8EV5YUV_write_cmos_sensor(0x242A,0x30);
    T8EV5YUV_write_cmos_sensor(0x242B,0x39);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x35);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x34);
    T8EV5YUV_write_cmos_sensor(0x240B,0x41);
    T8EV5YUV_write_cmos_sensor(0x240C,0x43);
    T8EV5YUV_write_cmos_sensor(0x240D,0x34);
    T8EV5YUV_write_cmos_sensor(0x240E,0x38);
    T8EV5YUV_write_cmos_sensor(0x240F,0x46);
    T8EV5YUV_write_cmos_sensor(0x2410,0x46);
    T8EV5YUV_write_cmos_sensor(0x2411,0x32);
    T8EV5YUV_write_cmos_sensor(0x2412,0x37);
    T8EV5YUV_write_cmos_sensor(0x2413,0x38);
    T8EV5YUV_write_cmos_sensor(0x2414,0x34);
    T8EV5YUV_write_cmos_sensor(0x2415,0x41);
    T8EV5YUV_write_cmos_sensor(0x2416,0x39);
    T8EV5YUV_write_cmos_sensor(0x2417,0x43);
    T8EV5YUV_write_cmos_sensor(0x2418,0x36);
    T8EV5YUV_write_cmos_sensor(0x2419,0x30);
    T8EV5YUV_write_cmos_sensor(0x241A,0x46);
    T8EV5YUV_write_cmos_sensor(0x241B,0x32);
    T8EV5YUV_write_cmos_sensor(0x241C,0x46);
    T8EV5YUV_write_cmos_sensor(0x241D,0x39);
    T8EV5YUV_write_cmos_sensor(0x241E,0x30);
    T8EV5YUV_write_cmos_sensor(0x241F,0x33);
    T8EV5YUV_write_cmos_sensor(0x2420,0x34);
    T8EV5YUV_write_cmos_sensor(0x2421,0x41);
    T8EV5YUV_write_cmos_sensor(0x2422,0x45);
    T8EV5YUV_write_cmos_sensor(0x2423,0x39);
    T8EV5YUV_write_cmos_sensor(0x2424,0x39);
    T8EV5YUV_write_cmos_sensor(0x2425,0x30);
    T8EV5YUV_write_cmos_sensor(0x2426,0x46);
    T8EV5YUV_write_cmos_sensor(0x2427,0x32);
    T8EV5YUV_write_cmos_sensor(0x2428,0x46);
    T8EV5YUV_write_cmos_sensor(0x2429,0x39);
    T8EV5YUV_write_cmos_sensor(0x242A,0x39);
    T8EV5YUV_write_cmos_sensor(0x242B,0x37);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x36);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x46);
    T8EV5YUV_write_cmos_sensor(0x240B,0x44);
    T8EV5YUV_write_cmos_sensor(0x240C,0x34);
    T8EV5YUV_write_cmos_sensor(0x240D,0x41);
    T8EV5YUV_write_cmos_sensor(0x240E,0x45);
    T8EV5YUV_write_cmos_sensor(0x240F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2410,0x39);
    T8EV5YUV_write_cmos_sensor(0x2411,0x30);
    T8EV5YUV_write_cmos_sensor(0x2412,0x46);
    T8EV5YUV_write_cmos_sensor(0x2413,0x32);
    T8EV5YUV_write_cmos_sensor(0x2414,0x37);
    T8EV5YUV_write_cmos_sensor(0x2415,0x38);
    T8EV5YUV_write_cmos_sensor(0x2416,0x34);
    T8EV5YUV_write_cmos_sensor(0x2417,0x41);
    T8EV5YUV_write_cmos_sensor(0x2418,0x45);
    T8EV5YUV_write_cmos_sensor(0x2419,0x42);
    T8EV5YUV_write_cmos_sensor(0x241A,0x39);
    T8EV5YUV_write_cmos_sensor(0x241B,0x30);
    T8EV5YUV_write_cmos_sensor(0x241C,0x46);
    T8EV5YUV_write_cmos_sensor(0x241D,0x32);
    T8EV5YUV_write_cmos_sensor(0x241E,0x46);
    T8EV5YUV_write_cmos_sensor(0x241F,0x39);
    T8EV5YUV_write_cmos_sensor(0x2420,0x33);
    T8EV5YUV_write_cmos_sensor(0x2421,0x30);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x41);
    T8EV5YUV_write_cmos_sensor(0x2424,0x45);
    T8EV5YUV_write_cmos_sensor(0x2425,0x43);
    T8EV5YUV_write_cmos_sensor(0x2426,0x39);
    T8EV5YUV_write_cmos_sensor(0x2427,0x30);
    T8EV5YUV_write_cmos_sensor(0x2428,0x46);
    T8EV5YUV_write_cmos_sensor(0x2429,0x32);
    T8EV5YUV_write_cmos_sensor(0x242A,0x41);
    T8EV5YUV_write_cmos_sensor(0x242B,0x44);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x30);
    T8EV5YUV_write_cmos_sensor(0x2403,0x38);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x37);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x46);
    T8EV5YUV_write_cmos_sensor(0x240B,0x39);
    T8EV5YUV_write_cmos_sensor(0x240C,0x46);
    T8EV5YUV_write_cmos_sensor(0x240D,0x41);
    T8EV5YUV_write_cmos_sensor(0x240E,0x46);
    T8EV5YUV_write_cmos_sensor(0x240F,0x45);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x30);
    T8EV5YUV_write_cmos_sensor(0x2412,0x38);
    T8EV5YUV_write_cmos_sensor(0x2413,0x30);
    T8EV5YUV_write_cmos_sensor(0x2414,0x43);
    T8EV5YUV_write_cmos_sensor(0x2415,0x30);
    T8EV5YUV_write_cmos_sensor(0x2416,0x33);
    T8EV5YUV_write_cmos_sensor(0x2417,0x41);
    T8EV5YUV_write_cmos_sensor(0x2418,0x46);
    T8EV5YUV_write_cmos_sensor(0x2419,0x41);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x33);
    T8EV5YUV_write_cmos_sensor(0x241C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x241D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x30);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x33);
    T8EV5YUV_write_cmos_sensor(0x240B,0x46);
    T8EV5YUV_write_cmos_sensor(0x240C,0x31);
    T8EV5YUV_write_cmos_sensor(0x240D,0x34);
    T8EV5YUV_write_cmos_sensor(0x240E,0x35);
    T8EV5YUV_write_cmos_sensor(0x240F,0x33);
    T8EV5YUV_write_cmos_sensor(0x2410,0x45);
    T8EV5YUV_write_cmos_sensor(0x2411,0x31);
    T8EV5YUV_write_cmos_sensor(0x2412,0x46);
    T8EV5YUV_write_cmos_sensor(0x2413,0x37);
    T8EV5YUV_write_cmos_sensor(0x2414,0x30);
    T8EV5YUV_write_cmos_sensor(0x2415,0x30);
    T8EV5YUV_write_cmos_sensor(0x2416,0x34);
    T8EV5YUV_write_cmos_sensor(0x2417,0x30);
    T8EV5YUV_write_cmos_sensor(0x2418,0x36);
    T8EV5YUV_write_cmos_sensor(0x2419,0x34);
    T8EV5YUV_write_cmos_sensor(0x241A,0x36);
    T8EV5YUV_write_cmos_sensor(0x241B,0x30);
    T8EV5YUV_write_cmos_sensor(0x241C,0x36);
    T8EV5YUV_write_cmos_sensor(0x241D,0x37);
    T8EV5YUV_write_cmos_sensor(0x241E,0x34);
    T8EV5YUV_write_cmos_sensor(0x241F,0x30);
    T8EV5YUV_write_cmos_sensor(0x2420,0x38);
    T8EV5YUV_write_cmos_sensor(0x2421,0x32);
    T8EV5YUV_write_cmos_sensor(0x2422,0x46);
    T8EV5YUV_write_cmos_sensor(0x2423,0x45);
    T8EV5YUV_write_cmos_sensor(0x2424,0x32);
    T8EV5YUV_write_cmos_sensor(0x2425,0x41);
    T8EV5YUV_write_cmos_sensor(0x2426,0x33);
    T8EV5YUV_write_cmos_sensor(0x2427,0x31);
    T8EV5YUV_write_cmos_sensor(0x2428,0x34);
    T8EV5YUV_write_cmos_sensor(0x2429,0x41);
    T8EV5YUV_write_cmos_sensor(0x242A,0x37);
    T8EV5YUV_write_cmos_sensor(0x242B,0x32);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x31);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x31);
    T8EV5YUV_write_cmos_sensor(0x240B,0x39);
    T8EV5YUV_write_cmos_sensor(0x240C,0x36);
    T8EV5YUV_write_cmos_sensor(0x240D,0x30);
    T8EV5YUV_write_cmos_sensor(0x240E,0x45);
    T8EV5YUV_write_cmos_sensor(0x240F,0x31);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x31);
    T8EV5YUV_write_cmos_sensor(0x2412,0x30);
    T8EV5YUV_write_cmos_sensor(0x2413,0x31);
    T8EV5YUV_write_cmos_sensor(0x2414,0x34);
    T8EV5YUV_write_cmos_sensor(0x2415,0x30);
    T8EV5YUV_write_cmos_sensor(0x2416,0x36);
    T8EV5YUV_write_cmos_sensor(0x2417,0x34);
    T8EV5YUV_write_cmos_sensor(0x2418,0x30);
    T8EV5YUV_write_cmos_sensor(0x2419,0x37);
    T8EV5YUV_write_cmos_sensor(0x241A,0x31);
    T8EV5YUV_write_cmos_sensor(0x241B,0x39);
    T8EV5YUV_write_cmos_sensor(0x241C,0x30);
    T8EV5YUV_write_cmos_sensor(0x241D,0x30);
    T8EV5YUV_write_cmos_sensor(0x241E,0x45);
    T8EV5YUV_write_cmos_sensor(0x241F,0x38);
    T8EV5YUV_write_cmos_sensor(0x2420,0x46);
    T8EV5YUV_write_cmos_sensor(0x2421,0x30);
    T8EV5YUV_write_cmos_sensor(0x2422,0x45);
    T8EV5YUV_write_cmos_sensor(0x2423,0x38);
    T8EV5YUV_write_cmos_sensor(0x2424,0x39);
    T8EV5YUV_write_cmos_sensor(0x2425,0x31);
    T8EV5YUV_write_cmos_sensor(0x2426,0x45);
    T8EV5YUV_write_cmos_sensor(0x2427,0x32);
    T8EV5YUV_write_cmos_sensor(0x2428,0x34);
    T8EV5YUV_write_cmos_sensor(0x2429,0x36);
    T8EV5YUV_write_cmos_sensor(0x242A,0x31);
    T8EV5YUV_write_cmos_sensor(0x242B,0x37);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x32);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x31);
    T8EV5YUV_write_cmos_sensor(0x240B,0x36);
    T8EV5YUV_write_cmos_sensor(0x240C,0x34);
    T8EV5YUV_write_cmos_sensor(0x240D,0x43);
    T8EV5YUV_write_cmos_sensor(0x240E,0x30);
    T8EV5YUV_write_cmos_sensor(0x240F,0x33);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x30);
    T8EV5YUV_write_cmos_sensor(0x2412,0x45);
    T8EV5YUV_write_cmos_sensor(0x2413,0x45);
    T8EV5YUV_write_cmos_sensor(0x2414,0x41);
    T8EV5YUV_write_cmos_sensor(0x2415,0x31);
    T8EV5YUV_write_cmos_sensor(0x2416,0x46);
    T8EV5YUV_write_cmos_sensor(0x2417,0x44);
    T8EV5YUV_write_cmos_sensor(0x2418,0x43);
    T8EV5YUV_write_cmos_sensor(0x2419,0x32);
    T8EV5YUV_write_cmos_sensor(0x241A,0x43);
    T8EV5YUV_write_cmos_sensor(0x241B,0x31);
    T8EV5YUV_write_cmos_sensor(0x241C,0x34);
    T8EV5YUV_write_cmos_sensor(0x241D,0x39);
    T8EV5YUV_write_cmos_sensor(0x241E,0x30);
    T8EV5YUV_write_cmos_sensor(0x241F,0x33);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x30);
    T8EV5YUV_write_cmos_sensor(0x2422,0x45);
    T8EV5YUV_write_cmos_sensor(0x2423,0x45);
    T8EV5YUV_write_cmos_sensor(0x2424,0x38);
    T8EV5YUV_write_cmos_sensor(0x2425,0x39);
    T8EV5YUV_write_cmos_sensor(0x2426,0x34);
    T8EV5YUV_write_cmos_sensor(0x2427,0x38);
    T8EV5YUV_write_cmos_sensor(0x2428,0x31);
    T8EV5YUV_write_cmos_sensor(0x2429,0x33);
    T8EV5YUV_write_cmos_sensor(0x242A,0x30);
    T8EV5YUV_write_cmos_sensor(0x242B,0x45);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x33);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x30);
    T8EV5YUV_write_cmos_sensor(0x240B,0x30);
    T8EV5YUV_write_cmos_sensor(0x240C,0x45);
    T8EV5YUV_write_cmos_sensor(0x240D,0x45);
    T8EV5YUV_write_cmos_sensor(0x240E,0x38);
    T8EV5YUV_write_cmos_sensor(0x240F,0x31);
    T8EV5YUV_write_cmos_sensor(0x2410,0x46);
    T8EV5YUV_write_cmos_sensor(0x2411,0x44);
    T8EV5YUV_write_cmos_sensor(0x2412,0x42);
    T8EV5YUV_write_cmos_sensor(0x2413,0x42);
    T8EV5YUV_write_cmos_sensor(0x2414,0x43);
    T8EV5YUV_write_cmos_sensor(0x2415,0x34);
    T8EV5YUV_write_cmos_sensor(0x2416,0x45);
    T8EV5YUV_write_cmos_sensor(0x2417,0x31);
    T8EV5YUV_write_cmos_sensor(0x2418,0x30);
    T8EV5YUV_write_cmos_sensor(0x2419,0x32);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x31);
    T8EV5YUV_write_cmos_sensor(0x241C,0x34);
    T8EV5YUV_write_cmos_sensor(0x241D,0x30);
    T8EV5YUV_write_cmos_sensor(0x241E,0x36);
    T8EV5YUV_write_cmos_sensor(0x241F,0x34);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x33);
    T8EV5YUV_write_cmos_sensor(0x2422,0x31);
    T8EV5YUV_write_cmos_sensor(0x2423,0x39);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x30);
    T8EV5YUV_write_cmos_sensor(0x2426,0x45);
    T8EV5YUV_write_cmos_sensor(0x2427,0x38);
    T8EV5YUV_write_cmos_sensor(0x2428,0x36);
    T8EV5YUV_write_cmos_sensor(0x2429,0x46);
    T8EV5YUV_write_cmos_sensor(0x242A,0x41);
    T8EV5YUV_write_cmos_sensor(0x242B,0x41);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x34);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x30);
    T8EV5YUV_write_cmos_sensor(0x240B,0x31);
    T8EV5YUV_write_cmos_sensor(0x240C,0x30);
    T8EV5YUV_write_cmos_sensor(0x240D,0x30);
    T8EV5YUV_write_cmos_sensor(0x240E,0x44);
    T8EV5YUV_write_cmos_sensor(0x240F,0x45);
    T8EV5YUV_write_cmos_sensor(0x2410,0x32);
    T8EV5YUV_write_cmos_sensor(0x2411,0x36);
    T8EV5YUV_write_cmos_sensor(0x2412,0x45);
    T8EV5YUV_write_cmos_sensor(0x2413,0x38);
    T8EV5YUV_write_cmos_sensor(0x2414,0x36);
    T8EV5YUV_write_cmos_sensor(0x2415,0x46);
    T8EV5YUV_write_cmos_sensor(0x2416,0x30);
    T8EV5YUV_write_cmos_sensor(0x2417,0x32);
    T8EV5YUV_write_cmos_sensor(0x2418,0x30);
    T8EV5YUV_write_cmos_sensor(0x2419,0x30);
    T8EV5YUV_write_cmos_sensor(0x241A,0x38);
    T8EV5YUV_write_cmos_sensor(0x241B,0x32);
    T8EV5YUV_write_cmos_sensor(0x241C,0x46);
    T8EV5YUV_write_cmos_sensor(0x241D,0x45);
    T8EV5YUV_write_cmos_sensor(0x241E,0x31);
    T8EV5YUV_write_cmos_sensor(0x241F,0x37);
    T8EV5YUV_write_cmos_sensor(0x2420,0x33);
    T8EV5YUV_write_cmos_sensor(0x2421,0x31);
    T8EV5YUV_write_cmos_sensor(0x2422,0x45);
    T8EV5YUV_write_cmos_sensor(0x2423,0x31);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x33);
    T8EV5YUV_write_cmos_sensor(0x2426,0x30);
    T8EV5YUV_write_cmos_sensor(0x2427,0x31);
    T8EV5YUV_write_cmos_sensor(0x2428,0x34);
    T8EV5YUV_write_cmos_sensor(0x2429,0x30);
    T8EV5YUV_write_cmos_sensor(0x242A,0x33);
    T8EV5YUV_write_cmos_sensor(0x242B,0x35);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x35);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x45);
    T8EV5YUV_write_cmos_sensor(0x240B,0x31);
    T8EV5YUV_write_cmos_sensor(0x240C,0x32);
    T8EV5YUV_write_cmos_sensor(0x240D,0x38);
    T8EV5YUV_write_cmos_sensor(0x240E,0x30);
    T8EV5YUV_write_cmos_sensor(0x240F,0x31);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x37);
    T8EV5YUV_write_cmos_sensor(0x2412,0x44);
    T8EV5YUV_write_cmos_sensor(0x2413,0x42);
    T8EV5YUV_write_cmos_sensor(0x2414,0x30);
    T8EV5YUV_write_cmos_sensor(0x2415,0x36);
    T8EV5YUV_write_cmos_sensor(0x2416,0x45);
    T8EV5YUV_write_cmos_sensor(0x2417,0x31);
    T8EV5YUV_write_cmos_sensor(0x2418,0x30);
    T8EV5YUV_write_cmos_sensor(0x2419,0x33);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x31);
    T8EV5YUV_write_cmos_sensor(0x241C,0x46);
    T8EV5YUV_write_cmos_sensor(0x241D,0x30);
    T8EV5YUV_write_cmos_sensor(0x241E,0x46);
    T8EV5YUV_write_cmos_sensor(0x241F,0x43);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x38);
    T8EV5YUV_write_cmos_sensor(0x2422,0x46);
    T8EV5YUV_write_cmos_sensor(0x2423,0x44);
    T8EV5YUV_write_cmos_sensor(0x2424,0x43);
    T8EV5YUV_write_cmos_sensor(0x2425,0x34);
    T8EV5YUV_write_cmos_sensor(0x2426,0x38);
    T8EV5YUV_write_cmos_sensor(0x2427,0x39);
    T8EV5YUV_write_cmos_sensor(0x2428,0x46);
    T8EV5YUV_write_cmos_sensor(0x2429,0x31);
    T8EV5YUV_write_cmos_sensor(0x242A,0x36);
    T8EV5YUV_write_cmos_sensor(0x242B,0x41);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x36);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x30);
    T8EV5YUV_write_cmos_sensor(0x240B,0x32);
    T8EV5YUV_write_cmos_sensor(0x240C,0x30);
    T8EV5YUV_write_cmos_sensor(0x240D,0x31);
    T8EV5YUV_write_cmos_sensor(0x240E,0x46);
    T8EV5YUV_write_cmos_sensor(0x240F,0x39);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x30);
    T8EV5YUV_write_cmos_sensor(0x2412,0x46);
    T8EV5YUV_write_cmos_sensor(0x2413,0x44);
    T8EV5YUV_write_cmos_sensor(0x2414,0x39);
    T8EV5YUV_write_cmos_sensor(0x2415,0x34);
    T8EV5YUV_write_cmos_sensor(0x2416,0x39);
    T8EV5YUV_write_cmos_sensor(0x2417,0x46);
    T8EV5YUV_write_cmos_sensor(0x2418,0x46);
    T8EV5YUV_write_cmos_sensor(0x2419,0x45);
    T8EV5YUV_write_cmos_sensor(0x241A,0x32);
    T8EV5YUV_write_cmos_sensor(0x241B,0x41);
    T8EV5YUV_write_cmos_sensor(0x241C,0x33);
    T8EV5YUV_write_cmos_sensor(0x241D,0x31);
    T8EV5YUV_write_cmos_sensor(0x241E,0x34);
    T8EV5YUV_write_cmos_sensor(0x241F,0x38);
    T8EV5YUV_write_cmos_sensor(0x2420,0x35);
    T8EV5YUV_write_cmos_sensor(0x2421,0x33);
    T8EV5YUV_write_cmos_sensor(0x2422,0x30);
    T8EV5YUV_write_cmos_sensor(0x2423,0x32);
    T8EV5YUV_write_cmos_sensor(0x2424,0x34);
    T8EV5YUV_write_cmos_sensor(0x2425,0x39);
    T8EV5YUV_write_cmos_sensor(0x2426,0x31);
    T8EV5YUV_write_cmos_sensor(0x2427,0x33);
    T8EV5YUV_write_cmos_sensor(0x2428,0x30);
    T8EV5YUV_write_cmos_sensor(0x2429,0x30);
    T8EV5YUV_write_cmos_sensor(0x242A,0x45);
    T8EV5YUV_write_cmos_sensor(0x242B,0x32);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x37);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x45);
    T8EV5YUV_write_cmos_sensor(0x240B,0x45);
    T8EV5YUV_write_cmos_sensor(0x240C,0x38);
    T8EV5YUV_write_cmos_sensor(0x240D,0x39);
    T8EV5YUV_write_cmos_sensor(0x240E,0x34);
    T8EV5YUV_write_cmos_sensor(0x240F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x30);
    T8EV5YUV_write_cmos_sensor(0x2412,0x30);
    T8EV5YUV_write_cmos_sensor(0x2413,0x30);
    T8EV5YUV_write_cmos_sensor(0x2414,0x46);
    T8EV5YUV_write_cmos_sensor(0x2415,0x44);
    T8EV5YUV_write_cmos_sensor(0x2416,0x32);
    T8EV5YUV_write_cmos_sensor(0x2417,0x38);
    T8EV5YUV_write_cmos_sensor(0x2418,0x43);
    T8EV5YUV_write_cmos_sensor(0x2419,0x31);
    T8EV5YUV_write_cmos_sensor(0x241A,0x45);
    T8EV5YUV_write_cmos_sensor(0x241B,0x38);
    T8EV5YUV_write_cmos_sensor(0x241C,0x38);
    T8EV5YUV_write_cmos_sensor(0x241D,0x36);
    T8EV5YUV_write_cmos_sensor(0x241E,0x44);
    T8EV5YUV_write_cmos_sensor(0x241F,0x46);
    T8EV5YUV_write_cmos_sensor(0x2420,0x32);
    T8EV5YUV_write_cmos_sensor(0x2421,0x35);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x38);
    T8EV5YUV_write_cmos_sensor(0x2424,0x35);
    T8EV5YUV_write_cmos_sensor(0x2425,0x33);
    T8EV5YUV_write_cmos_sensor(0x2426,0x30);
    T8EV5YUV_write_cmos_sensor(0x2427,0x32);
    T8EV5YUV_write_cmos_sensor(0x2428,0x34);
    T8EV5YUV_write_cmos_sensor(0x2429,0x39);
    T8EV5YUV_write_cmos_sensor(0x242A,0x35);
    T8EV5YUV_write_cmos_sensor(0x242B,0x31);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x38);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x31);
    T8EV5YUV_write_cmos_sensor(0x240B,0x33);
    T8EV5YUV_write_cmos_sensor(0x240C,0x30);
    T8EV5YUV_write_cmos_sensor(0x240D,0x30);
    T8EV5YUV_write_cmos_sensor(0x240E,0x45);
    T8EV5YUV_write_cmos_sensor(0x240F,0x45);
    T8EV5YUV_write_cmos_sensor(0x2410,0x38);
    T8EV5YUV_write_cmos_sensor(0x2411,0x39);
    T8EV5YUV_write_cmos_sensor(0x2412,0x34);
    T8EV5YUV_write_cmos_sensor(0x2413,0x43);
    T8EV5YUV_write_cmos_sensor(0x2414,0x30);
    T8EV5YUV_write_cmos_sensor(0x2415,0x42);
    T8EV5YUV_write_cmos_sensor(0x2416,0x30);
    T8EV5YUV_write_cmos_sensor(0x2417,0x30);
    T8EV5YUV_write_cmos_sensor(0x2418,0x45);
    T8EV5YUV_write_cmos_sensor(0x2419,0x45);
    T8EV5YUV_write_cmos_sensor(0x241A,0x41);
    T8EV5YUV_write_cmos_sensor(0x241B,0x31);
    T8EV5YUV_write_cmos_sensor(0x241C,0x45);
    T8EV5YUV_write_cmos_sensor(0x241D,0x39);
    T8EV5YUV_write_cmos_sensor(0x241E,0x34);
    T8EV5YUV_write_cmos_sensor(0x241F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2420,0x45);
    T8EV5YUV_write_cmos_sensor(0x2421,0x38);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x39);
    T8EV5YUV_write_cmos_sensor(0x2424,0x45);
    T8EV5YUV_write_cmos_sensor(0x2425,0x43);
    T8EV5YUV_write_cmos_sensor(0x2426,0x34);
    T8EV5YUV_write_cmos_sensor(0x2427,0x38);
    T8EV5YUV_write_cmos_sensor(0x2428,0x46);
    T8EV5YUV_write_cmos_sensor(0x2429,0x44);
    T8EV5YUV_write_cmos_sensor(0x242A,0x33);
    T8EV5YUV_write_cmos_sensor(0x242B,0x42);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x39);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x39);
    T8EV5YUV_write_cmos_sensor(0x240B,0x37);
    T8EV5YUV_write_cmos_sensor(0x240C,0x43);
    T8EV5YUV_write_cmos_sensor(0x240D,0x37);
    T8EV5YUV_write_cmos_sensor(0x240E,0x34);
    T8EV5YUV_write_cmos_sensor(0x240F,0x39);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x42);
    T8EV5YUV_write_cmos_sensor(0x2412,0x30);
    T8EV5YUV_write_cmos_sensor(0x2413,0x30);
    T8EV5YUV_write_cmos_sensor(0x2414,0x45);
    T8EV5YUV_write_cmos_sensor(0x2415,0x45);
    T8EV5YUV_write_cmos_sensor(0x2416,0x38);
    T8EV5YUV_write_cmos_sensor(0x2417,0x39);
    T8EV5YUV_write_cmos_sensor(0x2418,0x34);
    T8EV5YUV_write_cmos_sensor(0x2419,0x38);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x37);
    T8EV5YUV_write_cmos_sensor(0x241C,0x30);
    T8EV5YUV_write_cmos_sensor(0x241D,0x30);
    T8EV5YUV_write_cmos_sensor(0x241E,0x45);
    T8EV5YUV_write_cmos_sensor(0x241F,0x45);
    T8EV5YUV_write_cmos_sensor(0x2420,0x38);
    T8EV5YUV_write_cmos_sensor(0x2421,0x31);
    T8EV5YUV_write_cmos_sensor(0x2422,0x46);
    T8EV5YUV_write_cmos_sensor(0x2423,0x44);
    T8EV5YUV_write_cmos_sensor(0x2424,0x39);
    T8EV5YUV_write_cmos_sensor(0x2425,0x34);
    T8EV5YUV_write_cmos_sensor(0x2426,0x43);
    T8EV5YUV_write_cmos_sensor(0x2427,0x34);
    T8EV5YUV_write_cmos_sensor(0x2428,0x46);
    T8EV5YUV_write_cmos_sensor(0x2429,0x43);
    T8EV5YUV_write_cmos_sensor(0x242A,0x46);
    T8EV5YUV_write_cmos_sensor(0x242B,0x38);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x41);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x31);
    T8EV5YUV_write_cmos_sensor(0x240B,0x36);
    T8EV5YUV_write_cmos_sensor(0x240C,0x34);
    T8EV5YUV_write_cmos_sensor(0x240D,0x38);
    T8EV5YUV_write_cmos_sensor(0x240E,0x31);
    T8EV5YUV_write_cmos_sensor(0x240F,0x33);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x30);
    T8EV5YUV_write_cmos_sensor(0x2412,0x45);
    T8EV5YUV_write_cmos_sensor(0x2413,0x45);
    T8EV5YUV_write_cmos_sensor(0x2414,0x38);
    T8EV5YUV_write_cmos_sensor(0x2415,0x31);
    T8EV5YUV_write_cmos_sensor(0x2416,0x34);
    T8EV5YUV_write_cmos_sensor(0x2417,0x39);
    T8EV5YUV_write_cmos_sensor(0x2418,0x35);
    T8EV5YUV_write_cmos_sensor(0x2419,0x33);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x32);
    T8EV5YUV_write_cmos_sensor(0x241C,0x34);
    T8EV5YUV_write_cmos_sensor(0x241D,0x43);
    T8EV5YUV_write_cmos_sensor(0x241E,0x30);
    T8EV5YUV_write_cmos_sensor(0x241F,0x37);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x30);
    T8EV5YUV_write_cmos_sensor(0x2422,0x45);
    T8EV5YUV_write_cmos_sensor(0x2423,0x45);
    T8EV5YUV_write_cmos_sensor(0x2424,0x41);
    T8EV5YUV_write_cmos_sensor(0x2425,0x31);
    T8EV5YUV_write_cmos_sensor(0x2426,0x45);
    T8EV5YUV_write_cmos_sensor(0x2427,0x39);
    T8EV5YUV_write_cmos_sensor(0x2428,0x34);
    T8EV5YUV_write_cmos_sensor(0x2429,0x41);
    T8EV5YUV_write_cmos_sensor(0x242A,0x38);
    T8EV5YUV_write_cmos_sensor(0x242B,0x44);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x42);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x45);
    T8EV5YUV_write_cmos_sensor(0x240B,0x38);
    T8EV5YUV_write_cmos_sensor(0x240C,0x34);
    T8EV5YUV_write_cmos_sensor(0x240D,0x39);
    T8EV5YUV_write_cmos_sensor(0x240E,0x45);
    T8EV5YUV_write_cmos_sensor(0x240F,0x43);
    T8EV5YUV_write_cmos_sensor(0x2410,0x34);
    T8EV5YUV_write_cmos_sensor(0x2411,0x38);
    T8EV5YUV_write_cmos_sensor(0x2412,0x46);
    T8EV5YUV_write_cmos_sensor(0x2413,0x44);
    T8EV5YUV_write_cmos_sensor(0x2414,0x39);
    T8EV5YUV_write_cmos_sensor(0x2415,0x37);
    T8EV5YUV_write_cmos_sensor(0x2416,0x43);
    T8EV5YUV_write_cmos_sensor(0x2417,0x37);
    T8EV5YUV_write_cmos_sensor(0x2418,0x34);
    T8EV5YUV_write_cmos_sensor(0x2419,0x39);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x37);
    T8EV5YUV_write_cmos_sensor(0x241C,0x30);
    T8EV5YUV_write_cmos_sensor(0x241D,0x30);
    T8EV5YUV_write_cmos_sensor(0x241E,0x45);
    T8EV5YUV_write_cmos_sensor(0x241F,0x45);
    T8EV5YUV_write_cmos_sensor(0x2420,0x38);
    T8EV5YUV_write_cmos_sensor(0x2421,0x39);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x38);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x46);
    T8EV5YUV_write_cmos_sensor(0x2426,0x30);
    T8EV5YUV_write_cmos_sensor(0x2427,0x30);
    T8EV5YUV_write_cmos_sensor(0x2428,0x45);
    T8EV5YUV_write_cmos_sensor(0x2429,0x45);
    T8EV5YUV_write_cmos_sensor(0x242A,0x34);
    T8EV5YUV_write_cmos_sensor(0x242B,0x34);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x43);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x38);
    T8EV5YUV_write_cmos_sensor(0x240B,0x31);
    T8EV5YUV_write_cmos_sensor(0x240C,0x46);
    T8EV5YUV_write_cmos_sensor(0x240D,0x44);
    T8EV5YUV_write_cmos_sensor(0x240E,0x39);
    T8EV5YUV_write_cmos_sensor(0x240F,0x34);
    T8EV5YUV_write_cmos_sensor(0x2410,0x43);
    T8EV5YUV_write_cmos_sensor(0x2411,0x34);
    T8EV5YUV_write_cmos_sensor(0x2412,0x34);
    T8EV5YUV_write_cmos_sensor(0x2413,0x39);
    T8EV5YUV_write_cmos_sensor(0x2414,0x34);
    T8EV5YUV_write_cmos_sensor(0x2415,0x30);
    T8EV5YUV_write_cmos_sensor(0x2416,0x30);
    T8EV5YUV_write_cmos_sensor(0x2417,0x31);
    T8EV5YUV_write_cmos_sensor(0x2418,0x34);
    T8EV5YUV_write_cmos_sensor(0x2419,0x38);
    T8EV5YUV_write_cmos_sensor(0x241A,0x35);
    T8EV5YUV_write_cmos_sensor(0x241B,0x33);
    T8EV5YUV_write_cmos_sensor(0x241C,0x30);
    T8EV5YUV_write_cmos_sensor(0x241D,0x32);
    T8EV5YUV_write_cmos_sensor(0x241E,0x34);
    T8EV5YUV_write_cmos_sensor(0x241F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x30);
    T8EV5YUV_write_cmos_sensor(0x2422,0x30);
    T8EV5YUV_write_cmos_sensor(0x2423,0x30);
    T8EV5YUV_write_cmos_sensor(0x2424,0x46);
    T8EV5YUV_write_cmos_sensor(0x2425,0x44);
    T8EV5YUV_write_cmos_sensor(0x2426,0x32);
    T8EV5YUV_write_cmos_sensor(0x2427,0x38);
    T8EV5YUV_write_cmos_sensor(0x2428,0x43);
    T8EV5YUV_write_cmos_sensor(0x2429,0x31);
    T8EV5YUV_write_cmos_sensor(0x242A,0x44);
    T8EV5YUV_write_cmos_sensor(0x242B,0x33);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x44);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x45);
    T8EV5YUV_write_cmos_sensor(0x240B,0x38);
    T8EV5YUV_write_cmos_sensor(0x240C,0x38);
    T8EV5YUV_write_cmos_sensor(0x240D,0x36);
    T8EV5YUV_write_cmos_sensor(0x240E,0x44);
    T8EV5YUV_write_cmos_sensor(0x240F,0x46);
    T8EV5YUV_write_cmos_sensor(0x2410,0x31);
    T8EV5YUV_write_cmos_sensor(0x2411,0x32);
    T8EV5YUV_write_cmos_sensor(0x2412,0x34);
    T8EV5YUV_write_cmos_sensor(0x2413,0x39);
    T8EV5YUV_write_cmos_sensor(0x2414,0x34);
    T8EV5YUV_write_cmos_sensor(0x2415,0x34);
    T8EV5YUV_write_cmos_sensor(0x2416,0x30);
    T8EV5YUV_write_cmos_sensor(0x2417,0x31);
    T8EV5YUV_write_cmos_sensor(0x2418,0x34);
    T8EV5YUV_write_cmos_sensor(0x2419,0x38);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x46);
    T8EV5YUV_write_cmos_sensor(0x241C,0x30);
    T8EV5YUV_write_cmos_sensor(0x241D,0x30);
    T8EV5YUV_write_cmos_sensor(0x241E,0x45);
    T8EV5YUV_write_cmos_sensor(0x241F,0x45);
    T8EV5YUV_write_cmos_sensor(0x2420,0x38);
    T8EV5YUV_write_cmos_sensor(0x2421,0x31);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x41);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x33);
    T8EV5YUV_write_cmos_sensor(0x2426,0x30);
    T8EV5YUV_write_cmos_sensor(0x2427,0x30);
    T8EV5YUV_write_cmos_sensor(0x2428,0x46);
    T8EV5YUV_write_cmos_sensor(0x2429,0x44);
    T8EV5YUV_write_cmos_sensor(0x242A,0x46);
    T8EV5YUV_write_cmos_sensor(0x242B,0x33);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x45);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x32);
    T8EV5YUV_write_cmos_sensor(0x240B,0x38);
    T8EV5YUV_write_cmos_sensor(0x240C,0x43);
    T8EV5YUV_write_cmos_sensor(0x240D,0x31);
    T8EV5YUV_write_cmos_sensor(0x240E,0x45);
    T8EV5YUV_write_cmos_sensor(0x240F,0x38);
    T8EV5YUV_write_cmos_sensor(0x2410,0x38);
    T8EV5YUV_write_cmos_sensor(0x2411,0x36);
    T8EV5YUV_write_cmos_sensor(0x2412,0x44);
    T8EV5YUV_write_cmos_sensor(0x2413,0x46);
    T8EV5YUV_write_cmos_sensor(0x2414,0x32);
    T8EV5YUV_write_cmos_sensor(0x2415,0x31);
    T8EV5YUV_write_cmos_sensor(0x2416,0x34);
    T8EV5YUV_write_cmos_sensor(0x2417,0x39);
    T8EV5YUV_write_cmos_sensor(0x2418,0x34);
    T8EV5YUV_write_cmos_sensor(0x2419,0x30);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x31);
    T8EV5YUV_write_cmos_sensor(0x241C,0x34);
    T8EV5YUV_write_cmos_sensor(0x241D,0x38);
    T8EV5YUV_write_cmos_sensor(0x241E,0x35);
    T8EV5YUV_write_cmos_sensor(0x241F,0x33);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x32);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x41);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x33);
    T8EV5YUV_write_cmos_sensor(0x2426,0x30);
    T8EV5YUV_write_cmos_sensor(0x2427,0x30);
    T8EV5YUV_write_cmos_sensor(0x2428,0x46);
    T8EV5YUV_write_cmos_sensor(0x2429,0x44);
    T8EV5YUV_write_cmos_sensor(0x242A,0x31);
    T8EV5YUV_write_cmos_sensor(0x242B,0x38);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x46);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x32);
    T8EV5YUV_write_cmos_sensor(0x240B,0x38);
    T8EV5YUV_write_cmos_sensor(0x240C,0x43);
    T8EV5YUV_write_cmos_sensor(0x240D,0x31);
    T8EV5YUV_write_cmos_sensor(0x240E,0x45);
    T8EV5YUV_write_cmos_sensor(0x240F,0x38);
    T8EV5YUV_write_cmos_sensor(0x2410,0x38);
    T8EV5YUV_write_cmos_sensor(0x2411,0x36);
    T8EV5YUV_write_cmos_sensor(0x2412,0x44);
    T8EV5YUV_write_cmos_sensor(0x2413,0x46);
    T8EV5YUV_write_cmos_sensor(0x2414,0x31);
    T8EV5YUV_write_cmos_sensor(0x2415,0x43);
    T8EV5YUV_write_cmos_sensor(0x2416,0x34);
    T8EV5YUV_write_cmos_sensor(0x2417,0x39);
    T8EV5YUV_write_cmos_sensor(0x2418,0x34);
    T8EV5YUV_write_cmos_sensor(0x2419,0x38);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x31);
    T8EV5YUV_write_cmos_sensor(0x241C,0x34);
    T8EV5YUV_write_cmos_sensor(0x241D,0x38);
    T8EV5YUV_write_cmos_sensor(0x241E,0x30);
    T8EV5YUV_write_cmos_sensor(0x241F,0x46);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x30);
    T8EV5YUV_write_cmos_sensor(0x2422,0x45);
    T8EV5YUV_write_cmos_sensor(0x2423,0x45);
    T8EV5YUV_write_cmos_sensor(0x2424,0x38);
    T8EV5YUV_write_cmos_sensor(0x2425,0x31);
    T8EV5YUV_write_cmos_sensor(0x2426,0x34);
    T8EV5YUV_write_cmos_sensor(0x2427,0x41);
    T8EV5YUV_write_cmos_sensor(0x2428,0x30);
    T8EV5YUV_write_cmos_sensor(0x2429,0x30);
    T8EV5YUV_write_cmos_sensor(0x242A,0x44);
    T8EV5YUV_write_cmos_sensor(0x242B,0x43);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x31);
    T8EV5YUV_write_cmos_sensor(0x2406,0x30);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x30);
    T8EV5YUV_write_cmos_sensor(0x240B,0x30);
    T8EV5YUV_write_cmos_sensor(0x240C,0x46);
    T8EV5YUV_write_cmos_sensor(0x240D,0x44);
    T8EV5YUV_write_cmos_sensor(0x240E,0x32);
    T8EV5YUV_write_cmos_sensor(0x240F,0x38);
    T8EV5YUV_write_cmos_sensor(0x2410,0x43);
    T8EV5YUV_write_cmos_sensor(0x2411,0x31);
    T8EV5YUV_write_cmos_sensor(0x2412,0x45);
    T8EV5YUV_write_cmos_sensor(0x2413,0x38);
    T8EV5YUV_write_cmos_sensor(0x2414,0x38);
    T8EV5YUV_write_cmos_sensor(0x2415,0x36);
    T8EV5YUV_write_cmos_sensor(0x2416,0x41);
    T8EV5YUV_write_cmos_sensor(0x2417,0x41);
    T8EV5YUV_write_cmos_sensor(0x2418,0x34);
    T8EV5YUV_write_cmos_sensor(0x2419,0x44);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x30);
    T8EV5YUV_write_cmos_sensor(0x241C,0x30);
    T8EV5YUV_write_cmos_sensor(0x241D,0x31);
    T8EV5YUV_write_cmos_sensor(0x241E,0x35);
    T8EV5YUV_write_cmos_sensor(0x241F,0x35);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x33);
    T8EV5YUV_write_cmos_sensor(0x2422,0x46);
    T8EV5YUV_write_cmos_sensor(0x2423,0x39);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x30);
    T8EV5YUV_write_cmos_sensor(0x2426,0x35);
    T8EV5YUV_write_cmos_sensor(0x2427,0x35);
    T8EV5YUV_write_cmos_sensor(0x2428,0x30);
    T8EV5YUV_write_cmos_sensor(0x2429,0x32);
    T8EV5YUV_write_cmos_sensor(0x242A,0x43);
    T8EV5YUV_write_cmos_sensor(0x242B,0x42);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x31);
    T8EV5YUV_write_cmos_sensor(0x2406,0x31);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x46);
    T8EV5YUV_write_cmos_sensor(0x240B,0x39);
    T8EV5YUV_write_cmos_sensor(0x240C,0x30);
    T8EV5YUV_write_cmos_sensor(0x240D,0x32);
    T8EV5YUV_write_cmos_sensor(0x240E,0x46);
    T8EV5YUV_write_cmos_sensor(0x240F,0x44);
    T8EV5YUV_write_cmos_sensor(0x2410,0x39);
    T8EV5YUV_write_cmos_sensor(0x2411,0x34);
    T8EV5YUV_write_cmos_sensor(0x2412,0x39);
    T8EV5YUV_write_cmos_sensor(0x2413,0x46);
    T8EV5YUV_write_cmos_sensor(0x2414,0x46);
    T8EV5YUV_write_cmos_sensor(0x2415,0x43);
    T8EV5YUV_write_cmos_sensor(0x2416,0x31);
    T8EV5YUV_write_cmos_sensor(0x2417,0x33);
    T8EV5YUV_write_cmos_sensor(0x2418,0x34);
    T8EV5YUV_write_cmos_sensor(0x2419,0x39);
    T8EV5YUV_write_cmos_sensor(0x241A,0x31);
    T8EV5YUV_write_cmos_sensor(0x241B,0x33);
    T8EV5YUV_write_cmos_sensor(0x241C,0x30);
    T8EV5YUV_write_cmos_sensor(0x241D,0x30);
    T8EV5YUV_write_cmos_sensor(0x241E,0x45);
    T8EV5YUV_write_cmos_sensor(0x241F,0x45);
    T8EV5YUV_write_cmos_sensor(0x2420,0x38);
    T8EV5YUV_write_cmos_sensor(0x2421,0x39);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x38);
    T8EV5YUV_write_cmos_sensor(0x2424,0x35);
    T8EV5YUV_write_cmos_sensor(0x2425,0x33);
    T8EV5YUV_write_cmos_sensor(0x2426,0x30);
    T8EV5YUV_write_cmos_sensor(0x2427,0x32);
    T8EV5YUV_write_cmos_sensor(0x2428,0x46);
    T8EV5YUV_write_cmos_sensor(0x2429,0x44);
    T8EV5YUV_write_cmos_sensor(0x242A,0x30);
    T8EV5YUV_write_cmos_sensor(0x242B,0x38);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x30);
    T8EV5YUV_write_cmos_sensor(0x2403,0x45);
    T8EV5YUV_write_cmos_sensor(0x2404,0x33);
    T8EV5YUV_write_cmos_sensor(0x2405,0x31);
    T8EV5YUV_write_cmos_sensor(0x2406,0x32);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x39);
    T8EV5YUV_write_cmos_sensor(0x240B,0x34);
    T8EV5YUV_write_cmos_sensor(0x240C,0x43);
    T8EV5YUV_write_cmos_sensor(0x240D,0x34);
    T8EV5YUV_write_cmos_sensor(0x240E,0x46);
    T8EV5YUV_write_cmos_sensor(0x240F,0x31);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x32);
    T8EV5YUV_write_cmos_sensor(0x2412,0x30);
    T8EV5YUV_write_cmos_sensor(0x2413,0x31);
    T8EV5YUV_write_cmos_sensor(0x2414,0x46);
    T8EV5YUV_write_cmos_sensor(0x2415,0x39);
    T8EV5YUV_write_cmos_sensor(0x2416,0x30);
    T8EV5YUV_write_cmos_sensor(0x2417,0x31);
    T8EV5YUV_write_cmos_sensor(0x2418,0x46);
    T8EV5YUV_write_cmos_sensor(0x2419,0x44);
    T8EV5YUV_write_cmos_sensor(0x241A,0x39);
    T8EV5YUV_write_cmos_sensor(0x241B,0x34);
    T8EV5YUV_write_cmos_sensor(0x241C,0x39);
    T8EV5YUV_write_cmos_sensor(0x241D,0x46);
    T8EV5YUV_write_cmos_sensor(0x241E,0x44);
    T8EV5YUV_write_cmos_sensor(0x241F,0x33);
    T8EV5YUV_write_cmos_sensor(0x2420,0x33);
    T8EV5YUV_write_cmos_sensor(0x2421,0x37);
    T8EV5YUV_write_cmos_sensor(0x2422,0x31);
    T8EV5YUV_write_cmos_sensor(0x2423,0x34);
    T8EV5YUV_write_cmos_sensor(0x2424,0x46);
    T8EV5YUV_write_cmos_sensor(0x2425,0x41);
    T8EV5YUV_write_cmos_sensor(0x2426,0x31);
    T8EV5YUV_write_cmos_sensor(0x2427,0x33);
    T8EV5YUV_write_cmos_sensor(0x2428,0x0D);
    T8EV5YUV_write_cmos_sensor(0x2429,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x30);
    T8EV5YUV_write_cmos_sensor(0x2403,0x35);
    T8EV5YUV_write_cmos_sensor(0x2404,0x38);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x30);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x46);
    T8EV5YUV_write_cmos_sensor(0x240B,0x45);
    T8EV5YUV_write_cmos_sensor(0x240C,0x30);
    T8EV5YUV_write_cmos_sensor(0x240D,0x30);
    T8EV5YUV_write_cmos_sensor(0x240E,0x32);
    T8EV5YUV_write_cmos_sensor(0x240F,0x30);
    T8EV5YUV_write_cmos_sensor(0x2410,0x46);
    T8EV5YUV_write_cmos_sensor(0x2411,0x43);
    T8EV5YUV_write_cmos_sensor(0x2412,0x46);
    T8EV5YUV_write_cmos_sensor(0x2413,0x45);
    T8EV5YUV_write_cmos_sensor(0x2414,0x36);
    T8EV5YUV_write_cmos_sensor(0x2415,0x33);
    T8EV5YUV_write_cmos_sensor(0x2416,0x0D);
    T8EV5YUV_write_cmos_sensor(0x2417,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x30);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x30);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x30);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x31);
    T8EV5YUV_write_cmos_sensor(0x240A,0x46);
    T8EV5YUV_write_cmos_sensor(0x240B,0x46);
    T8EV5YUV_write_cmos_sensor(0x240C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x240D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2400,0x03);
    T8EV5YUV_write_cmos_sensor(0x2400,0x02);
    #else
    T8EV5YUV_write_cmos_sensor(0x2400,0x04);
    T8EV5YUV_write_cmos_sensor(0x2400,0x00);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x30);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x43);
    T8EV5YUV_write_cmos_sensor(0x240B,0x38);
    T8EV5YUV_write_cmos_sensor(0x240C,0x33);
    T8EV5YUV_write_cmos_sensor(0x240D,0x41);
    T8EV5YUV_write_cmos_sensor(0x240E,0x34);
    T8EV5YUV_write_cmos_sensor(0x240F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2410,0x30);
    T8EV5YUV_write_cmos_sensor(0x2411,0x30);
    T8EV5YUV_write_cmos_sensor(0x2412,0x38);
    T8EV5YUV_write_cmos_sensor(0x2413,0x30);
    T8EV5YUV_write_cmos_sensor(0x2414,0x45);
    T8EV5YUV_write_cmos_sensor(0x2415,0x41);
    T8EV5YUV_write_cmos_sensor(0x2416,0x36);
    T8EV5YUV_write_cmos_sensor(0x2417,0x46);
    T8EV5YUV_write_cmos_sensor(0x2418,0x30);
    T8EV5YUV_write_cmos_sensor(0x2419,0x30);
    T8EV5YUV_write_cmos_sensor(0x241A,0x43);
    T8EV5YUV_write_cmos_sensor(0x241B,0x30);
    T8EV5YUV_write_cmos_sensor(0x241C,0x44);
    T8EV5YUV_write_cmos_sensor(0x241D,0x42);
    T8EV5YUV_write_cmos_sensor(0x241E,0x31);
    T8EV5YUV_write_cmos_sensor(0x241F,0x31);
    T8EV5YUV_write_cmos_sensor(0x2420,0x30);
    T8EV5YUV_write_cmos_sensor(0x2421,0x41);
    T8EV5YUV_write_cmos_sensor(0x2422,0x30);
    T8EV5YUV_write_cmos_sensor(0x2423,0x46);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x30);
    T8EV5YUV_write_cmos_sensor(0x2426,0x45);
    T8EV5YUV_write_cmos_sensor(0x2427,0x32);
    T8EV5YUV_write_cmos_sensor(0x2428,0x34);
    T8EV5YUV_write_cmos_sensor(0x2429,0x30);
    T8EV5YUV_write_cmos_sensor(0x242A,0x43);
    T8EV5YUV_write_cmos_sensor(0x242B,0x34);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x31);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x30);
    T8EV5YUV_write_cmos_sensor(0x240B,0x41);
    T8EV5YUV_write_cmos_sensor(0x240C,0x30);
    T8EV5YUV_write_cmos_sensor(0x240D,0x46);
    T8EV5YUV_write_cmos_sensor(0x240E,0x38);
    T8EV5YUV_write_cmos_sensor(0x240F,0x30);
    T8EV5YUV_write_cmos_sensor(0x2410,0x46);
    T8EV5YUV_write_cmos_sensor(0x2411,0x32);
    T8EV5YUV_write_cmos_sensor(0x2412,0x37);
    T8EV5YUV_write_cmos_sensor(0x2413,0x38);
    T8EV5YUV_write_cmos_sensor(0x2414,0x33);
    T8EV5YUV_write_cmos_sensor(0x2415,0x32);
    T8EV5YUV_write_cmos_sensor(0x2416,0x45);
    T8EV5YUV_write_cmos_sensor(0x2417,0x41);
    T8EV5YUV_write_cmos_sensor(0x2418,0x36);
    T8EV5YUV_write_cmos_sensor(0x2419,0x46);
    T8EV5YUV_write_cmos_sensor(0x241A,0x30);
    T8EV5YUV_write_cmos_sensor(0x241B,0x30);
    T8EV5YUV_write_cmos_sensor(0x241C,0x43);
    T8EV5YUV_write_cmos_sensor(0x241D,0x30);
    T8EV5YUV_write_cmos_sensor(0x241E,0x44);
    T8EV5YUV_write_cmos_sensor(0x241F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2420,0x45);
    T8EV5YUV_write_cmos_sensor(0x2421,0x46);
    T8EV5YUV_write_cmos_sensor(0x2422,0x34);
    T8EV5YUV_write_cmos_sensor(0x2423,0x41);
    T8EV5YUV_write_cmos_sensor(0x2424,0x30);
    T8EV5YUV_write_cmos_sensor(0x2425,0x35);
    T8EV5YUV_write_cmos_sensor(0x2426,0x38);
    T8EV5YUV_write_cmos_sensor(0x2427,0x41);
    T8EV5YUV_write_cmos_sensor(0x2428,0x45);
    T8EV5YUV_write_cmos_sensor(0x2429,0x38);
    T8EV5YUV_write_cmos_sensor(0x242A,0x45);
    T8EV5YUV_write_cmos_sensor(0x242B,0x38);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x31);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x32);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x30);
    T8EV5YUV_write_cmos_sensor(0x240B,0x35);
    T8EV5YUV_write_cmos_sensor(0x240C,0x46);
    T8EV5YUV_write_cmos_sensor(0x240D,0x32);
    T8EV5YUV_write_cmos_sensor(0x240E,0x37);
    T8EV5YUV_write_cmos_sensor(0x240F,0x38);
    T8EV5YUV_write_cmos_sensor(0x2410,0x34);
    T8EV5YUV_write_cmos_sensor(0x2411,0x41);
    T8EV5YUV_write_cmos_sensor(0x2412,0x30);
    T8EV5YUV_write_cmos_sensor(0x2413,0x36);
    T8EV5YUV_write_cmos_sensor(0x2414,0x38);
    T8EV5YUV_write_cmos_sensor(0x2415,0x41);
    T8EV5YUV_write_cmos_sensor(0x2416,0x46);
    T8EV5YUV_write_cmos_sensor(0x2417,0x32);
    T8EV5YUV_write_cmos_sensor(0x2418,0x37);
    T8EV5YUV_write_cmos_sensor(0x2419,0x38);
    T8EV5YUV_write_cmos_sensor(0x241A,0x34);
    T8EV5YUV_write_cmos_sensor(0x241B,0x41);
    T8EV5YUV_write_cmos_sensor(0x241C,0x31);
    T8EV5YUV_write_cmos_sensor(0x241D,0x33);
    T8EV5YUV_write_cmos_sensor(0x241E,0x38);
    T8EV5YUV_write_cmos_sensor(0x241F,0x41);
    T8EV5YUV_write_cmos_sensor(0x2420,0x46);
    T8EV5YUV_write_cmos_sensor(0x2421,0x32);
    T8EV5YUV_write_cmos_sensor(0x2422,0x37);
    T8EV5YUV_write_cmos_sensor(0x2423,0x38);
    T8EV5YUV_write_cmos_sensor(0x2424,0x34);
    T8EV5YUV_write_cmos_sensor(0x2425,0x41);
    T8EV5YUV_write_cmos_sensor(0x2426,0x39);
    T8EV5YUV_write_cmos_sensor(0x2427,0x43);
    T8EV5YUV_write_cmos_sensor(0x2428,0x36);
    T8EV5YUV_write_cmos_sensor(0x2429,0x30);
    T8EV5YUV_write_cmos_sensor(0x242A,0x36);
    T8EV5YUV_write_cmos_sensor(0x242B,0x36);
    T8EV5YUV_write_cmos_sensor(0x242C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x242D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x30);
    T8EV5YUV_write_cmos_sensor(0x2403,0x39);
    T8EV5YUV_write_cmos_sensor(0x2404,0x32);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x33);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x46);
    T8EV5YUV_write_cmos_sensor(0x240B,0x32);
    T8EV5YUV_write_cmos_sensor(0x240C,0x46);
    T8EV5YUV_write_cmos_sensor(0x240D,0x39);
    T8EV5YUV_write_cmos_sensor(0x240E,0x30);
    T8EV5YUV_write_cmos_sensor(0x240F,0x31);
    T8EV5YUV_write_cmos_sensor(0x2410,0x46);
    T8EV5YUV_write_cmos_sensor(0x2411,0x45);
    T8EV5YUV_write_cmos_sensor(0x2412,0x30);
    T8EV5YUV_write_cmos_sensor(0x2413,0x30);
    T8EV5YUV_write_cmos_sensor(0x2414,0x38);
    T8EV5YUV_write_cmos_sensor(0x2415,0x30);
    T8EV5YUV_write_cmos_sensor(0x2416,0x43);
    T8EV5YUV_write_cmos_sensor(0x2417,0x30);
    T8EV5YUV_write_cmos_sensor(0x2418,0x33);
    T8EV5YUV_write_cmos_sensor(0x2419,0x41);
    T8EV5YUV_write_cmos_sensor(0x241A,0x46);
    T8EV5YUV_write_cmos_sensor(0x241B,0x41);
    T8EV5YUV_write_cmos_sensor(0x241C,0x34);
    T8EV5YUV_write_cmos_sensor(0x241D,0x39);
    T8EV5YUV_write_cmos_sensor(0x241E,0x0D);
    T8EV5YUV_write_cmos_sensor(0x241F,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x30);
    T8EV5YUV_write_cmos_sensor(0x2403,0x35);
    T8EV5YUV_write_cmos_sensor(0x2404,0x38);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x30);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x30);
    T8EV5YUV_write_cmos_sensor(0x240A,0x46);
    T8EV5YUV_write_cmos_sensor(0x240B,0x45);
    T8EV5YUV_write_cmos_sensor(0x240C,0x30);
    T8EV5YUV_write_cmos_sensor(0x240D,0x30);
    T8EV5YUV_write_cmos_sensor(0x240E,0x32);
    T8EV5YUV_write_cmos_sensor(0x240F,0x30);
    T8EV5YUV_write_cmos_sensor(0x2410,0x46);
    T8EV5YUV_write_cmos_sensor(0x2411,0x43);
    T8EV5YUV_write_cmos_sensor(0x2412,0x46);
    T8EV5YUV_write_cmos_sensor(0x2413,0x45);
    T8EV5YUV_write_cmos_sensor(0x2414,0x36);
    T8EV5YUV_write_cmos_sensor(0x2415,0x33);
    T8EV5YUV_write_cmos_sensor(0x2416,0x0D);
    T8EV5YUV_write_cmos_sensor(0x2417,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2401,0x3A);
    T8EV5YUV_write_cmos_sensor(0x2402,0x30);
    T8EV5YUV_write_cmos_sensor(0x2403,0x30);
    T8EV5YUV_write_cmos_sensor(0x2404,0x30);
    T8EV5YUV_write_cmos_sensor(0x2405,0x30);
    T8EV5YUV_write_cmos_sensor(0x2406,0x30);
    T8EV5YUV_write_cmos_sensor(0x2407,0x30);
    T8EV5YUV_write_cmos_sensor(0x2408,0x30);
    T8EV5YUV_write_cmos_sensor(0x2409,0x31);
    T8EV5YUV_write_cmos_sensor(0x240A,0x46);
    T8EV5YUV_write_cmos_sensor(0x240B,0x46);
    T8EV5YUV_write_cmos_sensor(0x240C,0x0D);
    T8EV5YUV_write_cmos_sensor(0x240D,0x0A);
    T8EV5YUV_write_cmos_sensor(0x2400,0x03);
    T8EV5YUV_write_cmos_sensor(0x2400,0x02);
#endif
}
/*******************************************************************************
*
********************************************************************************/
//extern int Af_VCOMA2_Power_On(void);

static void T8EV5YUV_Sensor_Init(void)
{
  //  Af_VCOMA2_Power_On();
    T8EV5YUV_Sensor_Init_setting();
    T8EV5YUV_Sensor_AF_Patch();
    
    Sleep(100);
    T8EV5_FOCUS_Init();

    is_first_preview = 1;
    
    SENSORDB("Init Success \n");
}   /*  T8EV5YUV_Sensor_Init  */

//no use
void T8EV5YUV_set_720p_init(void)
{
    //720p 30fps
    T8EV5YUV_write_cmos_sensor(0x0102,0x03);//-/-/-/-/-/-/VLAT_ON/GROUP_HOLD
    T8EV5YUV_write_cmos_sensor(0x0105,0x01);//- / - / - / - / - / H_COUNT[10:8] 
    T8EV5YUV_write_cmos_sensor(0x0106,0x10);//H_COUNT[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0107,0x00);//- / - / - / - / - / V_COUNT[10:8] 
    T8EV5YUV_write_cmos_sensor(0x0108,0x82);//V_COUNT[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0109,0x00);//- / - / - / - / - / - / - / SCALE_M[8] 
    T8EV5YUV_write_cmos_sensor(0x010A,0x20);//SCALE_M[7:0] 
    T8EV5YUV_write_cmos_sensor(0x010B,0x11);//- / V_ANABIN[2:0] / - / - / - / H_ANABIN 
    T8EV5YUV_write_cmos_sensor(0x010D,0x05);//- / - / - / - / HOUTPIX[11:8] 
    T8EV5YUV_write_cmos_sensor(0x010E,0x00);//HOUTPIX[7:0] 
    T8EV5YUV_write_cmos_sensor(0x010F,0x02);//- / - / - / - / - / VOUTPIX[10:8] 
    T8EV5YUV_write_cmos_sensor(0x0110,0xD0);//VOUTPIX[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0111,0x00);//- / - / - / - / - / - / HCROP[1:0] 
    T8EV5YUV_write_cmos_sensor(0x0112,0x00);//- / - / - / - / - / - / VCROP[9:8] 
    T8EV5YUV_write_cmos_sensor(0x0113,0x00);//VCROP[7:0] 
    T8EV5YUV_write_cmos_sensor(0x013A,0x03);//-/-/-/-/VT_SYS_CNTL[3:0]
    T8EV5YUV_write_cmos_sensor(0x0229,0x2B);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
    T8EV5YUV_write_cmos_sensor(0x0305,0x01);//-/-/-/-/AGMAX[11:8]
    T8EV5YUV_write_cmos_sensor(0x0306,0xFF);//AGMAX[7:0]
    T8EV5YUV_write_cmos_sensor(0x0307,0x0F);//MES[15:8]
    T8EV5YUV_write_cmos_sensor(0x0315,0x98);//FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
    T8EV5YUV_write_cmos_sensor(0x0316,0x6c);//FLLONGON/FRMSPD[1:0]/FL600S[12:8]
    T8EV5YUV_write_cmos_sensor(0x0317,0x68);//FL600S[7:0]
    T8EV5YUV_write_cmos_sensor(0x0102,0x02);//-/-/-/-/-/-/VLAT_ON/GROUP_HOLD
}


void T8EV5YUV_set_1280_960_init(void)
{
    //1280*960 7.5fps
    T8EV5YUV_write_cmos_sensor(0x0102,0x03);//-/-/-/-/-/-/VLAT_ON/GROUP_HOLD
    T8EV5YUV_write_cmos_sensor(0x0105,0x01);//- / - / - / - / - / H_COUNT[10:8] 
    T8EV5YUV_write_cmos_sensor(0x0106,0x10);//H_COUNT[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0107,0x00);//- / - / - / - / - / V_COUNT[10:8] 
    T8EV5YUV_write_cmos_sensor(0x0108,0x82);//V_COUNT[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0109,0x00);//- / - / - / - / - / - / - / SCALE_M[8] 
    T8EV5YUV_write_cmos_sensor(0x010A,0x20);//SCALE_M[7:0] 
    T8EV5YUV_write_cmos_sensor(0x010B,0x11);//- / V_ANABIN[2:0] / - / - / - / H_ANABIN 
    T8EV5YUV_write_cmos_sensor(0x010D,0x05);//- / - / - / - / HOUTPIX[11:8] 
    T8EV5YUV_write_cmos_sensor(0x010E,0x00);//HOUTPIX[7:0] 
    T8EV5YUV_write_cmos_sensor(0x010F,0x03);//- / - / - / - / - / VOUTPIX[10:8] 
    T8EV5YUV_write_cmos_sensor(0x0110,0xC0);//VOUTPIX[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0111,0x00);//- / - / - / - / - / - / HCROP[1:0] 
    T8EV5YUV_write_cmos_sensor(0x0112,0x00);//- / - / - / - / - / - / VCROP[9:8] 
    T8EV5YUV_write_cmos_sensor(0x0113,0x00);//VCROP[7:0] 
    T8EV5YUV_write_cmos_sensor(0x013A,0x03);//-/-/-/-/VT_SYS_CNTL[3:0]
    T8EV5YUV_write_cmos_sensor(0x0229,0x2B);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
    T8EV5YUV_write_cmos_sensor(0x0305,0x01);//-/-/-/-/AGMAX[11:8]
    T8EV5YUV_write_cmos_sensor(0x0306,0xFF);//AGMAX[7:0]
    T8EV5YUV_write_cmos_sensor(0x0307,0x0F);//MES[15:8]
    T8EV5YUV_write_cmos_sensor(0x0315,0x98);//FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]//0x98
    T8EV5YUV_write_cmos_sensor(0x0316,0x65);//FLLONGON/FRMSPD[1:0]/FL600S[12:8]
    T8EV5YUV_write_cmos_sensor(0x0317,0x7E);//FL600S[7:0]
    T8EV5YUV_write_cmos_sensor(0x0102,0x02);//-/-/-/-/-/-/VLAT_ON/GROUP_HOLD
    
}

void T8EV5YUV_set_5M_init(void)
{
    //2592*1944 7.5fps
    T8EV5YUV_write_cmos_sensor(0x0102,0x03);//-/-/-/-/-/-/VLAT_ON/GROUP_HOLD
    T8EV5YUV_write_cmos_sensor(0x0105,0x01);//- / - / - / - / - / H_COUNT[10:8] 
    T8EV5YUV_write_cmos_sensor(0x0106,0x5F);//H_COUNT[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0107,0x00);//- / - / - / - / - / V_COUNT[10:8] 
    T8EV5YUV_write_cmos_sensor(0x0108,0xF7);//V_COUNT[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0109,0x00);//- / - / - / - / - / - / - / SCALE_M[8] 
    T8EV5YUV_write_cmos_sensor(0x010A,0x10);//SCALE_M[7:0] 
    T8EV5YUV_write_cmos_sensor(0x010B,0x00);//- / V_ANABIN[2:0] / - / - / - / H_ANABIN 
    T8EV5YUV_write_cmos_sensor(0x010D,0x0A);//- / - / - / - / HOUTPIX[11:8] 
    T8EV5YUV_write_cmos_sensor(0x010E,0x00);//HOUTPIX[7:0] 
    T8EV5YUV_write_cmos_sensor(0x010F,0x07);//- / - / - / - / - / VOUTPIX[10:8] 
    T8EV5YUV_write_cmos_sensor(0x0110,0x80);//VOUTPIX[7:0] 
    T8EV5YUV_write_cmos_sensor(0x0111,0x00);//- / - / - / - / - / - / HCROP[1:0] 
    T8EV5YUV_write_cmos_sensor(0x0112,0x00);//- / - / - / - / - / - / VCROP[9:8] 
    T8EV5YUV_write_cmos_sensor(0x0113,0x00);//VCROP[7:0] 
    T8EV5YUV_write_cmos_sensor(0x013A,0x03);//-/-/-/-/VT_SYS_CNTL[3:0]
    T8EV5YUV_write_cmos_sensor(0x0229,0x2B);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
    T8EV5YUV_write_cmos_sensor(0x0305,0x01);//-/-/-/-/AGMAX[11:8]
    T8EV5YUV_write_cmos_sensor(0x0306,0x4B);//AGMAX[7:0]
	T8EV5YUV_write_cmos_sensor(0x0315,0x08);//FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
    T8EV5YUV_write_cmos_sensor(0x0316,0x6D);//FLLONGON/FRMSPD[1:0]/FL600S[12:8]
    T8EV5YUV_write_cmos_sensor(0x0317,0x68);//FL600S[7:0]
    T8EV5YUV_write_cmos_sensor(0x0102,0x02);//-/-/-/-/-/-/VLAT_ON/GROUP_HOLD

}
void T8EV5YUV_set_5M(void)
{
    SENSORDB("Set 5M begin\n"); 
    T8EV5YUV_g_RES = T8EV5_5M;

    T8EV5YUV_set_5M_init();

    SENSORDB("Set 5M End\n");  
}

void T8EV5YUV_dump_5M(void)
{
}

/*****************************************************************************/
/* Windows Mobile Sensor Interface */
/*****************************************************************************/
/*************************************************************************
* FUNCTION
*   T8EV5YUVOpen
*
* DESCRIPTION
*   This function initialize the registers of CMOS sensor
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
#define CAMERA_POWER_VCAM_A2  PMIC_APP_MAIN_CAMERA_POWER_AF

UINT32 T8EV5YUVOpen(void)
{
    int  retry = 0; 
    kal_uint16 T8EV5YUV_sensor_id=0;
#if WINMO_USE
    T8EV5YUVhDrvI2C=DRV_I2COpen(1);
    DRV_I2CSetParam(T8EV5YUVhDrvI2C, I2C_VAL_FS_SAMPLE_DIV, 3);
    DRV_I2CSetParam(T8EV5YUVhDrvI2C, I2C_VAL_FS_STEP_DIV, 8);
    DRV_I2CSetParam(T8EV5YUVhDrvI2C, I2C_VAL_DELAY_LEN, 2);
    DRV_I2CSetParam(T8EV5YUVhDrvI2C, I2C_VAL_CLK_EXT, I2C_CLKEXT_DISABLE);
    T8EV5YUVI2CConfig.trans_mode=I2C_TRANSFER_FAST_MODE;
    T8EV5YUVI2CConfig.slaveAddr=T8EV5YUV_sensor_write_I2C_address>>1;
    T8EV5YUVI2CConfig.RS_ST=I2C_TRANSFER_STOP;  /* for fast mode */
#endif 

    T8EV5YUV_sensor_id = ((T8EV5YUV_read_cmos_sensor(0x0000) << 8) | T8EV5YUV_read_cmos_sensor(0x0001));
        printk("T8EV5YUVOpen Read Sensor ID = 0x%04x\n", T8EV5YUV_sensor_id); 
    // check if sensor ID correct
    retry = 3; 
    do {
        T8EV5YUV_sensor_id = ((T8EV5YUV_read_cmos_sensor(0x00) << 8) | T8EV5YUV_read_cmos_sensor(0x01)); 
        printk("T8EV5YUVOpen Read Sensor ID = 0x%04x\n", T8EV5YUV_sensor_id);        
        if (T8EV5YUV_sensor_id == T8EV5MIPI_SENSOR_ID)
            break; 
        printk("T8EV5YUVOpen Read Sensor ID Fail = 0x%04x\n", T8EV5YUV_sensor_id); 
        retry--; 
    } while (retry > 0);

    if (T8EV5YUV_sensor_id != T8EV5MIPI_SENSOR_ID)
        return ERROR_SENSOR_CONNECT_FAIL;
hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,"kd_camera_hw");
returnAfMode=1;
    T8EV5YUV_Sensor_Init();
    
    T8EV5YUV_g_iBackupExtraExp = 0;

	if(false == AF_Power_OnOff)
	{
		SENSORDB("T8EV5YUV AF Power on.\n");
		if(TRUE != hwPowerOn(CAMERA_POWER_VCAM_A2, VOL_2800,"T8EV5YUV_AF"))
		{
			printk("[CAMERA SENSOR AF] Fail to enable analog power\n");
			return -EIO;
		}  
		AF_Power_OnOff = true;
	}
	else
	{
		SENSORDB("[T8EV5YUV] AF Power has already on.\n");
	}

    return ERROR_NONE;
}



/*************************************************************************
* FUNCTION
*   T8EV5YUV_SetShutter
*
* DESCRIPTION
*   This function set e-shutter of T8EV5 to change exposure time.
*
* PARAMETERS
*   shutter : exposured lines
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
void T8EV5YUV_SetShutter(kal_uint16 iShutter)
{
#if 0 
    if (iShutter < 4 )
        iShutter = 4;
#else 
    if (iShutter < 1)
        iShutter = 1; 
#endif     

    T8EV5YUV_pv_exposure_lines = iShutter;

    //T8EV5YUV_imgSensorProfileStart();
   // T8EV5YUV_write_shutter(iShutter);
    //T8EV5YUV_imgSensorProfileEnd("T8EV5YUV_SetShutter"); 
    //SENSORDB("iShutter = %d\n", iShutter);

}   /*  T8EV5YUV_SetShutter   */



/*************************************************************************
* FUNCTION
*   T8EV5YUV_read_shutter
*
* DESCRIPTION
*   This function to  Get exposure time.
*
* PARAMETERS
*   None
*
* RETURNS
*   shutter : exposured lines
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT16 T8EV5YUV_read_shutter(void)
{
    kal_uint16 temp_reg;

   temp_reg=(T8EV5YUV_read_cmos_sensor(0x035F)<<8)+T8EV5YUV_read_cmos_sensor(0x0360);

   SENSORDB("t8ev5read shutter = 0x%x\n", temp_reg);
    
    return (UINT16)temp_reg;
}

/*************************************************************************
* FUNCTION
*   T8EV5_night_mode
*
* DESCRIPTION
*   This function night mode of T8EV5.
*
* PARAMETERS
*   none
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/

//no use
void T8EV5YUV_NightMode(kal_bool bEnable)
{

    if(bEnable)
    {
        if(T8EV5YUV_MPEG4_encode_mode==KAL_TRUE)
        {
            T8EV5YUV_write_cmos_sensor(0x0315,0xB8);  //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]

        }
        else
        {
            T8EV5YUV_write_cmos_sensor(0x0315,0xB8);  //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
        }
    }
    else
    {
        if(T8EV5YUV_MPEG4_encode_mode==KAL_TRUE)
        {
            T8EV5YUV_write_cmos_sensor(0x0315,0x08); //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
        }
        else
        {
            T8EV5YUV_write_cmos_sensor(0x0315,0x98); //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
        }

    }
    
}

/*************************************************************************
* FUNCTION
*   T8EV5YUVClose
*
* DESCRIPTION
*   This function is to turn off sensor module power.
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 T8EV5YUVClose(void)
{
    //  CISModulePowerOn(FALSE);

    //s_porting
    //  DRV_I2CClose(T8EV5YUVhDrvI2C);
    //e_porting
    
    	if(true == AF_Power_OnOff)
	{
		SENSORDB("[T8EV5YUV] AF Power down.\n");
	    if(TRUE != hwPowerDown(CAMERA_POWER_VCAM_A2,"T8EV5YUV_AF"))
	    {
		 	printk("[CAMERA SENSOR AF] Fail to enable analog power\n");
			return -EIO;
		}
		AF_Power_OnOff = false;
	}
	else
	{
		SENSORDB("[T8EV5YUVSensor] AF Power is already off.\n");
	}
    return ERROR_NONE;
}   /* T8EV5YUVClose() */

static bool T8EV5_FOCUS_Get_CPU_ST(void)
{
	UINT32 i=10;
	while(i)
	{
		i--;
		if(T8EV5YUV_read_cmos_sensor(0x2800)&0x80)
			continue;
		else
			return 0;
	}
}

static void T8EV5_FOCUS_Get_AF_Status(UINT32 *pFeatureReturnPara32)
{
    UINT32 state_focused=0,v_focus;
    UINT32 state_focusing=0;
    static UINT32 status = 0;
    static UINT32 focus_count = 0;
    static bool focused = 0;

    if(g_sensorAfMode)
    {
        //CAF
        T8EV5YUV_write_cmos_sensor(0x27FF,0x72);
        v_focus = T8EV5YUV_read_cmos_sensor(0x2FFE);
        state_focusing=v_focus&0x03;
        if(state_focusing==1)
        {
            //focus finished
            T8EV5YUV_write_cmos_sensor(0x27FF,0x71);
            state_focused = T8EV5YUV_read_cmos_sensor(0x2FFE)&0x80;
            focus_count=0;
            if(state_focused == 0x00)
            {
                 *pFeatureReturnPara32 = SENSOR_AF_FOCUSED;
            }
            else
            {
                 *pFeatureReturnPara32 = SENSOR_AF_ERROR;
            }
            
        }
        else
        {
            *pFeatureReturnPara32 = SENSOR_AF_FOCUSING;
             focus_count++;
             if(focus_count > 30)
             {
                *pFeatureReturnPara32 = SENSOR_AF_ERROR;
                focus_count=0;
                printk("CAF focus timeout \r\n");
             }
        }
    }
    else
    {
        //one shot AF
        v_focus = T8EV5YUV_read_cmos_sensor(0x2800);
        state_focusing = v_focus & 0x80;
        state_focused = v_focus & 0x10;
        
        printk("YXW get af status--state_focusing =%x,state_focused=%x\r\n",state_focusing,state_focused);  
        if (state_focusing == 0x80)
        {
            *pFeatureReturnPara32 = SENSOR_AF_FOCUSING;
             focus_count++;
             if(focus_count > 30)
             {
                *pFeatureReturnPara32 = SENSOR_AF_ERROR;
                focus_count=0;
                printk("one shot focus timeout \r\n");
             }
        }
        else
        {
            focus_count=0;
            if(state_focused == 0x00)
            {
                 *pFeatureReturnPara32 = SENSOR_AF_FOCUSED;
            }
            else
            {
                 *pFeatureReturnPara32 = SENSOR_AF_ERROR;
            }
        }

    }
    
}


/*************************************************************************
* FUNCTION
*   T8EV5_FOCUS_Init
*
* DESCRIPTION
*   This function is to load micro code for AF function
*
* PARAMETERS
*   None
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/


static void T8EV5_FOCUS_Init(void)
{
    SENSORDB("T8EV5_FOCUS_Init\n");     

    T8EV5YUV_write_cmos_sensor(0x2400,0x03);//   
    T8EV5YUV_write_cmos_sensor(0x2400,0x02);//   
    T8EV5YUV_write_cmos_sensor(0x0100,0x01);//STREAM ON
    T8EV5YUV_write_cmos_sensor(0x27FF,0xF0);// ENABLE DRIVER
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x00);// INF MSB[1:0]
    T8EV5YUV_write_cmos_sensor(0x2402,0x50);// INF LSB[7:0]
    T8EV5YUV_write_cmos_sensor(0x27FF,0xF1);// SET_INF_POSITION
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x01);// MAC MSB[1:0]
    T8EV5YUV_write_cmos_sensor(0x2402,0xB0);// MAC LSB[7:0]
    T8EV5YUV_write_cmos_sensor(0x27FF,0xF2);// SET_MACRO_POSITION
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x40);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x40);//
    T8EV5YUV_write_cmos_sensor(0x2403,0x00);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x62);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x42);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x80);//
    T8EV5YUV_write_cmos_sensor(0x2403,0x00);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x62);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x44);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x14);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x61);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x48);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x13);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x61);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x4C);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x12);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x61);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x45);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x00);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x61);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x46);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x00);//
    T8EV5YUV_write_cmos_sensor(0x2403,0x00);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x62);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x49);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x00);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x61);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x4A);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x00);//
    T8EV5YUV_write_cmos_sensor(0x2403,0x00);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x62);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x4D);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x00);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x61);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x4E);//
    T8EV5YUV_write_cmos_sensor(0x2402,0x2C);//
    T8EV5YUV_write_cmos_sensor(0x2403,0x01);//
    T8EV5YUV_write_cmos_sensor(0x27FF,0x62);//
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x00);// SET DEFAULT POSITION=0
    T8EV5YUV_write_cmos_sensor(0x27FF,0x3F);// INITIALIZE
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x16);// Type1 Reg Addr Lower
    T8EV5YUV_write_cmos_sensor(0x2402,0x13);// Write Data (AF Window size 25%)
    T8EV5YUV_write_cmos_sensor(0x27FF,0xE1);// Write Trigger
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x18);//; AF parameter number 0x18
    T8EV5YUV_write_cmos_sensor(0x2402,0x07);//;//24 frames to wait after detecting scene changed;
    T8EV5YUV_write_cmos_sensor(0x27FF,0x61);//; 
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x34);//; AF parameter number 0x34
    T8EV5YUV_write_cmos_sensor(0x2402,0x00);//;
    T8EV5YUV_write_cmos_sensor(0x2403,0x00);//;
    T8EV5YUV_write_cmos_sensor(0x2404,0x18);//;
    T8EV5YUV_write_cmos_sensor(0x2405,0x41);//;0x40
    T8EV5YUV_write_cmos_sensor(0x27FF,0x64);//; Threshold to go "Rough Searching" = 7.0, bigger is easy to go "Rough Searching"
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x38);//; AF parameter number 0x38
    T8EV5YUV_write_cmos_sensor(0x2402,0x00);//;
    T8EV5YUV_write_cmos_sensor(0x2403,0x00);//;
    T8EV5YUV_write_cmos_sensor(0x2404,0x18);//;
    T8EV5YUV_write_cmos_sensor(0x2405,0x41);//;0x40
    T8EV5YUV_write_cmos_sensor(0x27FF,0x64);//; Threshold to go "Rough Searching" = 7.0, smaller is easy to go "Rough Searching"
    T8EV5_FOCUS_Get_CPU_ST();
    T8EV5YUV_write_cmos_sensor(0x2401,0x17);
    T8EV5YUV_write_cmos_sensor(0x2402,0x00);
    T8EV5YUV_write_cmos_sensor(0x27FF,0xE1);
    T8EV5_FOCUS_Get_CPU_ST();

 
}   /*  T8EV5_FOCUS_Init  */




// Step to Specified position
static void T8EV5_FOCUS_Move_to(UINT16 a_u2MovePosition)
{
    return 0;//don't support for t8ev5
}

static void T8EV5_FOCUS_Get_AF_Inf(UINT32 *pFeatureReturnPara32)
{
    *pFeatureReturnPara32 = 0;
}

static void T8EV5_FOCUS_Get_AF_Macro(UINT32 *pFeatureReturnPara32)
{
    *pFeatureReturnPara32 = 1023;
}

//update focus window
//@input zone[] addr

static void T8EV5_FOCUS_Set_AE_Window(zone_addr)
{//update global zone
    UINT8 state = 0x8F;
return; 
    //input:
    //UINT32 times = 1;
    UINT32 FD_XS = 4;
    UINT32 FD_YS = 3;   
    UINT32 x0, y0, x1, y1;
    UINT32* zone = (UINT32*)zone_addr;
    
   
    x0 = *zone;
    y0 = *(zone + 1);
    x1 = *(zone + 2);
    y1 = *(zone + 3);   

    FD_XS = *(zone + 4);
    FD_YS = *(zone + 5);
    //times = FD_XS / AF_XS;
    printk("tek.xing yxw set AE windown x0=%d,y0=%d,x1=%d,y1=%d,FD_XS=%d,FD_YS=%d\n",
        x0, y0, x1, y1, FD_XS, FD_YS);  

    if((x0==x1)&&(y0==y1))
     {
        printk("tek.xing T8EV5_FOCUS_Set_AE_Window defalut\n");
       T8EV5YUV_write_cmos_sensor(0x030C,0x55);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
       T8EV5YUV_write_cmos_sensor(0x030D,0x5A);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
       T8EV5YUV_write_cmos_sensor(0x030E,0x95);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
       T8EV5YUV_write_cmos_sensor(0x030F,0x54);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
       return;
     }
   
	  m_zone_center_x=(x0+x1)/2;
	  m_zone_center_y=(y0+y1)/2;
   
   	SENSORDB("[%s] x=%d,y=%d\n",__FUNCTION__,m_zone_center_x,m_zone_center_y);
	  //update AE zone
	  if(m_zone_center_x<64)
	  {
		  if(m_zone_center_y<80)
		  {
			  //A1
			  SENSORDB("[%s] A1\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0xD0);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x10);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B1
			  
			  SENSORDB("[%s] B1\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x40);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x34);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x04);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C1
			  
			  SENSORDB("[%s] C1\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x10);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x0D);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
	  }
	  else if(m_zone_center_x<128)
	  {
		  if(m_zone_center_y<80)
		  {
			  //A2
			  
			  SENSORDB("[%s] A2\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x74);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x04);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B2
			  
			  SENSORDB("[%s] B2\n",__FUNCTION__);
			   T8EV5YUV_write_cmos_sensor(0x030C,0x10);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x1D);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x01);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C2
			  
			  SENSORDB("[%s] C2\n",__FUNCTION__);
			   T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x04);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x07);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x40);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
	  }
	  else if(m_zone_center_x<192)
	  {
		  if(m_zone_center_y<80)
		  {
			  //A3
			  
			  SENSORDB("[%s] A3\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x1D);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x01);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B3
			  
			  SENSORDB("[%s] B3\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x04);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x07);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x40);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x40);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C3
			  
			  SENSORDB("[%s] C3\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x01);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x01);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0xD0);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
	  }
	  else if(m_zone_center_x<256)
	  {   
		  if(m_zone_center_y<80)
		  {
			  //A4
			  
			  SENSORDB("[%s] A4\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x07);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x40);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x40);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B4
			  
			  SENSORDB("[%s] B4\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x01);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x01);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0xD0);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x54);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C4
			  
			  SENSORDB("[%s] C4\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x40);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x74);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  
	  }
	  else
	  {
		  if(m_zone_center_y<80)
		  {
			  //A5
			  
			  SENSORDB("[%s] A5\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x01);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0xC0);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x10);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B5
			  
			  SENSORDB("[%s] B5\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x40);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x70);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x04);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C5
			  
			  SENSORDB("[%s] C5\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x10);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x1C);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
	   }
	/*  if(m_zone_center_x<64)
	  {
		  if(m_zone_center_y<80)
		  {
			  //A1
			  SENSORDB("[%s] A1\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x80);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B1
			  
			  SENSORDB("[%s] B1\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x20);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C1
			  
			  SENSORDB("[%s] C1\n",__FUNCTION__);
			   T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x08);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
	  }
	  else if(m_zone_center_x<128)
	  {
		  if(m_zone_center_y<80)
		  {
			  //A2
			  
			  SENSORDB("[%s] A2\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x20);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B2
			  
			  SENSORDB("[%s] B2\n",__FUNCTION__);
			   T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x08);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C2
			  
			  SENSORDB("[%s] C2\n",__FUNCTION__);
			   T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x02);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
	  }
	  else if(m_zone_center_x<192)
	  {
		  if(m_zone_center_y<80)
		  {
			  //A3
			  
			  SENSORDB("[%s] A3\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x08);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B3
			  
			  SENSORDB("[%s] B3\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x02);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C3
			  
			  SENSORDB("[%s] C3\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x80);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
	  }
	  else if(m_zone_center_x<256)
	  {   
		  if(m_zone_center_y<80)
		  {
			  //A4
			  
			  SENSORDB("[%s] A4\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x02);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B4
			  
			  SENSORDB("[%s] B4\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x80);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C4
			  
			  SENSORDB("[%s] C4\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x20);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  
	  }
	  else
	  {
		  if(m_zone_center_y<80)
		  {
			  //A5
			  
			  SENSORDB("[%s] A5\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x80);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else if(m_zone_center_y<160)
		  {
			  //B5
			  
			  SENSORDB("[%s] B5\n",__FUNCTION__);
			 T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x20);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x00);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
		  else 
		  {
			  //C5
			  
			  SENSORDB("[%s] C5\n",__FUNCTION__);
			  T8EV5YUV_write_cmos_sensor(0x030C,0x00);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030D,0x00);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030E,0x00);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
			  T8EV5YUV_write_cmos_sensor(0x030F,0x08);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
		  }
	   }
*/
  
   /* x0 = (UINT8)(x0 / times);   
    y0 = (UINT8)(y0 / times);   
    x1 = (UINT8)(x1 / times);   
    y1 = (UINT8)(y1 / times);       

//zone changed, update global zone.
      ZONE[0]=x0;
      ZONE[1]=y0;
      ZONE[2]=x1;
      ZONE[3]=y1;*/
}

static void T8EV5_FOCUS_Set_AF_Window(zone_addr)
{//update global zone
    UINT8 state = 0x8F;

    //input:
    UINT32 times = 1;
    UINT32 FD_XS = 4;
    UINT32 FD_YS = 3;   
    UINT32 x0, y0, x1, y1;
    UINT32* zone = (UINT32*)zone_addr;
    x0 = *zone;
    y0 = *(zone + 1);
    x1 = *(zone + 2);
    y1 = *(zone + 3);   
    FD_XS = *(zone + 4);
    FD_YS = *(zone + 5);
    //times = FD_XS / AF_XS;
    SENSORDB("yxw set Af windown x0=%d,y0=%d,x1=%d,y1=%d,FD_XS=%d,FD_YS=%d\n",
        x0, y0, x1, y1, FD_XS, FD_YS);  

    m_zone_center_x=(x0+x1)/2;
    m_zone_center_y=(y0+y1)/2;

    /*  
    x0 = (UINT8)(x0 / times);   
    y0 = (UINT8)(y0 / times);   
    x1 = (UINT8)(x1 / times);   
    y1 = (UINT8)(y1 / times);       

//zone changed, update global zone.
      ZONE[0]=x0;
      ZONE[1]=y0;
      ZONE[2]=x1;
      ZONE[3]=y1;*/
}

//update zone[]
static void T8EV5_FOCUS_Update_Zone(void)
{
    //update AF zone
    UINT32 af_x,af_y;
    
    if(m_zone_center_x<60)
    {
        af_x=0;
    }
    else if(m_zone_center_x<100)
    {
        af_x=1;
    }
    else if(m_zone_center_x<140)
    {
        af_x=2;
    }
    else if(m_zone_center_x<180)
    {
        af_x=3;
    }
    else if(m_zone_center_x<220)
    {
        af_x=4;
    }
    else if(m_zone_center_x<260)
    {
        af_x=5;
    }
    else
    {
        af_x=6;
    }
    
    if(m_zone_center_y<45)
    {
        af_y=0;
    }
    else if(m_zone_center_y<75)
    {
        af_y=1;
    }
    else if(m_zone_center_y<105)
    {
        af_y=2;
    }
    else if(m_zone_center_y<135)
    {
        af_y=3;
    }
    else if(m_zone_center_y<165)
    {
        af_y=4;
    }
    else if(m_zone_center_y<195)
    {
        af_y=5;
    }
    else
    {
        af_y=6;
    }
    SENSORDB("[%s] af_x=%d, af_y=%d\n",__FUNCTION__,af_x,af_y);
    T8EV5YUV_write_cmos_sensor(0x2401,0x15);//
    T8EV5YUV_write_cmos_sensor(0x2402,((af_y<<4)|af_x));//
    T8EV5YUV_write_cmos_sensor(0x27FF,0xE1);// af position set
    T8EV5_FOCUS_Get_CPU_ST();

}

//set constant focus
static void T8EV5_FOCUS_Constant_Focus(void)
{
    UINT8 state = 0x8F;
    UINT32 iteration = 300;

     printk("YXW T8EV5_FOCUS_Constant_Focus\n");
    g_sensorAfMode = 1;
    //send idle command to firmware
    #if 0
        T8EV5YUV_write_cmos_sensor(0x2401,0x17);
	T8EV5YUV_write_cmos_sensor(0x2402,0x1E);
	T8EV5YUV_write_cmos_sensor(0x27FF,0xE1);
	T8EV5_FOCUS_Get_CPU_ST();
    #endif

	if(returnAfMode)
		{
   T8EV5YUV_write_cmos_sensor(0x27FF,0x30); //GO_CONTINUOUS
		}
    //Sleep(5);	
    return;
}

static void T8EV5_FOCUS_Single_Focus()
{
    

    UINT8 state = 0x8F;
    UINT8 state_ack = 0x8F;	
    UINT8 state_cmd = 0x8F;		
    UINT32 iteration = 300;

    g_sensorAfMode = 1;// add by leaf for af grid one time 
	printk("YXW T8EV5_FOCUS_Single_Focus\n");

    //1.update zone
    T8EV5_FOCUS_Update_Zone();

    //send single focus mode command to firmware

	//T8EV5YUV_write_cmos_sensor(0x27FF,0x37);
	//T8EV5_FOCUS_Get_CPU_ST();
	#if 0
	T8EV5YUV_write_cmos_sensor(0x2401,0x17);
	T8EV5YUV_write_cmos_sensor(0x2402,0x00);
	T8EV5YUV_write_cmos_sensor(0x27FF,0xE1);
	T8EV5_FOCUS_Get_CPU_ST();
	#endif
	//T8EV5YUV_write_cmos_sensor(0x27FF,0x28);// GO_ONESHOT	
	T8EV5YUV_write_cmos_sensor(0x27FF,0x30); // add by leaf for af grid one time 
	
	returnAfMode=0;

    return;
	
}

static void T8EV5_FOCUS_Pause_Focus()
{
    UINT8 state = 0x8F;
    UINT32 iteration = 300;

    
    printk("YXW T8EV5_FOCUS_Pause_Focus\n");

    //send idle command to firmware
    /*T8EV5YUV_write_cmos_sensor(0x27FF,0x37); //STOP_CONTINUOUS

    iteration = 100;  
    do{
        state = (UINT8)T8EV5YUV_read_cmos_sensor(0x2800);
        
        if(state == 0x00)
        {
            SENSORDB("idle!\n");
         //setAfStatus(SENSOR_AF_IDLE);
            break;
        }           
        Sleep(1);
        iteration --;

    }while(iteration);*/

}
static void T8EV5_FOCUS_Cancel_Focus()
{
     printk("YXW T8EV5_FOCUS_Cancel_Focus\n");

    //UINT8 state = 0x8F;
    //UINT32 iteration = 300;

    //send idle command to firmware
    
    //T8EV5YUV_write_cmos_sensor(0x27FF,0x37); //STOP_CONTINUOUS
    // Sleep(1);            
}

void T8EV5YUV_Set_Mirror_Flip(kal_uint8 image_mirror)
{
    //  bit0 HREVON   bit1 VERVON
    switch (image_mirror)
    {
        case IMAGE_NORMAL:  
            T8EV5YUV_write_cmos_sensor(0x0101,0x00);    //Set normal //0x03
            break;
        case IMAGE_H_MIRROR:    
            T8EV5YUV_write_cmos_sensor(0x0101,0x01);    //
            break;
        case IMAGE_V_MIRROR:    
            T8EV5YUV_write_cmos_sensor(0x0101,0x02);
            break;              
        case IMAGE_HV_MIRROR:       
            T8EV5YUV_write_cmos_sensor(0x0101,0x03);    //Set normal
            break;
    }
}


/*************************************************************************
* FUNCTION
*   T8EV5YUVPreview
*
* DESCRIPTION
*   This function start the sensor preview.
*
* PARAMETERS
*   *image_window : address pointer of pixel numbers in one period of HSYNC
*  *sensor_config_data : address pointer of line numbers in one period of VSYNC
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 T8EV5YUVPreview(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    kal_uint16 iStartX = 0, iStartY = 0;
    int i=0 ;
    kal_uint16 es_pv,PRE_MES;

    if(sensor_config_data->SensorOperationMode==MSDK_SENSOR_OPERATION_MODE_VIDEO)		// MPEG4 Encode Mode
    {
    	printk("------tl------video");	
    	T8EV5YUV_write_cmos_sensor(0x27FF,0x37);
		T8EV5_FOCUS_Get_CPU_ST();
        T8EV5YUV_MPEG4_encode_mode = KAL_TRUE;
    }
    else
    {    
    	printk("------tl------preview");
        T8EV5YUV_MPEG4_encode_mode = KAL_FALSE;
    }

	//zh add for HDR 20130618
	spin_lock(&t8ev5yuv_drv_lock);		
	t8ev5yuv_stru.Camco_mode = T8EV5_MODE_PREVIEW;
	PRE_MES=t8ev5yuv_stru.iShutter;
	es_pv=t8ev5yuv_stru.iShutter*t8ev5yuv_stru.iGain/0x38;//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	spin_unlock(&t8ev5yuv_drv_lock);
 
    if(T8EV5_1280_960 == T8EV5YUV_g_RES)
    {  
		T8EV5YUV_set_1280_960_init();
		
		if(is_first_preview)
		{
			is_first_preview=0;
		}
		else
		{
			T8EV5YUV_write_cmos_sensor(0x0307, ((es_pv>>8)&0xFF));//MES[15:8]
			T8EV5YUV_write_cmos_sensor(0x0308, (es_pv&0xFF));//MES[7:0]
			T8EV5YUV_write_cmos_sensor(0x0309, 0x00);//ESLIMMODE/ROOMDET/-/-/MAG[11:8]
			T8EV5YUV_write_cmos_sensor(0x030A, 0x38);//MAG[7:0]!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			while(i<2)
			{
				T8EV5YUV_write_cmos_sensor(0x0300,0x02);//-/-/-/-/-/-/ALCSW/ALCLOCK
				msleep(200);
				if(T8EV5YUV_read_shutter()>(PRE_MES/2))
				{
					//SENSORDB("[%s]i=%d; temp_prev_es=0x%x\n",__FUNCTION__,i,temp_prev_es); 
				break;
				}
							
				T8EV5YUV_write_cmos_sensor(0x0300,0x00);//-/-/-/-/-/-/ALCSW/ALCLOCK
				msleep(10);
				i++;
			}
						
			T8EV5YUV_write_cmos_sensor(0x0300,0x02);//-/-/-/-/-/-/ALCSW/ALCLOCK
		}
		
				

		}
	

    iStartX += T8EV5_IMAGE_SENSOR_PV_STARTX;
    iStartY += T8EV5_IMAGE_SENSOR_PV_STARTY;
    
    T8EV5YUV_Set_Mirror_Flip(sensor_config_data->SensorImageMirror);

    T8EV5YUV_dummy_pixels = 0;
    T8EV5YUV_dummy_lines = 0;
    T8EV5YUV_PV_dummy_pixels = T8EV5YUV_dummy_pixels;
    T8EV5YUV_PV_dummy_lines = T8EV5YUV_dummy_lines;

    //T8EV5YUV_SetDummy(T8EV5YUV_dummy_pixels, T8EV5YUV_dummy_lines);
    //T8EV5YUV_SetShutter(T8EV5YUV_pv_exposure_lines);

    memcpy(&T8EV5YUVSensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    image_window->GrabStartX= iStartX;
    image_window->GrabStartY= iStartY;
    image_window->ExposureWindowWidth= T8EV5_IMAGE_SENSOR_PV_WIDTH - 2*iStartX;
    image_window->ExposureWindowHeight= T8EV5_IMAGE_SENSOR_PV_HEIGHT - 2*iStartY;
    return ERROR_NONE;
}   /* T8EV5YUVPreview() */


UINT32 T8EV5YUVCapture(MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *image_window,
                                                MSDK_SENSOR_CONFIG_STRUCT *sensor_config_data)
{
    //kal_uint32 shutter=T8EV5YUV_pv_exposure_lines;
 
    kal_uint16 iStartX = 0, iStartY = 0;
	
	kal_uint32 AWB_R = 0;
	kal_uint32 AWB_B = 0;


	returnAfMode=1;
	T8EV5YUV_write_cmos_sensor(0x27FF,0x37); //STOP_CONTINUOUS
	T8EV5_FOCUS_Get_CPU_ST();

	//zh over exploure for capture with flash 20130822
	if(is_flash_on)
	{
		msleep(500);
	}
	//alc lock
	T8EV5YUV_write_cmos_sensor(0x0300,0x03);   //-/-/-/-/-/-/ALCSW/ALCLOCK
	//zh add for HDR 20130618		
	spin_lock(&t8ev5yuv_drv_lock);
	if(t8ev5yuv_stru.Camco_mode != T8EV5_MODE_PREVIEW)
	{
		spin_unlock(&t8ev5yuv_drv_lock);
       	return;
	}
	t8ev5yuv_stru.iShutter= T8EV5YUV_read_shutter();
    t8ev5yuv_stru.iGain= read_T8EV5YUV_gain();
    t8ev5yuv_stru.d_gain=((T8EV5YUV_read_cmos_sensor(0x0363)&0x03)<<8)+T8EV5YUV_read_cmos_sensor(0x0364);

    spin_unlock(&t8ev5yuv_drv_lock);
    
    //4. set 5M mode
    if ((image_window->ImageTargetWidth<= T8EV5_IMAGE_SENSOR_PV_WIDTH) &&
        (image_window->ImageTargetHeight<= T8EV5_IMAGE_SENSOR_PV_HEIGHT)) {
   

        //zh add for HDR 20130618       
        spin_lock(&t8ev5yuv_drv_lock);
        t8ev5yuv_stru.Camco_mode = T8EV5_MODE_CAPTURE_OTHERS;
        spin_unlock(&t8ev5yuv_drv_lock);

        T8EV5YUV_set_capture_shutter_gain(AE_EV_COMP_00);

        iStartX = T8EV5_IMAGE_SENSOR_PV_STARTX;
        iStartY = T8EV5_IMAGE_SENSOR_PV_STARTY;
        image_window->GrabStartX=iStartX;
        image_window->GrabStartY=iStartY;
        image_window->ExposureWindowWidth=T8EV5_IMAGE_SENSOR_PV_WIDTH - 2*iStartX;
        image_window->ExposureWindowHeight=T8EV5_IMAGE_SENSOR_PV_HEIGHT- 2*iStartY;
    }
    else 
    { // 5M  Mode     
        //zh add for HDR 20130618       
        spin_lock(&t8ev5yuv_drv_lock);
        t8ev5yuv_stru.Camco_mode = T8EV5_MODE_CAPTURE_5M;
        spin_unlock(&t8ev5yuv_drv_lock);

        T8EV5YUV_set_capture_shutter_gain(AE_EV_COMP_00);
        T8EV5YUV_set_5M_init();
        
        iStartX = 2* T8EV5_IMAGE_SENSOR_PV_STARTX;
        iStartY = 2* T8EV5_IMAGE_SENSOR_PV_STARTY;

        image_window->GrabStartX=iStartX;
        image_window->GrabStartY=iStartY;
        image_window->ExposureWindowWidth=T8EV5_IMAGE_SENSOR_FULL_WIDTH -2*iStartX;
        image_window->ExposureWindowHeight=T8EV5_IMAGE_SENSOR_FULL_HEIGHT-2*iStartY;
    }//5M Capture

    
    memcpy(&T8EV5YUVSensorConfigData, sensor_config_data, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}   /* T8EV5YUVCapture() */

UINT32 T8EV5YUVGetResolution(MSDK_SENSOR_RESOLUTION_INFO_STRUCT *pSensorResolution)
{
    pSensorResolution->SensorFullWidth=IMAGE_SENSOR_FULL_WIDTH - 4*T8EV5_IMAGE_SENSOR_PV_STARTX;
    pSensorResolution->SensorFullHeight=IMAGE_SENSOR_FULL_HEIGHT - 4*T8EV5_IMAGE_SENSOR_PV_STARTY;
    pSensorResolution->SensorPreviewWidth=IMAGE_SENSOR_PV_WIDTH - 2*T8EV5_IMAGE_SENSOR_PV_STARTX;
    pSensorResolution->SensorPreviewHeight=IMAGE_SENSOR_PV_HEIGHT - 2*T8EV5_IMAGE_SENSOR_PV_STARTY;
    pSensorResolution->SensorVideoWidth=IMAGE_SENSOR_PV_WIDTH - 2*T8EV5_IMAGE_SENSOR_PV_STARTX;
    pSensorResolution->SensorVideoHeight=IMAGE_SENSOR_PV_HEIGHT - 2*T8EV5_IMAGE_SENSOR_PV_STARTY;

    return ERROR_NONE;
}   /* T8EV5YUVGetResolution() */
UINT32 T8EV5YUVGetInfo(MSDK_SCENARIO_ID_ENUM ScenarioId,
                                                MSDK_SENSOR_INFO_STRUCT *pSensorInfo,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    pSensorInfo->SensorPreviewResolutionX=IMAGE_SENSOR_PV_WIDTH - 2*T8EV5_IMAGE_SENSOR_PV_STARTX;
    pSensorInfo->SensorPreviewResolutionY=IMAGE_SENSOR_PV_HEIGHT - 2*T8EV5_IMAGE_SENSOR_PV_STARTY;
    pSensorInfo->SensorFullResolutionX=IMAGE_SENSOR_FULL_WIDTH - 4*T8EV5_IMAGE_SENSOR_PV_STARTX;
    pSensorInfo->SensorFullResolutionY=IMAGE_SENSOR_FULL_HEIGHT - 4*T8EV5_IMAGE_SENSOR_PV_STARTY;

    pSensorInfo->SensorCameraPreviewFrameRate=30;
    pSensorInfo->SensorVideoFrameRate=30;
    pSensorInfo->SensorStillCaptureFrameRate=10;
    pSensorInfo->SensorWebCamCaptureFrameRate=15;
    pSensorInfo->SensorResetActiveHigh=FALSE;
    pSensorInfo->SensorResetDelayCount=5;
    pSensorInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;//UYVY;
    
    pSensorInfo->SensorClockPolarity=SENSOR_CLOCK_POLARITY_LOW; /*??? */
    pSensorInfo->SensorClockFallingPolarity=SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorHsyncPolarity = SENSOR_CLOCK_POLARITY_LOW;
    pSensorInfo->SensorVsyncPolarity = SENSOR_CLOCK_POLARITY_HIGH;
    pSensorInfo->SensorInterruptDelayLines = 1;
////////////////////////////////MIPI//////////////////////////////////
    pSensorInfo->SensroInterfaceType=SENSOR_INTERFACE_TYPE_MIPI;//SENSOR_INTERFACE_TYPE_PARALLEL;//SENSOR_INTERFACE_TYPE_MIPI;
/////////////////////////////////////////////////////////////////
/*
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_100_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].ISOSupported=TRUE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_200_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxWidth=CAM_SIZE_2M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].MaxHeight=CAM_SIZE_2M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].ISOSupported=FALSE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_400_MODE].BinningEnable=FALSE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxWidth=CAM_SIZE_05M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].MaxHeight=CAM_SIZE_1M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].ISOSupported=FALSE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_800_MODE].BinningEnable=TRUE;

    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxWidth=CAM_SIZE_05M_WIDTH;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].MaxHeight=CAM_SIZE_05M_HEIGHT;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].ISOSupported=FALSE;
    pSensorInfo->SensorISOBinningInfo.ISOBinningInfo[ISO_1600_MODE].BinningEnable=TRUE;
*/

    //pSensorInfo->CaptureDelayFrame = 1; 
    pSensorInfo->CaptureDelayFrame = 2; 
    pSensorInfo->PreviewDelayFrame = 2; 
    pSensorInfo->VideoDelayFrame = 5; 
    pSensorInfo->SensorMasterClockSwitch = 0; 
    pSensorInfo->SensorDrivingCurrent = ISP_DRIVING_6MA;      

//
    pSensorInfo->AEShutDelayFrame = 0;          /* The frame of setting shutter default 0 for TG int */
    pSensorInfo->AESensorGainDelayFrame = 1;     /* The frame of setting sensor gain */
    pSensorInfo->AEISPGainDelayFrame = 1;   
       
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount= 3;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorMIPILaneNumber =SENSOR_MIPI_1_LANE;// SENSOR_MIPI_1_LANE;
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
            
            pSensorInfo->SensorGrabStartX = T8EV5_IMAGE_SENSOR_PV_STARTX; 
            pSensorInfo->SensorGrabStartY = T8EV5_IMAGE_SENSOR_PV_STARTY;             
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount= 3;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorMIPILaneNumber =SENSOR_MIPI_1_LANE;// SENSOR_MIPI_1_LANE;            
            pSensorInfo->MIPIDataLowPwr2HighSpeedTermDelayCount = 0; 
            pSensorInfo->MIPIDataLowPwr2HighSpeedSettleDelayCount = 14; 
            pSensorInfo->MIPICLKLowPwr2HighSpeedTermDelayCount = 0;
            pSensorInfo->SensorWidthSampling = 0;  // 0 is default 1x
            pSensorInfo->SensorHightSampling = 0;   // 0 is default 1x 
            pSensorInfo->SensorPacketECCOrder = 1;
    
            pSensorInfo->SensorGrabStartX = T8EV5_IMAGE_SENSOR_PV_STARTX; 
            pSensorInfo->SensorGrabStartY = T8EV5_IMAGE_SENSOR_PV_STARTY;             
            break;
        default:
            pSensorInfo->SensorClockFreq=24;
            pSensorInfo->SensorClockDividCount= 3;
            pSensorInfo->SensorClockRisingCount= 0;
            pSensorInfo->SensorClockFallingCount= 2;
            pSensorInfo->SensorPixelClockCount= 3;
            pSensorInfo->SensorDataLatchCount= 2;
            pSensorInfo->SensorGrabStartX = 1; 
            pSensorInfo->SensorGrabStartY = 1;             
            break;
    }

    T8EV5YUVPixelClockDivider=pSensorInfo->SensorPixelClockCount;
    memcpy(pSensorConfigData, &T8EV5YUVSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));

    return ERROR_NONE;
}   /* T8EV5YUVGetInfo() */


UINT32 T8EV5YUVControl(MSDK_SCENARIO_ID_ENUM ScenarioId, MSDK_SENSOR_EXPOSURE_WINDOW_STRUCT *pImageWindow,
                                                MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData)
{
    switch (ScenarioId)
    {
        case MSDK_SCENARIO_ID_CAMERA_PREVIEW:
        case MSDK_SCENARIO_ID_VIDEO_PREVIEW:
        //case MSDK_SCENARIO_ID_VIDEO_CAPTURE_MPEG4:
            T8EV5YUVPreview(pImageWindow, pSensorConfigData);
            break;
        case MSDK_SCENARIO_ID_CAMERA_CAPTURE_JPEG:
        //case MSDK_SCENARIO_ID_CAMERA_CAPTURE_MEM:
            T8EV5YUVCapture(pImageWindow, pSensorConfigData);
            break;
            //s_porting add
            //s_porting add
            //s_porting add
        default:
            return ERROR_INVALID_SCENARIO_ID;
            //e_porting add
            //e_porting add
            //e_porting add
    }
    return TRUE;
} /* T8EV5YUVControl() */


UINT32 T8EV5YUVSetVideoMode(UINT16 u2FrameRate)
{
    return TRUE;
}

void T8EV5YUV_set_saturation(UINT16 iPara)
{
    //SENSORDB("iPara=%d",iPara);
    switch(iPara)
    {
        case ISP_SAT_LOW:
            T8EV5YUV_write_cmos_sensor(0x0229,0x29);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
        break;
        case ISP_SAT_MIDDLE:
            T8EV5YUV_write_cmos_sensor(0x0229,0x2B);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
        break;
        case ISP_SAT_HIGH:
            T8EV5YUV_write_cmos_sensor(0x0229,0x2D);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
        break;
        default:
        break;
    }
}


void T8EV5YUV_set_brightness(UINT16 iPara)
{
    //SENSORDB("iPara=%d",iPara);
    switch(iPara)
    {
        case ISP_BRIGHT_LOW:
            T8EV5YUV_write_cmos_sensor(0x0248,0xE8);
        break;
        case ISP_BRIGHT_MIDDLE:
            T8EV5YUV_write_cmos_sensor(0x0248,0x18);
        break;
        case ISP_BRIGHT_HIGH:
            T8EV5YUV_write_cmos_sensor(0x0248,0x28);
        break;
        default:
        break;
    }
}

void T8EV5YUV_set_contrast(UINT16 iPara)
{
    //SENSORDB("iPara=%d",iPara);
    switch(iPara)
    {
        case ISP_CONTRAST_LOW:
            T8EV5YUV_write_cmos_sensor(0x0249,0x80);
        break;
        case ISP_CONTRAST_MIDDLE:
            T8EV5YUV_write_cmos_sensor(0x0249,0x88);
        break;
        case ISP_CONTRAST_HIGH:
            T8EV5YUV_write_cmos_sensor(0x0249,0xA7);
        break;
        default:
        break;
    }
} 

void T8EV5YUV_set_ISO(UINT16 iPara)
{ 
    switch(iPara)
    {
        case AE_ISO_AUTO:
        T8EV5YUV_write_cmos_sensor(0x0305,0x01);//-/-/-/-/AGMAX[11:8]
        T8EV5YUV_write_cmos_sensor(0x0306,0xFF);//AGMAX[7:0]
        break;
        case AE_ISO_100:
        T8EV5YUV_write_cmos_sensor(0x0305,0x00);//-/-/-/-/AGMAX[11:8]
        T8EV5YUV_write_cmos_sensor(0x0306,0x67);//AGMAX[7:0]
        break;
        case AE_ISO_200:
        T8EV5YUV_write_cmos_sensor(0x0305,0x00);//-/-/-/-/AGMAX[11:8]
        T8EV5YUV_write_cmos_sensor(0x0306,0xCE);//AGMAX[7:0]
        break;
        case AE_ISO_400:
        T8EV5YUV_write_cmos_sensor(0x0305,0x01);//-/-/-/-/AGMAX[11:8]
        T8EV5YUV_write_cmos_sensor(0x0306,0x97);//AGMAX[7:0]
        break;
        case AE_ISO_800:
        T8EV5YUV_write_cmos_sensor(0x0305,0x02);//-/-/-/-/AGMAX[11:8]
        T8EV5YUV_write_cmos_sensor(0x0306,0x7F);//AGMAX[7:0]
        break;
        case AE_ISO_1600:
        T8EV5YUV_write_cmos_sensor(0x0305,0x04);//-/-/-/-/AGMAX[11:8]
        T8EV5YUV_write_cmos_sensor(0x0306,0xFE);//AGMAX[7:0]
        break;
        default:        
        T8EV5YUV_write_cmos_sensor(0x0305,0x01);//-/-/-/-/AGMAX[11:8]
        T8EV5YUV_write_cmos_sensor(0x0306,0xFF);//AGMAX[7:0]
        break;
    }
}

kal_uint32 T8EV5_set_param_wb(kal_uint32 para)
{


    switch (para)
    {
        case AWB_MODE_AUTO:
            T8EV5YUV_write_cmos_sensor(0x0320,0x81);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8]
            T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0]
            T8EV5YUV_write_cmos_sensor(0x0422,0x5A);//PWBGAINR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0423,0x3B);//PWBGAINB[7:0]

          break;

        case AWB_MODE_CLOUDY_DAYLIGHT: //cloudy           
          T8EV5YUV_write_cmos_sensor(0x0320,0x01);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8];
          T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0];
          T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0];
          T8EV5YUV_write_cmos_sensor(0x0422,0x8B);//PWBGAINR[7:0];
          T8EV5YUV_write_cmos_sensor(0x0423,0x38);//PWBGAINB[7:0];        
          break;

        case AWB_MODE_DAYLIGHT: //sunny            
          T8EV5YUV_write_cmos_sensor(0x0320,0x01);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8];
          T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0];
          T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0];
          T8EV5YUV_write_cmos_sensor(0x0422,0x73);//PWBGAINR[7:0];
          T8EV5YUV_write_cmos_sensor(0x0423,0x48);//PWBGAINB[7:0];            
          break;

        case AWB_MODE_INCANDESCENT: //office
            T8EV5YUV_write_cmos_sensor(0x0320,0x01);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8];
            T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0];
            T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0];
            T8EV5YUV_write_cmos_sensor(0x0422,0x63);//PWBGAINR[7:0];
            T8EV5YUV_write_cmos_sensor(0x0423,0x78);  //PWBGAINB[7:0];
          break;

        case AWB_MODE_TUNGSTEN: //home
          T8EV5YUV_write_cmos_sensor(0x0320,0x01);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8];
          T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0];
          T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0];
          T8EV5YUV_write_cmos_sensor(0x0422,0x48);//PWBGAINR[7:0];
          T8EV5YUV_write_cmos_sensor(0x0423,0x73);//PWBGAINB[7:0];        
          break;

        case AWB_MODE_FLUORESCENT:            
            T8EV5YUV_write_cmos_sensor(0x0320,0x01);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8];
             T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0];
             T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0];
             T8EV5YUV_write_cmos_sensor(0x0422,0x53);//PWBGAINR[7:0];
             T8EV5YUV_write_cmos_sensor(0x0423,0x88);//PWBGAINB[7:0];

          break;
        default:
            return KAL_FALSE;
    }

    return KAL_TRUE;
} 

void T8EV5YUV_set_capture_shutter_gain(UINT16 para)
{
   kal_uint32 cap_es =0,cap_ag=0,temp_val=0;
   kal_uint32 cap_dg=0,sceneMode;
   T8EV5_MODE Camco_mode;


   spin_lock(&t8ev5yuv_drv_lock);
   temp_val=t8ev5yuv_stru.iGain*t8ev5yuv_stru.iShutter;
   Camco_mode=t8ev5yuv_stru.Camco_mode;
   sceneMode=t8ev5yuv_stru.sceneMode;
   cap_dg=t8ev5yuv_stru.d_gain;
   spin_unlock(&t8ev5yuv_drv_lock);
   
 //  printk("%s Read  MES = %d, MAG = %d,   MDG = %d\n",__FUNCTION__,t8ev5yuv_stru.iShutter, t8ev5yuv_stru.iGain, t8ev5yuv_stru.d_gain);
   if(sceneMode==SCENE_MODE_HDR)
   {

       switch (para)
       {
          case AE_EV_COMP_20:
          case AE_EV_COMP_10:
               {
                   SENSORDB("EV * 2 \n");
                   temp_val=temp_val<<2;    
               }
               break;
          case AE_EV_COMP_00:
                {
                  SENSORDB("EV 0 \n");
                }
                break;
          case AE_EV_COMP_n10:
          case AE_EV_COMP_n20:
                {
                  SENSORDB("EV chu 2 \n");
                    temp_val=temp_val>>2;
                }
                 break;
          default:
                 break; 
        }
   }

	#define H_PV (0x110)
	#define V_PV (0x82*8)
	#define PCLK_PV 3
			
	#define H_CP (0x15F)
	#define V_CP (0xF7*8)
	#define PCLK_CP 3
	#define AG_1X 0x38

   if(Camco_mode == T8EV5_MODE_CAPTURE_5M)
   {
		if((temp_val*H_PV/PCLK_PV)>=(H_CP*V_CP*AG_1X/PCLK_CP))//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
		{
			cap_es=V_CP;
			cap_ag=(temp_val*H_PV/PCLK_PV)/(H_CP*V_CP/PCLK_CP);
		}
		else
		{
			cap_es=(temp_val*H_PV/PCLK_PV)/(H_CP*AG_1X/PCLK_CP);//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
			cap_ag=AG_1X;
		}
   }
   else if(Camco_mode == T8EV5_MODE_CAPTURE_OTHERS)
   {
   		if(sceneMode==SCENE_MODE_NIGHTSCENE)
   		{
   			
	   		if(temp_val>=V_PV*AG_1X*3)
	   		{
	   			cap_es=V_PV*3;
				cap_ag=temp_val/V_PV;
	   		}
			else
			{
				cap_es=temp_val/AG_1X;
				cap_ag=AG_1X;
			}
   		}
		else
		{
	   		if(temp_val>=V_PV*AG_1X*2)
	   		{
	   			cap_es=V_PV*2;
				cap_ag=temp_val/V_PV;
	   		}
			else
			{
				cap_es=temp_val/AG_1X;
				cap_ag=AG_1X;
			}
		}
   }

   if(cap_es==0)
   {
   	  cap_es=1; //at least 1 line exposure
   }
   
   printk("T8EV5YUVCapture	 Write	MES = %d, MAG = %d, MDG = %d\n", cap_es, cap_ag, cap_dg);
   	
	T8EV5YUV_write_cmos_sensor(0x0300, 0x00);//-/-/-/-/-/-/ALCSW/ALCLOCK
	T8EV5YUV_write_cmos_sensor(0x0307, ((cap_es>>8)&0xFF));//MES[15:8]
	T8EV5YUV_write_cmos_sensor(0x0308, (cap_es&0xFF));//MES[7:0]
	T8EV5YUV_write_cmos_sensor(0x0309, ((cap_ag>>8)&0x0F));//ESLIMMODE/ROOMDET/-/-/MAG[11:8]
	T8EV5YUV_write_cmos_sensor(0x030A, (cap_ag&0xFF));//MAG[7:0]
	T8EV5YUV_write_cmos_sensor(0x030B, (cap_dg>>2)&0xff);//MDG[7:0]
		
}

kal_uint32 T8EV5_set_param_exposure(kal_uint32 para)
{
    if((t8ev5yuv_stru.sceneMode == SCENE_MODE_HDR)
        &&(t8ev5yuv_stru.Camco_mode != T8EV5_MODE_PREVIEW))
    {
         T8EV5YUV_set_capture_shutter_gain(para);
         return true;
    }
    SENSORDB("para=%d",para);


    switch (para)
    {
        case AE_EV_COMP_n20:
            T8EV5YUV_write_cmos_sensor(0x0302,0x00);
            T8EV5YUV_write_cmos_sensor(0x0303,0xD0);
            
        break;

        case AE_EV_COMP_n15:
            T8EV5YUV_write_cmos_sensor(0x0302,0x00);
            T8EV5YUV_write_cmos_sensor(0x0303,0xE0);

        break;

        case AE_EV_COMP_n10:
            T8EV5YUV_write_cmos_sensor(0x0302,0x00);
            T8EV5YUV_write_cmos_sensor(0x0303,0xF0);

        break;

        case AE_EV_COMP_n05:
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);
            T8EV5YUV_write_cmos_sensor(0x0303,0x00);

        break;

        case AE_EV_COMP_00:
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);
            T8EV5YUV_write_cmos_sensor(0x0303,0x20);

        break;

        case AE_EV_COMP_05:
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);
            T8EV5YUV_write_cmos_sensor(0x0303,0x30);
            
        break;

        case AE_EV_COMP_10:
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);
            T8EV5YUV_write_cmos_sensor(0x0303,0x40);
            
        break;

        case AE_EV_COMP_15:
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);
            T8EV5YUV_write_cmos_sensor(0x0303,0x50);

        break;

        case AE_EV_COMP_20:
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);
            T8EV5YUV_write_cmos_sensor(0x0303,0x60);

        break;

        default:
            return KAL_FALSE;
    }
    return KAL_TRUE;
} 

#define T8EV5YUV_FLASH_BV_THRESHOLD 0x1b2 
static void T8EV5YUVMIPI_FlashTriggerCheck(unsigned int *pFeatureReturnPara32)
{
    unsigned int NormBr;       

	//zh add for capture with flash 20130822
	if(g_sensorAfMode==0)
	{
		msleep(200);
	}
	
	NormBr=((T8EV5YUV_read_cmos_sensor(0x0361)&0xff)<<8)+T8EV5YUV_read_cmos_sensor(0x0362);	
    
    SENSORDB("[%s]gain =0x%x \n",__FUNCTION__,NormBr);
	
	
	if (NormBr < T8EV5YUV_FLASH_BV_THRESHOLD)
	{
	   *pFeatureReturnPara32 = FALSE;
	   is_flash_on=0;
		return;
	}
	is_flash_on=1;
	*pFeatureReturnPara32 = TRUE;
	return;
}

void T8EV5YUVSetSceneMode(UINT16 iPara)
{
    //SENSORDB("iPara=%d",iPara);

    //zh add for HDR 20130618
    spin_lock(&t8ev5yuv_drv_lock);
    t8ev5yuv_stru.sceneMode=iPara;
    spin_unlock(&t8ev5yuv_drv_lock);

    switch(iPara)
    {
        case SCENE_MODE_HDR:
         SENSORDB("------tl------setSceneMode HDR iPara = %d , SCENE_MODE_HDR = %d\n",iPara,SCENE_MODE_HDR);
         
         //zh add for HDR 20130618
         spin_lock(&t8ev5yuv_drv_lock);
         t8ev5yuv_stru.iShutter= 0;
         t8ev5yuv_stru.iGain= 0;
         t8ev5yuv_stru.d_gain=0;
         spin_unlock(&t8ev5yuv_drv_lock);
         break;

        case SCENE_MODE_OFF:
            T8EV5YUV_write_cmos_sensor(0x0107,0x00);//- / - / - / - / - / V_COUNT[10:8] 
            T8EV5YUV_write_cmos_sensor(0x0108,0x82);//V_COUNT[7:0] 
            T8EV5YUV_write_cmos_sensor(0x0229,0x2B);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
            T8EV5YUV_write_cmos_sensor(0x0249,0x88);//CONT_LEV[7:0]
            T8EV5YUV_write_cmos_sensor(0x0315,0x98);//FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]//0x98
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);//-/-/-/-/-/-/ALCAIM[9:8]
            T8EV5YUV_write_cmos_sensor(0x0303,0x20);//ALCAIM[7:0]
    #if 0
    T8EV5YUV_write_cmos_sensor(0x030C,0x55); //A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
    T8EV5YUV_write_cmos_sensor(0x030D,0x5A); //A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
    T8EV5YUV_write_cmos_sensor(0x030E,0x95); //B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
    T8EV5YUV_write_cmos_sensor(0x030F,0x50); //C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-

    #endif
			
            T8EV5YUV_write_cmos_sensor(0x0320,0x81);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8]
            T8EV5YUV_write_cmos_sensor(0x0321,0x00);//WBMRG[7:0]
            T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0]
            T8EV5YUV_write_cmos_sensor(0x0422,0x5A);//PWBGAINR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0423,0x3B);//PWBGAINB[7:0]

            if(T8EV5YUV_MPEG4_encode_mode==KAL_TRUE)
            {
                T8EV5YUV_write_cmos_sensor(0x0315,0x08);  //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]

            }
        break;
        case SCENE_MODE_NIGHTSCENE:
            T8EV5YUV_write_cmos_sensor(0x0107,0x00);//- / - / - / - / - / V_COUNT[10:8] 
            T8EV5YUV_write_cmos_sensor(0x0108,0x82);//V_COUNT[7:0] 
            T8EV5YUV_write_cmos_sensor(0x0229,0x2B);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
            T8EV5YUV_write_cmos_sensor(0x0249,0x88);//CONT_LEV[7:0]
            T8EV5YUV_write_cmos_sensor(0x0315,0xb8);//FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);//-/-/-/-/-/-/ALCAIM[9:8]
            T8EV5YUV_write_cmos_sensor(0x0303,0x20);//ALCAIM[7:0]
            #if 0
            T8EV5YUV_write_cmos_sensor(0x030C,0x55);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030D,0x5A);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030E,0x95);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030F,0x50);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
            #endif
            T8EV5YUV_write_cmos_sensor(0x0320,0x81);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8]
            T8EV5YUV_write_cmos_sensor(0x0321,0x00);//WBMRG[7:0]
            T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0]
            T8EV5YUV_write_cmos_sensor(0x0422,0x5A);//PWBGAINR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0423,0x3B);//PWBGAINB[7:0]
            if(T8EV5YUV_MPEG4_encode_mode==KAL_TRUE)
            {
                T8EV5YUV_write_cmos_sensor(0x0315,0x98);  //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]

            }
        break;

        case SCENE_MODE_PORTRAIT:
            T8EV5YUV_write_cmos_sensor(0x0107,0x00);//- / - / - / - / - / V_COUNT[10:8] 
            T8EV5YUV_write_cmos_sensor(0x0108,0x82);//V_COUNT[7:0]
            T8EV5YUV_write_cmos_sensor(0x0229,0x2D);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
            T8EV5YUV_write_cmos_sensor(0x0249,0x88);//CONT_LEV[7:0]
            T8EV5YUV_write_cmos_sensor(0x0315,0x98);//FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);//-/-/-/-/-/-/ALCAIM[9:8]
            T8EV5YUV_write_cmos_sensor(0x0303,0x30);//ALCAIM[7:0]
			#if 0
            T8EV5YUV_write_cmos_sensor(0x030C,0x55);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030D,0x5F);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030E,0xD5);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030F,0x50);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
			#endif
            T8EV5YUV_write_cmos_sensor(0x0320,0x81);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8]
            T8EV5YUV_write_cmos_sensor(0x0321,0x00);//WBMRG[7:0]
            T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0]
            T8EV5YUV_write_cmos_sensor(0x0422,0x5A);//PWBGAINR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0423,0x3B);//PWBGAINB[7:0]
            if(T8EV5YUV_MPEG4_encode_mode==KAL_TRUE)
            {
                T8EV5YUV_write_cmos_sensor(0x0315,0x08);  //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]

            }

        break;
        case SCENE_MODE_LANDSCAPE:
            T8EV5YUV_write_cmos_sensor(0x0107,0x00);//- / - / - / - / - / V_COUNT[10:8] 
            T8EV5YUV_write_cmos_sensor(0x0108,0x82);//V_COUNT[7:0]
            T8EV5YUV_write_cmos_sensor(0x0229,0x2D);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
            T8EV5YUV_write_cmos_sensor(0x0249,0x88);//CONT_LEV[7:0]
            T8EV5YUV_write_cmos_sensor(0x0315,0x98);//FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);//-/-/-/-/-/-/ALCAIM[9:8]
            T8EV5YUV_write_cmos_sensor(0x0303,0x20);//ALCAIM[7:0]
			#if 0
            T8EV5YUV_write_cmos_sensor(0x030C,0x55);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030D,0x5A);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030E,0xBF);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030F,0xF0);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
			#endif
            T8EV5YUV_write_cmos_sensor(0x0320,0x81);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8]
            T8EV5YUV_write_cmos_sensor(0x0321,0x00);//WBMRG[7:0]
            T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0]
            T8EV5YUV_write_cmos_sensor(0x0422,0x5A);//PWBGAINR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0423,0x3B);//PWBGAINB[7:0]
            if(T8EV5YUV_MPEG4_encode_mode==KAL_TRUE)
            {
                T8EV5YUV_write_cmos_sensor(0x0315,0x08);  //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]

            }
            
        break;
        case SCENE_MODE_SPORTS:
            T8EV5YUV_write_cmos_sensor(0x0107,0x00);//- / - / - / - / - / V_COUNT[10:8] 
            T8EV5YUV_write_cmos_sensor(0x0108,0x82);//V_COUNT[7:0]
            T8EV5YUV_write_cmos_sensor(0x0229,0x2B);//Cbr_MGAIN1[3:0]/Cbr_MGAIN0[3:0]
            T8EV5YUV_write_cmos_sensor(0x0249,0x88);//CONT_LEV[7:0]
            T8EV5YUV_write_cmos_sensor(0x0315,0x08);//FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]
            T8EV5YUV_write_cmos_sensor(0x0302,0x01);//-/-/-/-/-/-/ALCAIM[9:8]
            T8EV5YUV_write_cmos_sensor(0x0303,0x20);//ALCAIM[7:0]
			#if 0
            T8EV5YUV_write_cmos_sensor(0x030C,0x55);//A1WEIGHT[1:0]/A2WEIGHT[1:0]/A3WEIGHT[1:0]/A4WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030D,0x5A);//A5WEIGHT[1:0]/B1WEIGHT[1:0]/B2WEIGHT[1:0]/B3WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030E,0x95);//B4WEIGHT[1:0]/B5WEIGHT[1:0]/C1WEIGHT[1:0]/C2WEIGHT[1:0]
            T8EV5YUV_write_cmos_sensor(0x030F,0x50);//C3WEIGHT[1:0]/C4WEIGHT[1:0]/C5WEIGHT[1:0]/-/-
			#endif
            T8EV5YUV_write_cmos_sensor(0x0320,0x81);//AWBSW/AWBONDOT[2:0]/-/-/WBMRG[9:8]
            T8EV5YUV_write_cmos_sensor(0x0321,0x00);//WBMRG[7:0]
            T8EV5YUV_write_cmos_sensor(0x0420,0x00);//PWBGAINGR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0421,0x00);//PWBGAINGB[7:0]
            T8EV5YUV_write_cmos_sensor(0x0422,0x5A);//PWBGAINR[7:0]
            T8EV5YUV_write_cmos_sensor(0x0423,0x3B);//PWBGAINB[7:0]
            if(T8EV5YUV_MPEG4_encode_mode==KAL_TRUE)
            {
                T8EV5YUV_write_cmos_sensor(0x0315,0x08);  //FAUTO/FCOUNT[2:0]/FCLSBON/EXPLIM[2:0]

            }
            
        break;
        default:
        break;
    }   
}



kal_uint32 T8EV5_set_param_effect(kal_uint32 para)
{
    kal_uint32 ret = KAL_TRUE;



    switch (para)
    {
        case MEFFECT_OFF:            
            T8EV5YUV_write_cmos_sensor(0x0115,0x00);//-/-/-/-/PICEFF[3:0];
            break;

        case MEFFECT_MONO:
            T8EV5YUV_write_cmos_sensor(0x0115,0x07);//-/-/-/-/PICEFF[3:0];
            break;
            
        case MEFFECT_SEPIA:      
            T8EV5YUV_write_cmos_sensor(0x0115,0x05);//-/-/-/-/PICEFF[3:0];
            break;

        case MEFFECT_NEGATIVE:
            T8EV5YUV_write_cmos_sensor(0x0115,0x03);//-/-/-/-/PICEFF[3:0];
            break;

        case MEFFECT_SEPIAGREEN:   
            T8EV5YUV_write_cmos_sensor(0x0115,0x04);//-/-/-/-/PICEFF[3:0];
            T8EV5YUV_write_cmos_sensor(0x0258,0x55);//UNICOFSU[7:0];
            T8EV5YUV_write_cmos_sensor(0x0259,0x49);//UNICOFSV[7:0];
            break;
 /*           
         case CAM_EFFECT_ENC_REDDISH:           
            T8EV5YUV_write_cmos_sensor(0x3212, 0x03);
            T8EV5YUV_write_cmos_sensor(0x5580, 0x1e);
            T8EV5YUV_write_cmos_sensor(0x5583, 0x80);
            T8EV5YUV_write_cmos_sensor(0x5584, 0xc0);
            T8EV5YUV_write_cmos_sensor(0x5003, 0x08);
            T8EV5YUV_write_cmos_sensor(0x3212, 0x13);
            T8EV5YUV_write_cmos_sensor(0x3212, 0xa3);           
            break;
*/
        case MEFFECT_SEPIABLUE:   
            T8EV5YUV_write_cmos_sensor(0x0115,0x04);//-/-/-/-/PICEFF[3:0];
            T8EV5YUV_write_cmos_sensor(0x0258,0xC0);//UNICOFSU[7:0];
            T8EV5YUV_write_cmos_sensor(0x0259,0x49);//UNICOFSV[7:0];        
            break;
 
/*
        case CAM_EFFECT_ENC_GRAYINV:
        case CAM_EFFECT_ENC_COPPERCARVING:
        case CAM_EFFECT_ENC_BLUECARVING:
        case CAM_EFFECT_ENC_CONTRAST:
        case CAM_EFFECT_ENC_EMBOSSMENT:
        case CAM_EFFECT_ENC_SKETCH:
        case CAM_EFFECT_ENC_BLACKBOARD:
        case CAM_EFFECT_ENC_WHITEBOARD:
        case CAM_EFFECT_ENC_JEAN:
        case CAM_EFFECT_ENC_OIL:
*/
        default:
            ret = KAL_FALSE;
    return ret;
    }

    return KAL_TRUE;
} 

kal_uint32 T8EV5_set_param_banding(kal_uint32 para)
{

    switch (para)
    {
        case AE_FLICKER_MODE_50HZ:
            T8EV5YUV_write_cmos_sensor(0x0318,0x16);//ACFDET/AC60M/FLMANU/ACDETDLY/MSKLINE[1:0]/ACDPWAIT[1:0]
            break;

        case AE_FLICKER_MODE_60HZ:
            T8EV5YUV_write_cmos_sensor(0x0318,0x56);//ACFDET/AC60M/FLMANU/ACDETDLY/MSKLINE[1:0]/ACDPWAIT[1:0]
            break;
        case AE_FLICKER_MODE_AUTO:
        case AE_FLICKER_MODE_OFF:
            T8EV5YUV_write_cmos_sensor(0x0318,0xD6);//ACFDET/AC60M/FLMANU/ACDETDLY/MSKLINE[1:0]/ACDPWAIT[1:0]
            break;
        default:
              return KAL_FALSE;
    }
    return KAL_TRUE;
} 

kal_uint32 T8EV5_set_param_af_mode(kal_uint32 para)
{
    T8EV5_FOCUS_Cancel_Focus();
    switch (para)
    {
        case AF_MODE_AFS:
            T8EV5_FOCUS_Single_Focus();
            SENSORDB("yxw yuvsensorsetting T8EV5_FOCUS_Single_Focus() is called\n");
            //printk("\n [robin_camera]T8EV5_FOCUS_Single_Focus() is called ; \n");
            break;
        case AF_MODE_AFC:
            T8EV5_FOCUS_Constant_Focus();
            SENSORDB("yxw yuvsensorsetting T8EV5_FOCUS_Constant_Focus() is called\n");
            //printk("\n  [robin_camera]T8EV5_FOCUS_Constant_Focus() is called ; \n");
            break;
     default:
        return KAL_FALSE;
    }
    return KAL_TRUE;
} 

UINT32 T8EV5YUVSensorSetting(FEATURE_ID iCmd, UINT32 iPara)
{
   // printk("\n T8EV5YUVSensorSetting() is called ; \n");
    SENSORDB("T8EV5YUVSensorSetting() is called,cmd=%d, para = 0x%x\n", iCmd, iPara);

    switch (iCmd) {
    case FID_SCENE_MODE:
        SENSORDB("yxw yuvsensorsetting scene mode 1\n");       
        T8EV5YUVSetSceneMode(iPara);
        break;      
    case FID_AWB_MODE:
        SENSORDB("yxw yuvsensorsetting awb mode 2\n");
        T8EV5_set_param_wb(iPara);
    break;
    case FID_COLOR_EFFECT:
        SENSORDB("yxw yuvsensorsetting color effect mode 3\n");
        T8EV5_set_param_effect(iPara);
    break;
    
    case FID_ISP_SAT:
        T8EV5YUV_set_saturation(iPara);
    break;

    case FID_ISP_BRIGHT:
        T8EV5YUV_set_brightness(iPara);
    break;

    case FID_ISP_CONTRAST:
        T8EV5YUV_set_contrast(iPara);
    break;

    case FID_AE_EV:
        SENSORDB("yxw yuvsensorsetting AE_EV mode 4\n");
        T8EV5_set_param_exposure(iPara);
    break;
    case FID_AE_FLICKER:
        SENSORDB("yxw yuvsensorsetting AE flicker mode 5\n");
        T8EV5_set_param_banding(iPara);
    break;
    
    case FID_AE_ISO:
        T8EV5YUV_set_ISO(iPara);
    break;

    case FID_ZOOM_FACTOR:
        SENSORDB("yxw yuvsensorsetting zoom factor mode 6\n");
            T8EV5YUV_zoom_factor = iPara;       
    break;
    case FID_AF_MODE:
        SENSORDB("yxw yuvsensorsetting AF mode 7\n");
        T8EV5_set_param_af_mode(iPara);
    break;
    
    default:
    break;
    }

    return TRUE;

}


/*************************************************************************
* FUNCTION
*   T8EV5YUVGetSensorID
*
* DESCRIPTION
*   This function get the sensor ID 
*
* PARAMETERS
*   *sensorID : return the sensor ID 
*
* RETURNS
*   None
*
* GLOBALS AFFECTED
*
*************************************************************************/
UINT32 T8EV5YUVGetSensorID(UINT32 *sensorID) 
{
    int  retry = 3;
    UINT32 u_sensorid1,u_sensorid2;
    
    // check if sensor ID correct
    do {
       // *sensorID = ((T8EV5YUV_read_cmos_sensor(0x0000) << 8) | T8EV5YUV_read_cmos_sensor(0x0001));     
       u_sensorid1 = T8EV5YUV_read_cmos_sensor(0x0000);
       u_sensorid2 = T8EV5YUV_read_cmos_sensor(0x0001);
        printk("T8EV5YUVGetSensorID Read Sensor ID1,ID2  = %x,%x\n", u_sensorid1,u_sensorid2);   
        *sensorID =   (((u_sensorid1&0XFF) << 8 )| (u_sensorid2&0XFF));
        if (*sensorID == T8EV5MIPI_SENSOR_ID)
            break; 
        printk("T8EV5YUVGetSensorID Read Sensor ID Fail = 0x%04x\n", *sensorID); 
        retry--; 
    } while (retry > 0);
        printk(" T8EV5YUVGetSensorID Read Sensor ID = 0x%04x\n", *sensorID); 
    if (*sensorID != T8EV5MIPI_SENSOR_ID) {
        *sensorID = 0xFFFFFFFF; 
        return ERROR_SENSOR_CONNECT_FAIL;
    }
    return ERROR_NONE;
}
UINT32 T8EV5YUVFeatureControl(MSDK_SENSOR_FEATURE_ENUM FeatureId,
                                                                UINT8 *pFeaturePara,UINT32 *pFeatureParaLen)
{    
    UINT8   *pFeatureData8 =pFeaturePara;
    
    UINT16 *pFeatureReturnPara16=(UINT16 *) pFeaturePara;
    UINT16 *pFeatureData16=(UINT16 *) pFeaturePara;
    UINT32 *pFeatureReturnPara32=(UINT32 *) pFeaturePara;
    UINT32 *pFeatureData32=(UINT32 *) pFeaturePara;
    UINT32 SensorRegNumber;
    UINT32 i;
    PNVRAM_SENSOR_DATA_STRUCT pSensorDefaultData=(PNVRAM_SENSOR_DATA_STRUCT) pFeaturePara;
    MSDK_SENSOR_CONFIG_STRUCT *pSensorConfigData=(MSDK_SENSOR_CONFIG_STRUCT *) pFeaturePara;
    MSDK_SENSOR_REG_INFO_STRUCT *pSensorRegData=(MSDK_SENSOR_REG_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_GROUP_INFO_STRUCT *pSensorGroupInfo=(MSDK_SENSOR_GROUP_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ITEM_INFO_STRUCT *pSensorItemInfo=(MSDK_SENSOR_ITEM_INFO_STRUCT *) pFeaturePara;
    MSDK_SENSOR_ENG_INFO_STRUCT *pSensorEngInfo=(MSDK_SENSOR_ENG_INFO_STRUCT *) pFeaturePara;

    switch (FeatureId)
    {

#if WINMO_USE
        case SENSOR_FEATURE_GET_INIT_OPERATION_PARA:
        {
            PCAMERA_DRIVER_OPERATION_PARA_STRUCT pSensorOperData;
            pSensorOperData = (PCAMERA_DRIVER_OPERATION_PARA_STRUCT)pFeaturePara;

            pSensorOperData->CaptureDelayFrame = 2;         /* wait stable frame when sensor change mode (pre to cap) */
            pSensorOperData->PreviewDelayFrame = 3;         /* wait stable frame when sensor change mode (cap to pre) */
            pSensorOperData->PreviewDisplayWaitFrame = 2;   /* wait stable frame when sensor change mode (cap to pre) */
            pSensorOperData->AECalDelayFrame = 0;               /* The frame of calculation default 0 */
            pSensorOperData->AEShutDelayFrame = 0;              /* The frame of setting shutter default 0 for TG int */
            pSensorOperData->AESensorGainDelayFrame = 1;    /* The frame of setting sensor gain */
            pSensorOperData->AEISPGainDelayFrame = 2;          /* The frame of setting gain */
            pSensorOperData->AECalPeriod = 3;                           /* AE AWB calculation period */
            pSensorOperData->FlashlightMode=FLASHLIGHT_LED_CONSTANT;

            break;
        }
#endif 

        case SENSOR_FEATURE_GET_RESOLUTION:
            *pFeatureReturnPara16++=IMAGE_SENSOR_FULL_WIDTH;
            *pFeatureReturnPara16=IMAGE_SENSOR_FULL_HEIGHT;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PERIOD:
            *pFeatureReturnPara16++=T8EV5_PV_PERIOD_EXTRA_PIXEL_NUMS + T8EV5_PV_PERIOD_PIXEL_NUMS + T8EV5YUV_dummy_pixels;//T8EV5_PV_PERIOD_PIXEL_NUMS+T8EV5YUV_dummy_pixels;
            *pFeatureReturnPara16=T8EV5_PV_PERIOD_LINE_NUMS+T8EV5YUV_dummy_lines;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_PIXEL_CLOCK_FREQ:
            *pFeatureReturnPara32 = 55250000; //19500000;
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_SET_ESHUTTER:
            T8EV5YUV_SetShutter(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_NIGHTMODE:
            T8EV5YUV_NightMode((BOOL) *pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_GAIN:
            T8EV5YUV_SetGain((UINT16) *pFeatureData16);
            break;
     case SENSOR_FEATURE_GET_TRIGGER_FLASHLIGHT_INFO:
            T8EV5YUVMIPI_FlashTriggerCheck(pFeatureData32);
            SENSORDB("[T8EV5] F_GET_TRIGGER_FLASHLIGHT_INFO: %d\n", pFeatureData32);
            break;      
        
        case SENSOR_FEATURE_SET_FLASHLIGHT:
            break;
        case SENSOR_FEATURE_SET_ISP_MASTER_CLOCK_FREQ:
            T8EV5YUV_isp_master_clock=*pFeatureData32;
            break;
        case SENSOR_FEATURE_SET_REGISTER:
            T8EV5YUV_write_cmos_sensor(pSensorRegData->RegAddr, pSensorRegData->RegData);
            break;
        case SENSOR_FEATURE_GET_REGISTER:
            pSensorRegData->RegData = T8EV5YUV_read_cmos_sensor(pSensorRegData->RegAddr);
            break;
        case SENSOR_FEATURE_SET_CCT_REGISTER:
        SENSORDB("SENSOR_FEATURE_SET_CCT_REGISTER\n");
            SensorRegNumber=FACTORY_END_ADDR;
            for (i=0;i<SensorRegNumber;i++)
            {
                T8EV5YUVSensorCCT[i].Addr=*pFeatureData32++;
                T8EV5YUVSensorCCT[i].Para=*pFeatureData32++;
            }
            break;
        case SENSOR_FEATURE_GET_CCT_REGISTER:
        SENSORDB("SENSOR_FEATURE_GET_CCT_REGISTER\n");
            SensorRegNumber=FACTORY_END_ADDR;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=T8EV5YUVSensorCCT[i].Addr;
                *pFeatureData32++=T8EV5YUVSensorCCT[i].Para;
            }
            break;
        case SENSOR_FEATURE_SET_ENG_REGISTER:
        SENSORDB("SENSOR_FEATURE_SET_ENG_REGISTER\n");
            SensorRegNumber=ENGINEER_END;
            for (i=0;i<SensorRegNumber;i++)
            {
                T8EV5YUVSensorReg[i].Addr=*pFeatureData32++;
                T8EV5YUVSensorReg[i].Para=*pFeatureData32++;
            }
            break;
        case SENSOR_FEATURE_GET_ENG_REGISTER:
        SENSORDB("SENSOR_FEATURE_GET_ENG_REGISTER\n");
            SensorRegNumber=ENGINEER_END;
            if (*pFeatureParaLen<(SensorRegNumber*sizeof(SENSOR_REG_STRUCT)+4))
                return FALSE;
            *pFeatureData32++=SensorRegNumber;
            for (i=0;i<SensorRegNumber;i++)
            {
                *pFeatureData32++=T8EV5YUVSensorReg[i].Addr;
                *pFeatureData32++=T8EV5YUVSensorReg[i].Para;
            }
            break;
        case SENSOR_FEATURE_GET_REGISTER_DEFAULT:
        SENSORDB("SENSOR_FEATURE_GET_REGISTER_DEFAULT\n");
            if (*pFeatureParaLen>=sizeof(NVRAM_SENSOR_DATA_STRUCT))
            {
                pSensorDefaultData->Version=NVRAM_CAMERA_SENSOR_FILE_VERSION;
                pSensorDefaultData->SensorId=T8EV5MIPI_SENSOR_ID;
                memcpy(pSensorDefaultData->SensorEngReg, T8EV5YUVSensorReg, sizeof(SENSOR_REG_STRUCT)*ENGINEER_END);
                memcpy(pSensorDefaultData->SensorCCTReg, T8EV5YUVSensorCCT, sizeof(SENSOR_REG_STRUCT)*FACTORY_END_ADDR);
            }
            else
                return FALSE;
            *pFeatureParaLen=sizeof(NVRAM_SENSOR_DATA_STRUCT);
            break;
        case SENSOR_FEATURE_GET_CONFIG_PARA:
        SENSORDB("SENSOR_FEATURE_GET_CONFIG_PARA\n");
            memcpy(pSensorConfigData, &T8EV5YUVSensorConfigData, sizeof(MSDK_SENSOR_CONFIG_STRUCT));
            *pFeatureParaLen=sizeof(MSDK_SENSOR_CONFIG_STRUCT);
            break;
        case SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR:
        SENSORDB("SENSOR_FEATURE_CAMERA_PARA_TO_SENSOR\n");
            T8EV5YUV_camera_para_to_sensor();
            break;

        case SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA:
        SENSORDB("SENSOR_FEATURE_SENSOR_TO_CAMERA_PARA\n");
            T8EV5YUV_sensor_to_camera_para();
            break;
        case SENSOR_FEATURE_GET_GROUP_COUNT:
        SENSORDB("SENSOR_FEATURE_GET_GROUP_COUNT\n");
            *pFeatureReturnPara32++=T8EV5YUV_get_sensor_group_count();
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_GROUP_INFO:
        SENSORDB("SENSOR_FEATURE_GET_GROUP_INFO\n");
            T8EV5YUV_get_sensor_group_info(pSensorGroupInfo->GroupIdx, pSensorGroupInfo->GroupNamePtr, &pSensorGroupInfo->ItemCount);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_GROUP_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_ITEM_INFO:
        SENSORDB("SENSOR_FEATURE_GET_ITEM_INFO\n");
            T8EV5YUV_get_sensor_item_info(pSensorItemInfo->GroupIdx,pSensorItemInfo->ItemIdx, pSensorItemInfo);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_SET_ITEM_INFO:
        SENSORDB("SENSOR_FEATURE_SET_ITEM_INFO\n");
            T8EV5YUV_set_sensor_item_info(pSensorItemInfo->GroupIdx, pSensorItemInfo->ItemIdx, pSensorItemInfo->ItemValue);
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ITEM_INFO_STRUCT);
            break;

        case SENSOR_FEATURE_GET_ENG_INFO:
        SENSORDB("SENSOR_FEATURE_GET_ENG_INFO\n");
            pSensorEngInfo->SensorId = 129;
            pSensorEngInfo->SensorType = CMOS_SENSOR;
            //test by lingnan
            //pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_RAW_B;
            pSensorEngInfo->SensorOutputDataFormat=SENSOR_OUTPUT_FORMAT_YUYV;
            *pFeatureParaLen=sizeof(MSDK_SENSOR_ENG_INFO_STRUCT);
            break;
        case SENSOR_FEATURE_GET_LENS_DRIVER_ID:
        SENSORDB("SENSOR_FEATURE_GET_LENS_DRIVER_ID\n");
            // get the lens driver ID from EEPROM or just return LENS_DRIVER_ID_DO_NOT_CARE
            // if EEPROM does not exist in camera module.
            *pFeatureReturnPara32=LENS_DRIVER_ID_DO_NOT_CARE;
            *pFeatureParaLen=4;
            break;

        case SENSOR_FEATURE_INITIALIZE_AF:
        SENSORDB("yxw SENSOR_FEATURE_INITIALIZE_AF\n");
            //SENSORDB("T8EV5_FOCUS_Init\n");
            T8EV5_FOCUS_Init();
            break;
        case SENSOR_FEATURE_CONSTANT_AF:
        SENSORDB("yxw SENSOR_FEATURE_CONSTANT_AF\n");
            //SENSORDB("T8EV5_FOCUS_Constant_Focus\n");
         //printk("kiwi-T8EV5_FOCUS_Constant_Focus\n");
            T8EV5_FOCUS_Constant_Focus();
            break;
        case SENSOR_FEATURE_MOVE_FOCUS_LENS:
        SENSORDB("yxw SENSOR_FEATURE_MOVE_FOCUS_LENS\n");
            //SENSORDB("T8EV5_FOCUS_Move_to %d\n", *pFeatureData16);
            T8EV5_FOCUS_Move_to(*pFeatureData16);
            break;
        case SENSOR_FEATURE_GET_AF_STATUS:
        SENSORDB("yxw SENSOR_FEATURE_GET_AF_STATUS\n");
            //for yuv use:
            //SENSORDB("SENSOR_FEATURE_GET_AF_STATUS pFeatureReturnPara32=0x%x\n",pFeatureReturnPara32);
            T8EV5_FOCUS_Get_AF_Status(pFeatureReturnPara32);
            *pFeatureParaLen=4;
            break;
        case SENSOR_FEATURE_GET_AF_INF:
        SENSORDB("yxw SENSOR_FEATURE_GET_AF_INF\n");
            T8EV5_FOCUS_Get_AF_Inf(pFeatureReturnPara32);
            *pFeatureParaLen=4;            
            break;
        case SENSOR_FEATURE_GET_AF_MACRO:
        SENSORDB("yxw SENSOR_FEATURE_GET_AF_MACRO\n");
            T8EV5_FOCUS_Get_AF_Macro(pFeatureReturnPara32);
            *pFeatureParaLen=4;            
            break;                
        case SENSOR_FEATURE_SET_VIDEO_MODE:
        SENSORDB("yxw SENSOR_FEATURE_SET_VIDEO_MODE\n");
            T8EV5YUVSetVideoMode(*pFeatureData16);
            break;
        case SENSOR_FEATURE_SET_YUV_CMD:
        SENSORDB("yxw SENSOR_FEATURE_SET_YUV_CMD\n");
            T8EV5YUVSensorSetting((FEATURE_ID)*pFeatureData32, *(pFeatureData32+1));    
            break;
        case SENSOR_FEATURE_CHECK_SENSOR_ID:
        SENSORDB("SENSOR_FEATURE_CHECK_SENSOR_ID\n");
            T8EV5YUVGetSensorID(pFeatureReturnPara32); 
            break;                          
        case SENSOR_FEATURE_SINGLE_FOCUS_MODE:
            SENSORDB("yxw SENSOR_FEATURE_SINGLE_FOCUS_MODE\n");
            T8EV5_FOCUS_Single_Focus();
            break;      
        case SENSOR_FEATURE_CANCEL_AF:
            SENSORDB("yxw SENSOR_FEATURE_CANCEL_AF\n");
            T8EV5_FOCUS_Cancel_Focus();
            break;          
    case SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS:
        SENSORDB("yxw SENSOR_FEATURE_GET_AF_MAX_NUM_FOCUS_AREAS\n");
        *pFeatureReturnPara32 = 1;
        *pFeatureParaLen = 4;
        //printk("AF *pFeatureReturnPara32 = %d\n",*pFeatureReturnPara32);
         break;
    case SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS:
        SENSORDB("yxw SENSOR_FEATURE_GET_AE_MAX_NUM_METERING_AREAS\n");
        *pFeatureReturnPara32 = 1;
        *pFeatureParaLen = 4;
        //printk("AE *pFeatureReturnPara32 = %d\n",*pFeatureReturnPara32);
            break;

    case SENSOR_FEATURE_SET_AE_WINDOW:
            SENSORDB("yxw SENSOR_FEATURE_SET_AE_WINDOW\n");
           // printk("hwj SENSOR_FEATURE_SET_AE_WINDOW");
            SENSORDB("get zone addr = 0x%x\n",*pFeatureData32);         
            T8EV5_FOCUS_Set_AE_Window(*pFeatureData32);
            break;  
        case SENSOR_FEATURE_SET_AF_WINDOW:
            SENSORDB("yxw SENSOR_FEATURE_SET_AF_WINDOW\n");
           // printk("hwj SENSOR_FEATURE_SET_AF_WINDOW");
            SENSORDB("get zone addr = 0x%x\n",*pFeatureData32);         
            T8EV5_FOCUS_Set_AF_Window(*pFeatureData32);
            break;  
        
        default:
            break;
    }
    return ERROR_NONE;
}   /* T8EV5YUVFeatureControl() */

SENSOR_FUNCTION_STRUCT  SensorFuncT8EV5YUV=
{
    T8EV5YUVOpen,
    T8EV5YUVGetInfo,
    T8EV5YUVGetResolution,
    T8EV5YUVFeatureControl,
    T8EV5YUVControl,
    T8EV5YUVClose
};

UINT32 T8EV5_MIPI_YUV_SensorInit(PSENSOR_FUNCTION_STRUCT *pfFunc)
{
    /* To Do : Check Sensor status here */
    if (pfFunc!=NULL)
        *pfFunc=&SensorFuncT8EV5YUV;

    return ERROR_NONE;
}   /* T8EV5_YUV_SensorInit() */



