/*
 * linux/drivers/char/etek_irremote.c
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

#include "consumerir_et4003yd18.h"



#define DEVICE_NAME				"irremote"



#define		_ET4003_ADDRESS_					0x35
#define		_ET4003_CONTROL_SEND_CODE_1_		0x53
#define		_ET4003_CONTROL_SEND_CODE_3_		0x54

#define		_ET4003_CONTROL_SEND_CODE_2_		0x55
#define		_ET4003_CONTROL_START_LEARND_		0x57
#define		_ET4003_CONTROL_READ_VERSION_			0x5c
#define		_ET4003_CONTROL_READ_CODE_			0x5d
#define		_ET4003_CONTROL_STOP_LEARN_			0x5e
#define		_ET4003_CONTROL_SEND_REPEAT_		0x5f
#define     ET_CMD_START_LEARN 					0
#define     ET_CMD_STOP_LEARN 					1
#define     ET_CMD_REPEAT					2
#define     ET_CMD_VERSION					3


#if 1 //def ET4003YD18 //et4003yd18
#define  PLAT_MTK_MT6577
#endif
#if 0 
#define ET4003_SCL_GPIO			(GPIO74|0x80000000)
#define ET4003_SDA_GPIO			(GPIO73|0x80000000)
#define ET4003_BUSY_GPIO			(GPIO75|0x80000000)

#endif



#ifdef   PLAT_MINI210
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#define ET4003_SCL_GPIO			    S5PV210_GPB(0)
#define ET4003_SDA_GPIO			    S5PV210_GPB(3)
#define ET4003_BUSY_GPIO			S5PV210_GPB(2)
#define ET4003_SET_SCL_OUTPUT()		s3c_gpio_cfgpin(ET4003_SCL_GPIO, S3C_GPIO_OUTPUT)
#define ET4003_SET_SDA_INPUT()		s3c_gpio_cfgpin(ET4003_SDA_GPIO, S3C_GPIO_INPUT)
#define ET4003_SET_SDA_OUTPUT()		s3c_gpio_cfgpin(ET4003_SDA_GPIO, S3C_GPIO_OUTPUT)
#define ET4003_SET_BUSY_INPUT()		s3c_gpio_cfgpin(ET4003_BUSY_GPIO, S3C_GPIO_INPUT)

#define ET4003_GET_SDA_STATE()		gpio_get_value(ET4003_SDA_GPIO)
#define ET4003_GET_BUSY_STATE()			gpio_get_value(ET4003_BUSY_GPIO)

#define ET4003_SET_SCL_LOW()			gpio_set_value(ET4003_SCL_GPIO, 0)
#define ET4003_SET_SCL_HIGH()			gpio_set_value(ET4003_SCL_GPIO, 1)
#define ET4003_SET_SDA_LOW()			gpio_set_value(ET4003_SDA_GPIO, 0)
#define ET4003_SET_SDA_HIGH()			gpio_set_value(ET4003_SDA_GPIO, 1)
#endif



#ifdef   PLAT_MTK_MT6577
#include <mach/mt_gpio.h>

#define ET4003_SCL_GPIO			(GPIO89|0x80000000 ) 
#define ET4003_SDA_GPIO			(GPIO91|0x80000000 )
#define ET4003_BUSY_GPIO		(GPIO92|0x80000000 )
#define LDO_EN_GPIO		        (GPIO90|0x80000000 )



#define ET4003_SET_SCL_OUTPUT()	      \
{\
    mt_set_gpio_mode(ET4003_SCL_GPIO, GPIO_MODE_00);  \
    mt_set_gpio_dir(ET4003_SCL_GPIO, GPIO_DIR_OUT);\
}
#define ET4003_SET_SDA_INPUT()		\
{\
    mt_set_gpio_mode(ET4003_SDA_GPIO, GPIO_MODE_00)  ;\
    mt_set_gpio_dir(ET4003_SDA_GPIO, GPIO_DIR_IN);\
}
#define ET4003_SET_SDA_OUTPUT()		\
{\
    mt_set_gpio_mode(ET4003_SDA_GPIO, GPIO_MODE_00);  \
    mt_set_gpio_dir(ET4003_SDA_GPIO, GPIO_DIR_OUT);\
}
#define ET4003_SET_BUSY_INPUT()\
{\
    mt_set_gpio_mode(ET4003_BUSY_GPIO, GPIO_MODE_00);  \
    mt_set_gpio_dir(ET4003_BUSY_GPIO, GPIO_DIR_IN);\
}

#define ET4003_GET_SDA_STATE()		        mt_get_gpio_in(ET4003_SDA_GPIO)
#define ET4003_GET_BUSY_STATE()			mt_get_gpio_in(ET4003_BUSY_GPIO)

#define ET4003_SET_SCL_LOW()			mt_set_gpio_out(ET4003_SCL_GPIO, 0)
#define ET4003_SET_SCL_HIGH()			mt_set_gpio_out(ET4003_SCL_GPIO, 1)
#define ET4003_SET_SDA_LOW()			mt_set_gpio_out(ET4003_SDA_GPIO, 0)
#define ET4003_SET_SDA_HIGH()			mt_set_gpio_out(ET4003_SDA_GPIO, 1)
#endif 


#ifdef   PLAT_MTK_MT6592
#include <mach/mt_gpio.h>

#define ET4003_SCL_GPIO			GPIO109
#define ET4003_SDA_GPIO			GPIO110
#define ET4003_BUSY_GPIO			GPIO111


#define ET4003_SET_SCL_OUTPUT()	      \
{\
    mt_set_gpio_mode(ET4003_SCL_GPIO, GPIO_MODE_00);  \
    mt_set_gpio_dir(ET4003_SCL_GPIO, GPIO_DIR_OUT);\
}
#define ET4003_SET_SDA_INPUT()		\
{\
    mt_set_gpio_mode(ET4003_SDA_GPIO, GPIO_MODE_00)  ;\
    mt_set_gpio_dir(ET4003_SDA_GPIO, GPIO_DIR_IN);\
}
#define ET4003_SET_SDA_OUTPUT()		\
{\
    mt_set_gpio_mode(ET4003_SDA_GPIO, GPIO_MODE_00);  \
    mt_set_gpio_dir(ET4003_SDA_GPIO, GPIO_DIR_OUT);\
}
#define ET4003_SET_BUSY_INPUT()\
{\
    mt_set_gpio_mode(ET4003_BUSY_GPIO, GPIO_MODE_00);  \
    mt_set_gpio_dir(ET4003_BUSY_GPIO, GPIO_DIR_IN);\
}

#define ET4003_GET_SDA_STATE()		        mt_get_gpio_in(ET4003_SDA_GPIO)
#define ET4003_GET_BUSY_STATE()			mt_get_gpio_in(ET4003_BUSY_GPIO)

#define ET4003_SET_SCL_LOW()			mt_set_gpio_out(ET4003_SCL_GPIO, 0)
#define ET4003_SET_SCL_HIGH()			mt_set_gpio_out(ET4003_SCL_GPIO, 1)
#define ET4003_SET_SDA_LOW()			mt_set_gpio_out(ET4003_SDA_GPIO, 0)
#define ET4003_SET_SDA_HIGH()			mt_set_gpio_out(ET4003_SDA_GPIO, 1)
#endif 

#ifdef 	PLAT_SPRD_SC6820
#include <linux/gpio.h>

extern int sprd_3rdparty_gpio_remote_scl;
extern int sprd_3rdparty_gpio_remote_sda;
extern int sprd_3rdparty_gpio_remote_busy;

#define ET4003_SCL_GPIO			sprd_3rdparty_gpio_remote_scl
#define ET4003_SDA_GPIO			sprd_3rdparty_gpio_remote_sda
#define ET4003_BUSY_GPIO		sprd_3rdparty_gpio_remote_busy

#define ET4003_SET_SCL_OUTPUT()	       gpio_direction_output(ET4003_SCL_GPIO, 1);

#define ET4003_SET_SDA_INPUT()			gpio_direction_input(ET4003_SDA_GPIO);

#define ET4003_SET_SDA_OUTPUT()		gpio_direction_output(ET4003_SDA_GPIO, 1);

#define ET4003_SET_BUSY_INPUT()			gpio_direction_input(ET4003_BUSY_GPIO);


#define ET4003_GET_SDA_STATE()		        gpio_direction_input(ET4003_SDA_GPIO)
#define ET4003_GET_BUSY_STATE()		gpio_direction_input(ET4003_BUSY_GPIO)

#define ET4003_SET_SCL_LOW()			gpio_direction_output(ET4003_SCL_GPIO, 0)
#define ET4003_SET_SCL_HIGH()			gpio_direction_output(ET4003_SCL_GPIO, 1)
#define ET4003_SET_SDA_LOW()			gpio_direction_output(ET4003_SDA_GPIO, 0)
#define ET4003_SET_SDA_HIGH()			gpio_direction_output(ET4003_SDA_GPIO, 1)

#endif

#ifdef   PLAT_MSM8610
#include <mach/gpio.h>
#include <mach/regs-gpio.h>
#include <plat/gpio-cfg.h>

#define ET4003_SCL_GPIO			    101
#define ET4003_SDA_GPIO			    102
#define ET4003_BUSY_GPIO			103
#define ET4003_SET_SCL_OUTPUT()		gpio_tlmm_config(GPIO_CFG(ET4003_SCL_GPIO, 0, GPIO_CFG_OUTPUT,
                                                  GPIO_CFG_NO_PULL, GPIO_CFG_8MA),GPIO_CFG_ENABLE);
#define ET4003_SET_SDA_INPUT()		gpio_tlmm_config(GPIO_CFG(ET4003_SDA_GPIO, 0, GPIO_CFG_INPUT
                                                  GPIO_CFG_NO_PULL, GPIO_CFG_8MA),GPIO_CFG_ENABLE);
#define ET4003_SET_SDA_OUTPUT()		gpio_tlmm_config(GPIO_CFG(ET4003_SDA_GPIO, 0, GPIO_CFG_OUTPUT,
                                                  GPIO_CFG_NO_PULL, GPIO_CFG_8MA),GPIO_CFG_ENABLE);
#define ET4003_SET_BUSY_INPUT()		gpio_tlmm_config(GPIO_CFG(ET4003_BUSY_GPIO, 0, GPIO_CFG_INPUT
                                                  GPIO_CFG_NO_PULL, GPIO_CFG_8MA),GPIO_CFG_ENABLE);

#define ET4003_GET_SDA_STATE()		gpio_get_value(ET4003_SDA_GPIO)
#define ET4003_GET_BUSY_STATE()			gpio_get_value(ET4003_BUSY_GPIO)

#define ET4003_SET_SCL_LOW()			gpio_direction_output(ET4003_SCL_GPIO, 0)
#define ET4003_SET_SCL_HIGH()			gpio_direction_output(ET4003_SCL_GPIO, 1)
#define ET4003_SET_SDA_LOW()			gpio_direction_output(ET4003_SDA_GPIO, 0)
#define ET4003_SET_SDA_HIGH()			gpio_direction_output(ET4003_SDA_GPIO, 1)
#endif


static DECLARE_WAIT_QUEUE_HEAD(remote_waitq);

static volatile int busy_status = 0;


struct timer_list remote_timer;


static struct ir_remocon_data	*ir_data;


struct class *sec_class;


/******************************************************/
/*Funcation: et_xCal_crc                      	        				*/
/*Input:  	uint8_t *ptr	uint32_t len						*/
/*Output: 	uint8_t crc 	  								 */
/*Desc: 		get whole ptr data array crc					*/

/******************************************************/
uint8_t xCal_crc(uint8_t *ptr,uint32_t len)
{
	uint8_t crc;
 	uint8_t i;
    crc = 0;
    while(len--)
    {
       crc ^= *ptr++;
       for(i = 0;i < 8;i++)
       {
           if(crc & 0x01)
           {

               crc = (crc >> 1) ^ 0x8C;
           }else crc >>= 1;
       }
    }
    return crc;
}

/******************************************************/
/*Funcation: et_compare_time                      	        				*/
/*Input:  	emote_data data, uint16_t high_level,
		uint16_t low_level					*/
/*Output: 	true or false									 */
/*Desc: 		compare  remote data time 				*/

/******************************************************/


char compare_time(struct remote_data data, uint16_t high_level,
		uint16_t low_level) {

	if (((data.high_level - high_level) < 2) &&((data.high_level - high_level ) > -2)
			&&((data.low_level - low_level) < 2) && ((data.low_level - low_level) > -2)){
//		printk("compare time  is equal");
		return 1;
	} else {
//		printk("compare time is unequal");
		return 0;
	}
	return 0;
}
/******************************************************/
/*Funcation: et_compare_alldata                      	        			*/
/*Input:  	emote_data data, uint16_t *sample int index		*/
/*Output: 	true or false						 			*/
/*Desc: 		compare  remote data to all sample  				*/

/******************************************************/
int et_compare_alldata(struct remote_data rmt_data, uint16_t *sample, int index) {
	int i;
	uint16_t timeHigh, timeLow;

	for (i = 0; i < index; i += 2) {
		timeHigh = sample[i];
		timeLow = sample[i + 1];
		if (compare_time(rmt_data, timeHigh, timeLow)) {
			return 1;			//rmt_data is equal sample data
		}
	}
	return 0;			//rmt_data is not equal sample data
		
}

void et_push_sample_time_data(struct remote_data data, uint16_t *sample, int index) {
	sample[index] = data.high_level;
	sample[index + 1] = data.low_level;
}

int et_sample_time_selection(struct ir_remocon_data *ir_data) {
	int i, index;
	struct remote_data rmt_data;
	index = 0;

	for (i = 0; i < ir_data->count; i += 2) {
		rmt_data.high_level = ir_data->original[i];
		rmt_data.low_level = ir_data->original[i + 1];

		if (index != 0) {
			if (et_compare_alldata(rmt_data, ir_data->sample, index)==0) {
				et_push_sample_time_data(rmt_data,  ir_data->sample, index);
				index += 2;
				if (index>MAX_SAMPLE_INDEX){
					index=MAX_SAMPLE_INDEX;
					return -1;
					}
			}
		} else { /* first data send*/
			et_push_sample_time_data(rmt_data, ir_data->sample, index);
			index += 2;
		}
	}
	ir_data->index = index;
	return index;
}
/******************************************************/
/*Funcation: et_get_index                        	        			*/
/*Input:  	ir_remocon_data *ir_data
		uint16_t *sample, int index	   					*/
/*Output: 	index 	  								 */
/*Desc: 	data compare sample to get sample index  */

/******************************************************/

int et_get_index(struct remote_data data, uint16_t *sample, int index) {
	int i = 0;
	uint16_t timeHigh, timeLow;
	//	printk("index ----> %d  \n", index);
	for (i = 0; i < index; i += 2) {
		timeHigh = sample[i];
		timeLow = sample[i + 1];

		if (compare_time(data,timeHigh,timeLow)) {
			//	printk("timeHigh ----> %d timeLow ----> %d     i  ---> %d \n\r",  timeHigh,timeLow,i);
			return i;
			
		}
	}
	return 32;

}
/******************************************************/
/*Funcation: et_get_data_index                        	        			*/
/*Input:  	ir_remocon_data *ir_data, char *data,
		uint16_t *sample, int index	   					*/
/*Output: 	index 	  								 */
/*Desc: 	original data to get sample index to compress data     */

/******************************************************/
int et_get_data_index(struct ir_remocon_data *ir_data) {
	int i, j = 0, count = 0;
	char temp;
	struct remote_data rmt_data;

	for (i = 0; i < ir_data->count; i += 2) {
		rmt_data.high_level = ir_data->original[i];
		rmt_data.low_level = ir_data->original[i + 1];

		temp = et_get_index(rmt_data, ir_data->sample, ir_data->index);
		if (temp>32){
			return -1;
			}
		ir_data->data[count++] = (temp/2 ) ;
		
	}


	ir_data->couple = count;

	i = 0;
	j = 0;
	while (i < count) {
		
		temp = (ir_data->data[i++] << 4) & 0xf0;
		temp |= (ir_data->data[i++]) & 0x0f;
//printk("ir_data->data[%d] ----> 0x%x \n",j,temp);
		ir_data->data[j++] = temp;
		
		if (j>MAX_DATA){
			j= MAX_DATA;
			return -1;
			}
	}
//	data[j - 1] &= 0x0f;

	ir_data->data_count = j;

	return j;
}


/******************************************************/
/*Funcation: et_depress_sample                        	        */
/*Input:  	uint16_t *in int index			   */
/*Output: 	char *out  	   */
/*Desc: 	change uint16_t sample to double char sample  */

/******************************************************/

int depress_sample(struct ir_remocon_data *ir_data) {
	int i,j = 0;
	if (ir_data->index>MAX_SAMPLE_INDEX){
	ir_data->index = MAX_SAMPLE_INDEX;
	return -1;
		}
	for (i = 0; i < ir_data->index; i++) {
		
		ir_data->zp_sample[j++] = (uint8_t)(ir_data->sample[i] >> 8) & 0xff;
		ir_data->zp_sample[j++] = (uint8_t)ir_data->sample[i];

	}
	ir_data->index = j;
	return j;
}


/******************************************************/
/*Funcation: et_compress_original_data                        	        */
/*Input:  	ir_remocon_data *ir_data			   */
/*Output: 	ir_remocon_data *ir_data	ir_data length   	   */
/*Desc: 	translate original consumer data to ET compress data	   */

/******************************************************/

int et_compress_original_data(struct ir_remocon_data *ir_data) {
	
	uint8_t temp[MAX_SEND_DATA];

	int i;
	int err;

	memset(temp,0x00,MAX_DATA);
	memset(ir_data->data,0x00,MAX_DATA*2);
	memset(ir_data->sample,0x00,MAX_INDEX);
	memset(ir_data->zp_sample,0x00,MAX_INDEX*2);
	err = et_sample_time_selection(ir_data);
	if (err<0){
		printk(KERN_INFO "  et_sample_time_selection program error \n\r");
		return err;
		}
	

	err = et_get_data_index(ir_data);
	if (err<0){
		printk(KERN_INFO "  et_get_data_index program error \n\r");
		return err;
		}
	//printk(KERN_INFO "  et_get_data_index index is  = %d \n\r", ir_data->index);
	
	err = depress_sample(ir_data);
	if (err<0){
		printk(KERN_INFO "  depress_sample program error \n\r");
		return err;
		}
	

	
	
	ir_data->length= MAX_INDEX + ir_data->data_count  +10;

	for (i=0;i<ir_data->index;i++){
		temp[i] = ir_data->zp_sample[i];
	}
	for (i=0;i<ir_data->data_count;i++){
		temp[i  + MAX_INDEX] = ir_data->data[i];
		//printk("temp[%d] is 0x%x",i,ir_data->data[i]);
		}

	ir_data->signal[0] = _ET4003_CONTROL_SEND_CODE_3_;
	ir_data->signal[1] = (ir_data->length>>8)&0xff;
	
	ir_data->signal[2] = ir_data->length&0xff;
	ir_data->signal[3] = ir_data->freq;
	
	ir_data->signal[4] = (ir_data->couple>>8)&0xff;
	ir_data->signal[5] = (ir_data->couple)&0xff;;   //reserve
	ir_data->signal[6] = 0x00;	 //reserve
	ir_data->signal[7] = 0x00;
	ir_data->signal[8] = 0x01;
	ir_data->signal[9] = xCal_crc(temp,ir_data->length-10);
	
	
	for (i=0;i<MAX_INDEX+ir_data->data_count;i++){
			ir_data ->signal[i+10]= temp[i];
			}
#if 0
	
	for (i = 0; i < ir_data->length; i++) {
		printk("remoteData[%d] ----> 0x%02x \n", i, ir_data->signal[i]);
		}
	printk(KERN_INFO  "remote couple is %d \n", ir_data->couple);
	printk(KERN_INFO  "remote freq is %d \n", ir_data->freq);
	printk(KERN_INFO  "remote length is %d \n", ir_data->length);
	printk(KERN_INFO  "remote index is %d \n", ir_data->index);
	printk(KERN_INFO  "remote data_count is %d \n", ir_data->data_count);
	printk(KERN_INFO  "remote count is %d \n", ir_data->count);
for (i = 0; i < MAX_INDEX; i++) {
		printk("sample [%d] ----> 0x%02x \n", i, ir_data->zp_sample[i]);
		}
#endif
	return ir_data->length;

}



/******************************************************/
/*Funcation: ET4003 TIMER HANDLE	                        	        */
/*Input:  	unsigned long _data			   */
/*Output: 					   						   */
/*Desc: 	get busy state according to time base				   */

/******************************************************/



static void remote_timer_handle(unsigned long _data)
{
	 if (!busy_status){
		mod_timer(&remote_timer,jiffies+msecs_to_jiffies(100)); 
		 
		if (!ET4003_GET_BUSY_STATE()) {
			busy_status = 1;
	//		printk("ET4003 wake up interruptiable remote watiq \n");
			wake_up_interruptible(&remote_waitq);
		}
	 }
}




void ET4003_start(void)
{
	ET4003_SET_SDA_OUTPUT();
	ET4003_SET_SDA_HIGH();
	ET4003_SET_SCL_OUTPUT();
	ET4003_SET_SCL_HIGH();
	ET4003_SET_SDA_LOW();
	udelay(200);
	udelay(200);
	udelay(200);
	ET4003_SET_SCL_LOW();
	udelay(200);
	udelay(200);
}


void ET4003_stop(void)
{
	udelay(200);
	ET4003_SET_SDA_LOW();
	ET4003_SET_SDA_OUTPUT();
	udelay(200);
	ET4003_SET_SCL_HIGH();
	udelay(200);
	ET4003_SET_SDA_HIGH();
	udelay(200);
//	ET4003_SET_SDA_INPUT();
	udelay(200);
}


/******************************************************/
/*Funcation: ET4003 GPIO I2C writebyte	                                */
/*Input: 	uint8_t dat			   						   */
/*Output: 					   						   */
/******************************************************/

uint8_t ET4003_writebyte(uint8_t dat)  //写一个字节
{
	uint8_t i,dat_temp,err;
	
	err=0;
	dat_temp=dat;
	ET4003_SET_SDA_OUTPUT();
	for( i = 0; i != 8; i++ )
	{
		udelay(10);
		if( dat_temp & 0x80 ) 
		{
			ET4003_SET_SDA_HIGH();
		}
		else 
		{
			ET4003_SET_SDA_LOW();
		}
		udelay(50);
		ET4003_SET_SCL_HIGH();
		dat_temp <<= 1;
		udelay(50);  // 可选延时
		ET4003_SET_SCL_LOW();
		udelay(10);
	}
	ET4003_SET_SDA_HIGH();
	ET4003_SET_SDA_INPUT();
	udelay(50);
	ET4003_SET_SCL_HIGH();
	udelay(50);
	//if(ET4003_GET_SDA_STATE())err=1;
	//else err=0;
	ET4003_SET_SCL_LOW();
	udelay(50);
	return err;
}



/******************************************************/
/*Funcation: ET4003 GPIO I2C readbyte	                                */
/*Input: 					   							   */
/*Output: 	one byte 			   						   */
/******************************************************/

uint8_t ET4003_readbyte(void)
{
	uint8_t dat,i;
	ET4003_SET_SDA_HIGH();
	ET4003_SET_SDA_INPUT();
	dat = 0;
	for( i = 0; i != 8; i++ )
	{
		udelay(50); 
		ET4003_SET_SCL_HIGH();
		udelay(50); 
		dat <<= 1;
		if( ET4003_GET_SDA_STATE() ) dat++;
		ET4003_SET_SCL_LOW();
		udelay(10);
	}
	ET4003_SET_SDA_OUTPUT();
	ET4003_SET_SDA_LOW();
	udelay(50);
	ET4003_SET_SCL_HIGH();
	udelay(50);
	ET4003_SET_SCL_LOW();
	udelay(50);
	return dat;
}
/******************************************************/
/*Funcation: read et4003yd18 version code					   */
/*Input: 					   							   */
/*Output: 	version 			   						   */
/******************************************************/

static int ET_remote_read_device_info(void)
{
	
	int ret = 0;
	int i;
//	printk(KERN_INFO"%s called\n", __func__);
	char buf_ir_test[8];

	
	ET4003_start(); 
	ET4003_writebyte(_ET4003_ADDRESS_);
	ET4003_writebyte(_ET4003_CONTROL_READ_VERSION_);
	for(i=0;i<4;i++){
		buf_ir_test[i]=ET4003_readbyte();
		}
	ET4003_stop();
	


	printk(KERN_INFO "%s: dev_id: 0x%02x, 0x%02x, 0x%02x, 0x%02x \n", __func__,
				buf_ir_test[0], buf_ir_test[1],buf_ir_test[2],buf_ir_test[3]);
	ret = buf_ir_test[0]*16777216 + buf_ir_test[1]*65536 + buf_ir_test[2]*256+ buf_ir_test[3];
	
	return ret;
}


static long ET_remote_ioctl(struct file *filp, unsigned int cmd,
		unsigned long arg)
{
	switch(cmd) {
	
		case ET_CMD_START_LEARN:
		ET4003_start();
		ET4003_writebyte(_ET4003_ADDRESS_);
		ET4003_writebyte(_ET4003_CONTROL_START_LEARND_);
		ET4003_stop();
		busy_status =0;
		break;
		case ET_CMD_STOP_LEARN:
		ET4003_start();
		ET4003_writebyte(_ET4003_ADDRESS_);
		ET4003_writebyte(_ET4003_CONTROL_STOP_LEARN_);
		ET4003_stop();	
	//	mod_timer(&remote_timer,0); 
		busy_status = 1;
		printk("ET4003 wake up interruptiable remote watiq \n");
		wake_up_interruptible(&remote_waitq);
		break;
		case	ET_CMD_REPEAT:
		ET4003_start();
		ET4003_writebyte(_ET4003_ADDRESS_);
		ET4003_writebyte(_ET4003_CONTROL_SEND_REPEAT_);
		ET4003_stop();	
		break;
		case	ET_CMD_VERSION:
		ET4003_start();
		ET4003_writebyte(_ET4003_ADDRESS_);
		ET4003_writebyte(_ET4003_CONTROL_READ_VERSION_);
		ET4003_stop();	
		break;
		default:
			return -EINVAL;
	}

	return 0;
}

static unsigned int ET_remote_poll( struct file *file,
		struct poll_table_struct *wait)
{
	unsigned int mask = 0;
	mod_timer(&remote_timer, jiffies + msecs_to_jiffies(40));
	printk("et4003---poll---\n");
	poll_wait(file, &remote_waitq, wait);
	if (busy_status){
		
		busy_status=0;
		mask |= POLLIN | POLLRDNORM;
	}
	return mask;
}

static ssize_t ET_remote_write(struct file *file, const char *buffer,
		size_t count, loff_t *ppos)
{
	char send_buff[129];
	int ret;
	int i;

	if (count == 0) {
		return count;
	}
	if (count> sizeof send_buff)
	{
		count = sizeof send_buff-1;
	}
	
	memset(send_buff,0x00,129);
	ret = copy_from_user(send_buff, buffer, count) ;
	if (ret) {
		printk("et4003---copy error---\n");
		return ret;
	}
	/*
	for(i=0;i<129;i++)
		{
		printk("et4003---writedata  %d = 0x%x \n",i, send_buff[i]);
			
		}
//	memset();
    printk("et4003---write---\n");
    */
	/*while(ET4003_GET_BUSY_STATE())*/{
	ET4003_start();
	ET4003_writebyte(_ET4003_ADDRESS_);
	for(i=0;i<129;i++)
	{
		ET4003_writebyte(send_buff[i]);
	}
	ET4003_stop();
	}
	
		

   // printk("et4003--write =%d\n", count);

	return count;
}

static ssize_t ET_remote_read(struct file *filp, char *buff,
		size_t count, loff_t *ppos){
	int i,err;
	char learn_buffer[128];

 //   printk("et4003---read---\n");
	
	
	ET4003_start(); 
	ET4003_writebyte(_ET4003_ADDRESS_);
	ET4003_writebyte(_ET4003_CONTROL_READ_CODE_);
	for(i=0;i<128;i++)
	{
		learn_buffer[i]=ET4003_readbyte();
	}
	ET4003_stop();

	

	err = copy_to_user((void *)buff, (const void *)(&learn_buffer[0]),
			min(sizeof(learn_buffer), count));

  //  printk("et4003---read---err=%d\n", err);

	return err ? -EFAULT : min(sizeof(learn_buffer), count);
}



static int ET_remote_open(struct inode *inode, struct file *file) {
/*
	if (!down_trylock(&lock)) {
	    printk("et4003---open OK---\n");
		return 0;
	}
	else {
	    printk("et4003---open BUSY---\n");	
		return -EBUSY;
	}
*/
	int err = ET_remote_read_device_info();
	setup_timer(&remote_timer, remote_timer_handle,
					(unsigned long)"remote");
	
#ifdef PLAT_MTK_MT6577
    mt_set_gpio_mode(LDO_EN_GPIO, GPIO_MODE_00); 
    mt_set_gpio_dir(LDO_EN_GPIO, GPIO_DIR_OUT); 
    mt_set_gpio_out(LDO_EN_GPIO, 1); 
#endif

    printk("et4003---open OK---\n");
    return 0;
}

static int ET_remote_close(struct inode *inode, struct file *file) {
//	up(&lock);
	del_timer_sync(&remote_timer);

#ifdef PLAT_MTK_MT6577
    mt_set_gpio_mode(LDO_EN_GPIO, GPIO_MODE_00); 
    mt_set_gpio_dir(LDO_EN_GPIO, GPIO_DIR_OUT); 
    mt_set_gpio_out(LDO_EN_GPIO, 0); 
#endif
    printk("et4003---close---\n");
	return 0;
}



static struct file_operations ET_remote_ops = {
	.owner			= THIS_MODULE,
	.open			= ET_remote_open,
	.release		= ET_remote_close, 
	.write          = ET_remote_write,
	.read			=	ET_remote_read,
	.poll			= 	ET_remote_poll,
	.unlocked_ioctl	=	ET_remote_ioctl,
	
};


static void ir_remocon_work(struct ir_remocon_data *ir_data)
{

	struct ir_remocon_data *data = ir_data;

	int sleep_timing;
	int emission_time;
	
	int i;
//	int tmp;
//	int allLength;
	//mutex_lock(&data->mutex);

	ET4003_start();
	ET4003_writebyte(_ET4003_ADDRESS_);
	for(i=0;i<ir_data->length;i++)
	{
		ET4003_writebyte(ir_data->signal[i]);
	}
	ET4003_stop();
	
	
//	mutex_unlock(&data->mutex);

	//printk(KERN_INFO "%s: ir_sum = %d ir_freq = %d\n", __func__, data->ir_sum,data->freq);
	emission_time = \
		( (data->ir_sum) * (data->freq) * 4 / 5000);
	sleep_timing = emission_time -10;
	if (sleep_timing<0){
		sleep_timing = 10;
		}
	//if (sleep_timing>1000)
	//	sleep_timing = 950;
	//if (sleep_timing > 0&&sleep_timing<6000)
	//	msleep(sleep_timing);
	//printk(KERN_INFO "%s: sleep_timing = %d\n", __func__, sleep_timing);

	data->freq = 0;
	data->ir_sum = 0;
	

}

static ssize_t remocon_store(struct device *dev, struct device_attribute *attr,
		const char *buf, size_t size)
{
		
	unsigned int _data;
	int err;
	int count,  i;
	count = 0;
	ir_data->count = 0;
	//printk(KERN_INFO "%s:  = %s \n\r", __func__,buf);
	//printk(KERN_INFO "%s: size is  = %d \n\r", __func__,size);
	
	for (i = 0; i < size; i++) {

		if (sscanf(buf++, "%u", &_data) == 1) {
		
	//	printk("data buf[%d] = %d  \n\r",buf_count,buf[buf_count]);
		
			if (_data == 0 || buf == '\0'){
	
				break;
				}
		
			if (count == 0) {
				ir_data->orig_freq = _data;
				if (ir_data->orig_freq<10000||ir_data->orig_freq>100000){
					printk("freq is error \n\r");
					return -1;
					}
				ir_data->freq = (char)(1200000/_data+1) ;
				
				count++;
				
		
			} else {
				ir_data->ir_sum  += _data;
				ir_data->original[ir_data->count++] = (uint16_t)_data ;
				count++;
				if(ir_data->count>1024){
				printk("data->count > 1024 error \n\r");
				return -1;
				}	
	
			}
			
			while (_data > 0) {
				buf++;
				
				_data /= 10;
			}
		} else {

			break;
		}
	}
	//printk("count is %d \n\r",ir_data->count);
	
	if(ir_data->count<5){
		printk("data->count < 5 error \n\r");
		return -1;
		}
	
	err = et_compress_original_data(ir_data);
	if (err < 0){
		printk("et_compress_original_data  error \n\r");
		return -1;
		}

	ir_remocon_work(ir_data);


	return size;
}
static ssize_t remocon_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int i;
	int len = 0;
//	printk(KERN_INFO"%s called\n", __func__);
	char buf_ir_test[8];

	
	ET4003_start(); 
	ET4003_writebyte(_ET4003_ADDRESS_);
	ET4003_writebyte(_ET4003_CONTROL_READ_VERSION_);
	for(i=0;i<4;i++){
		buf_ir_test[i]=ET4003_readbyte();
		}
	ET4003_stop();
	for(i=0;i<4;i++){
		len += sprintf(buf + len, "0x%02x,", buf_ir_test[i]);
		}
	
	printk(KERN_INFO "%s: dev_id: 0x%02x, 0x%02x, 0x%02x, 0x%02x \n", __func__,
				buf_ir_test[0], buf_ir_test[1],buf_ir_test[2],buf_ir_test[3]);
	
	return  len;
}

static DEVICE_ATTR(ir_send, 0664, remocon_show, remocon_store);

static ssize_t check_ir_show(struct device *dev, struct device_attribute *attr,
		char *buf)
{
	int i;
		int len = 0;
	//	printk(KERN_INFO"%s called\n", __func__);
		char buf_ir_test[8];
	
		
		ET4003_start(); 
		ET4003_writebyte(_ET4003_ADDRESS_);
		ET4003_writebyte(_ET4003_CONTROL_READ_VERSION_);
		for(i=0;i<4;i++){
			buf_ir_test[i]=ET4003_readbyte();
			}
		ET4003_stop();
		for(i=0;i<4;i++){
			len += sprintf(buf + len, "0x%02x,", buf_ir_test[i]);
			}
		
		printk(KERN_INFO "%s: dev_id: 0x%02x, 0x%02x, 0x%02x, 0x%02x \n", __func__,
					buf_ir_test[0], buf_ir_test[1],buf_ir_test[2],buf_ir_test[3]);
		
		return	len;

}

static DEVICE_ATTR(check_ir, 0664, check_ir_show, NULL);




static struct miscdevice ET_misc_dev = {
	.minor = MISC_DYNAMIC_MINOR,
	.name = DEVICE_NAME,
	.fops = &ET_remote_ops,
};

static int __init ET_remote_dev_init(void) {

	struct device *ir_remocon_dev;
	int error;

#ifdef PLAT_MTK_MT6577
    mt_set_gpio_mode(LDO_EN_GPIO, GPIO_MODE_00); 
    mt_set_gpio_dir(LDO_EN_GPIO, GPIO_DIR_OUT); 
    mt_set_gpio_out(LDO_EN_GPIO, 1); 

#endif
	udelay(1000);

	ET4003_SET_SCL_OUTPUT();
	ET4003_SET_SDA_OUTPUT();
	ET4003_SET_BUSY_INPUT();
	
	ET4003_SET_SCL_HIGH();
	ET4003_SET_SDA_HIGH();


	
//	ret = misc_register(&ET_misc_dev);
	ir_data = kzalloc(sizeof(struct ir_remocon_data), GFP_KERNEL);
	if (NULL == ir_data) {
		printk("Failed to data allocate %s\n", __func__);
		error = -ENOMEM;
		goto err_free_mem;
	}

	
	printk("et4003 initialized\n");
	sec_class = class_create(THIS_MODULE, "sec");
		if (IS_ERR(sec_class)) {
			pr_err("Failed to create class(sec)!\n");
			return PTR_ERR(sec_class);
		}
		
	ir_remocon_dev = device_create(sec_class, NULL, 0, ir_data, "sec_ir");
	if (IS_ERR(ir_remocon_dev))
		printk("Failed to create ir_remocon_dev device\n");
	
	if (device_create_file(ir_remocon_dev, &dev_attr_ir_send) < 0)
			printk("Failed to create device file(%s)!\n",
				   dev_attr_ir_send.attr.name);	
	
	if (device_create_file(ir_remocon_dev, &dev_attr_check_ir) < 0)
			printk("Failed to create device file(%s)!\n",
				   dev_attr_check_ir.attr.name);

	
	
	
	
		
	if((error = misc_register(&ET_misc_dev)))
		{
		printk("et4003: misc_register register failed\n");
		}
//while(1)
	ET_remote_read_device_info();
return error;
	 err_free_mem:
		kfree(ir_data);
	return error;
}

static void __exit ET_remote_dev_exit(void) {
	misc_deregister(&ET_misc_dev);	
}

module_init(ET_remote_dev_init);
module_exit(ET_remote_dev_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("ETEK Inc.");
MODULE_DESCRIPTION("ETEK consumerir  Driver");

