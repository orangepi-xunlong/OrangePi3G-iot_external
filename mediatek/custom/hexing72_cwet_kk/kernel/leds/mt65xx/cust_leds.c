#include <cust_leds.h>
#include <cust_leds_def.h>
#include <mach/mt_pwm.h>

#include <linux/kernel.h>
#include <mach/pmic_mt6329_hw_bank1.h> 
#include <mach/pmic_mt6329_sw_bank1.h> 
#include <mach/pmic_mt6329_hw.h>
#include <mach/pmic_mt6329_sw.h>
#include <mach/upmu_common_sw.h>
#include <mach/upmu_hw.h>
#include <mach/mt_pwm.h>
#include "cust_gpio_usage.h"
#include <mach/mt_gpio.h>
#include <linux/delay.h>
//extern int mtkfb_set_backlight_level(unsigned int level);
//extern int mtkfb_set_backlight_pwm(int div);
extern int disp_bls_set_backlight(unsigned int level);
/*
#define ERROR_BL_LEVEL 0xFFFFFFFF

unsigned int brightness_mapping(unsigned int level)
{  
	return ERROR_BL_LEVEL;
}
*/
/*
*  the following if for control breath light fy6203
*  
*/

#if defined(FY6203_BREATH_LIGHT)
static int fy6203_breath_ready(void)
{
  mt_set_gpio_mode(GPIO_KPD_KCOL2_PIN, GPIO_KPD_KCOL2_PIN_M_GPIO);  
  mt_set_gpio_dir(GPIO_KPD_KCOL2_PIN,1);
  //first,set low
  mt_set_gpio_out(GPIO_KPD_KCOL2_PIN,0);
  udelay(1000);
  //set high
  printk("[**]fy6203_en high \n");
  mt_set_gpio_out(GPIO_KPD_KCOL2_PIN,1);
  udelay(60);
  printk("[**]fy6203_breath_ready \n");
 return 0;
}
static int fy6203_en_pulse_count(int count)
{
  int i=0;

  for(i=0;i<count;i++)
   {
    mt_set_gpio_out(GPIO_KPD_KCOL2_PIN,0);
    udelay(60);
    mt_set_gpio_out(GPIO_KPD_KCOL2_PIN,1);
    udelay(10);
   }
   printk("[**]fy6203_en_pulse_count %d \n",i);	
return 0;
}

static int fy6203_breath_poweroff(void)
{
    mt_set_gpio_mode(GPIO_KPD_KCOL2_PIN, GPIO_KPD_KCOL2_PIN_M_GPIO);  
    mt_set_gpio_dir(GPIO_KPD_KCOL2_PIN,1);
    mt_set_gpio_out(GPIO_KPD_KCOL2_PIN,0);
    udelay(1000);
    printk("[**]fy6203_power_off \n");
return 0;
}

static int fy6203_breath_onoff(int mode)
{ 
  if(mode==0){
   fy6203_breath_poweroff();
  }
  else{
  fy6203_breath_ready();
  fy6203_en_pulse_count(mode);
  printk("[**]fy6203 enable pulse \n");
  }
return 0;  
}
#endif
unsigned int brightness_mapping(unsigned int level)
{
    unsigned int mapped_level;
	

    
    //mapped_level = level;
    mapped_level = level>>2;   

	printk("old level=%d , new level=%d \n",  level ,mapped_level );
	return mapped_level;
}

unsigned int Cust_SetBacklight(int level, int div)
{
    //mtkfb_set_backlight_pwm(div);
    //mtkfb_set_backlight_level(brightness_mapping(level));
    disp_bls_set_backlight(brightness_mapping(level));
    return 0;
}


static struct cust_mt65xx_led cust_led_list[MT65XX_LED_TYPE_TOTAL] = {
	{"red",               MT65XX_LED_MODE_NONE, -1, {0}},
	{"green",             MT65XX_LED_MODE_NONE, -1, {0}},
	{"blue",              MT65XX_LED_MODE_NONE, -1, {0}},
	{"jogball-backlight", MT65XX_LED_MODE_NONE, -1, {0}},
	{"keyboard-backlight",MT65XX_LED_MODE_NONE, -1, {0}},
	{"button-backlight",  MT65XX_LED_MODE_NONE, -1, {0}},
//	{"lcd-backlight",     MT65XX_LED_MODE_PMIC, MT65XX_LED_PMIC_LCD_ISINK,{0,0,0,0,0}},
// 	{"lcd-backlight",     MT65XX_LED_MODE_CUST_BLS_PWM, (int)disp_bls_set_backlight,{0}},
	{"lcd-backlight",     MT65XX_LED_MODE_PWM, PWM1, {1,1,4,4}},
	#if defined(FY6203_BREATH_LIGHT)
	{"breathing-light",	MT65XX_LED_MODE_GPIO_PULSE, fy6203_breath_onoff,{0}},
	#endif
};

struct cust_mt65xx_led *get_cust_led_list(void)
{
	return cust_led_list;
}

