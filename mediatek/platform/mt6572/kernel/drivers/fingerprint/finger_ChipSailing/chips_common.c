/*************************************************************************
  版权： ShenZhen ChipSailing Technology Co., Ltd. All rights reserved.
  文件名称: chips_common.c
  文件描述: 与spi操作相关的函数接口
  作者: zwp    ID:58    版本:2.0   日期:2016/10/16
  其他:
  历史:
      1. 日期:           作者:          ID:
	     修改说明:
	  2.
 *************************************************************************/

 
 /********************************头文件定义******************************/
#include <linux/spi/spi.h>
//#include <mt_spi.h>

#include "./inc/chips_main.h"
#include "./inc/chips_common.h"

#define MTK_SPI_ALIGN_MASK_NUM  10
#define MTK_SPI_ALIGN_MASK  ((0x1 << MTK_SPI_ALIGN_MASK_NUM) - 1)
#define	SPI_BUFSIZ	32

/*********************************函数定义********************************/

static void chips_fp_complete(void *arg)
{
	complete(arg);
}

 /**
 *  @brief chips_sync 同步/阻塞SPI数据传输
 *  
 *  @param [in] chips_data chips_data结构体指针
 *  @param [in] message    spi_message结构体指针
 *  
 *  @return 成功返回0，失败返回负数
 */
static int chips_sync(struct chips_data *chips_data,struct spi_message *message)
{
	
	DECLARE_COMPLETION_ONSTACK(done);
	int status;

	if(NULL == chips_data || NULL == message){
		chips_dbg("invalid arguments\n");
		return -EINVAL;
	}
	message->complete = chips_fp_complete;
	message->context = &done;

	spin_lock_irq(&chips_data->spin_lock);
	if (chips_data->spi == NULL){
		status = -ESHUTDOWN;
	}else{
		status = spi_async(chips_data->spi, message);
	}
		
	spin_unlock_irq(&chips_data->spin_lock);

	if (status == 0){
		wait_for_completion(&done);

		status = message->status;
		//chips_dbg("spi_async call success,message->status = %d\n",status);
		//if (status == 0)
		//	status = message->actual_length;
	}else{
		chips_dbg("Failed to async message,status = %d\n",status);
	}
	return status;
}
 

static unsigned char buf[SPI_BUFSIZ] = {0};

/**
 * @func：chips_spi_full_duplex - SPI synchronous write followed by read
 * @txbuf: data to be written (need not be dma-safe)
 * @n_tx: size of txbuf, in bytes
 * @rxbuf: buffer into which data will be read (need not be dma-safe)
 * @n_rx: size of rxbuf, in bytes
 * @return：return 0 on success,negative on failure
 * Context: can sleep
 
 *
 * This performs a half duplex MicroWire style transaction with the
 * device, sending txbuf and then reading rxbuf.  The return value
 * is zero for success, else a negative errno status code.
 * This call may only be used from a context that may sleep.
 *
 * Parameters to this routine are always copied using a small buffer;
 * portable code should never use this for more than 32 bytes.
 * Performance-sensitive or bulk transfer code should instead use
 * spi_{async,sync}() calls with dma-safe buffers.
 */
static int chips_spi_full_duplex(struct chips_data *chips_data,void *txbuf, unsigned n_tx,void *rxbuf, unsigned n_rx)
{
	static DEFINE_MUTEX(lock);

	int			status;
	struct spi_message	message;
	//struct spi_transfer	x[2];
	struct spi_transfer x = {0};  
	unsigned char *local_buf;
	uint32_t package_num = 0;
	uint32_t remainder = 0;
	uint32_t packet_size = 0;

	/* Use preallocated DMA-safe buffer if we can.  We can't avoid
	 * copying here, (as a pure convenience thing), but we can
	 * keep heap costs out of the hot path unless someone else is
	 * using the pre-allocated buffer or the transfer is too large.
	 */
	if((!txbuf && !rxbuf)||(!n_tx && !n_rx)){
		chips_dbg("invalid arguments\n");
		return -EINVAL;
	}
  
    if((n_tx + n_rx) > 32){
		chips_data->spi_mcc->com_mod = DMA_TRANSFER;
	}else{
		chips_data->spi_mcc->com_mod = FIFO_TRANSFER;
	}
	spi_setup(chips_data->spi);

    package_num = (n_tx + n_rx)>>MTK_SPI_ALIGN_MASK_NUM;
	remainder = (n_tx + n_rx) & MTK_SPI_ALIGN_MASK;
	if((package_num > 0) && (remainder != 0)){
		packet_size = ((package_num+1) << MTK_SPI_ALIGN_MASK_NUM);
	}else{
		packet_size = n_tx + n_rx;
	}
	
	if (packet_size > SPI_BUFSIZ || !mutex_trylock(&lock)) {
		local_buf = kmalloc(max((unsigned)SPI_BUFSIZ, packet_size),GFP_KERNEL);  
		if (NULL == local_buf){
			chips_dbg("Failed to allocate mem for spi_full_duplex buffer\n");
			return -ENOMEM;
		}
	} else {
		local_buf = buf;
	}
	
	spi_message_init(&message);
	
	//initialize
	memset(&x,0,sizeof(x));
	memcpy(local_buf,txbuf,n_tx);
	
	x.cs_change = 0;
	x.delay_usecs = 1;
	x.speed_hz = 7000000;
	x.tx_buf = local_buf;
	x.rx_buf = local_buf;
	x.len = packet_size;

	spi_message_add_tail(&x, &message);
	status = chips_sync(chips_data,&message);
	if(status == 0){
		memcpy(rxbuf,local_buf+n_tx,n_rx);
	}else{
	  chips_dbg("Failed to sync message,status = %d\n",status);	
	}
	
	if (x.tx_buf == buf){
		mutex_unlock(&lock);
	}else{
		kfree(local_buf);
	}
		
	return status;
}

#if 0
/**
 *  @brief chips_spi_write spi同步写
 *  
 *  @param [in] chips_data chips_data结构体指针
 *  @param [in] buf  要写入的数据的buffer指针     
 *  @param [in] len  写入的数据的字节数     
 *  
 *  @return 成功返回0,失败返回负数
 */
static int chips_spi_write(struct chips_data *chips_data,void *buf, int len)
{	
	struct spi_transfer	t = {
			.tx_buf		= buf,
			.len		= len,
		};
	struct spi_message	m;
	
	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return chips_sync(chips_data,&m);
}


 /**
 *  @brief chips_spi_read   spi同步读
 *  
 *  @param [in] chips_data  chips_data结构体指针
 *  @param [out] buf  成功时存储读取数据的buffer指针      
 *  @param [in] len  要读取的字节数      
 *  
 *  @return 成功返回0,失败返回负数
 */
static int chips_spi_read(struct chips_data *chips_data,void *buf, int len)
{
	struct spi_transfer	t = {
			.rx_buf		= buf,
			.len		= len,
		};
	struct spi_message	m;

	spi_message_init(&m);
	spi_message_add_tail(&t, &m);
	return chips_sync(chips_data,&m);
}

#endif

 /**
 *  @brief chips_sfr_read 读SFR寄存器
 *  
 *  @param [in] addr  寄存器起始地址
 *  @param [out] data 读到的数据
 *  @param [in] len   读取的数据长度
 *  
 *  @return 成功返回0，失败返回负数
 */
int chips_sfr_read(struct chips_data *chips_data,unsigned short addr,unsigned char *data,unsigned short len)
{
	int status = -1;
	unsigned char tx_buf[2] = {0};

	tx_buf[0] = CHIPS_R_SFR;
	tx_buf[1] = (unsigned char)(addr & 0x00FF);

	status = chips_spi_full_duplex(chips_data, tx_buf, 2, data, len);
	if(status < 0){
		chips_dbg("Failed to read SFR from addr = 0x%x,len = %d\n",addr,len);
	}
	
	return status;
}


 /**
 *  @brief chips_sfr_write 写SFR寄存器
 *  
 *  @param [in] addr 寄存器起始地址
 *  @param [in] data 写入的数据
 *  @param [in] len  写入的数据长度
 *  
 *  @return 成功返回0，失败返回负数
 */
int chips_sfr_write(struct chips_data *chips_data,unsigned short addr,unsigned char *data,unsigned short len)
{
	
	unsigned char *tx_buf;
	int status = -1;

	tx_buf = (unsigned char *)kmalloc(len+2,GFP_KERNEL);
	if(NULL == tx_buf){
		chips_dbg("Failed to allocate mem for write sfr buffer\n");
		return -ENOMEM;
	}
	
	tx_buf[0] = CHIPS_W_SFR;
	tx_buf[1] = (unsigned char)(addr & 0x00FF);
	memcpy(tx_buf+2,data,len);

	status = chips_spi_full_duplex(chips_data,tx_buf,len+2,NULL,0);
	if(status < 0){
		chips_dbg("Failed to write SFR at addr = 0x%x,len = %d\n",addr,len);
	}
	
	if(NULL != tx_buf){
	    kfree(tx_buf);
		tx_buf = NULL;
	}
	
	return status;
}


 /**
 *  @brief chips_sram_read 读SRAM寄存器
 *  
 *  @param [in] addr 寄存器起始地址
 *  @param [in] data 读到的数据
 *  @param [in] len  读取的数据长度
 *  
 *  @return 成功返回0，失败返回负数
 */
int chips_sram_read(struct chips_data *chips_data,unsigned short addr,unsigned char *data,unsigned short len)
{
	unsigned char tx_buf[3] = {0};
	int status = -1;
	
	unsigned char *rx_buf;
	rx_buf = (unsigned char *)kmalloc(len+1, GFP_KERNEL);   //first nop
	if(NULL == rx_buf){
		chips_dbg("Failed to allocate mem for read sram buffer\n");
		return -ENOMEM;
	}
	
	tx_buf[0] = CHIPS_R_SRAM;
	tx_buf[1] = (unsigned char)((addr&0xFF00)>>8);
	tx_buf[2] = (unsigned char)(addr&0x00FF);
	
	status = chips_spi_full_duplex(chips_data, tx_buf, 3, rx_buf, len+1);
	if(status ==  0){
		memcpy(data,rx_buf+1,len);
	}else{
		chips_dbg("Failed to read SRAM from addr = 0x%x,len = %d\n",addr,len);
	}
		
	if(NULL != rx_buf){
		kfree(rx_buf);
		rx_buf = NULL;
	}
	
	return status;
}


 /**
 *  @brief chips_sram_write 写SRAM寄存器
 *  
 *  @param [in] addr 寄存器起始地址
 *  @param [in] data 写入的数据
 *  @param [in] len  写入的数据长度
 *  
 *  @return 成功返回0，失败返回负数
 */
int chips_sram_write(struct chips_data *chips_data,unsigned short addr,unsigned char *data,unsigned short len)
{ 
	unsigned char *tx_buf;
	int status = -1;
	
	tx_buf = (unsigned char *)kmalloc(len+3,GFP_KERNEL);
	if(NULL == tx_buf){
		chips_dbg("Failed to allocate mem for write sram buffer\n");
		return -ENOMEM;
	}

	tx_buf[0] = CHIPS_W_SRAM;
	tx_buf[1] = (unsigned char)((addr&0xFF00)>>8);
	tx_buf[2] = (unsigned char)(addr&0x00FF);        
	memcpy(tx_buf+3,data,len);

	status = chips_spi_full_duplex(chips_data,tx_buf,len+3,NULL,0);
	if(status < 0){
		chips_dbg("Failed to write SRAM at addr = 0x%x,len = %d\n",addr,len);
	}
	
	if(NULL != tx_buf){
		kfree(tx_buf);
		tx_buf = NULL;
	}
	
	return status;
}


 /**
 *  @brief chips_spi_send_cmd 发送spi命令
 *  
 *  @param [in] cmd spi命令
 *  @param [in] len spi命令数据长度
 *  
 *  @return 成功返回0，失败返回负数
 */
int chips_spi_send_cmd(struct chips_data *chips_data,unsigned char *cmd,unsigned short len)
{
	int status = -1;
	status = chips_spi_full_duplex(chips_data,cmd,len,NULL,0);
	if(status < 0){
		chips_dbg("Failed to send spi cmd\n");
	}
	
	return status;
}


 /**
 *  @brief chips_write_configs 给IC配置参数
 *  
 *  @param [in] p_param 寄存器参数结构体指针
 *  @param [in] num 参数个数    
 *  
 *  @return 成功返回0，失败返回负数
 */
int chips_write_configs(struct chips_data *chips_data,struct param *p_param, int num)
{
	struct param param;
	unsigned char data;
	int i = 0;
	int retval = 0;
	unsigned char tx_buf[2] = {0};
	
	for(i = 0; i < num; i++)
	{
		param = p_param[i];
		
		if(param.cmd == CHIPS_W_SFR) {
			data = (unsigned char)(param.data&0x00FF);
			retval = chips_sfr_write(chips_data,param.addr,&data,1);
			if(retval < 0){
				chips_dbg("write config err>1\n");
				return retval;
			}
			chips_dbg("param.cmd = %x,param.addr = %x,param.data = %x\n",param.cmd,param.addr,data);
		}else if(param.cmd == CHIPS_W_SRAM){
		    tx_buf[0] = (unsigned char)(param.data&0x00FF);  //低8位
	        tx_buf[1] = (unsigned char)((param.data&0xFF00)>>8);  //高8位
			retval = chips_sram_write(chips_data,param.addr,tx_buf,2);
			if(retval < 0){
				chips_dbg("write config err>2\n");
				return retval;
			}
			chips_dbg("param.cmd = %x,param.addr = %x,param.data = %x\n",param.cmd,param.addr,param.data);
		}else{
			chips_dbg("write config err>3\n");
		}
	}	
	return 0;
}





