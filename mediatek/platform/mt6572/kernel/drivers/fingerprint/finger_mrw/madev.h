#if 0
/* MicroArray Fingerprint
 * madev.h
 * date: 2015-08-15
 * version: v2.0
 * Author: czl
 */

#ifndef MADEV_H
#define MADEV_H

#define AFS83 	0x53

#if defined(AFS83)

#define W   	80    
#define H   	192   
#define WBUF	81
#define FIMG	(W*H)
#define FBUF  	(1024*48)	

#define AFS83T		1 
#define SENSOR_TYPE   	AFS80T
#endif

//接口命令
#define IOCTL_DEBUG			0x100	// 调试信息
#define IOCTL_IRQ_ENABLE	0x101	// 中断使能
#define IOCTL_SPI_SPEED   	0x102	// SPI速度
#define IOCTL_READ_FLEN		0x103	// 读帧长度(保留)
#define IOCTL_LINK_DEV		0x104	// 连接设备(保留)
#define IOCTO_NUM_STUFF		0x105	// 材料编号
#define IOCTL_GET_VDATE		0x106	// 版本日期
#define IOCTL_PRESS		0x107 	// 按键事件(单击、双击和长按)

#define IOCTL_RES_INTF		0x110	// 复位中断标志
#define IOCTL_GET_INTF		0x111	// 获取中断标志
#define IOCTL_KEY_REPO		0x112 	// 键值开关

#define REPORT_ON		1		// 上报开
#define REPORT_OFF  	0	    // 上报关

#define SPI_SPEED 	(6*1000000)

#endif /* MADEV_H */

#endif

/* Copyright (C) MicroArray
 * MicroArray Fprint Driver Code * madev.h
 * Date: 2016-12-18
 * Version: v4.0.04 
 * Author: guq
 * Contact: guq@microarray.com.cn
 */

#ifndef __MADEV_H_
#define __MADEV_H_
 
//settings macro
#define MTK   			//[MTK|QUALCOMM|SPRD]
#define MA_DRV_VERSION	    (0x00004004)

#define MALOGD_LEVEL	KERN_EMERG     //[KERN_DEBUG|KERN_EMERG] usually, the debug level is used for the release version

#define MA_CHR_FILE_NAME 	"madev0"  //do not neeed modify usually 
#define MA_CHR_DEV_NAME 	"madev"	  //do not neeed modify usually 

#define MA_EINT_NAME            "mediatek,finger_print-eint"
//#define DOUBLE_EDGE_IRQ

//#define COMPATIBLE_VERSION3

//key define   just modify the KEY_FN_* for different platform
#define FINGERPRINT_SWIPE_UP 			KEY_FN_F1//827
#define FINGERPRINT_SWIPE_DOWN 			KEY_FN_F2//828
#define FINGERPRINT_SWIPE_LEFT 			KEY_FN_F3//829
#define FINGERPRINT_SWIPE_RIGHT 		KEY_FN_F4//830
#define FINGERPRINT_TAP 				KEY_FN_F5//	831
#define FINGERPRINT_DTAP				KEY_FN_F6// 	832
#define FINGERPRINT_LONGPRESS 			KEY_FN_F7//833

//key define end


//old macro
#define SPI_SPEED 	(6*1000000) 	//120/121:10M, 80/81:6M

//±íÃæÀàÐÍ
#define	COVER_T		1
#define COVER_N		2
#define COVER_M		3
#define COVER_NUM	COVER_T

//Ö¸ÎÆÀàÐÍ
//#define AFS120	0x78
//#define AFS80 	0x50
#define AFS83 	0x53

#if defined(AFS120)
	#define W   	120   //¿í
	#define H   	120   //¸ß
	#define WBUF	121
	#define FBUF  	(1024*32)	//¶ÁÐ´³¤¶È
#elif defined(AFS80)
	#define W   	80    //¿í
	#define H   	192   //¸ß
	#define WBUF	81
	#define FIMG	(W*H)
	#define FBUF  	(1024*32)	//¶ÁÐ´³¤¶È
#elif defined(AFS83)
	#define W   	80    //¿í
	#define H   	192   //¸ß
	#define WBUF	81
	#define FIMG	(W*H)
	#define FBUF  	(1024*48)	//¶ÁÐ´³¤¶È
#endif


//settings macro end
#include <linux/poll.h>
#include <linux/notifier.h>
#include <linux/fb.h>
//this two head file for the screen on/off test
#include <asm/ioctl.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <asm/uaccess.h>
#include <linux/of.h>
#include <linux/of_irq.h>
#include <linux/gpio.h>
#include <linux/spi/spi.h>
#include <linux/ioctl.h>
#include <linux/wakelock.h>
#include <linux/mm.h>
//#include "ioctl_cmd.h"
//#include "mtk-settings.h"
//#include "plat-mtk.h"

#if 0
#ifdef MTK
#include "mtk-settings.h"
#elif defined QUALCOMM
#include "qualcomm-settings.h"
#elif defined SPRD
#include "sprd-settings.h"
#endif
#endif 

#if 0
//value define
 //fprint_spi struct use to save the value
struct fprint_spi {
    u8 do_what;             //¹¤×÷ÄÚÈÝ
    u8 f_wake;              //»½ÐÑ±êÖ¾  
    int value;              
    volatile u8 f_irq;      //ÖÐ¶Ï±êÖ¾
    volatile u8 u1_flag;    //reserve for ours thread interrupt
    volatile u8 u2_flag;    //reserve for ours thread interrupt
    volatile u8 f_repo;     //ÉÏ±¨¿ª¹Ø  
    spinlock_t spi_lock;
    struct spi_device *spi;
    struct list_head dev_entry;
    struct spi_message msg;
    struct spi_transfer xfer;
    struct input_dev *input;
    struct work_struct work;
    struct workqueue_struct *workq;
#ifdef CONFIG_HAS_EARLYSUSPEND
    struct early_suspend suspend;
#endif
    struct wake_lock wl;
};
//end

struct fprint_dev {
    dev_t idd;          
    int major;          
    int minor;
    struct cdev *chd;
    struct class *cls;
    struct device *dev;
};
#endif 

//function define

//extern the settings.h function 
//extern void mas_select_transfer(struct spi_device *spi, int len);
//extern int mas_finger_get_gpio_info(struct platform_device *pdev);
//extern int mas_finger_set_gpio_info(int cmd);
//extern void mas_enable_spi_clock(struct spi_device *spi);
//extern void mas_disable_spi_clock(struct spi_device *spi);
//extern unsigned int mas_get_irq(void);
//extern int mas_get_platform(void);
//extern int mas_remove_platform(void);
//extern void ma_spi_change(struct spi_device *spi, unsigned int speed, int flag);
//extern int mas_get_interrupt_gpio(unsigned int index);
//extern int mas_switch_power(unsigned int on_off);
//end

//use for the log print
#define MALOG_TAG "MAFP_"
#define MALOGE(x) printk(KERN_ERR "%s%s: error log! the function %s is failed, ret = %d\n", MALOG_TAG, __func__, x, ret);  	//error log
#define MALOGF(x) printk(MALOGD_LEVEL "%s%s: debug log! %s!\n", MALOG_TAG, __func__, x);										//flag log
#define MALOGD(x) MALOGF(x)																									//debug log
#define MALOGW(x) printk(KERN_WARNING "%s%s: warning log! the function %s's ret = %d\n", MALOG_TAG, __func__,x, ret);			//warning log
//use for the log print

/**
 *	the old ioctl command, compatible for the old version 
 */
//ioctl cmd
#ifdef COMPATIBLE_VERSION3
#define IOCTL_DEBUG			0x100	//µ÷ÊÔÐÅÏ¢ 			//debug message
#define IOCTL_IRQ_ENABLE	0x101	//ÖÐ¶ÏÊ¹ÄÜ 			//enable interrupt
#define IOCTL_SPI_SPEED   	0x102	//SPIËÙ¶È 			//spi speed
#define IOCTL_READ_FLEN		0x103	//¶ÁÖ¡³¤¶È(±£Áô)		//the length of one frame
#define IOCTL_LINK_DEV		0x104	//Á¬½ÓÉè±¸(±£Áô)		//connect the device
#define IOCTL_COVER_NUM		0x105	//²ÄÁÏ±àºÅ			//the index of the material
#define IOCTL_GET_VDATE		0x106	//°æ±¾ÈÕÆÚ			//the date fo the version

#define IOCTL_CLR_INTF		0x110	//Çå³ýÖÐ¶Ï±êÖ¾
#define IOCTL_GET_INTF		0x111	//»ñÈ¡ÖÐ¶Ï±êÖ¾
#define IOCTL_REPORT_FLAG	0x112 	//ÉÏ±¨±êÖ¾
#define IOCTL_REPORT_KEY	0x113	//ÉÏ±¨¼üÖµ
#define IOCTL_SET_WORK		0x114	//ÉèÖÃ¹¤×÷
#define IOCTL_GET_WORK		0x115	//»ñÈ¡¹¤×÷
#define IOCTL_SET_VALUE		0x116	//ÉèÖµ
#define IOCTL_GET_VALUE		0x117	//È¡Öµ
#define IOCTL_TRIGGER		0x118	//×Ô´¥·¢
#define IOCTL_WAKE_LOCK		0x119	//»½ÐÑÉÏËø
#define IOCTL_WAKE_UNLOCK	0x120	//»½ÐÑ½âËø

#define IOCTL_SCREEN_ON		0x121

#define IOCTL_KEY_DOWN		0x121	//°´ÏÂ
#define IOCTL_KEY_UP		0x122	//Ì§Æð
#define IOCTL_SET_X			0x123	//Æ«ÒÆX
#define IOCTL_SET_Y			0x124	//Æ«ÒÆY
#define IOCTL_KEY_TAP		0x125	//µ¥»÷
#define IOCTL_KEY_DTAP		0x126	//Ë«»÷
#define IOCTL_KEY_LTAP		0x127	//³¤°´

#define IOCTL_ENABLE_CLK    0x128
#define TRUE 	1
#define FALSE 	0
#endif


//#define MA_DRV_VERSION	    (0x00004004)

#define MA_IOC_MAGIC            'M'
//#define MA_IOC_INIT           _IOR(MA_IOC_MAGIC, 0, unsigned char)
#define TIMEOUT_WAKELOCK        _IO(MA_IOC_MAGIC, 1)
#define SLEEP                   _IO(MA_IOC_MAGIC, 2)    //ÏÝÈëÄÚºË
#define WAKEUP                  _IO(MA_IOC_MAGIC, 3)    //»½ÐÑ
#define ENABLE_CLK              _IO(MA_IOC_MAGIC, 4)    //´ò¿ªspiÊ±ÖÓ
#define DISABLE_CLK             _IO(MA_IOC_MAGIC, 5)    //¹Ø±ÕspiÊ±ÖÓ
#define ENABLE_INTERRUPT        _IO(MA_IOC_MAGIC, 6)    //¿ªÆôÖÐ¶ÏÉÏ±¨
#define DISABLE_INTERRUPT       _IO(MA_IOC_MAGIC, 7)    //¹Ø±ÕÖÐ¶ÏÉÏ±¨
#define TAP_DOWN                _IO(MA_IOC_MAGIC, 8)
#define TAP_UP                  _IO(MA_IOC_MAGIC, 9)
#define SINGLE_TAP              _IO(MA_IOC_MAGIC, 11)
#define DOUBLE_TAP              _IO(MA_IOC_MAGIC, 12)
#define LONG_TAP                _IO(MA_IOC_MAGIC, 13)

#define MA_IOC_VTIM             _IOR(MA_IOC_MAGIC,  14, unsigned char)     //version time
#define MA_IOC_CNUM             _IOR(MA_IOC_MAGIC,  15, unsigned char)     //cover num
#define MA_IOC_SNUM             _IOR(MA_IOC_MAGIC,  16, unsigned char)     //sensor type
#define MA_IOC_UKRP             _IOW(MA_IOC_MAGIC,  17, unsigned char)     //user define the report key

#define MA_KEY_UP                  _IO(MA_IOC_MAGIC,  18)                  //nav up
#define MA_KEY_LEFT                _IO(MA_IOC_MAGIC,  19)                  //nav left
#define MA_KEY_DOWN                _IO(MA_IOC_MAGIC,  20)                  //nav down
#define MA_KEY_RIGHT               _IO(MA_IOC_MAGIC,  21)                  //nav right

#define MA_KEY_F14                 _IO(MA_IOC_MAGIC,  23)  //for chuanyin
#define SET_MODE                _IOW(MA_IOC_MAGIC, 33, unsigned int)    //for yude
#define GET_MODE                _IOR(MA_IOC_MAGIC, 34, unsigned int)    //for yude


#define ENABLE_IRQ               _IO(MA_IOC_MAGIC, 31)
#define DISABLE_IRQ              _IO(MA_IOC_MAGIC, 32)

#define MA_IOC_GVER             _IOR(MA_IOC_MAGIC,   35, unsigned int)      //get the driver version,the version mapping in the u32 is the final  4+4+8,as ******** ******* ****(major verson number) ****(minor version number) ********(revised version number), the front 16 byte is reserved.
#define SCREEN_OFF              _IO(MA_IOC_MAGIC,    36)
#define SCREEN_ON               _IO(MA_IOC_MAGIC,    37)
#define SET_SPI_SPEED           _IOW(MA_IOC_MAGIC,   38, unsigned int)


#define WAIT_FACTORY_CMD        _IO(MA_IOC_MAGIC,    39)//for fingerprintd
#define WAKEUP_FINGERPRINTD     _IO(MA_IOC_MAGIC,    40)//for factory test
#define WAIT_FINGERPRINTD_RESPONSE                                  _IOR(MA_IOC_MAGIC,    41, unsigned int)//for factory test
#define WAKEUP_FACTORY_TEST_SEND_FINGERPRINTD_RESPONSE              _IOW(MA_IOC_MAGIC,    42, unsigned int)//for fingerprintd
#define WAIT_SCREEN_STATUS_CHANGE                                   _IOR(MA_IOC_MAGIC,    43, unsigned int)
#define GET_INTERRUPT_STATUS                                        _IOR(MA_IOC_MAGIC,    44, unsigned int)
#define MA_SYNC                 _IO(MA_IOC_MAGIC, 45)
#define MA_SYNC2                 _IO(MA_IOC_MAGIC, 46)


#endif /* __MADEV_H_ */




