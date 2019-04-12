#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/types.h>
#include <linux/wait.h>
#include <linux/slab.h>
#include <linux/fs.h>
#include <linux/sched.h>
#include <linux/poll.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/cdev.h>
#include <linux/errno.h>
#include <linux/time.h>
#include "kd_flashlight.h"
#include <asm/io.h>
#include <asm/uaccess.h>
#include "kd_camera_hw.h"
#include <linux/hrtimer.h>
#include <linux/ktime.h>
#include <linux/xlog.h>
#include <mach/upmu_common.h>

#include <mach/mt_gpio.h>

struct class *iotest_class;
struct device *iotest_dev;
#define PLATFORM_DRIVER_NAME "gpiotest"


static const u16 gpio_export_map[] = {
	GPIO24,GPIO25,GPIO26,GPIO27,GPIO30,GPIO56,GPIO58,
	GPIO89,GPIO90,GPIO97,GPIO98,GPIO99,GPIO100,
	GPIO128,GPIO139,GPIO140,GPIO141,GPIO144,GPIO145,
};


/* test interface */
static ssize_t runyee_gpiotest_store(struct device *dev,struct device_attribute *attr, const char *buf, size_t size)
{
    int enable = 0;
	unsigned char i = 0;
	
    if(buf != NULL && size != 0)
    {
        enable = (int)simple_strtoul(buf, NULL, 0);
    }
	
    if(enable)
    {
		for (i = 0; i < ARRAY_SIZE(gpio_export_map); i++)
		{
			mt_set_gpio_mode(gpio_export_map[i], GPIO_MODE_GPIO);
			mt_set_gpio_out(gpio_export_map[i], 1);
		}
    }
    else
    {
		for (i = 0; i < ARRAY_SIZE(gpio_export_map); i++)
		{
			mt_set_gpio_mode(gpio_export_map[i], GPIO_MODE_GPIO);
			mt_set_gpio_out(gpio_export_map[i], 0);
		}
    }
	
    return size;
}
static DEVICE_ATTR(runyee_gpiotest, 0644, NULL, runyee_gpiotest_store);


/* platform structure */
/**************************get hdmi dts pams***************************/

void iotest_init(void)
{
	unsigned char i = 0;

	for (i = 0; i < ARRAY_SIZE(gpio_export_map); i++)
	{
		mt_set_gpio_mode(gpio_export_map[i], GPIO_MODE_GPIO);
		mt_set_gpio_out(gpio_export_map[i], 0);
	}
}

static int platform_iotest_probe(struct platform_device *pdev)
{
	printk("[runyee gpio test]  %s \n", __func__);	

	iotest_init();
	return 0;
}

static int platform_iotest_remove(struct platform_device *pdev)
{
	printk("[runyee gpio test]	%s \n", __func__);

	return 0;
}

static int platform_iotest_suspend(struct platform_device *pdev, pm_message_t mesg)
{
	return 0;
}

static int platform_iotest_resume(struct platform_device *pdev)
{
	return 0;
}

static struct platform_driver g_iotest_Driver = {
	.probe = platform_iotest_probe,
	.remove = platform_iotest_remove,
	.suspend = platform_iotest_suspend,
	.resume = platform_iotest_resume,
	.driver = {
		   .name = PLATFORM_DRIVER_NAME,
		   .owner = THIS_MODULE,
		   }
};

static struct platform_device g_iotest_device = {
	.name = PLATFORM_DRIVER_NAME,
	.id = 0,
	.dev = {}
};

static int __init platform_iotest_init(void)
{
#if 1
	iotest_class = class_create(THIS_MODULE, "gpiotest");
	iotest_dev = device_create(iotest_class,NULL, 0, NULL,  "gpiotest");
    device_create_file(iotest_dev, &dev_attr_runyee_gpiotest);		

	if (platform_device_register(&g_iotest_device)) {
		printk("failed to register iotest device\n");
		return -1;
	}

	if (platform_driver_register(&g_iotest_Driver)) {
		printk("failed to register iotest driver\n");
		return -1;
	}
#endif
	return 0;
}

static void __exit platform_iotest_exit(void)
{
	platform_driver_unregister(&g_iotest_Driver);
}

module_init(platform_iotest_init);
module_exit(platform_iotest_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("board iotest driver");
MODULE_AUTHOR("jst<aren.jiang@runyee.com.cn>");
