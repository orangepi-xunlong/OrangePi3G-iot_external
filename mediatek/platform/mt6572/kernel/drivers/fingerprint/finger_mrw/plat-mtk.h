/* MicroArray Fingerprint
 * plat-mtk.h
 * date: 2015-08-20
 * version: v2.0
 * Author: czl
 */

#ifndef PLAT_MTK_H
#define PLAT_MTK_H

#include <linux/spi/spi.h>
#include <linux/cdev.h>
#include <linux/types.h>
#include <mach/mt_pm_ldo.h>
#include <mach/mt_spi.h>
#include <cust_gpio_usage.h>
#include <mach/mt_gpio.h>
#include <cust_eint.h>
#include <cust_eint_md1.h>

/**********runyee zhou defined degin**************/
#define CUST_EINT_POLARITY_LOW              0
#define CUST_EINT_POLARITY_HIGH             1
#define CUST_EINT_DEBOUNCE_DISABLE          0
#define CUST_EINT_DEBOUNCE_ENABLE           1
#define CUST_EINT_EDGE_SENSITIVE            0
#define CUST_EINT_LEVEL_SENSITIVE           1


#define MADEV_EINT_PIN     	   (GPIO144 | 0x80000000)
#define MADEV_EINT_PIN_MODE    GPIO_MODE_03
#define MADEV_EINT_NUM     	   5

#define  CUST_EINT_EDGE_SENSITIVE            0   //MT_EDGE_SENSITIVE
#define  CUST_EINTF_TRIGGER_RISING  0x00000001

/*********runyee zhou defined end*****************/

#if 0
#define GPIO_SPI_CS_PIN        GPIO65 
#define GPIO_SPI_SCK_PIN       GPIO66
#define GPIO_SPI_MISO_PIN      GPIO67 
#define GPIO_SPI_MOSI_PIN      GPIO68 

#define GPIO_SPI_CS_PIN_M_SPI_CS         GPIO_MODE_01 
#define GPIO_SPI_SCK_PIN_M_SPI_CK        GPIO_MODE_01 
#define GPIO_SPI_MISO_PIN_M_SPI_MI       GPIO_MODE_01 
#define GPIO_SPI_MOSI_PIN_M_SPI_MO       GPIO_MODE_01 
#endif //runyee zhou del


#define FLEN  (1024*15)		 // 读写长度
#define MODE_NAME  "MADEV"

extern void mt_eint_mask(unsigned int eint_num);
extern void mt_eint_unmask(unsigned int eint_num);
extern void mt_eint_set_hw_debounce(unsigned int eint_num, unsigned int ms);
extern void mt_eint_set_polarity(unsigned int eint_num, unsigned int pol);
extern unsigned int mt_eint_set_sens(unsigned int eint_num, unsigned int sens);
extern void mt_eint_registration(unsigned int eint_num, unsigned int flow, void (EINT_FUNC_PTR)(void), unsigned int is_auto_umask);

extern void mt_eint_print_status(void);

extern irqreturn_t madev_irq_handle(int irq, void *dev_id);
extern int mas_probe(struct spi_device *spi);
extern int mas_remove(struct spi_device *spi);

struct spi_device_id sdev_id = {"madev", 0};
struct spi_driver sdrv = {
	.driver = {
		.name =	"madev",
		.owner = THIS_MODULE,
	},
	.probe = mas_probe,
	.remove = mas_remove,
	.id_table = &sdev_id,
};

struct mt_chip_conf smt_conf = {
	.setuptime = 12,
	.holdtime = 12,
	.high_time = 12, // 10--6m 15--4m 20--3m 30--2m [ 60--1m 120--0.5m  300--0.2m]
	.low_time = 12,
	.cs_idletime = 10,
	.ulthgh_thrsh = 0,
	.cpol = 0,
	.cpha = 0,
	.rx_mlsb = SPI_MSB,
	.tx_mlsb = SPI_MSB,
	.tx_endian = 0,
	.rx_endian = 0,
	.com_mod = DMA_TRANSFER,
	.pause = 0,
	.finish_intr = 1,
	.deassert = 0,
	.ulthigh = 0,
	.tckdly = 0,
};

struct spi_board_info smt_info[] __initdata = {
	[0] = {
		.modalias = "madev",
		.max_speed_hz = SPI_SPEED,
		.bus_num = 0,
		.chip_select = 0,
		.mode = SPI_MODE_0,
		.controller_data = &smt_conf
	}
};

void mt_spi_set_mod(void) {
#if 0
	//mt_set_gpio_mode(GPIO_SPI_CS_PIN, GPIO_SPI_CS_PIN_M_GPIO);
	mt_set_gpio_mode(GPIO_SPI_CS_PIN, GPIO_SPI_CS_PIN_M_SPI_CSA);
	mt_set_gpio_dir(GPIO_SPI_CS_PIN, GPIO_DIR_OUT);
	mt_set_gpio_pull_select(GPIO_SPI_CS_PIN, GPIO_PULL_UP);
	mt_set_gpio_pull_enable(GPIO_SPI_CS_PIN, GPIO_PULL_ENABLE);

	mt_set_gpio_mode(GPIO_SPI_SCK_PIN, GPIO_SPI_SCK_PIN_M_SPI_CKA);
	mt_set_gpio_dir(GPIO_SPI_SCK_PIN, GPIO_DIR_OUT);
	mt_set_gpio_pull_select(GPIO_SPI_SCK_PIN, GPIO_PULL_UP);
	mt_set_gpio_pull_enable(GPIO_SPI_SCK_PIN, GPIO_PULL_ENABLE);

	mt_set_gpio_mode(GPIO_SPI_MISO_PIN, GPIO_SPI_MISO_PIN_M_SPI_MIA);
	mt_set_gpio_dir(GPIO_SPI_MISO_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_select(GPIO_SPI_MISO_PIN, GPIO_PULL_UP);
	mt_set_gpio_pull_enable(GPIO_SPI_MISO_PIN, GPIO_PULL_ENABLE);

	mt_set_gpio_mode(GPIO_SPI_MOSI_PIN, GPIO_SPI_MOSI_PIN_M_SPI_MOA);
	mt_set_gpio_dir(GPIO_SPI_MOSI_PIN, GPIO_DIR_OUT);
	mt_set_gpio_pull_select(GPIO_SPI_MOSI_PIN, GPIO_PULL_UP);
	mt_set_gpio_pull_enable(GPIO_SPI_MOSI_PIN, GPIO_PULL_ENABLE);
#endif	
	msleep(10);
}

int plat_register_driver(void) {
	int ret;

	printd("%s: start\n", __func__);

	//	mt_spi_set_mod();
	spi_register_board_info(smt_info, ARRAY_SIZE(smt_info));
	ret = spi_register_driver(&sdrv);

	printd("%s: end.\n", __func__);

	return ret;
}

void plat_unregister_driver(void) {
	spi_unregister_driver(&sdrv);
}

/* MTK电源开关
 * @power 1:开，0：关
 * @return 0成功，-1失败
 */
int plat_power(int power) {
	int ret = 0;

	// 仅PMU管理电源
//	if(power) {
//		ret = hwPowerOn(MT6323_POWER_LDO_VGP1, VOL_3000, MODE_NAME);
//	} else {
//		ret = hwPowerDown(MT6323_POWER_LDO_VGP1, MODE_NAME);
//	}
	return (ret==1)? 0: -1;
}

void plat_tansfer(struct spi_device *spi, int len) {
	static int mode = -1;
	int tmp = len>32? DMA_TRANSFER: FIFO_TRANSFER;

	//printd("%s: start\n", __func__);

	if(tmp!=mode) {
		struct mt_chip_conf *conf = (struct mt_chip_conf *) spi->controller_data;
		conf->com_mod = tmp;
		spi_setup(spi);
		mode = tmp;
	}

	//printd("%s: end.\n", __func__);
}

void plat_enable_irq(struct spi_device *spi, u8 flag) {
	static int state = -1;

	//printd("%s: start\n", __func__);

	if (state != flag) {
		if (flag) {
			printd("%s: enable_irq.\n", __func__);
			mt_eint_unmask(MADEV_EINT_NUM);
		} else {
			printd("%s: disable_irq.\n", __func__);
			mt_eint_mask(MADEV_EINT_NUM);
		}
		state = flag;
	}

	//printd("%s: end.\n", __func__);
}
int plat_request_irq(unsigned int irq, 
//int plat_request_irq(unsigned int irq, irq_handler_t handler, unsigned long flags,
//int plat_request_irq(unsigned int irq, void handler, unsigned long flags,
        const char *name, void *dev) {

	//printd("%s: start\n", __func__);

	mt_set_gpio_mode(MADEV_EINT_PIN, MADEV_EINT_PIN_MODE);
	mt_set_gpio_dir(MADEV_EINT_PIN, GPIO_DIR_IN);
	mt_set_gpio_pull_enable(MADEV_EINT_PIN, TRUE);
	mt_set_gpio_pull_select(MADEV_EINT_PIN, GPIO_PULL_UP);

	mt_eint_set_sens(MADEV_EINT_NUM, CUST_EINT_EDGE_SENSITIVE); //edge
	mt_eint_set_polarity(MADEV_EINT_NUM, CUST_EINTF_TRIGGER_RISING);  //high

      //  mt_eint_set_hw_debounce(MADEV_EINT_NUM, CUST_EINT_ALS_DEBOUNCE_CN);
	///mt_eint_registration(MADEV_EINT_NUM, CUST_EINTF_TRIGGER_RISING, handler, 0);
	//mt_eint_mask(MADEV_EINT_NUM);


	//printd("%s: end.\n", __func__);

	return 0;
}

#endif



