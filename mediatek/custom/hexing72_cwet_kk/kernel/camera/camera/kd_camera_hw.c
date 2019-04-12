#include <linux/videodev2.h>
#include <linux/i2c.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/cdev.h>
#include <linux/uaccess.h>
#include <linux/fs.h>
#include <asm/atomic.h>
#include <linux/xlog.h>
#include <linux/kernel.h>

#include "kd_camera_hw.h"

#include "kd_imgsensor.h"
#include "kd_imgsensor_define.h"
#include "kd_camera_feature.h"

/******************************************************************************
 * Debug configuration
******************************************************************************/
#define PFX "[kd_camera_hw]"
#define PK_DBG_NONE(fmt, arg...)    do {} while (0)
#define PK_DBG_FUNC printk

#define DEBUG_CAMERA_HW_K
#ifdef DEBUG_CAMERA_HW_K
#define PK_DBG PK_DBG_FUNC
#define PK_ERR(fmt, arg...)         xlog_printk(ANDROID_LOG_ERR, PFX , fmt, ##arg)
#define PK_XLOG_INFO(fmt, args...) \
                do {    \
                   xlog_printk(ANDROID_LOG_INFO, PFX , fmt, ##arg); \
                } while(0)
#else
#define PK_DBG(a,...)
#define PK_ERR(a,...)
#define PK_XLOG_INFO(fmt, args...)
#endif

#define IDX_PS_CMRST 0
#define IDX_PS_CMPDN 4

#define IDX_PS_MODE 1
#define IDX_PS_ON   2
#define IDX_PS_OFF  3

static u32 pinSet[2][8] = {
                    //for main sensor
                    {GPIO_CAMERA_CMRST_PIN,
                        GPIO_CAMERA_CMRST_PIN_M_GPIO,   /* mode */
                        GPIO_OUT_ONE,                   /* ON state */
                        GPIO_OUT_ZERO,                  /* OFF state */
                     GPIO_CAMERA_CMPDN_PIN,
                        GPIO_CAMERA_CMPDN_PIN_M_GPIO,
                        GPIO_OUT_ONE,
                        GPIO_OUT_ZERO,
                    },
                    //for sub sensor
                  {GPIO_CAMERA_CMRST1_PIN,
                      GPIO_CAMERA_CMRST1_PIN_M_GPIO,
                      GPIO_OUT_ONE,
                      GPIO_OUT_ZERO,
                   GPIO_CAMERA_CMPDN1_PIN,
                      GPIO_CAMERA_CMPDN1_PIN_M_GPIO,
                      GPIO_OUT_ONE,
                      GPIO_OUT_ZERO,
                  }
                 };


void changge_ctr(int id, int rst , int pwd)
{
	id = (id>0)?1:0 ;
	pinSet[id][IDX_PS_CMRST+IDX_PS_ON] = (rst==0)?1:0 ; 
	pinSet[id][IDX_PS_CMPDN+IDX_PS_ON] = (pwd==0)?1:0 ; 
	pinSet[id][IDX_PS_CMRST+IDX_PS_OFF] = (rst==0)?0:1 ; 
	pinSet[id][IDX_PS_CMPDN+IDX_PS_OFF] = (pwd==0)?0:1 ;				
}



int kdCISModulePowerOn(CAMERA_DUAL_CAMERA_SENSOR_ENUM SensorIdx, char *currSensorName, BOOL On, char* mode_name)
{
	u32 pinSetIdx = 0;//default main sensor

    if (DUAL_CAMERA_MAIN_SENSOR == SensorIdx){
        pinSetIdx = 0;
    }
    else if (DUAL_CAMERA_SUB_SENSOR == SensorIdx) {
        pinSetIdx = 1;
    }


 
	if (currSensorName && ((0 == strcmp(SENSOR_DRVNAME_OV5645_MIPI_YUV,currSensorName)) 
	/*	||(0 == strcmp(SENSOR_DRVNAME_MT9P017_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_OV5645_MIPI_YUV,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_OV5648_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_OV8825_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_OV8820_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_OV8865_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_OV8858_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_OV8865_4LANE_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_HM5065_MIPI_YUV,currSensorName))
        	||(0 == strcmp(SENSOR_DRVNAME_HM5040MIPI_RAW,currSensorName))  
		||(0 == strcmp(SENSOR_DRVNAME_HM8131_RAW,currSensorName) )
		||(0 == strcmp(SENSOR_DRVNAME_IMX111_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_GS8604MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_IMX219_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_A5142_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_S5K4H5YX_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_S5K5E2YA_MIPI_RAW,currSensorName))
		||(0 == strcmp(SENSOR_DRVNAME_S5K5EAYX_YUV,currSensorName)) */     ))
	{
		changge_ctr(pinSetIdx, 0 , 0);
	}
	else
	{
		changge_ctr(pinSetIdx, 0 , 1);		
	}

    //power ON
    if (On) {



// disable all
if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMRST],pinSet[1-pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}

if(mt_set_gpio_mode(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
if(mt_set_gpio_dir(pinSet[1-pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
if(mt_set_gpio_out(pinSet[1-pinSetIdx][IDX_PS_CMPDN],pinSet[1-pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}

if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}



        //DOVDD

        {
            PK_DBG("[ON_general 1.8V]sensorIdx:%d \n",SensorIdx);
            if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_IO, VOL_1800,mode_name))
            {
                PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
                //return -EIO;
                //goto _kdCISModulePowerOn_exit_;
            }
        }
        mdelay(10);

        //AVDD
        if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_A, VOL_2800,mode_name))
        {
            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
            //return -EIO;
            //goto _kdCISModulePowerOn_exit_;
        }
        mdelay(10);


        //DVDD
	{
            if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_D, VOL_1800,mode_name))
        {
             PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
             //return -EIO;
             //goto _kdCISModulePowerOn_exit_;
        }
        }            
            

	if (0) // ((0 != strcmp(SENSOR_DRVNAME_OV5645_MIPI_YUV,currSensorName)))
	{
        //AF_VCC
        if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_AF, VOL_2800,mode_name))
        {
            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
            //return -EIO;
            goto _kdCISModulePowerOn_exit_;
        }

	 mdelay(10);	
	}

#if 1



//enable active sensor
if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
mdelay(5);
if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_ON])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");}
mdelay(5);

//PDN pin
if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_ON])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");}
mdelay(5);
}
#endif


    }
    else {//power OFF

 
        //PK_DBG("[OFF]sensorIdx:%d \n",SensorIdx);
        if (GPIO_CAMERA_INVALID != pinSet[pinSetIdx][IDX_PS_CMRST]) {
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_MODE])){PK_DBG("[CAMERA SENSOR] set gpio mode failed!! \n");}
            if(mt_set_gpio_mode(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_MODE])){PK_DBG("[CAMERA LENS] set gpio mode failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMRST],GPIO_DIR_OUT)){PK_DBG("[CAMERA SENSOR] set gpio dir failed!! \n");}
            if(mt_set_gpio_dir(pinSet[pinSetIdx][IDX_PS_CMPDN],GPIO_DIR_OUT)){PK_DBG("[CAMERA LENS] set gpio dir failed!! \n");}
            if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMRST],pinSet[pinSetIdx][IDX_PS_CMRST+IDX_PS_OFF])){PK_DBG("[CAMERA SENSOR] set gpio failed!! \n");} //low == reset sensor
    	    if(mt_set_gpio_out(pinSet[pinSetIdx][IDX_PS_CMPDN],pinSet[pinSetIdx][IDX_PS_CMPDN+IDX_PS_OFF])){PK_DBG("[CAMERA LENS] set gpio failed!! \n");} //high == power down lens module
        }

    	if(TRUE != hwPowerDown(PMIC_APP_MAIN_CAMERA_POWER_A,mode_name)) {
            PK_DBG("[CAMERA SENSOR] Fail to OFF analog power\n");
            //return -EIO;
            goto _kdCISModulePowerOn_exit_;
        }
        if(TRUE != hwPowerDown(PMIC_APP_MAIN_CAMERA_POWER_AF,mode_name))
        {
            PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
            //return -EIO;
            goto _kdCISModulePowerOn_exit_;
        }
        if(TRUE != hwPowerDown(PMIC_APP_MAIN_CAMERA_POWER_D, mode_name)) {
            PK_DBG("[CAMERA SENSOR] Fail to OFF digital power\n");
            //return -EIO;
            goto _kdCISModulePowerOn_exit_;
        }
        if(TRUE != hwPowerDown(PMIC_APP_MAIN_CAMERA_POWER_IO,mode_name))
        {
            PK_DBG("[CAMERA SENSOR] Fail to enable digital power\n");
            //return -EIO;
            goto _kdCISModulePowerOn_exit_;
        }
    }//

	return 0;

_kdCISModulePowerOn_exit_:
    return -EIO;
}

#if 1   //David add for ov5640 af bug 20130411
int Af_VCOMA2_Power_On(void)
{
#if defined(HM5065_YUV)||defined(HM5065_MIPI_YUV)
    if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_AF,VOL_1800,"kd_camera_hw")) //
#else
    if(TRUE != hwPowerOn(PMIC_APP_MAIN_CAMERA_POWER_AF,VOL_2800,"kd_camera_hw")) //
#endif
    {
        PK_DBG("[CAMERA SENSOR] Fail to enable analog power\n");
        return -EIO;
     }
    return 0;
}
#endif

EXPORT_SYMBOL(Af_VCOMA2_Power_On);

EXPORT_SYMBOL(kdCISModulePowerOn);


//!--
//




