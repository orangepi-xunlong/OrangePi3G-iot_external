#include <cust_leds.h>
#include <cust_leds_def.h>
#include <mach/mt_pwm.h>
#include <cust_gpio_usage.h>
#include <linux/delay.h>

#include <linux/kernel.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_boot.h>
#include <mach/mt_gpio.h>

//extern int mtkfb_set_backlight_level(unsigned int level);
//extern int mtkfb_set_backlight_pwm(int div);
unsigned int set_backlight_first = 1;

unsigned int tmp_current_level = 0;
//brian-add
unsigned int tmp_previous_level = 32;
extern int disp_bls_set_backlight(unsigned int level);
/*
#define ERROR_BL_LEVEL 0xFFFFFFFF

unsigned int brightness_mapping(unsigned int level)
{  
	return ERROR_BL_LEVEL;
}
*/


 unsigned int brightness_mappingto16(unsigned int level)
{
	unsigned int map_level = 0;
	map_level = 16-level/16; //0-15,16-31,31-47......
	return map_level;
}

unsigned int brightness_mappingto32(unsigned int level)
{
	unsigned int map_level = 0;
	map_level = 32-level/8; //0-15,16-31,31-47......
	return map_level;
}

unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;
    
    mapped_level = level;
       
	return mapped_level;
}

unsigned int Cust_SetBacklight(int level, int div)
{
    //mtkfb_set_backlight_pwm(div);
    //mtkfb_set_backlight_level(brightness_mapping(level));
    disp_bls_set_backlight(brightness_mapping(level));
    return 0;
}
#ifdef GPIO_RY_LCM_BACKLIAGHT_PIN
unsigned int Cust_SetBacklight_RY(int level, int div)
{

//    mtkfb_set_backlight_pwm(div);
//  mtkfb_set_backlight_level(brightness_mapping(level));

	//signalnal backlight set
	volatile int i =0, j = 0,step = 0,tmp_level = 0;

	printk("Cust_SetBacklight,level=%d\n",level);

	#if defined(Z01)
	tmp_level = brightness_mappingto16(level);//map 255 to 16
	#else
	tmp_level = brightness_mappingto32(level);//map 255 to 16
	#endif

	mt_set_gpio_mode(GPIO_RY_LCM_BACKLIAGHT_PIN, GPIO_MODE_00);
	mt_set_gpio_dir(GPIO_RY_LCM_BACKLIAGHT_PIN, GPIO_DIR_OUT);
	printk("Cust_SetBacklight,tmp_level=%d\n",tmp_level);
	
	if(tmp_level > tmp_previous_level)
	{
			step = tmp_level - tmp_previous_level;
			if((tmp_level == 32)&&(tmp_previous_level == 31))
			{
					step--;
			}			
	}
	else if(tmp_level < tmp_previous_level)
	{
			step = tmp_level + 32 - tmp_previous_level;	
	}
	else
	{
	//	if((RECOVERY_BOOT == get_boot_mode()) && (level == 0))
		if(level ==0)
		{
			mt_set_gpio_out(GPIO_RY_LCM_BACKLIAGHT_PIN, GPIO_OUT_ZERO);
			mdelay(2);
			tmp_previous_level=32;
		}
			return 0; // do nothing		
	}
	if(level == 0)
	{
		    printk("############SET BACKLIGHT OFF\n");
			mt_set_gpio_out(GPIO_RY_LCM_BACKLIAGHT_PIN, GPIO_OUT_ZERO);
			mdelay(2);	
	}
	else
	{	
		 printk("############SET BACKLIGHT ON\n");
	if ((ALARM_BOOT == get_boot_mode())&& (set_backlight_first == 1))
	{
		printk("ALARM BOOT,backlight set first\n");
		//set_backlight_first = 0 ; remove it for alarm feature
		//tmp_previous_level = 32;
		//return 0;
	}
			if(tmp_previous_level ==  32)
			{
					mt_set_gpio_out(GPIO_RY_LCM_BACKLIAGHT_PIN, GPIO_OUT_ZERO);
					mdelay(2);	
			}	
			local_irq_disable();
			for(i=0;i<step;i++)
			{
		   		mt_set_gpio_out(GPIO_RY_LCM_BACKLIAGHT_PIN,GPIO_OUT_ZERO);
					udelay(1);
	   			mt_set_gpio_out(GPIO_RY_LCM_BACKLIAGHT_PIN,GPIO_OUT_ONE);
	   			udelay(1);						
			}
			local_irq_enable();
			mdelay(2);		
	}
	tmp_previous_level = tmp_level;	
    return 0;
}
#endif
#ifdef GPIO_RY_LED_RED_PIN 
unsigned int Cust_SetRedlight(int level, int div)
{
	//printk("Cust_SetRedlight,level=%d\n",level);

	mt_set_gpio_mode(GPIO_RY_LED_RED_PIN, 0);
	mt_set_gpio_dir(GPIO_RY_LED_RED_PIN, GPIO_DIR_OUT);

	if(level == 0)
		mt_set_gpio_out(GPIO_RY_LED_RED_PIN, GPIO_OUT_ZERO);
	else
		mt_set_gpio_out(GPIO_RY_LED_RED_PIN, GPIO_OUT_ONE);
	mdelay(2);		
    return 0;
}
#endif

#ifdef GPIO_RY_LED_GREEN_PIN 
unsigned int Cust_SetGreenlight(int level, int div)
{
//	printk("Cust_SetRedlight,level=%d\n",level);

	mt_set_gpio_mode(GPIO_RY_LED_GREEN_PIN, 0);
	mt_set_gpio_dir(GPIO_RY_LED_GREEN_PIN, GPIO_DIR_OUT);

	if(level == 0)
		mt_set_gpio_out(GPIO_RY_LED_GREEN_PIN, GPIO_OUT_ZERO);
	else
		mt_set_gpio_out(GPIO_RY_LED_GREEN_PIN, GPIO_OUT_ONE);
	mdelay(2);		
    return 0;
}
#endif

#ifdef GPIO_RY_LED_BLUE_PIN 
unsigned int Cust_SetBluelight(int level, int div)
{
	//printk("Cust_SetRedlight,level=%d\n",level);

	mt_set_gpio_mode(GPIO_RY_LED_BLUE_PIN, 0);
	mt_set_gpio_dir(GPIO_RY_LED_BLUE_PIN, GPIO_DIR_OUT);

	if(level == 0)
		mt_set_gpio_out(GPIO_RY_LED_BLUE_PIN, GPIO_OUT_ZERO);
	else
		mt_set_gpio_out(GPIO_RY_LED_BLUE_PIN, GPIO_OUT_ONE);
	mdelay(2);		
    return 0;
}
#endif
static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_NONE, -1, {0}},
	{"green",             MT65XX_LED_MODE_NONE, -1, {0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1, {0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1, {0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1, {0}},
	{"button-backlight",  MT65XX_LED_MODE_NONE, -1, {0}},
#if 0//def GPIO_RY_LCM_BACKLIAGHT_PIN
	{"lcd-backlight",     MT65XX_LED_MODE_GPIO,  (long)Cust_SetBacklight_RY, {0}},
#else
  //{"lcd-backlight",   MT65XX_LED_MODE_CUST_BLS_PWM, (int)disp_bls_set_backlight, {0}},
  {"lcd-backlight",     MT65XX_LED_MODE_PWM, PWM1, {0}},
#endif
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

