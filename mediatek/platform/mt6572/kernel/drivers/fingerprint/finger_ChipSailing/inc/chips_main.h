#ifndef __CHIPS_MAIN_H__
#define __CHIPS_MAIN_H__

#include <linux/mutex.h>

#define CHIPS_W_SRAM 0xAA
#define CHIPS_R_SRAM 0xBB
#define CHIPS_W_SFR  0xCC
#define CHIPS_R_SFR  0xDD

#define DWS_CFG  1
#if DWS_CFG
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

/********************GPIO***********************/
//runyee zhou define
//#define GPIO_FINGER_EINT_PIN   GPIO144
//#define GPIO_FINGER_RST_PIN    GPIO26

//eint
#define CS_EINT_GPIO			       GPIO_FINGER_EINT_PIN 
#define CS_EINT_NUMBER			     CUST_EINT_FINGER_NUM
#define CS_SPI_EINT_PIN_MODE	   GPIO_FINGER_EINT_PIN_M_EINT //GPIO_MODE_03  //GPIO_MODE_00
//reset
#define CS_RST_PIN		           GPIO_FINGER_RST_PIN

//spi
#define CS_SPI_SCK_PIN			(GPIO98 | 0x00000000)
#define CS_SPI_SCK_PIN_M_SCK	GPIO_MODE_01

#define	CS_SPI_CS_PIN			(GPIO97 | 0x00000000)
#define CS_SPI_CS_PIN_M_CS		GPIO_MODE_01

#define	CS_SPI_MOSI_PIN			(GPIO99 | 0x00000000)
#define CS_SPI_MOSI_PIN_M_MOSI	GPIO_MODE_01

#define	CS_SPI_MISO_PIN			(GPIO100 | 0x00000000)
#define CS_SPI_MISO_PIN_M_MISO	GPIO_MODE_01
#endif

struct chips_data {
	dev_t devt;
	unsigned char *buffer;
	int users;
	int irq;
	int irq_gpio;
	int reset_gpio;
	bool irq_enabled;
	spinlock_t spin_lock;
	struct class *cls;
	struct mutex buf_lock;
	struct input_dev *input;
	struct list_head device_entry;
	struct spi_device *spi;
	struct mt_chip_conf *spi_mcc;

#if DWS_CFG
	//unsigned int fingerprint_irq = 0; 
#else	
  struct pinctrl *pinctrl;
  struct pinctrl_state *eint_as_int, *rst_output0, *rst_output1;
  struct pinctrl_state *cs_finger_spi0_mi_as_spi0_mi,*cs_finger_spi0_mi_as_gpio,*cs_finger_spi0_mo_as_spi0_mo,*cs_finger_spi0_mo_as_gpio;
  struct pinctrl_state *cs_finger_spi0_clk_as_spi0_clk,*cs_finger_spi0_clk_as_gpio,*cs_finger_spi0_cs_as_spi0_cs,*cs_finger_spi0_cs_as_gpio;
#endif
};

#define DEBUG

#ifdef DEBUG
#define chips_dbg(fmt, args...) do {\
    printk("[chipsailing]%5d:<%s>  "fmt,__LINE__,__func__,##args);\
} while(0)
	
#else
#define chips_dbg(fmt, args...)
#endif

#endif