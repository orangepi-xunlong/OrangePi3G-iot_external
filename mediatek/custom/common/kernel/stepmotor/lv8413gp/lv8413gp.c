/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/fb.h>
#include <linux/poll.h>

#include <linux/slab.h>
#include <linux/miscdevice.h>
#include <linux/delay.h>

#include<linux/timer.h> 
#include<linux/jiffies.h>

#include <mach/mt_gpio.h>
#include "lv8413gp.h"


#define DEVICE_NAME			"stepmotor"


static DECLARE_WAIT_QUEUE_HEAD(stepmotor_waitq);
static volatile int busy_status = 0;
struct timer_list remote_timer;
static struct stepmotor_data *sm_data = NULL;
struct class *stepmotor_class;

int phase12_step_count = 0;
int phase02_step_count = 0;

int stepmotor_enable = 1;
int fmenable = 1;

static void remote_timer_handle(unsigned long _data)
{
	if (!busy_status)
	{
		mod_timer(&remote_timer,jiffies+msecs_to_jiffies(100)); 
		//printk("stepmotor wake up interruptiable remote watiq \n");
		wake_up_interruptible(&stepmotor_waitq);
	}
}

static long stepmotor_ioctl(struct file *filp, unsigned int cmd,unsigned long arg)
{
	switch(cmd) 
	{

	}

	return 0;
}

static unsigned int stepmotor_poll( struct file *file,struct poll_table_struct *wait)
{
	unsigned int mask = 0;

	mod_timer(&remote_timer, jiffies + msecs_to_jiffies(40));
	printk("stepmotor---poll---\n");

	poll_wait(file, &stepmotor_waitq, wait);
	if (busy_status)
	{
		busy_status=0;
		mask |= POLLIN | POLLRDNORM;
	}

	return mask;
}

static ssize_t stepmotor_write(struct file *file, const char *buffer,size_t count, loff_t *ppos)
{
	return 0;
}

static ssize_t stepmotor_read(struct file *filp, char *buff,size_t count, loff_t *ppos)
{
	return 0;
}

static int stepmotor_open(struct inode *inode, struct file *file)
{
	setup_timer(&remote_timer, remote_timer_handle,(unsigned long)"stepmotor");

	stepmotor_pwr_enable(1);

	printk("stepmotor---open OK---\n");

	return 0;
}

static int stepmotor_close(struct inode *inode, struct file *file)
{
	del_timer_sync(&remote_timer);

	stepmotor_pwr_enable(0);

	printk("stepmotor---close---\n");

	return 0;
}

static struct file_operations stepmotor_ops = {
	.owner			= THIS_MODULE,
	.open			= stepmotor_open,
	.release		= stepmotor_close, 
	.write          	= stepmotor_write,
	.read			= stepmotor_read,
	.poll			= stepmotor_poll,
	.unlocked_ioctl		= stepmotor_ioctl,
	
};

static void ir_stepmotor_work(struct stepmotor_data *sm_data)
{
	struct stepmotor_data *data = sm_data;

}

static ssize_t stepmotor_two_phase_show(struct device* dev,struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	
	sprintf(buf, "stepmotor_one_phase_show %d\n",stepmotor_enable);
	
	ret = strlen(buf) + 1;

	return ret;
}
static ssize_t stepmotor_two_phase_store(struct device* dev, struct device_attribute *attr,const char* buf, size_t size)
{
	unsigned long on_off = simple_strtoul(buf, NULL, 10);
    	int ret=0;

	printk("%s  on_off = %d\n", __func__, on_off);
    
	if(on_off == 11)
	{
		stepmotor_cw_two_phase_excitation();
	}
	else
	{
		stepmotor_ccw_two_phase_excitation();
	}


	printk("%s: stepmotor_enable = %d\n",__func__, stepmotor_enable);
	
	return size;	
}

static ssize_t stepmotor_one_two_phase_show(struct device* dev,struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	
	sprintf(buf, "stepmotor_one_two_phase_show %d\n",stepmotor_enable);
	
	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t stepmotor_one_two_phase_store(struct device* dev, struct device_attribute *attr,const char* buf, size_t size)
{
	unsigned long on_off = simple_strtoul(buf, NULL, 10);
    	int ret=0;

	printk("%s  on_off = %d\n", __func__, on_off);
    
	if(on_off == 11)
	{
		stepmotor_cw_one_two_phase_excitation();
	}
	else
	{
		stepmotor_ccw_one_two_phase_excitation();
	}


	//printk("%s: stepmotor_enable = %d\n",__func__, stepmotor_enable);
	
	return size;	
}

static ssize_t stepmotor_quick_show(struct device* dev,struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	
	sprintf(buf, "stepmotor_one_two_phase_show %d\n",stepmotor_enable);
	
	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t stepmotor_quick_store(struct device* dev, struct device_attribute *attr,const char* buf, size_t size)
{
	unsigned long on_off = simple_strtoul(buf, NULL, 10);
    	int ret=0;

	printk("%s  on_off = %d\n", __func__, on_off);
    
	if(on_off == 11)
	{
		stepmotor_cw_two_phase_excitation_206();
	}
	else
	{
		stepmotor_ccw_two_phase_excitation_206();
	}


	//printk("%s: stepmotor_enable = %d\n",__func__, stepmotor_enable);
	
	return size;	
}

static ssize_t stepmotor_cw_slow_show(struct device* dev,struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	
	sprintf(buf, "stepmotor_one_two_phase_show %d\n",stepmotor_enable);
	
	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t stepmotor_cw_slow_store(struct device* dev, struct device_attribute *attr,const char* buf, size_t size)
{
	int nums = 0;
	
	if(1 == sscanf(buf, "%d", &nums))
	{
		printk("%s: nums = %d\n",__func__, nums);
		stepmotor_cw_one_two_phase_excitation_slow(nums);
	}
	
	return size;	
}

static ssize_t stepmotor_ccw_slow_show(struct device* dev,struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	
	sprintf(buf, "stepmotor_one_two_phase_show %d\n",stepmotor_enable);
	
	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t stepmotor_ccw_slow_store(struct device* dev, struct device_attribute *attr,const char* buf, size_t size)
{
	int nums = 0;
	
	if(1 == sscanf(buf, "%d", &nums))
	{
		printk("%s: nums = %d\n",__func__, nums);
		stepmotor_ccw_one_two_phase_excitation_slow(nums);
	}
	
	return size;	
}

static ssize_t stepmotor_stop_show(struct device* dev,struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	
	sprintf(buf, "stepmotor_one_two_phase_show %d\n",stepmotor_enable);
	
	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t stepmotor_stop_store(struct device* dev, struct device_attribute *attr,const char* buf, size_t size)
{
	int nums = 0;
	
	if(1 == sscanf(buf, "%d", &nums))
	{
		printk("%s: nums = %d\n",__func__, nums);
		stepmotor_enable = nums;
	}
	
	printk("%s: stepmotor_enable=%d\n",__func__, stepmotor_enable);
	
	return size;	
}

static ssize_t stepmotor_fmenable_show(struct device* dev,struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	
	sprintf(buf, "stepmotor_fmenable_show %d\n",fmenable);
	
	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t stepmotor_fmenable_store(struct device* dev, struct device_attribute *attr,const char* buf, size_t size)
{
	int nums = 0;
	
	if(1 == sscanf(buf, "%d", &nums))
	{
		printk("%s: nums = %d\n",__func__, nums);
		fmenable = nums;
	}
	
	printk("%s: fmenable=%d\n",__func__, fmenable);
	
	return size;	
}

static ssize_t stepmotor_fmtest_show(struct device* dev,struct device_attribute *attr, char* buf)
{
	ssize_t ret = 0;
	
	sprintf(buf, "stepmotor_fmtest_show %d\n",fmenable);
	
	ret = strlen(buf) + 1;

	return ret;
}

static ssize_t stepmotor_fmtest_store(struct device* dev, struct device_attribute *attr,const char* buf, size_t size)
{
	unsigned long on_off = simple_strtoul(buf, NULL, 10);
    	int ret=0;

	printk("%s  on_off = %d\n", __func__, on_off);
    
	if(on_off == 11)
	{
		stepmotor_cw_two_phase_excitation_fmtest();
	}
	else
	{
		stepmotor_ccw_two_phase_excitation_fmtest();
	}


	printk("%s: fmenable = %d\n",__func__, fmenable);
	
	return size;	
}

static DEVICE_ATTR(sm_two_phase, 0666, stepmotor_two_phase_show, stepmotor_two_phase_store);
static DEVICE_ATTR(sm_one_two_phase, 0666, stepmotor_one_two_phase_show, stepmotor_one_two_phase_store);
static DEVICE_ATTR(quick, 0666, stepmotor_quick_show, stepmotor_quick_store);//206
static DEVICE_ATTR(cw_slow, 0666, stepmotor_cw_slow_show, stepmotor_cw_slow_store);
static DEVICE_ATTR(ccw_slow, 0666, stepmotor_ccw_slow_show, stepmotor_ccw_slow_store);
static DEVICE_ATTR(stop, 0666, stepmotor_stop_show, stepmotor_stop_store);
static DEVICE_ATTR(fmtest, 0666, stepmotor_fmtest_show, stepmotor_fmtest_store);//150
static DEVICE_ATTR(fmenable, 0666, stepmotor_fmenable_show, stepmotor_fmenable_store);


static struct miscdevice sm_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &stepmotor_ops,
};

static int __init stepmotor_dev_init(void) {

	struct device *stepmotor_dev;
	int error;

	stepmotor_hw_init();

	sm_data = kzalloc(sizeof(struct stepmotor_data), GFP_KERNEL);
	if (NULL == sm_data)
	{
		printk("Failed to data allocate %s\n", __func__);
		error = -ENOMEM;
		goto err_free_mem;
	}

	
	printk("stepmotor initialized begin ....\n");
	stepmotor_class = class_create(THIS_MODULE, "stepmotor");
	if (IS_ERR(stepmotor_class))
	{
		pr_err("Failed to create class(sec)!\n");
		return PTR_ERR(stepmotor_class);
	}
		
	stepmotor_dev = device_create(stepmotor_class, NULL, 0, sm_data, "lv8413gp");
	if (IS_ERR(stepmotor_dev))
	{
		printk("Failed to create stepmotor_dev device\n");
	}
	
#if 0
	if (device_create_file(stepmotor_dev, &dev_attr_sm_one_phase) < 0)
	{
		printk("Failed to create device file(%s)!\n",dev_attr_sm_one_phase.attr.name);	
	}
	if (device_create_file(stepmotor_dev, &dev_attr_sm_two_phase) < 0)
	{
		printk("Failed to create device file(%s)!\n",dev_attr_sm_two_phase.attr.name);	
	}
	if (device_create_file(stepmotor_dev, &dev_attr_sm_one_two_phase) < 0)
	{
		printk("Failed to create device file(%s)!\n",dev_attr_sm_one_two_phase.attr.name);	
	}
#endif
	if (device_create_file(stepmotor_dev, &dev_attr_quick) < 0)
	{
		printk("Failed to create device file(%s)!\n",dev_attr_quick.attr.name);	
	}
	if (device_create_file(stepmotor_dev, &dev_attr_cw_slow) < 0)
	{
		printk("Failed to create device file(%s)!\n",dev_attr_cw_slow.attr.name);	
	}
	if (device_create_file(stepmotor_dev, &dev_attr_ccw_slow) < 0)
	{
		printk("Failed to create device file(%s)!\n",dev_attr_ccw_slow.attr.name);	
	}
	if (device_create_file(stepmotor_dev, &dev_attr_stop) < 0)
	{
		printk("Failed to create device file(%s)!\n",dev_attr_stop.attr.name);	
	}
	if (device_create_file(stepmotor_dev, &dev_attr_fmtest) < 0)
	{
		printk("Failed to create device file(%s)!\n",dev_attr_fmtest.attr.name);	
	}
	if (device_create_file(stepmotor_dev, &dev_attr_fmenable) < 0)
	{
		printk("Failed to create device file(%s)!\n",dev_attr_fmenable.attr.name);	
	}
	
	if((error = misc_register(&sm_misc_dev)))
	{
		printk("stepmotor: misc_register register failed\n");
	}

	printk("stepmotor initialized end .....\n");
	return error;

err_free_mem:
	kfree(sm_data);
	return error;
}

static void __exit stepmotor_dev_exit(void)
{
	misc_deregister(&sm_misc_dev);	
}

module_init(stepmotor_dev_init);
module_exit(stepmotor_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("aren.jiang@runyee.com.cn");
MODULE_DESCRIPTION("runyee stepmotor driver");

