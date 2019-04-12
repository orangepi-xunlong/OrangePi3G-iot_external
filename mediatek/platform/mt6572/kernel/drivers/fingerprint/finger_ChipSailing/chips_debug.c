#include <linux/delay.h>


#include "./inc/chips_main.h"
#include "./inc/chips_common.h"
#include "./inc/chips_debug.h"
#include "./inc/chips_platform.h"


/*********************************全局变量定义****************************/
extern struct chips_data *chips_spidev;

 /**
 *  @brief chips_probe_sensorID 读取芯片ID，用于兼容其他家IC
 *  
 *  @param [out] sensorid 成功时存储芯片ID
 *  
 *  @return 成功返回0,失败返回负数
 */
int chips_probe_sensorID(unsigned short *sensorid)
{
	int status = -1;
	unsigned char buffer[2] = {0};

	status = chips_sfr_read(chips_spidev,0x3E,buffer,2);
	if(status == 0){
		*sensorid = ((buffer[1] << 8)&0xFF00)|(buffer[0]&0x00FF);
	}
	chips_dbg("hwid_h = %x\n",buffer[1]);
	chips_dbg("hwid_l = %x\n",buffer[0]);

	return status;
}


/************************************************************************************
 以下接口函数为spi调试接口，供驱动人员调试spi通信是否正常，在fops的write()函数中使用
 不要在驱动中其他地方使用这些函数，因为在驱动稳定后，这些函数将会被移除 !!!!!!!!!!!!
*************************************************************************************/

int read_SFR(unsigned short addr,unsigned char *data)
{
	return chips_sfr_read(chips_spidev,addr,data,1);
}

int write_SFR(unsigned short addr,unsigned char data)
{
	return chips_sfr_write(chips_spidev,addr,&data,1);
}

int read_SRAM(unsigned short addr,unsigned short *data)
{
	unsigned char rx_buf[2] = {0};
	int status = -1;
	
	status = chips_sram_read(chips_spidev,addr,rx_buf,2);
	if(status == 0)
		*data = ((rx_buf[1] << 8)&0xFF00) | (rx_buf[0]&0x00FF);
	
	return status;
}

int write_SRAM(unsigned short addr,unsigned short data)
{
	unsigned char tx_buf[2] =  {0};
	tx_buf[0] = (unsigned char)(data&0x00FF);  //低8位
	tx_buf[1] = (unsigned char)((data&0xFF00)>>8);  //高8位

	return chips_sram_write(chips_spidev,addr,tx_buf,2);
}


 /**
 *  @brief chips_sensor_config 给IC下载参数
 *  
 *  @return 无返回值
 */
void chips_sensor_config(void)
{	
    write_SFR(0x0F, 0x01);
    write_SFR(0x1C, 0x1D);
    write_SFR(0x1F, 0x0A);
    write_SFR(0x42, 0xAA);
    write_SFR(0x60, 0x08);
    write_SFR(0x63, 0x60);
    //write_SFR(0x47, 0x60);//add 20160712
    //write_SFR(0x13, 0x31);//add 20160712
    //chips_sram_write(0xFC1E, 0x0);//add 20160712

    /*****for 3.3V********/
    write_SFR(0x22, 0x07);
    write_SRAM(0xFC8C, 0x0001);
    write_SRAM(0xFC90, 0x0001);
    /*********************/
    write_SRAM(0xFC02, 0x0420);
    write_SRAM(0xFC1A, 0x0C30);
    write_SRAM(0xFC22, 0x085C);//chips_sram_write(0xFC22, 0x0848);打开Normal下8个敏感区域
    write_SRAM(0xFC2E, 0x00F9);//chips_sram_write(0xFC2E, 0x008F);	//chips_sram_write(0xFC2E, 0x00F6); 20160624
    write_SRAM(0xFC30, 0x0270);//chips_sram_write(0xFC30, 0x0260);	//chips_sram_write(0xFC30, 0x0300);	// 20160624
    write_SRAM(0xFC06, 0x0039);
    write_SRAM(0xFC08, 0x0008);//add  times    20160624
    write_SRAM(0xFC0A, 0x0016);
    write_SRAM(0xFC0C, 0x0022);
    write_SRAM(0xFC12, 0x002A);
    write_SRAM(0xFC14, 0x0035);
    write_SRAM(0xFC16, 0x002B);
    write_SRAM(0xFC18, 0x0039);
    write_SRAM(0xFC28, 0x002E);
    write_SRAM(0xFC2A, 0x0018);
    write_SRAM(0xFC26, 0x282D);
    write_SRAM(0xFC82, 0x01FF);
    write_SRAM(0xFC84, 0x0007);
    write_SRAM(0xFC86, 0x0001);
  	write_SRAM(0xFC80, 0x0718); //chips_write_sram_bit(0xFC80, 12,0);
    write_SRAM(0xFC88, 0x380B); //chips_write_sram_bit(0xFC88, 9, 0);
    write_SRAM(0xFC8A, 0x7C8B); //chips_write_sram_bit(0xFC8A, 2, 0); 
    write_SRAM(0xFC8E, 0x3354);
}


 /**
 *  @brief chips_scan_one_image 获取图像数据
 *  
 *  @param [in] addr    图像数据存储地址
 *  @param [out] buffer 成功时存储图像数据
 *  @param [in] len     读取的图像数据长度
 *   
 *  @return 成功返回0，失败返回负数
 */
int chips_scan_one_image(unsigned short addr, unsigned char *buffer, unsigned short len)
{
    int status = -1;
  
	status = write_SRAM(0xFC00,0x0003); 
	if(status < 0){
		chips_dbg("Failed to write 0x0003 to reg_0xFC00\n");
		return status;	
	}
	
	mdelay(3);
	
	status = chips_sram_read(chips_spidev,addr,buffer,len);
	if(status < 0){
	 	chips_dbg("Failed to read from addr = 0x%x,status = %d\n",addr,status);
	}
	
	return status;
}


 /**
 *  @brief chips_spi_wakeup spi唤醒
 *  
 *  @return 成功返回0，失败返回负数
 *  
 *  @detail 用于在SLEEP以及DEEP_SLEEP模式下唤醒IC;不可靠函数.
 */
static int chips_spi_wakeup(void)
{
	int status = -1;
	unsigned char wakeup = 0xd5;
	
	status = chips_spi_send_cmd(chips_spidev,&wakeup, 1);
	if(status < 0){
		chips_dbg("Failed to send spi cmd, cmd = 0x%x",wakeup);
	}
	
	return status;
}


 /**
 *  @brief chips_force_to_idle 切换IC到IDLE模式
 *  
 *  @return 成功返回0，失败返回负数
 */
static int chips_force_to_idle(void)
{
	unsigned char mode = 0;
	unsigned char status = 0;
	int retval = -1;
	int i = 0;

	retval = read_SFR(0x46, &mode);
	if(retval < 0){
		chips_dbg("Failed to read reg_0x46\n");
		return -1;
	}
	
	retval = read_SFR(0x50,&status);
	if(retval < 0){
		chips_dbg("Failed to read reg_0x50+\n");
		return -1;
	}

	if(mode == 0x70){
		return 0;
	}else if(mode == 0x71){
		;
	}else{
		chips_spi_wakeup();	
		//mdelay(1);
	}

    //注：程序走到这里时，IC可能的模式有SLEEP、NORMAL以及DEEP_SLEEP
	//判断spi_wakeup是否成功,并判断此时IC处于何种模式
	retval = read_SFR(0x46, &mode);            
	if(retval < 0){
		chips_dbg("Failed to read reg_0x46\n");
		return -1;
	}
	retval = read_SFR(0x50,&status);         
	if(retval < 0){
		chips_dbg("Failed to read reg_0x50+1\n");
		return -1;
	}
    //chips_dbg("state before active_idle:reg_0x46 = 0x%x,reg_0x50 = 0x%x\n",mode,status);
  
	retval = write_SFR(0x46,0x70);            //active_idle
	if(retval < 0){
		chips_dbg("Failed to write 0x70 to reg_0x46\n");
		return -1;
	}

	for(i = 0; i < 20; i++)
	{
		retval = read_SFR(0x50, &status);
		if(retval < 0){
			chips_dbg("Failed to read reg_0x50+2\n");
			continue;;
		}		
		
		if(status == 0x01)        //sleep、normal模式下active_idle成功
			break;
	}

	if(i >= 20){
		chips_dbg("Failed to active idle,reg_0x50 = 0x%x\n",status);
		return -1;
	}
		
	retval = write_SFR(0x50,0x01);   
	if(retval < 0){
		chips_dbg("Failed to write 0x01 to reg_0x50\n");
		return -1;
	}
		
	for(i = 0; i < 5; i++)                //清除idle_irq
	{	
		retval = read_SFR(0x50, &status);
		if(retval < 0){
			chips_dbg("Failed to read reg_0x50+3\n");
			continue;;
		}
		
		if(status == 0x00)                 //清除idle_irq成功
			break;

	}	

	if(i >= 5){
		chips_dbg("Failed to clear active idle irq,reg_0x50 = 0x%x\n",status);
		return -1;
	}
	
	return 0;
}


 /**
 *  @brief chips_set_sensor_mode 设置IC工作模式
 *  
 *  @param [in] mode 要设置的IC工作模式
 *  
 *  @return 成功返回0，失败返回负数
 */
int chips_set_sensor_mode(int mode)
{
	int status = -1;

	status = chips_force_to_idle();
	if(status < 0){
		chips_dbg("Failed to set idle mode\n");
		return status;
	}
	
	switch(mode)
	{
		case IDLE:        //idle
			break;   
    
		case NORMAL:      //normal
			status = write_SFR(0x46, 0x71);
			if(status < 0){
				chips_dbg("Failed to write 0x71 to reg_0x46\n");
			}
			break;
 
		case SLEEP:       //sleep
			status = write_SFR(0x46, 0x72);
			if(status < 0){
				chips_dbg("Failed to write 0x72 to reg_0x46\n");
			}
			break;

		case DEEP_SLEEP:  //deep sleep
			status = write_SFR(0x46,0x76);
			if(status < 0){
				chips_dbg("Failed to write 0x76 to reg_0x46\n");
			}
			break;
	
		default:
			chips_dbg("unrecognized mode,mode = 0x%x\n",mode);
			status = -1;
			break;	 
	}	
	return status;	
}



static int chips_16clk_write(struct chips_data *chips_data)
{	  
	unsigned char tx_buf[2];
	int status = -1;

	tx_buf[0] = 0xEE;
	tx_buf[1] = 0xEE;

	status = chips_spi_send_cmd(chips_data,tx_buf,2);
	if(status < 0){
	    chips_dbg("Failed to write chips_16clk_write\n");
		return -1;
	}

	return 0; 
}
  
int chips_esd_reset(struct chips_data *chips_data,unsigned int delay_ms)
{
	int ret = -1;

	chips_set_reset_gpio(chips_data,0);

	ret = chips_16clk_write(chips_data);
	if(ret < 0){
	  chips_dbg("chips_16clk_write error\n");
	  return ret;
	}	  

	mdelay(12);
	  
	chips_set_reset_gpio(chips_data,1);
	
	mdelay((delay_ms > 1)?delay_ms:1);
	

	chips_set_reset_gpio(chips_data,0);

	mdelay((delay_ms > 1)?delay_ms:1);
	

	chips_set_reset_gpio(chips_data,1);

	mdelay((delay_ms > 1)?delay_ms:1);
	

	chips_set_reset_gpio(chips_data,0);

	mdelay((delay_ms > 1)?delay_ms:1);
	

	chips_set_reset_gpio(chips_data,1);

	mdelay(1);
	
	return 0;
}

  
  
/**
*  @brief chips_hw_reset IC复位
*  
*  @param [in] chips_data chips_data结构体指针
*  @param [in] delay_ms 延时参数
*  @return 成功返回0，失败返回负数
*/
int chips_hw_reset(struct chips_data *chips_data, unsigned int delay_ms)
{ 
	chips_set_reset_gpio(chips_data,0);

	mdelay((delay_ms > 1)?delay_ms:1);

	chips_set_reset_gpio(chips_data,1);

	mdelay(1);

	return 0;	 
}
  

