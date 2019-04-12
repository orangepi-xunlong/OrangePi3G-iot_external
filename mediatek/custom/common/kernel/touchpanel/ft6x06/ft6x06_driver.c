											
											
#include "tpd.h"											
#include <linux/interrupt.h>											
#include <cust_eint.h>											
#include <linux/i2c.h>											
#include <linux/sched.h>											
#include <linux/kthread.h>											
#include <linux/rtpm_prio.h>											
#include <linux/wait.h>											
#include <linux/time.h>											
#include <linux/delay.h>											
											
#include "custom_ft6x06.h"											
	   										
#include <mach/mt_pm_ldo.h>											
#include <mach/mt_typedefs.h>											
#include <mach/mt_boot.h>											
											
#include "cust_gpio_usage.h"											
											
/*////////////////////////////////////////////////////////////////////////////////////////////////////											
// 										D E B U	G
////////////////////////////////////////////////////////////////////////////////////////////////////*/											
#ifndef FT6X06_SAY											
#define FT6X06_SAY		//[==============FT6X06================] 									
#endif											
											
#define FT6X06_DMESG(a,arg...) 		printk(FT6X06_SAY ": " a,##arg)									
#if 1											
#define FT6X06_DEBUG(a,arg...) 		printk(FT6X06_SAY ": " a,##arg)									
#else											
#define FT6X06_DEBUG(arg...) 			//do {} while (0)								
#endif											


#define hwPowerOn(x,y,z)  
#define hwPowerDown(x,y)											
/*////////////////////////////////////////////////////////////////////////////////////////////////////											
// 										E X T E R N	
////////////////////////////////////////////////////////////////////////////////////////////////////*/											
extern void mt65xx_eint_unmask(unsigned int line);											
extern void mt65xx_eint_mask(unsigned int line);											
extern void mt65xx_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);											
extern unsigned int mt65xx_eint_set_sens(unsigned int eint_num, unsigned int sens);											
extern void mt65xx_eint_registration(unsigned int eint_num, unsigned int is_deb_en, unsigned int pol, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);											
											
extern int tpd_load_status;											
											
/*////////////////////////////////////////////////////////////////////////////////////////////////////											
// 										D E F I N E	
////////////////////////////////////////////////////////////////////////////////////////////////////*/											
#define ZF_FT6X06_NAME		"ft6x06"									
//------------------------------------------											
#define FT6X06_WIDTH  			simple_strtoul(LCM_WIDTH, NULL, 0)								
#define FT6X06_HEIGHT  			simple_strtoul(LCM_HEIGHT, NULL, 0)								
//------------------------------------------											
#define TPD_RESET_ISSUE_WORKAROUND											
#define TPD_MAX_RESET_COUNT 		3									
//------------------------------------------											
#define TPD_FT6X06_OK 				0							
//------------------------------------------											
#if defined(S7229)											
#define FT6X06_XY_SWAP				0							
#elif defined(S7277)											
#define FT6X06_XY_SWAP				1							
#else											
#define FT6X06_XY_SWAP				0							
#endif											
//------------------------------------------											
											
/*////////////////////////////////////////////////////////////////////////////////////////////////////											
// 										V A R I A B L E S	
////////////////////////////////////////////////////////////////////////////////////////////////////*/											
struct i2c_client *i2c_client = NULL;											
struct task_struct *ft6x06_thread = NULL;											
											
static DECLARE_WAIT_QUEUE_HEAD(waiter);											
static DEFINE_MUTEX(i2c_access);											
											
static int ft6x06_irq_flag = 0;											
static int ft6x06_halt=0;											
											
/*////////////////////////////////////////////////////////////////////////////////////////////////////											
// Key Process 											
////////////////////////////////////////////////////////////////////////////////////////////////////*/											
extern struct tpd_device *tpd;											
											
#ifdef TPD_HAVE_BUTTON 											
static int tpd_keys_local[TPD_KEY_COUNT] = TPD_KEYS;											
static int tpd_keys_dim_local[TPD_KEY_COUNT][4] = TPD_KEYS_DIM;											
#endif											
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))											
static int tpd_wb_start_local[TPD_WARP_CNT] = TPD_WARP_START;											
static int tpd_wb_end_local[TPD_WARP_CNT]   = TPD_WARP_END;											
#endif											
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))											
static int tpd_calmat_local[8]     = TPD_CALIBRATION_MATRIX;											
static int tpd_def_calmat_local[8] = TPD_CALIBRATION_MATRIX;											
#endif											
											
											
static void ft6x06_report_down(int x, int y, int p)											
{											
	// input_report_abs(tpd->dev, ABS_PRESSURE, p);										
	if(x > TPD_RES_X)										
	{										
		TPD_DEBUG("warning: IC have sampled wrong value.\n");;									
		return;									
	}										
	 input_report_key(tpd->dev, BTN_TOUCH, 1);										
	 input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 20);										
	 input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);										
	 input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);										
	 //printk("D[%4d %4d %4d] ", x, y, p);										
	 /* track id Start 0 */										
   //    input_report_abs(tpd->dev, ABS_MT_TRACKING_ID, p); 											
	 input_mt_sync(tpd->dev);										
	 										
     if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())											
     {   											
       tpd_button(x, y, 1);  											
     }											
	 if(y > TPD_RES_Y) //virtual key debounce to avoid android ANR issue										
	 {										
         msleep(50);											
		 printk("D virtual key \n");									
	 }										
	 TPD_EM_PRINT(x, y, x, y, p-1, 1);										
	 										
    FT6X06_DEBUG("TP down, %d,%d.\n", x, y);											
}											
 											
static void ft6x06_report_up(int x, int y,int *count) 											
{											
	 //if(*count>0) {										
		 //input_report_abs(tpd->dev, ABS_PRESSURE, 0);									
		 input_report_key(tpd->dev, BTN_TOUCH, 0);									
		 //input_report_abs(tpd->dev, ABS_MT_TOUCH_MAJOR, 0);									
		 //input_report_abs(tpd->dev, ABS_MT_POSITION_X, x);									
		 //input_report_abs(tpd->dev, ABS_MT_POSITION_Y, y);									
		 //printk("U[%4d %4d %4d] ", x, y, 0);									
		 input_mt_sync(tpd->dev);									
		 TPD_EM_PRINT(x, y, x, y, 0, 0);									
	//	 (*count)--;									
											
	if (FACTORY_BOOT == get_boot_mode()|| RECOVERY_BOOT == get_boot_mode())										
	{   										
		tpd_button(x, y, 0); 									
	}   		 								
    FT6X06_DEBUG("TP up.\n");											
}											
											
static void ft6x06_report_end(void)											
{											
    input_sync(tpd->dev);											
}											
											
											
////////////////////////////////////////////////////////////////////////////////////////////////////											
//											
////////////////////////////////////////////////////////////////////////////////////////////////////											
											
//#define TPD_CLOSE_POWER_IN_SLEEP											
											
//register define											
											
#define DEVICE_MODE 0x00											
#define GEST_ID 	0x01										
#define TD_STATUS 	0x02										
											
#define TOUCH1_XH 	0x03										
#define TOUCH1_XL 	0x04										
#define TOUCH1_YH 	0x05										
#define TOUCH1_YL 	0x06										
											
#define TOUCH2_XH 	0x09										
#define TOUCH2_XL 	0x0A										
#define TOUCH2_YH 	0x0B										
#define TOUCH2_YL 	0x0C										
											
#define TOUCH3_XH 	0x0F										
#define TOUCH3_XL 	0x10										
#define TOUCH3_YH 	0x11										
#define TOUCH3_YL 	0x12										
//register define											
											
											
											
////////////////////////////////////////////////////////////////////////////////////////////////////											
//											
////////////////////////////////////////////////////////////////////////////////////////////////////											
#define VELOCITY_CUSTOM_FT6X06											
											
											
#ifdef VELOCITY_CUSTOM_FT6X06											
#include <linux/device.h>											
#include <linux/miscdevice.h>											
#include <asm/uaccess.h>											
											
// for magnify velocity********************************************											
											
#ifndef TPD_VELOCITY_CUSTOM_X											
#define TPD_VELOCITY_CUSTOM_X 10											
#endif											
#ifndef TPD_VELOCITY_CUSTOM_Y											
#define TPD_VELOCITY_CUSTOM_Y 10											
#endif											
											
#define TOUCH_IOC_MAGIC 'A'											
											
#define TPD_GET_VELOCITY_CUSTOM_X _IO(TOUCH_IOC_MAGIC,0)											
#define TPD_GET_VELOCITY_CUSTOM_Y _IO(TOUCH_IOC_MAGIC,1)											
											
int g_v_magnify_x =TPD_VELOCITY_CUSTOM_X;											
int g_v_magnify_y =TPD_VELOCITY_CUSTOM_Y;											
											
static int tpd_misc_open(struct inode *inode, struct file *file)											
{											
	return nonseekable_open(inode, file);										
}											
/*----------------------------------------------------------------------------*/											
static int tpd_misc_release(struct inode *inode, struct file *file)											
{											
	return 0;										
}											
/*----------------------------------------------------------------------------*/											
static long tpd_unlocked_ioctl(struct file *file, unsigned int cmd, unsigned long arg)											
{											
	void __user *data;										
											
	long err = 0;										
											
	if(_IOC_DIR(cmd) & _IOC_READ)										
	{										
		err = !access_ok(VERIFY_WRITE, (void __user *)arg, _IOC_SIZE(cmd));									
	}										
	else if(_IOC_DIR(cmd) & _IOC_WRITE)										
	{										
		err = !access_ok(VERIFY_READ, (void __user *)arg, _IOC_SIZE(cmd));									
	}										
											
	if(err)										
	{										
		printk("tpd: access error: %08X, (%2d, %2d)\n", cmd, _IOC_DIR(cmd), _IOC_SIZE(cmd));									
		return -EFAULT;									
	}										
											
	switch(cmd)										
	{										
		case TPD_GET_VELOCITY_CUSTOM_X:									
			data = (void __user *) arg;								
			if(data == NULL)								
			{								
				err = -EINVAL;							
				break;	  						
			}								
											
			if(copy_to_user(data, &g_v_magnify_x, sizeof(g_v_magnify_x)))								
			{								
				err = -EFAULT;							
				break;							
			}				 				
			break;								
											
	   case TPD_GET_VELOCITY_CUSTOM_Y:										
			data = (void __user *) arg;								
			if(data == NULL)								
			{								
				err = -EINVAL;							
				break;	  						
			}								
											
			if(copy_to_user(data, &g_v_magnify_y, sizeof(g_v_magnify_y)))								
			{								
				err = -EFAULT;							
				break;							
			}				 				
			break;								
											
		default:									
			printk("tpd: unknown IOCTL: 0x%08x\n", cmd);								
			err = -ENOIOCTLCMD;								
			break;								
	}										
											
	return err;										
}											
											
/*----------------------------------------------------------------------------*/											
static struct file_operations tpd_fops = 											
{											
//	.owner = THIS_MODULE,										
	.open = tpd_misc_open,										
	.release = tpd_misc_release,										
	.unlocked_ioctl = tpd_unlocked_ioctl,										
};											
/*----------------------------------------------------------------------------*/											
static struct miscdevice tpd_misc_device = 											
{											
	.minor = MISC_DYNAMIC_MINOR,										
	.name = "touch",										
	.fops = &tpd_fops,										
};											
#endif											
////////////////////////////////////////////////////////////////////////////////////////////////////											
//											
////////////////////////////////////////////////////////////////////////////////////////////////////											
static void ft6x06_tpd_eint_irq(void)											
{											
	//TPD_DEBUG("TPD interrupt has been triggered\n");										
	TPD_DEBUG_PRINT_INT;										
	ft6x06_irq_flag = 1;										
	wake_up_interruptible(&waiter);										
}											
////////////////////////////////////////////////////////////////////////////////////////////////////											
//											
////////////////////////////////////////////////////////////////////////////////////////////////////											
struct touch_info 											
{											
    int y[3];											
    int x[3];											
    int p[3];											
    int id[3];											
    int count;											
};											
											
static int finger_num = 0;											
											
static int tpd_touchinfo_ft6x06(struct touch_info *cinfo, struct touch_info *pinfo)											
{											
	int i = 0;										
	u8 report_rate =0;										
											
	char data[32] = {0};										
//    s16 status;											
    u16 high_byte, low_byte;											
											
	FT6X06_DEBUG("-------------------------------------------------\n");										
	FT6X06_DEBUG("Enter Function: %s.\n", __func__);										
//----------------------------------------------											
	mutex_lock(&i2c_access);										
											
	if (ft6x06_halt)										
	{										
		mutex_unlock(&i2c_access);									
		FT6X06_DMESG( "tpd_touchinfo_ft6x06 return ..\n");									
		return false;									
	}										
											
	i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 8, &(data[0]));										
	i2c_smbus_read_i2c_block_data(i2c_client, 0x08, 8, &(data[8]));										
	i2c_smbus_read_i2c_block_data(i2c_client, 0x10, 8, &(data[16]));										
	i2c_smbus_read_i2c_block_data(i2c_client, 0x18, 8, &(data[24]));										
	i2c_smbus_read_i2c_block_data(i2c_client, 0x88, 1, &report_rate);										
	/* Device Mode[2:0] == 0 :Normal operating Mode*/										
											
											
	FT6X06_DEBUG("TP_INFO:  report_rate =%d.\n", report_rate);										
											
	if(report_rate < 8)										
	{										
		report_rate = 0x8;									
		if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)									
		{									
			FT6X06_DMESG("I2C read report rate error, line: %d\n", __LINE__);								
		}									
	}										
	mutex_unlock(&i2c_access);										
											
//----------------------------------------------											
	/* Device Mode[2:0] == 0 :Normal operating Mode*/										
	if((data[0] & 0x70) != 0)										
		return false; 									
											
	finger_num = data[2] & 0x0f;										
	FT6X06_DEBUG("TP_INFO:  finger_num =%d.\n", finger_num);										
											
											
//----------------------------------------------											
	FT6X06_DEBUG("Procss raw data...\n");										
											
											
	for(i = 0; i < finger_num; i++)										
	{										
		//status =  (s16)((data[3+6*i] & 0xc0)>> 6); //event flag 									
											
		cinfo->p[i] = data[3+6*i] >> 6; //event flag 									
		cinfo->id[i] = data[3+6*i+2]>>4; //touch id									
											
       /*get the X coordinate, 2 bytes*/											
		high_byte = data[3+6*i];									
		high_byte <<= 8;									
		high_byte &= 0x0f00;									
		low_byte = data[3+6*i + 1];									
		cinfo->x[i] = high_byte |low_byte;									
		//cinfo->x[i] =  cinfo->x[i] * 480 >> 11; //calibra									
											
		/*get the Y coordinate, 2 bytes*/									
		high_byte = data[3+6*i+2];									
		high_byte <<= 8;									
		high_byte &= 0x0f00;									
		low_byte = data[3+6*i+3];									
		cinfo->y[i] = high_byte |low_byte;									
		//cinfo->y[i]=  cinfo->y[i] * 800 >> 11;									
											
	#if FT6X06_XY_SWAP										
		if(cinfo->y[i] > FT6X06_HEIGHT )									
		{									
			//for virtual key								
		}									
		else									
		{									
			cinfo->x[i] = FT6X06_WIDTH-1 - cinfo->x[i];								
			cinfo->y[i] = FT6X06_HEIGHT-1 - cinfo->y[i];								
		}									
	#endif										
											
	/* 										
		if(status==1)									
		{									
			cinfo->p[i] = 0;								
		}									
		else									
		{									
			cinfo->p[i] = 200;								
		}									
	*/										
		cinfo->count++;									
	}										
//----------------------------------------------											
/*											
	FT6X06_DEBUG("TP_INFO:   cinfo->x[0] =%11d, cinfo->y[0] = %11d, cinfo->p[0] = %11d\n", cinfo->x[0], cinfo->y[0], cinfo->p[0]);										
	FT6X06_DEBUG("TP_INFO:   cinfo->x[1] =%11d, cinfo->y[1] = %11d, cinfo->p[1] = %11d\n", cinfo->x[1], cinfo->y[1], cinfo->p[1]);										
	FT6X06_DEBUG("TP_INFO:   cinfo->x[2] =%11d, cinfo->y[2] = %11d, cinfo->p[2] = %11d\n", cinfo->x[2], cinfo->y[2], cinfo->p[2]);										
	FT6X06_DEBUG("TP_INFO:   cinfo->x[3] =%11d, cinfo->y[3] = %11d, cinfo->p[3] = %11d\n", cinfo->x[3], cinfo->y[3], cinfo->p[3]);										
	FT6X06_DEBUG("TP_INFO:   cinfo->x[4] =%11d, cinfo->y[4] = %11d, cinfo->p[4] = %11d\n", cinfo->x[4], cinfo->y[4], cinfo->p[4]);										
*/											
	return true;										
}											
											
											
static int ft6x06_touch_event_handler(void *unused)											
{											
	struct touch_info cinfo, pinfo;										
	int x1,y1,x2,y2,x3,y3,x4,y4,x5,y5;										
	int i=0;										
											
	struct sched_param param = { .sched_priority = RTPM_PRIO_TPD };										
											
	FT6X06_DEBUG("Enter Function: %s.\n", __func__);										
											
	sched_setscheduler(current, SCHED_RR, &param);										
											
	do										
	{										
		mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM); 									
		set_current_state(TASK_INTERRUPTIBLE); 									
		wait_event_interruptible(waiter,ft6x06_irq_flag!=0);									
				 							
		ft6x06_irq_flag = 0;									
											
		set_current_state(TASK_RUNNING);									
		 									
//-----------------------------------------------------------------------------------											
		if (tpd_touchinfo_ft6x06(&cinfo, &pinfo)) 									
		{									
			//TPD_DEBUG("finger_num = %d\n",finger_num);								
			TPD_DEBUG_SET_TIME;								
											
			FT6X06_DEBUG("finger_num = %d\n", finger_num);								
											
			x1=cinfo.x[0];		y1=cinfo.y[0];						
			x2=cinfo.x[1];		y2=cinfo.y[1];						
			x3=cinfo.x[2];		y3=cinfo.y[2];						
			x4=cinfo.x[3];		y4=cinfo.y[3];						
			x5=cinfo.x[4];		y5=cinfo.y[4];						
											
											
			FT6X06_DEBUG("TP_INFO:   x1=%11d, y1=%11d\n", x1, y1);								
			FT6X06_DEBUG("TP_INFO:   x2=%11d, y2=%11d\n", x2, y2);								
			FT6X06_DEBUG("TP_INFO:   x3=%11d, y3=%11d\n", x3, y3);								
			FT6X06_DEBUG("TP_INFO:   x4=%11d, y4=%11d\n", x4, y4);								
			FT6X06_DEBUG("TP_INFO:   x5=%11d, y5=%11d\n", x5, y5);								
											
			if(finger_num >0) 								
			{								
				for(i =0; i<finger_num && i<5; i++)//only support 3 point							
				{							
					 ft6x06_report_down(cinfo.x[i], cinfo.y[i], cinfo.id[i]);						
				}							
				ft6x06_report_end();							
			}								
											
			else  								
			{								
				ft6x06_report_up(cinfo.x[0], cinfo.y[0], 0);							
				ft6x06_report_end();							
			}								
		}									
//-----------------------------------------------------------------------------------											
        if(tpd_mode==12)											
        {											
           //power down for desence debug											
           //power off, need confirm with SA											
		#ifdef TPD_POWER_SOURCE_CUSTOM									
			hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "TP");								
		#else									
			//hwPowerDown(MT65XX_POWER_LDO_VGP2, "TP");								
		#endif									
											
		#ifdef TPD_POWER_SOURCE_1800									
			hwPowerDown(TPD_POWER_SOURCE_1800, "TP");								
		#endif 									
											
			msleep(20);         								
        }											
//-----------------------------------------------------------------------------------											
	}while(!kthread_should_stop());										
	return 0;										
}											
											
/*////////////////////////////////////////////////////////////////////////////////////////////////////											
// 										F U N C T I O M ---power	
////////////////////////////////////////////////////////////////////////////////////////////////////*/											
static void MDrv_Ft6x06_Power_PwrOn(unsigned char bPwrOn)											
{											
//------------											
// power											
//------------											
#if TP_POWER_USE_VGP											
  #ifdef(TPD_POWER_SOURCE_CUSTOM)											
	hwPowerOn(TPD_POWER_SOURCE_CUSTOM, VOL_2800, "TP");										
  #else											
	//hwPowerOn(MT65XX_POWER_LDO_VGP4, VOL_2800, "TP");										
  #endif											
											
  #ifdef TPD_POWER_SOURCE_1800											
	hwPowerOn(TPD_POWER_SOURCE_1800, VOL_1800, "TP");										
  #endif 											
#else // CUSTOM											
	//mt_set_gpio_mode(GPIO_CTP_EN_PIN, GPIO_CTP_EN_PIN_M_GPIO);  										
    //mt_set_gpio_dir(GPIO_CTP_EN_PIN, GPIO_DIR_OUT);											
	//mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ONE);										
#endif											
	msleep(10);  // 100										
//------------											
// reset											
//------------											
	mt_set_gpio_mode(GPIO_CTP_RST_PIN, GPIO_CTP_RST_PIN_M_GPIO);    mt_set_gpio_dir(GPIO_CTP_RST_PIN, GPIO_DIR_OUT);										
    mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);	msleep(1); // 10										
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);	msleep(2); // 10									
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);	msleep(1); // 10									
}											
											
static void MDrv_Ft6x06_Power_PwrOff(void)											
{											
//------------											
// reset											
//------------											
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ZERO);	msleep(2); // 10									
	mt_set_gpio_out(GPIO_CTP_RST_PIN, GPIO_OUT_ONE);	msleep(1); // 10									
//------------											
// power											
//------------											
#if TP_POWER_USE_VGP											
  #ifdef(TPD_POWER_SOURCE_CUSTOM)											
	hwPowerDown(TPD_POWER_SOURCE_CUSTOM, "TP");										
  #else											
	//hwPowerDown(MT65XX_POWER_LDO_VGP4, "TP");										
  #endif											
#else // CUSTOM											
	//mt_set_gpio_out(GPIO_CTP_EN_PIN, GPIO_OUT_ZERO);										
#endif											
}											
											
/*////////////////////////////////////////////////////////////////////////////////////////////////////											
// 										F U N C T I O M ---basic	
////////////////////////////////////////////////////////////////////////////////////////////////////*/											
static int __devinit ft6x06_tpd_i2c_probe(struct i2c_client *client, const struct i2c_device_id *id)											
{	 										
	int retval = TPD_FT6X06_OK;										
	char data;										
	u8 report_rate=0;										
	int err=0;										
	int reset_count = 0;										
											
reset_proc:   											
//-----------------------------------------------------------------------											
	MDrv_Ft6x06_Power_PwrOn(1);										
//-----------------------------------------------------------------------											
	i2c_client = client;										
	i2c_client->addr |= I2C_ENEXT_FLAG; 	//I2C_HS_FLAG;									
	i2c_client->timing = 400;			// 100 khz or 400 khz							
											
	if((i2c_smbus_read_i2c_block_data(i2c_client, 0x00, 1, &data))< 0)										
	{										
		TPD_DMESG("I2C transfer error, line: %d\n", __LINE__);									
	#ifdef TPD_RESET_ISSUE_WORKAROUND										
        if ( reset_count < TPD_MAX_RESET_COUNT )											
        {											
            reset_count++;											
            goto reset_proc;											
        }											
	#endif										
		FT6X06_DMESG("xxxxxxxxxxxxxxx      I2C transfer error, line: %d\n", __LINE__);									
		FT6X06_DMESG("xxxxxxxxxxxxxxx      I2C transfer error, line: %d\n", __LINE__);									
		FT6X06_DMESG("xxxxxxxxxxxxxxx      I2C transfer error, line: %d\n", __LINE__);									
											
		return -1; 									
	}										
											
//-----------------------------------------------------------------------											
	//set report rate 80Hz										
	report_rate = 0x8; 										
	if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)										
	{										
	    if((i2c_smbus_write_i2c_block_data(i2c_client, 0x88, 1, &report_rate))< 0)										
	    {										
		   TPD_DMESG("I2C read report rate error, line: %d\n", __LINE__);									
	    }										
		   									
	}										
											
//-----------------------------------------------------------------------											
    FT6X06_DMESG("%s, success !!\n", __func__);											
	tpd_load_status = 1;										
											
//-----------------------------------------------------------------------											
#ifdef VELOCITY_CUSTOM_FT6X06											
	if((err = misc_register(&tpd_misc_device)))										
	{										
		FT6X06_DMESG("mtk_tpd: tpd_misc_device register failed\n");									
	}										
#endif											
//-----------------------------------------------------------------------											
							
	// set INT mode
	mt_set_gpio_mode(GPIO_CTP_EINT_PIN, GPIO_CTP_EINT_PIN_M_EINT);
	mt_set_gpio_dir(GPIO_CTP_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(GPIO_CTP_EINT_PIN, GPIO_PULL_ENABLE);
	mt_set_gpio_pull_select(GPIO_CTP_EINT_PIN, GPIO_PULL_UP);

	msleep(50);
	  mt65xx_eint_set_sens(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_SENSITIVE);
	  mt65xx_eint_set_hw_debounce(CUST_EINT_TOUCH_PANEL_NUM, CUST_EINT_TOUCH_PANEL_DEBOUNCE_CN);

	if ( 0 )	//EINTF_TRIGGER
	{
		mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, EINTF_TRIGGER_RISING, ft6x06_tpd_eint_irq, 1);
	}
	else
	{
		mt_eint_registration(CUST_EINT_TOUCH_PANEL_NUM, EINTF_TRIGGER_FALLING, ft6x06_tpd_eint_irq, 1);
	}

	mt_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);
										
 		msleep(10);//msleep(100);										
											
//-----------------------------------------------------------------------											
	ft6x06_thread = kthread_run(ft6x06_touch_event_handler, 0, TPD_DEVICE);										
											
	if (IS_ERR(ft6x06_thread))										
	{ 										
		retval = PTR_ERR(ft6x06_thread);									
		FT6X06_DMESG(TPD_DEVICE " failed to create kernel ft6x06_thread: %d\n", retval);									
	}										
//-----------------------------------------------------------------------											
	FT6X06_DMESG("vvvvvvvvvvvvvvvvvvvvvv         Touch Panel Device Probe %s\n", (retval < TPD_FT6X06_OK) ? "FAIL" : "PASS");										
	FT6X06_DMESG("vvvvvvvvvvvvvvvvvvvvvv         Touch Panel Device Probe %s\n", (retval < TPD_FT6X06_OK) ? "FAIL" : "PASS");										
	FT6X06_DMESG("vvvvvvvvvvvvvvvvvvvvvv         Touch Panel Device Probe %s\n", (retval < TPD_FT6X06_OK) ? "FAIL" : "PASS");										
											
	return 0;										
}											
											
static int __devexit ft6x06_tpd_i2c_remove(struct i2c_client *client)											
{											
	FT6X06_DEBUG("Enter Function: %s.\n", __func__);										
											
	return 0;										
}											
 											
static int ft6x06_tpd_i2c_detect (struct i2c_client *client, int kind, struct i2c_board_info *info) 											
{											
	FT6X06_DEBUG("Enter Function: %s.\n", __func__);										
											
	strcpy(info->type, TPD_DEVICE);										
	return 0;										
}											
  											
//---------------------------------------------											
static const struct i2c_device_id ft6x06_tpd_id[] = {{ZF_FT6X06_NAME,0},{}};											
static struct i2c_board_info __initdata ft6x06_i2c_tpd={ I2C_BOARD_INFO(ZF_FT6X06_NAME, (0x70>>1))};											
 											
 											
struct i2c_driver ft6x06_tpd_i2c_driver =											
{											
	.driver = {										
				 .name = ZF_FT6X06_NAME, //.name = TPD_DEVICE,							
			  },								
	.probe = ft6x06_tpd_i2c_probe,										
	.remove = __devexit_p(ft6x06_tpd_i2c_remove),										
	.detect = ft6x06_tpd_i2c_detect,										
	.id_table = ft6x06_tpd_id,										
};											
											
////////////////////////////////////////////////////////////////////////////////////////////////////											
//											
////////////////////////////////////////////////////////////////////////////////////////////////////											
static int ft6x06_tpd_local_init(void)											
{ 											
	FT6X06_DMESG("Focaltech ft6x06 I2C Touchscreen Driver (Built %s @ %s)\n", __DATE__, __TIME__);										
 											
	if(i2c_add_driver(&ft6x06_tpd_i2c_driver)!=0)										
	{										
		FT6X06_DMESG("unable to add i2c driver.\n");									
		return -1;									
	}										
											
	if(tpd_load_status == 0) 										
	{										
	    FT6X06_DEBUG("%s------load ft6x06 failed!!!!!--xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx\n", __func__);										
											
        FT6X06_DMESG("add error touch panel driver.\n");											
        i2c_del_driver(&ft6x06_tpd_i2c_driver);											
		return -1;									
	}										
	else										
	{										
	    FT6X06_DEBUG("%s------load ft6x06 ok!!!!!--vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv\n", __func__);										
	}										
											
#ifdef TPD_HAVE_BUTTON     											
    tpd_button_setting(TPD_KEY_COUNT, tpd_keys_local, tpd_keys_dim_local);// initialize tpd button data											
#endif   											
  											
#if (defined(TPD_WARP_START) && defined(TPD_WARP_END))    											
    TPD_DO_WARP = 1;											
    memcpy(tpd_wb_start, tpd_wb_start_local, TPD_WARP_CNT*4);											
    memcpy(tpd_wb_end, tpd_wb_start_local, TPD_WARP_CNT*4);											
#endif 											
											
#if (defined(TPD_HAVE_CALIBRATION) && !defined(TPD_CUSTOM_CALIBRATION))											
    memcpy(tpd_calmat, tpd_def_calmat_local, 8*4);											
    memcpy(tpd_def_calmat, tpd_def_calmat_local, 8*4);											
#endif  											
											
	tpd_type_cap = 1;										
											
    FT6X06_DEBUG("%s------local init over 333333333333333333333333333333333333333333\n", __func__);											
    FT6X06_DEBUG("%s------local init over 333333333333333333333333333333333333333333\n", __func__);											
	FT6X06_DMESG("end %s, %d\n", __FUNCTION__, __LINE__);  										
											
    return 0; 											
 }											
											
//----------------------------------------------------------											
static void ft6x06_tpd_suspend( struct early_suspend *h )											
{											
	static char data = 0x3;										
											
	FT6X06_DEBUG("Enter Function: %s.\n", __func__);										
											
	mt65xx_eint_mask(CUST_EINT_TOUCH_PANEL_NUM);										
											
	mutex_lock(&i2c_access);										
//----------------------------------------											
	i2c_smbus_write_i2c_block_data(i2c_client, 0xA5, 1, &data);  //TP enter sleep mode										
	MDrv_Ft6x06_Power_PwrOff();	 									
//----------------------------------------											
											
	mutex_unlock(&i2c_access);										
											
	ft6x06_halt = 1;										
} 											
											
//----------------------------------------------------------											
 static void ft6x06_tpd_resume( struct early_suspend *h )											
 {											
	FT6X06_DEBUG("Enter Function: %s.\n", __func__);										
 											
//----------------------------------------											
	MDrv_Ft6x06_Power_PwrOn(1);										
//----------------------------------------											
	ft6x06_halt = 0;										
	mt65xx_eint_unmask(CUST_EINT_TOUCH_PANEL_NUM);  										
	msleep(5);//30										
											
	FT6X06_DEBUG("TPD wake up done\n");										
 }											
											
//----------------------------------------------------------											
static struct tpd_driver_t ft6x06_tpd_device_driver =											
{											
	.tpd_device_name = ZF_FT6X06_NAME,										
	.tpd_local_init = ft6x06_tpd_local_init,										
	.suspend = ft6x06_tpd_suspend,										
	.resume = ft6x06_tpd_resume,										
#ifdef TPD_HAVE_BUTTON											
	.tpd_have_button = 1,										
#else											
	.tpd_have_button = 0,										
#endif											
};											
////////////////////////////////////////////////////////////////////////////////////////////////////											
//											
////////////////////////////////////////////////////////////////////////////////////////////////////											
static int __init ft6x06_tpd_driver_init(void)			/* called when loaded into kernel */								
{											
	FT6X06_DMESG("MediaTek ft6x06 touch panel driver init\n");										
											
	i2c_register_board_info(TPD_I2C_NUMBER, &ft6x06_i2c_tpd, 1);										
											
	if(tpd_driver_add(&ft6x06_tpd_device_driver) < 0)										
	{										
		FT6X06_DMESG("add ft6x06 driver failed\n");									
	}										
											
	return 0;										
}											
											
static void __exit ft6x06_tpd_driver_exit(void)			/* should never be called */								
{											
	FT6X06_DMESG("MediaTek ft6x06 touch panel driver exit.\n");										
	//input_unregister_device(tpd->dev);										
	tpd_driver_remove(&ft6x06_tpd_device_driver);										
}											
 											
module_init(ft6x06_tpd_driver_init);											
module_exit(ft6x06_tpd_driver_exit);											
