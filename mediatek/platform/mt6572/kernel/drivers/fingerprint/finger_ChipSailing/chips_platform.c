/*************************************************************************
  版权： ShenZhen ChipSailing Technology Co., Ltd. All rights reserved.
  文件名称: chips_platform.c
  文件描述: 与硬件平台相关的函数接口
  作者: zwp    ID:58    版本:2.0   日期:2016/10/16
  其他:
  历史:
      1. 日期:           作者:          ID:
	     修改说明:
		 
	  2.
 *************************************************************************/
#include "./inc/chips_main.h" 

#ifdef DWS_CFG
#include "cust_gpio_usage.h"
#include <linux/power_supply.h>
#include <mach/mt_gpio.h>
#include <mach/mt_spi.h>
#include <mach/eint.h>
#include <cust_eint.h>
#include <asm/uaccess.h>
#include <linux/ktime.h>
#include <linux/interrupt.h>
#include <linux/of_irq.h>
#include <linux/completion.h>
#include <mach/mt_clkmgr.h>
#include <mach/upmu_sw.h>
#include <mach/upmu_hw.h>
#include <mach/upmu_common.h>
#include <linux/jiffies.h>
#include <linux/timer.h>
#else
#include <linux/of.h>
#include <linux/err.h>
#include <linux/of_platform.h>
#include <linux/pinctrl/consumer.h>
#include <linux/gpio.h>
#include <linux/of_irq.h>
#endif

 /**
 *  @brief chips_parse_dts 解析dts、获取硬件参数信息
 *  
 *  @param [in] chips_data chips_data结构体指针
 *  
 *  @return 成功返回0，失败返回非0
 */

int chips_parse_dts(struct chips_data* chips_data)
{
	#ifdef DWS_CFG
	 //setting RST pin
		mt_set_gpio_mode(CS_RST_PIN, GPIO_MODE_00);
		//mt_set_gpio_dir(CS_RST_PIN, GPIO_DIR_OUT);
		//mt_set_gpio_pull_enable(CS_RST_PIN, GPIO_PULL_ENABLE);
		//mt_set_gpio_pull_select(CS_RST_PIN, GPIO_PULL_UP);

		//IRQ setting	
		mt_set_gpio_mode(CS_EINT_GPIO,CS_SPI_EINT_PIN_MODE);			
 	  mt_set_gpio_dir(CS_EINT_GPIO, GPIO_DIR_IN);
 	  mt_set_gpio_pull_enable(CS_EINT_GPIO, GPIO_PULL_DISABLE);
 	  //mt_set_gpio_pull_enable(CS_EINT_GPIO, GPIO_PULL_ENABLE);
	 	//mt_set_gpio_pull_select(CS_EINT_GPIO, GPIO_PULL_DOWN);

	#else
	
		struct device_node *node = NULL;
		struct platform_device *pdev = NULL;

		node = of_find_compatible_node(NULL, NULL, "mediatek,cs_finger");
		if (IS_ERR(node)) {
			chips_dbg("device node is null\n");
			return PTR_ERR(node);
		}
		
		pdev = of_find_device_by_node(node);
		if (IS_ERR(pdev)) {
			chips_dbg("platform device is null\n");
			return PTR_ERR(pdev);
		}

		chips_data->pinctrl = devm_pinctrl_get(&pdev->dev);
		if (IS_ERR(chips_data->pinctrl)) {
			chips_dbg("devm_pinctrl_get error\n");
			return  PTR_ERR(chips_data->pinctrl);
		}

		/* cs、clk、miso、mosi、int、rst */
		chips_data->rst_output1 = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_reset_en1");
		if (IS_ERR(chips_data->rst_output1)) {
			chips_dbg("Cannot find cs_finger_reset_en1\n");
			return PTR_ERR(chips_data->rst_output1);
		}
	 
		chips_data->rst_output0 = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_reset_en0");
		if (IS_ERR(chips_data->rst_output0)) {
			chips_dbg("Cannot find cs_finger_reset_en0\n");
			return PTR_ERR(chips_data->rst_output0);
		}
		
		chips_data->cs_finger_spi0_mi_as_spi0_mi = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_spi0_mi_as_spi0_mi");
		if (IS_ERR(chips_data->cs_finger_spi0_mi_as_spi0_mi)) {
			chips_dbg("Cannot find cs_finger_spi0_mi_as_spi0_mi\n");
			return PTR_ERR(chips_data->cs_finger_spi0_mi_as_spi0_mi);
		}
		
		chips_data->cs_finger_spi0_mi_as_gpio = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_spi0_mi_as_gpio");
		if (IS_ERR(chips_data->cs_finger_spi0_mi_as_gpio)) {
			chips_dbg("Cannot find cs_finger_spi0_mi_as_gpio\n");
			return PTR_ERR(chips_data->cs_finger_spi0_mi_as_gpio);
		}
		
		chips_data->cs_finger_spi0_mo_as_spi0_mo = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_spi0_mo_as_spi0_mo");
		if (IS_ERR(chips_data->cs_finger_spi0_mo_as_spi0_mo)) {
			chips_dbg("Cannot find cs_finger_spi0_mo_as_spi0_mo\n");
			return PTR_ERR(chips_data->cs_finger_spi0_mo_as_spi0_mo);
		}
		
		chips_data->cs_finger_spi0_mo_as_gpio = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_spi0_mo_as_gpio");
		if (IS_ERR(chips_data->cs_finger_spi0_mo_as_gpio)) {
			chips_dbg("Cannot find cs_finger_spi0_mo_as_gpio\n");
			return PTR_ERR(chips_data->cs_finger_spi0_mo_as_gpio);
		}
		
		chips_data->cs_finger_spi0_clk_as_spi0_clk = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_spi0_clk_as_spi0_clk");
		if (IS_ERR(chips_data->cs_finger_spi0_clk_as_spi0_clk)) {
			chips_dbg("Cannot find cs_finger_spi0_clk_as_spi0_clk\n");
			return PTR_ERR(chips_data->cs_finger_spi0_clk_as_spi0_clk);
		}
		
		chips_data->cs_finger_spi0_clk_as_gpio = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_spi0_clk_as_gpio");
		if (IS_ERR(chips_data->cs_finger_spi0_clk_as_gpio)) {
			chips_dbg("Cannot find cs_finger_spi0_clk_as_gpio\n");
			return PTR_ERR(chips_data->cs_finger_spi0_clk_as_gpio);
		}
		
		chips_data->cs_finger_spi0_cs_as_spi0_cs = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_spi0_cs_as_spi0_cs");
		if (IS_ERR(chips_data->cs_finger_spi0_cs_as_spi0_cs)) {
			chips_dbg("Cannot find cs_finger_spi0_cs_as_spi0_cs\n");
			return PTR_ERR(chips_data->cs_finger_spi0_cs_as_spi0_cs);
		}
		
		chips_data->cs_finger_spi0_cs_as_gpio = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_spi0_cs_as_gpio");
		if (IS_ERR(chips_data->cs_finger_spi0_cs_as_gpio)) {
			chips_dbg("Cannot find cs_finger_spi0_cs_as_gpio\n");
			return PTR_ERR(chips_data->cs_finger_spi0_cs_as_gpio);
		}
		
		chips_data->eint_as_int = pinctrl_lookup_state(chips_data->pinctrl, "cs_finger_int_as_int");
		if (IS_ERR(chips_data->eint_as_int)) {
			chips_dbg("Cannot find s_finger_int_as_int\n");
			return PTR_ERR(chips_data->eint_as_int);
		}
	#endif
	chips_dbg("get pinctrl success\n");
	return 0;
}


 /**
 *  @brief chips_release_gpio 释放中断以及复位gpio
 *  
 *  @param [in] chips_data chips_data结构体指针
 *  
 *  @return 无返回值
 */
void chips_release_gpio(struct chips_data* chips_data)
{
	#ifndef DWS_CFG
		if (gpio_is_valid(chips_data->irq_gpio)){
			gpio_free(chips_data->irq_gpio);
		}
		
		if (gpio_is_valid(chips_data->reset_gpio)){
			gpio_free(chips_data->reset_gpio);
		}
	#endif
}


 /**
 *  @brief chips_set_reset_gpio 设置复位IO电平状态
 *  
 *  @param [in] chips_data chips_data结构体指针
 *  @param [in] delay_ms 延时参数
 *  @return 成功返回0，失败返回负数
 */
int chips_set_reset_gpio(struct chips_data *chips_data, unsigned int level)
{
/*
	chips_data->reset_gpio = 86;
	if(gpio_request(chips_data->reset_gpio,"reset_gpio")<0){
		gpio_free(chips_data->reset_gpio);
		gpio_request(chips_data->reset_gpio, "reset_gpio");
		gpio_direction_output(chips_data->reset_gpio,1);
	}
	if(level != 0){
		gpio_set_value(chips_data->reset_gpio,1);	
	}else{
		gpio_set_value(chips_data->reset_gpio,0);
	}
	return 0;
*/
 #ifdef  DWS_CFG
	 if(level != 0){
			mt_set_gpio_out(CS_RST_PIN, GPIO_OUT_ONE);	
		}else{
			mt_set_gpio_out(CS_RST_PIN, GPIO_OUT_ZERO); 	
		}
 #else 
	int ret = -1;	
	
	if(level != 0){
		ret = pinctrl_select_state(chips_data->pinctrl, chips_data->rst_output1);
		if(ret < 0){
			chips_dbg("pinctrl_select_state error,ret = %d\n",ret);
			return ret;
		}
				
	}else{
		ret = pinctrl_select_state(chips_data->pinctrl, chips_data->rst_output0);
		if(ret < 0){
			chips_dbg("pinctrl_select_state error,ret = %d\n",ret);
			return ret;
		}
	}
	#endif
	return 0;
}


 /**
 *  @brief chips_get_irqno 获取中断号
 *  
 *  @param [in] chips_data chips_data结构体指针
 *  
 *  @return 成功返回中断号，失败返回负数
 */
int chips_get_irqno(struct chips_data *chips_data)
{
/*
	chips_data->irq_gpio = 87;
	if(gpio_request(chips_data->irq_gpio,"irq_gpio") < 0){
		gpio_free(chips_data->irq_gpio);
		gpio_request(chips_data->irq_gpio,"irq_gpio");
		gpio_direction_input(chips_data->irq_gpio);		
	}
	
	chips_data->irq = gpio_to_irq(chips_data->irq_gpio);
	if(chips_data->irq < 0){
		chips_dbg("gpio can‘t be mapped to irq\n");
	}
	
	return chips_data->irq;
*/
  #ifndef DWS_CFG
	u32 ints[2]={0};
	struct device_node *node;

	node = of_find_compatible_node(NULL, NULL, "mediatek,cs_finger");
	if(NULL != node){
		chips_data->irq = irq_of_parse_and_map(node, 0);
		
		of_property_read_u32_array(node,"debounce",ints,ARRAY_SIZE(ints));
		gpio_set_debounce(ints[0], ints[1]);
	}else{
		chips_dbg("of_find_compatible_node error\n");
		return -ENODEV;
	}
	#endif
	return chips_data->irq;
}


 /**
 *  @brief chips_set_spi_mode 设置spi模式
 *  
 *  @param [in] chips_data chips_data结构体指针
 *  
 *  @return 成功返回0，失败返回负数
 */
int chips_set_spi_mode(struct chips_data *chips_data)
{
#if 0 //runyee zhou del
	#ifdef DWS_CFG
		mt_set_gpio_mode(CS_SPI_CS_PIN,CS_SPI_CS_PIN_M_CS);
	  mt_set_gpio_pull_enable(CS_SPI_CS_PIN,GPIO_PULL_ENABLE);
	  mt_set_gpio_pull_select(CS_SPI_CS_PIN,GPIO_PULL_UP);

	  mt_set_gpio_mode(CS_SPI_SCK_PIN,CS_SPI_SCK_PIN_M_SCK);
	  mt_set_gpio_pull_enable(CS_SPI_SCK_PIN,GPIO_PULL_ENABLE);
	  mt_set_gpio_pull_select(CS_SPI_SCK_PIN,GPIO_PULL_DOWN);
			
	  mt_set_gpio_mode(CS_SPI_MISO_PIN,CS_SPI_MISO_PIN_M_MISO);
	  mt_set_gpio_pull_enable(CS_SPI_MISO_PIN,GPIO_PULL_ENABLE);
	  mt_set_gpio_pull_select(CS_SPI_MISO_PIN,GPIO_PULL_DOWN);

	  mt_set_gpio_mode(CS_SPI_MOSI_PIN,CS_SPI_MOSI_PIN_M_MOSI);
	  mt_set_gpio_pull_enable(CS_SPI_MOSI_PIN,GPIO_PULL_ENABLE);
	  mt_set_gpio_pull_select(CS_SPI_MOSI_PIN,GPIO_PULL_DOWN);

	#else
	
		int ret = -1;
		ret = pinctrl_select_state(chips_data->pinctrl, chips_data->cs_finger_spi0_clk_as_spi0_clk);
		if(ret < 0){
			chips_dbg("pinctrl_select_state error,ret = %d\n",ret);
			return ret;
		}
				
		ret = pinctrl_select_state(chips_data->pinctrl, chips_data->cs_finger_spi0_cs_as_spi0_cs);
		if(ret < 0){
			chips_dbg("pinctrl_select_state error,ret = %d\n",ret);
			return ret;
		}
			
		ret = pinctrl_select_state(chips_data->pinctrl, chips_data->cs_finger_spi0_mi_as_spi0_mi);
		if(ret < 0){
			chips_dbg("pinctrl_select_state error,ret = %d\n",ret);
			return ret;
		}
			
		ret = pinctrl_select_state(chips_data->pinctrl, chips_data->cs_finger_spi0_mo_as_spi0_mo);
		if(ret < 0){
			chips_dbg("pinctrl_select_state error,ret = %d\n",ret);
			return ret;
		}
	#endif
#endif	
	return 0;
}

