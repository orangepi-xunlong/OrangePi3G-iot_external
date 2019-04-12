/* MicroArray Fingerprint
 * madev.c
 * date: 2015-08-15
 * version: v2.0
 * Author: czl
 */
#include "asm/uaccess.h"
#include "linux/freezer.h"

#include <asm/memory.h>
#include <asm/uaccess.h>
#include <asm/dma.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/ioctl.h>
#include <linux/fs.h>
#include <linux/device.h>
#include <linux/list.h>
#include <linux/mutex.h>
#include <linux/slab.h>
#include <linux/compat.h>
#include <linux/spi/spi.h>
#include <linux/interrupt.h>
#include <linux/workqueue.h>
#include <linux/kthread.h>
#include <linux/sched.h>
#include <linux/wait.h>
#include <linux/time.h>
#include <linux/input.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/earlysuspend.h>
#include <linux/kobject.h> 
#include <linux/platform_device.h>
#include <linux/delay.h>
#include "print.h"
#include "madev.h"
//#include "plat-general.h"
#include "plat-mtk.h"

#define REPORT_KEY 68

//static unsigned int irq, ret;
static unsigned int  ret;
static unsigned int ma_drv_reg;
//static unsigned int ma_speed;
//static unsigned int is_screen_on;
//static struct notifier_block notifier;
//static unsigned int int_pin_state;
//static unsigned int compatible;
//static unsigned int screen_flag;
static DECLARE_WAIT_QUEUE_HEAD(screenwaitq);
static DECLARE_WAIT_QUEUE_HEAD(sync_waitq);
static DECLARE_WAIT_QUEUE_HEAD(gWaitq);
static DECLARE_WAIT_QUEUE_HEAD(U1_Waitq);
static DECLARE_WAIT_QUEUE_HEAD(U2_Waitq);
struct wake_lock gIntWakeLock;
struct wake_lock gProcessWakeLock;
struct work_struct gWork;
struct workqueue_struct *gWorkq;
static unsigned int sync_flag;
//
//static LIST_HEAD(dev_list);
//static DEFINE_MUTEX(dev_lock);
//static DEFINE_MUTEX(drv_lock);
static DEFINE_MUTEX(ioctl_lock);

#if 0 //add by gur
#include <linux/hw_module_info.h>

/******droi  add tmd hardware info start*****/
static hw_module_info hw_info = {
	.type = HW_MODULE_TYPE_FP,
	.id = 0x78,
	.priority = HW_MODULE_PRIORITY_FP,
	.name = "MA120N",
	.vendor = "vendor",
	.more = "already"
};
static hw_module_info hw_info1 = {
	.type = HW_MODULE_TYPE_FP,
	.id = 0x79,
	.priority = HW_MODULE_PRIORITY_FP,
	.name = "MA121A",
	.vendor = "vendor",
	.more = "already"
};
static hw_module_info hw_info2 = {
	.type = HW_MODULE_TYPE_FP,
	.id = 0x50,
	.priority = HW_MODULE_PRIORITY_FP,
	.name = "MA080T",
	.vendor = "vendor",
	.more = "already"
};
/*****droi  add tmd hardware info end******/
#endif //add by gur

struct fprint_dev {
	dev_t idd;
	int major;
	int minor;
	struct cdev *chd;
	struct class *cls;
	struct device *dev;
};
static struct fprint_dev *sdev = NULL;

struct fprint_spi {
	u8 do_what;             //¹¤×÷ÄÚÈÝ
    u8 f_wake;              //»½ÐÑ±êÖ¾  
    int value;              

 	volatile u8 u1_flag;    //reserve for ours thread interrupt
    volatile u8 u2_flag;    //reserve for ours thread interrupt
	volatile u8 f_irq;		//中断标志
	volatile u8 f_repo;	//上报开关	
	
	spinlock_t spi_lock;
	struct spi_device *spi;
	struct list_head dev_entry;
	struct spi_message msg;
	struct spi_transfer xfer;
	struct input_dev *input;
	struct work_struct work;
	struct workqueue_struct *workq;
	struct work_struct press_work;
	struct workqueue_struct *press_workq;
#ifdef CONFIG_HAS_EARLYSUSPEND
	struct early_suspend suspend;
#endif
	//struct wake_lock wl;
};


static struct fprint_spi *smas = NULL;
//static u8 *stxb = NULL;
//static u8 *srxb = NULL;

static u8 stxb[FBUF];
static u8 srxb[FBUF];

static LIST_HEAD(dev_list);
static DEFINE_MUTEX(dev_lock);
static DEFINE_MUTEX(drv_lock);

int mas_sync(u8 *txb, u8 *rxb, int len);


#if 0
void mas_enable_spi_clock(struct spi_device *spi){
	mt_spi_enable_clk(spi_master_get_devdata(spi->master)); 
}

void mas_disable_spi_clock(struct spi_device *spi){
	mt_spi_disable_clk(spi_master_get_devdata(spi->master)); 
}
#endif


#if 0
static void mas_work(struct work_struct *pws) {
	printd("%s: start.\n", __func__);

	input_report_key(smas->input, KEY_F1, 1);
	input_sync(smas->input);
	input_report_key(smas->input, KEY_F1, 0);
	input_sync(smas->input);
	
	printd("%s: end. report key F1\n", __func__);
}

static void mas_press_work(struct work_struct *pws) {
	printd("%s: start.\n", __func__);

	input_report_key(smas->input, REPORT_KEY, 1);
	input_sync(smas->input);
	input_report_key(smas->input, REPORT_KEY, 0);
	input_sync(smas->input);
	
	printd("%s: end. report key REPORT_KEY\n", __func__);
}

static irqreturn_t mas_interrupt(int irq, void *dev_id) {
	printk("%s: start. f_irq=%d\n", __func__, smas->f_irq); 

	plat_enable_irq(smas->spi, 0);
	if(smas->f_irq==0) {
		smas->f_irq = 1;
		if(smas->f_repo==REPORT_ON) queue_work(smas->workq, &smas->work);
	}

	printw("%s: end. f_irq=%d\n", __func__, smas->f_irq);

	return IRQ_HANDLED;
}
#endif

static void mas_work(struct work_struct *pws) {
    smas->f_irq = 1;
	printk("mas_work is come1 \n");
    //wake_up(&gWaitq);
	wake_up(&gWaitq);
	printk("%s,mas_work is come2 \n",__func__);
#ifdef COMPATIBLE_VERSION3
	wake_up(&drv_waitq);
#endif
}

static void mas_interrupt(void){
//static irqreturn_t mas_interrupt(int irq, void *dev_id) {
#if 0
#ifdef DOUBLE_EDGE_IRQ
	if(mas_get_interrupt_gpio(0)==1){
		//TODO IRQF_TRIGGER_RISING
	}else{
		//TODO IRQF_TRIGGER_FALLING
	}
#else
    queue_work(gWorkq, &gWork);
#endif
#endif
	
	//plat_enable_irq(smas->spi, 0);
	printk("%s,mas_interrupt is come \n",__func__);
	queue_work(gWorkq, &gWork);
	//return IRQ_HANDLED;
}



/*---------------------------------- fops ------------------------------------*/
static int mas_open(struct inode *inode, struct file *filp) {
	int ret = -ENXIO;

	printd("%s: start\n", __func__);

	mutex_lock(&dev_lock);
	list_for_each_entry(smas, &dev_list, dev_entry) {
		if (sdev->idd == inode->i_rdev) {
			ret = 0;
			break;
		}
	}
	if (!ret) nonseekable_open(inode, filp);
	else printw("%s: nothing for minor\n", __func__);
	mutex_unlock(&dev_lock);
		
	printd("%s: end ret=%d\n", __func__, ret);

	return ret;
}

/* 读写数据
 * @buf 数据
 * @len 长度
 * @返回值：0成功，否则失败
 */
int mas_sync(u8 *txb, u8 *rxb, int len) {
	int ret;

	//printd("%s: start. cmd=0x%.2x len=%d\n", __func__, buf[0], len);

	mutex_lock(&drv_lock);
	plat_tansfer(smas->spi, len);
	smas->xfer.tx_buf = txb;
	smas->xfer.rx_buf = rxb;
	smas->xfer.delay_usecs = 1;
	smas->xfer.len = len;
	smas->xfer.bits_per_word = 8;
	smas->xfer.speed_hz = smas->spi->max_speed_hz;
	spi_message_init(&smas->msg);
	spi_message_add_tail(&smas->xfer, &smas->msg);
	ret = spi_sync(smas->spi, &smas->msg);
	mutex_unlock(&drv_lock);

	//printd("%s: end. ret=%d\n", __func__, ret);

	return ret;
}

/* 读数据
 * @return 成功:count, -1count太大，-2通讯失败, -3拷贝失败
 */
static ssize_t mas_read(struct file *filp, char __user *buf, size_t count, loff_t *f_pos) {
	int val, ret = 0;

	printd("%s: start. count=%d\n", __func__, count);
	
	//mutex_lock(&drv_lock);
#if 0   //add by gur
	if(count>FBUF) {
		printw("%s: too long. count=%dn", __func__, count);
		return -1;
	}
#endif //add by gur
	ret = mas_sync(stxb, srxb, count);
	if(ret) {
		printw("%s: mas_sync failed.\n", __func__);
		return -2;
	}
	ret = copy_to_user(buf, srxb, count);
	if(!ret) val = count;
	else {
		val = -3;
		printw("%s: copy_to_user failed.\n", __func__);
	}	
	//mutex_unlock(&drv_lock);

	printd("%s: end. ret=%d\n", __func__, ret);

	return val;
}
#if 0
int mas_ioctl(int cmd, int arg) {
	int ret = 0;

	printw("%s: start cmd=0x%.3x arg=%d\n", __func__, cmd, arg);

	switch (cmd) {
	case IOCTL_DEBUG:	
		sdeb = (u8) arg;
		break;
	case IOCTL_IRQ_ENABLE:
		plat_enable_irq(smas->spi, arg);
		break;
	case IOCTL_SPI_SPEED: //设置SPI速度
		smas->spi->max_speed_hz = (u32) arg;
		spi_setup(smas->spi);
		break;	
	case IOCTO_NUM_STUFF: //获取材料编号: 1:陶瓷，2:塑封
		ret = SENSOR_TYPE;
		break;
	case IOCTL_GET_VDATE:
		ret = 20150930;
		break;
	case IOCTL_RES_INTF:
		smas->f_irq = 0;
		break;
	case IOCTL_GET_INTF:
		ret = smas->f_irq;
		break;
	case IOCTL_KEY_REPO:
		smas->f_repo = arg;
		break;
	case IOCTL_PRESS:
		queue_work(smas->press_workq, &smas->press_work);
		break;	
	}

	printw("%s: end. ret=%d f_irq=%d\n", __func__, ret, smas->f_irq);

	return ret;
}
#endif



//static int mas_ioctl (struct inode *node, struct file *filp, unsigned int cmd, uns igned long arg)           
//this function only supported while the linux kernel version under v2.6.36,while the kernel version under v2.6.36, use this line
static long mas_ioctl(struct file *filp, unsigned int cmd, unsigned long arg) {
    //MALOGF("start");
    switch(cmd){
        case TIMEOUT_WAKELOCK:                                                       //ÑÓÊ±Ëø    timeout lock
            wake_lock_timeout(&gProcessWakeLock, 5*HZ);
            break;
        case SLEEP:                                                       //remove the process out of the runqueue
            smas->f_irq = 0;
	    ret = wait_event_freezable(gWaitq, smas->f_irq != 0);
	    //ret = wait_event_interruptible(gWaitq, smas->f_irq != 0);
	    break;
        case WAKEUP:                                                       //wake up, schedule the process into the runqueue
            smas->f_irq = 1;
            wake_up(&gWaitq);
	    //wake_up_interruptible(&gWaitq);
            break;
#if 0
        case ENABLE_CLK:
            mas_enable_spi_clock(smas->spi);                                    //if the spi clock is not opening always, do this methods
            break;
        case DISABLE_CLK:
            mas_disable_spi_clock(smas->spi);                                   //disable the spi clock
            break;
#endif
        case ENABLE_IRQ:
			plat_enable_irq(smas->spi, 1);
            //enable_irq(irq);                                                    //enable the irq,in fact, you can make irq enable always
            break;
        case DISABLE_IRQ:
			plat_enable_irq(smas->spi, 0);
            //disable_irq(irq);                                                    //disable the irq
            break;
        case TAP_DOWN:
            input_report_key(smas->input, 68, 1);
            input_sync(smas->input);                                                 //tap down
            break;
        case TAP_UP:
            input_report_key(smas->input, 68, 0);
            input_sync(smas->input);                                                     //tap up
            break;
        case SINGLE_TAP:
            input_report_key(smas->input, FINGERPRINT_TAP, 1);
            input_sync(smas->input);
            input_report_key(smas->input, FINGERPRINT_TAP, 0);
            input_sync(smas->input);                                                       //single tap
            break;
        case DOUBLE_TAP:
            input_report_key(smas->input, FINGERPRINT_DTAP, 1);
            input_sync(smas->input);
            input_report_key(smas->input, FINGERPRINT_DTAP, 0);
            input_sync(smas->input);                                              //double tap
            break;
        case LONG_TAP:
            input_report_key(smas->input, FINGERPRINT_LONGPRESS, 1);
            input_sync(smas->input);
            input_report_key(smas->input, FINGERPRINT_LONGPRESS, 0);
            input_sync(smas->input);                                               //long tap
            break;
        case MA_KEY_UP:
            input_report_key(smas->input, FINGERPRINT_SWIPE_UP, 1);
            input_sync(smas->input);
            input_report_key(smas->input, FINGERPRINT_SWIPE_UP, 0);
            input_sync(smas->input);
            break;
        case MA_KEY_LEFT:
            input_report_key(smas->input, FINGERPRINT_SWIPE_LEFT, 1);
            input_sync(smas->input);
            input_report_key(smas->input, FINGERPRINT_SWIPE_LEFT, 0);
            input_sync(smas->input);
            break;
        case MA_KEY_DOWN:
            input_report_key(smas->input, FINGERPRINT_SWIPE_DOWN, 1);
            input_sync(smas->input);
            input_report_key(smas->input, FINGERPRINT_SWIPE_DOWN, 0);
            input_sync(smas->input);
            break;
        case MA_KEY_RIGHT:
            input_report_key(smas->input, FINGERPRINT_SWIPE_RIGHT, 1);
            input_sync(smas->input);
            input_report_key(smas->input, FINGERPRINT_SWIPE_RIGHT, 0);
            input_sync(smas->input);
            break;
        case SET_MODE:
            mutex_lock(&ioctl_lock);
            ret = copy_from_user(&ma_drv_reg, (unsigned int*)arg, sizeof(unsigned int));
            mutex_unlock(&ioctl_lock);
            break;
        case GET_MODE:
            mutex_lock(&ioctl_lock);
            ret = copy_to_user((unsigned int*)arg, &ma_drv_reg, sizeof(unsigned int));
            mutex_unlock(&ioctl_lock);
            break;
        case MA_IOC_GVER:
            mutex_lock(&ioctl_lock);
            *((unsigned int*)arg) = MA_DRV_VERSION;
            mutex_unlock(&ioctl_lock);
            break;
		case SCREEN_OFF:
			//mas_switch_power(0);
			break;
		case SCREEN_ON:
			//mas_switch_power(1);
			break;
        case SET_SPI_SPEED:
            //ret = copy_from_user(&ma_speed, (unsigned int*)arg, sizeof(unsigned int));
            //ma_spi_change(smas->spi, ma_speed, 0);
            break;
        case WAIT_FACTORY_CMD:
            smas->u2_flag = 0;
            ret = 	wait_event_freezable(U2_Waitq, smas->u2_flag != 0);
			break;
        case WAKEUP_FINGERPRINTD:
            smas->u2_flag = 1;
            wake_up(&U2_Waitq);
            break;
        case WAIT_FINGERPRINTD_RESPONSE:
            smas->u1_flag = 0;
            ret = wait_event_freezable(U1_Waitq,  smas->u1_flag != 0);
            mutex_lock(&ioctl_lock);
            ret = copy_to_user((unsigned int*)arg, &ma_drv_reg, sizeof(unsigned int));
            mutex_unlock(&ioctl_lock);
			break;
        case WAKEUP_FACTORY_TEST_SEND_FINGERPRINTD_RESPONSE:
            mutex_lock(&ioctl_lock);
            ret = copy_from_user(&ma_drv_reg, (unsigned int*)arg, sizeof(unsigned int));
            mutex_unlock(&ioctl_lock);
            msleep(4);
            smas->u1_flag = 1;
            wake_up(&U1_Waitq);
            break;
		case WAIT_SCREEN_STATUS_CHANGE:
#if 0
                        screen_flag = 0;
                        ret = wait_event_freezable(screenwaitq,  screen_flag != 0);
			mutex_lock(&ioctl_lock);
			ret = copy_to_user((unsigned int*)arg, &is_screen_on, sizeof(unsigned int));
			mutex_unlock(&ioctl_lock);
#endif
			break;
		case GET_INTERRUPT_STATUS:
#if 0
			int_pin_state = mas_get_interrupt_gpio(0);
			if(int_pin_state == 0 || int_pin_state == 1){
				mutex_lock(&ioctl_lock);
				ret = copy_to_user((unsigned int*)arg, &int_pin_state, sizeof(unsigned int));
				mutex_unlock(&ioctl_lock);
			}
#endif
			break;
        case MA_SYNC:
                sync_flag = 1;
                wake_up(&sync_waitq);
                ret = wait_event_freezable(sync_waitq, sync_flag == 0);
        case MA_SYNC2:
                sync_flag = 0;
                wake_up(&sync_waitq);
                ret = wait_event_freezable(sync_waitq, sync_flag == 1);default:
            ret = -EINVAL;
            MALOGW("mas_ioctl no such cmd");
    }
    //MALOGF("end");
    return ret;
}





/* 写数据
 * @return 成功:count, -1count太大，-2拷贝失败
 */
static ssize_t mas_write(struct file *filp, const char __user *buf, size_t count, loff_t *f_pos) {
	int val = 0;
    //int ret;

	printd("%s: start. count=%d\n", __func__, count);

	if(count==6) { //命令
		int cmd, arg;
		u8 tmp[6];
		ret = copy_from_user(tmp, buf, count);
		cmd = tmp[0];
		cmd <<= 8;
		cmd += tmp[1];
		arg = tmp[2];
		arg <<= 8;
		arg += tmp[3];
		arg <<= 8;
		arg += tmp[4];
		arg <<= 8;
		arg += tmp[5];
		printd("%s: cmd=0x%.3x arg=%d\n", __func__, cmd, arg);
		//val = mas_ioctl(cmd, arg);
	} else {
#if 0 //add by gur	
		if(count>FBUF) {
			printw("%s: write data too long.\n", __func__);
			return -1;
		}
#endif //add by gur
		//mutex_lock(&drv_lock);				
		memset(stxb, 0, FBUF);
		ret = copy_from_user(stxb, buf, count);		
		if(ret) {
			printw("%s: copy_from_user failed.\n", __func__);
			val = -2;
		} else {
			val = count;
		//	printn(__func__, stxb, count);
		}	
		//mutex_unlock(&drv_lock);
	}

	printd("%s: end. ret =%d\n", __func__, ret);

	return val;
}
#if 0
void * kernel_memaddr = NULL;
unsigned long kernel_memesize = 0;

int mas_mmap(struct file *filp, struct vm_area_struct *vma){
    unsigned long page;
    if ( !kernel_memaddr ) { 
        kernel_memaddr = kmalloc(68*1024, GFP_KERNEL);
        if( !kernel_memaddr ) { 
            return -1; 
        }   
    }   
    page = virt_to_phys((void *)kernel_memaddr) >> PAGE_SHIFT;
    vma->vm_page_prot=pgprot_noncached(vma->vm_page_prot);
    if( remap_pfn_range(vma, vma->vm_start, page, (vma->vm_end - vma->vm_start), 
                vma->vm_page_prot) )
        return -1; 
    vma->vm_flags |= VM_RESERVED;
    return 0;
}
#endif

static int mas_release(struct inode *inode, struct file *filp) {
	int ret = 0;

	printd("%s: start\n", __func__);

	printd("%s: end\n", __func__);

	return ret;
}

static const struct file_operations sfops = {
	.owner = THIS_MODULE,
	.write = mas_write,
	.read = mas_read,
	.open = mas_open,
	.release = mas_release,
	.unlocked_ioctl = mas_ioctl,
	//.ioctl = mas_ioctl,       
    //using the previous line replacing the unlock_ioctl while the linux kernel under version2.6.36

};

static void mas_set_input(void) {
	struct input_dev *input = NULL;
	int ret = 0;

	printd("%s: start.\n", __func__);

	input = input_allocate_device();
	if (!input) {
		printw("%s: input_allocate_device failed.\n", __func__);
		return ;
	}
	set_bit(EV_KEY, input->evbit);
	set_bit(EV_ABS, input->evbit);
	set_bit(EV_SYN, input->evbit);
	set_bit(KEY_F1, input->keybit); //单触
	set_bit(KEY_F2, input->keybit);
	set_bit(KEY_F3, input->keybit);
	set_bit(KEY_F4, input->keybit);
	set_bit(KEY_F5, input->keybit);
	set_bit(68, input->keybit);
	set_bit(REPORT_KEY, input->keybit);
	set_bit(KEY_POWER, input->keybit);
	set_bit(BTN_TOUCH, input->keybit);
	set_bit(KEY_BACK, input->keybit);
	input->name = "madev";
    input->id.bustype = BUS_SPI;
	ret = input_register_device(input);
    if (ret) {
        input_free_device(input);
        printw("%s: failed to register input device.\n",__func__);
        return;
    }
	smas->input  = input;

	printd("%s: end.\n", __func__);
}
#ifdef CONFIG_HAS_EARLYSUSPEND
//void mas_suspend(struct device *dev, pm_message_t message)
void mas_suspend(struct early_suspend *handler) {
	printd("%s: start.\n", __func__);
	
	//mas_set_worker(FP_DRV);

	printd("%s: end.\n", __func__);
}

//void mas_resume(struct device *dev)
void mas_resume(struct early_suspend *handler) {
	printd("%s: start.\n", __func__);

	//mas_set_worker(FP_APP);

	printd("%s: end.\n", __func__);
}
#endif

int mas_probe(struct spi_device *spi) {
	int ret = 0;
	const char*tname = "madev_irq";

	printk("%s: start\n", __func__);
	mt_spi_set_mod();
	smas->spi = spi;
	smas->spi->max_speed_hz = SPI_SPEED;
	spi_setup(spi);
	spin_lock_init(&smas->spi_lock);
	INIT_LIST_HEAD(&smas->dev_entry);
	mutex_lock(&dev_lock);

	//device_init_wakeup(&spi->dev, 1);
#ifdef CONFIG_HAS_EARLYSUSPEND
	smas->suspend.level = EARLY_SUSPEND_LEVEL_BLANK_SCREEN + 1;
	smas->suspend.suspend = mas_suspend;
	smas->suspend.resume = mas_resume;
	register_early_suspend(&smas->suspend);
#endif
	plat_power(1);
	spi_set_drvdata(spi, smas);
	mas_set_input();

	INIT_WORK(&gWork, mas_work);
	//INIT_WORK(&smas->work, mas_work);
	//INIT_WORK(&smas->press_work, mas_press_work);
	//ret = plat_request_irq(spi->irq, mas_interrupt, IRQF_TRIGGER_RISING,
		//dev_name(&spi->dev), &spi->dev);

	//ret = plat_request_irq(spi->irq, mas_interrupt, IRQF_TRIGGER_RISING,
		//tname, &spi->dev);

	
	ret = plat_request_irq(spi->irq, tname, &spi->dev);
	mt_eint_registration(MADEV_EINT_NUM, CUST_EINTF_TRIGGER_RISING, mas_interrupt, 1);
	mt_eint_mask(MADEV_EINT_NUM);

	printk("%s: request_irq ret=%d\n", __func__, ret);
	if (ret) {
		printk("%s: request_irq failed.\n", __func__);
		goto fail;
	}
	plat_enable_irq(spi, 1);
	mutex_unlock(&dev_lock);

#if 0   //add by gur
/** 
 *    for test the hardware whether exist     
 */ 
    u8 r[4]={0}; 
    u8 t[4]={0}; 
    t[0]=0x8c; 
    t[1]=0xff; 
    t[2]=0xff; 
    t[3]=0xff; 
    mas_sync(t, r, 4);                 //for chip reset
    msleep(10);

    t[0]=0x00; 
    t[1]=0xff; 
    t[2]=0xff; 
    t[3]=0xff; 
    mas_sync(t, r, 4);                 //only read the 0x00 register
 
    if(r[3] != 0x41){ 
        spi_set_drvdata(spi, NULL); 
        input_unregister_device(smas->input); 
        cdev_del(sdev->chd); 
        device_destroy(sdev->cls, sdev->idd); 
        class_destroy(sdev->cls); 
        if(smas->workq) destroy_workqueue(smas->workq);     
    }

	/********************read hw_info start************************/
	// add by wangyongfu

		u8 rx[4]={0}; 
    		u8 tx[4]={0};
		tx[0]=0x04; 
    		tx[1]=0xff; 
    		tx[2]=0xff; 
    		tx[3]=0xff; 
		 mas_sync(tx, rx, 4);         	
		if(rx[3] == 0x78)//am120n
		{
			hw_module_info_add(&hw_info);	
		}
		else if(rx[3] == 0x79)//am121n
		{
			hw_module_info_add(&hw_info1);	
		}
		else if(rx[3] == 0x50)//am50n
		{
			hw_module_info_add(&hw_info2);	
		}
		else
		{				
			goto fail;			
		}
/*******************read hw_info end***************************/      
/** 
 *    for test the hardware whether exist     
 */
 #endif  //add by gur
	printw("%s: insmod successfully. version:2.0 time:0919_am11\n", __func__);
	return ret;

fail:
	printw("%s: insmod failed.\n", __func__);

	return ret;
}

int mas_remove(struct spi_device *spi) {
	printd("%s: start.\n", __func__);
	
	// make sure ops on existing fds can abort cleanly
	spin_lock_irq(&smas->spi_lock);
	plat_enable_irq(spi, 0);
	free_irq(spi->irq, &spi->dev);
	smas->spi = NULL;
	spi_set_drvdata(spi, NULL);
	spin_unlock_irq(&smas->spi_lock);

	// prevent new opens
	mutex_lock(&dev_lock);
	input_unregister_device(smas->input);
	//device_init_wakeup(&spi->dev, 0);
	mutex_unlock(&dev_lock);

#ifdef CONFIG_HAS_EARLYSUSPEND
	unregister_early_suspend(&smas->suspend);
#endif
	plat_power(0);

	printd("%s: end\n", __func__);

	return 0;
}

/*---------------------------------- module ------------------------------------*/
static int __init mas_init(void) {
	int ret;
	struct device *dev;
	
	printd("%s: start\n", __func__);

	sdev = kmalloc(sizeof(struct fprint_dev), GFP_KERNEL);
	smas = kmalloc(sizeof(struct fprint_spi), GFP_KERNEL);
	//srxb = kmalloc(FBUF*sizeof(u8), GFP_KERNEL);
	//stxb = kmalloc(FBUF*sizeof(u8), GFP_KERNEL);
	if (sdev==NULL || smas==NULL) {
		printw("%s: smas kmalloc failed.\n", __func__);
		if(sdev!=NULL) kfree(sdev);
		if(smas!=NULL) kfree(smas);
		//if(stxb!=NULL) kfree(stxb);
		//if(srxb!=NULL) kfree(srxb);
		return -ENOMEM;
	}
	
	//初始化cdev
	sdev->chd = cdev_alloc();
	cdev_init(sdev->chd, &sfops);
	sdev->chd->owner = THIS_MODULE;

	//动态获取主设备号(dev_t idd中包含"主设备号"和"次设备号"信息)
	alloc_chrdev_region(&sdev->idd, 0, 1, "madev");
	sdev->major = MAJOR(sdev->idd);
	sdev->minor = MINOR(sdev->idd);
	printd("%s: major=%d minor=%d\n", __func__, sdev->major, sdev->minor);

	//注册字符设备 (1)
	ret = cdev_add(sdev->chd, sdev->idd, 1);
	if (ret) {
		printw("%s: cdev_add failed. ret=%d\n", ret);
		return -1;
	}
	sdev->cls = class_create(THIS_MODULE, "madev");
	if (IS_ERR(sdev->cls)) {
		printw("%s: class_create failed.\n", __func__);
		return -1;
	}
	dev = device_create(sdev->cls, NULL, sdev->idd, NULL, "madev0");
	ret = IS_ERR(dev) ? PTR_ERR(dev) : 0;
	if (ret == 0) {
		list_add(&smas->dev_entry, &dev_list);
	} else {
		printw("%s: device_create failed. ret=%d\n", __func__, ret);
		return -1;
	}
	wake_lock_init(&gIntWakeLock, WAKE_LOCK_SUSPEND,"microarray_int_wakelock"); //add by gur
    wake_lock_init(&gProcessWakeLock, WAKE_LOCK_SUSPEND,"microarray_process_wakelock");//add by gur
	gWorkq = create_singlethread_workqueue("mas_workqueue");
    if (!gWorkq) {
        MALOGW("create_single_workqueue error!");
        return -ENOMEM;
    }	
#if 0
	smas->workq = create_singlethread_workqueue("mas_workqueue");
	if (!smas->workq) {
		printw("%s: create_single_workqueue failed\n", __func__);
		return -ENOMEM;
	}
	smas->press_workq = create_singlethread_workqueue("mas_workqueue");
	if (!smas->press_workq) {
		printw("%s: create_single_workqueue failed\n", __func__);
		return -ENOMEM;
	}
#endif
	ret = plat_register_driver();
	if (ret < 0) {
		printw("%s: spi_register_driver failed. ret=%d\n", __func__, ret);
		class_destroy(sdev->cls);
		unregister_chrdev_region(sdev->idd, 1);
	}
	printd("%s: end\n", __func__);

	return ret;
}

static void __exit mas_exit(void) {
	printd("%s: start\n", __func__);

	plat_unregister_driver();
	unregister_chrdev_region(sdev->idd, 1);
	cdev_del(sdev->chd);
	device_destroy(sdev->cls, sdev->idd);
	class_destroy(sdev->cls);
	//if(smas->workq) destroy_workqueue(smas->workq);
	//if(smas->press_workq) destroy_workqueue(smas->press_workq);
	if(gWorkq) destroy_workqueue(gWorkq);//add by gur
	wake_lock_destroy(&gIntWakeLock);//add by gur
    wake_lock_destroy(&gProcessWakeLock);//add by gur

	if(sdev!=NULL) kfree(sdev);
	if(smas!=NULL) kfree(smas);
	//if(stxb!=NULL) kfree(stxb);
	//if(srxb!=NULL) kfree(srxb);

	printd("%s: end\n", __func__);
}

module_init(mas_init);
module_exit(mas_exit);

MODULE_AUTHOR("czl");
MODULE_DESCRIPTION("for microarray fprint driver");
MODULE_LICENSE("GPL");

