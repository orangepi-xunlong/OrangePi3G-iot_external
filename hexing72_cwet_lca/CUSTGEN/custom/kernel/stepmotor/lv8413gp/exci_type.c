/*
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 */

#include "lv8413gp.h"

#include <linux/spinlock.h>


#define LDO_EN_GPIO		        (GPIO22)//(GPIO22|0x80000000)
#define LV8413_EN_GPIO		        (GPIO24)//(GPIO22|0x80000000)
#define A_PLUS_PIN_GPIO		        (GPIO14)//(GPIO14|0x80000000)
#define A_MINUS_PIN_GPIO		(GPIO15)//(GPIO15|0x80000000)
#define B_PLUS_PIN_GPIO		        (GPIO16)//(GPIO16|0x80000000)
#define B_MINUS_PIN_GPIO		(GPIO30)//(GPIO30|0x80000000)

#define SM_DELAY_HIGH  3
#define SM_DELAY_MEDIA  800
#define SM_DELAY_LOW  500

extern int stepmotor_enable;
extern int fmenable;
extern int ccw_enable; //if need ccw continue flag
extern int cw_enable;  //if need cw conginue flag
static struct mutex stepmotor_ops_mutex;
static DEFINE_SPINLOCK(g_stepmotorSMPLock); /* cotta-- SMP proection */

int step_status = 0;  //:min:0   max:1075
int dir_cw_ccw = 0;  //0:cw enable 1:ccw_enable

//////////////////////////////////////////////////////////////////////
void stepmotor_pwr_enable(int flag)
{
	if(1 == flag)
	{
		mt_set_gpio_mode(LDO_EN_GPIO, GPIO_MODE_00); 
		mt_set_gpio_pull_enable(LDO_EN_GPIO,GPIO_PULL_ENABLE);
		mt_set_gpio_dir(LDO_EN_GPIO, GPIO_DIR_OUT); 
		mt_set_gpio_out(LDO_EN_GPIO, 1); 
		udelay(1000);

		mt_set_gpio_mode(LV8413_EN_GPIO, GPIO_MODE_00); 
		mt_set_gpio_pull_enable(LV8413_EN_GPIO,GPIO_PULL_ENABLE);
		mt_set_gpio_dir(LV8413_EN_GPIO, GPIO_DIR_OUT); 
		mt_set_gpio_out(LV8413_EN_GPIO, 1); 
		udelay(1000);
	}
	else
	{
		mt_set_gpio_mode(LDO_EN_GPIO, GPIO_MODE_00); 
		mt_set_gpio_pull_enable(LDO_EN_GPIO,GPIO_PULL_ENABLE);
		mt_set_gpio_dir(LDO_EN_GPIO, GPIO_DIR_OUT); 
		mt_set_gpio_out(LDO_EN_GPIO, 0); 
		udelay(1000);

		mt_set_gpio_mode(LV8413_EN_GPIO, GPIO_MODE_00); 
		mt_set_gpio_pull_enable(LV8413_EN_GPIO,GPIO_PULL_ENABLE);
		mt_set_gpio_dir(LV8413_EN_GPIO, GPIO_DIR_OUT); 
		mt_set_gpio_out(LV8413_EN_GPIO, 0); 
		udelay(1000);
	}
}

void stepmotor_start(void)
{
	stepmotor_pwr_enable(1);

	mt_set_gpio_dir(A_PLUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_dir(A_MINUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_dir(B_PLUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_dir(B_MINUS_PIN_GPIO,GPIO_DIR_OUT);
}

void stepmotor_off(void)
{
	stepmotor_pwr_enable(0);

	mt_set_gpio_dir(A_PLUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_dir(A_MINUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_dir(B_PLUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_dir(B_MINUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
}

void stepmotor_break(void)
{	
	mt_set_gpio_dir(A_PLUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_dir(A_MINUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_dir(B_PLUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_dir(B_MINUS_PIN_GPIO,GPIO_DIR_OUT);
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
}

void stepmotor_hw_init(void)
{
	printk("stepmotor_hw_init ....\n");
	stepmotor_pwr_enable(1);

	mt_set_gpio_mode(A_PLUS_PIN_GPIO, GPIO_MODE_00); 
	mt_set_gpio_pull_enable(A_PLUS_PIN_GPIO,GPIO_PULL_ENABLE);
	mt_set_gpio_dir(A_PLUS_PIN_GPIO, GPIO_DIR_OUT); 
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0); 
	udelay(1000);
	mt_set_gpio_mode(A_MINUS_PIN_GPIO, GPIO_MODE_00);
	mt_set_gpio_pull_enable(A_MINUS_PIN_GPIO,GPIO_PULL_ENABLE); 
	mt_set_gpio_dir(A_MINUS_PIN_GPIO, GPIO_DIR_OUT); 
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0); 
	udelay(1000);
	mt_set_gpio_mode(B_PLUS_PIN_GPIO, GPIO_MODE_00); 
	mt_set_gpio_pull_enable(B_PLUS_PIN_GPIO,GPIO_PULL_ENABLE);
	mt_set_gpio_dir(B_PLUS_PIN_GPIO, GPIO_DIR_OUT); 
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0); 
	udelay(1000);
	mt_set_gpio_mode(B_MINUS_PIN_GPIO, GPIO_MODE_00); 
	mt_set_gpio_pull_enable(B_MINUS_PIN_GPIO,GPIO_PULL_ENABLE);
	mt_set_gpio_dir(B_MINUS_PIN_GPIO, GPIO_DIR_OUT); 
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0); 
	udelay(1000);
}



//////////////////////////////////////////////////////////////////////
void stepmotor_cw_one_phase_excitation(void)
{
	int i = 0;

	printk("stepmotor_cw_one_phase_excitation ....\n");

	//cw_table
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(1000);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(1000);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(1000);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(1000);
}

void stepmotor_ccw_one_phase_excitation(void)
{
	printk("stepmotor_ccw_one_phase_excitation ....\n");

	//ccw_table
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(1000);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(1000);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(1000);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(1000);
}

/////////////////////////////////////////////////////////////////////
void stepmotor_cw_two_phase_excitation(void)
{
	int i = 0;

	printk("stepmotor_cw_two_phase_excitation ....\n");

	//cw_table
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(SM_DELAY_LOW);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(SM_DELAY_LOW);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(SM_DELAY_LOW);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(SM_DELAY_LOW);
}
void stepmotor_ccw_two_phase_excitation(void)
{
	printk("stepmotor_cw_two_phase_excitation ....\n");

	//ccw_table
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(SM_DELAY_LOW);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(SM_DELAY_LOW);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(SM_DELAY_LOW);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(SM_DELAY_LOW);
}

/////////////////////////////////////////////////////////////////////////
void stepmotor_cw_one_two_phase_excitation(void)
{
	int i = 0;

	printk("stepmotor_cw_one_two_phase_excitation ....\n");

	//cw_table
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	mdelay(SM_DELAY_HIGH);
}
void stepmotor_ccw_one_two_phase_excitation(void)
{
	int i = 0;

	printk("stepmotor_cw_one_two_phase_excitation ....\n");

	//ccw_table
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	mdelay(SM_DELAY_HIGH);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	mdelay(SM_DELAY_HIGH);
}

/////////////////////////////////////////////////////////////////////////////
void stepmotor_cw_one_two_phase_excitation_quick(void)
{
	int i = 0;

	printk("stepmotor_cw_one_two_phase_excitation_quick ....\n");

	//cw_table
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(150);
}
void stepmotor_ccw_one_two_phase_excitation_quick(void)
{
	int i = 0;

	printk("stepmotor_ccw_one_two_phase_excitation_quick ....\n");

	//ccw_table
	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
	udelay(150);

	mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
	mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
	mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
	udelay(150);
}

/////////////////////////////////////////////////////////////////////////////////
void stepmotor_cw_two_phase_excitation_step(int nums)//total steps:537
{
	int i = 0;

	printk("cw_two_step: cw_enable=%d  begin....\n", cw_enable);

	if(0 == cw_enable)
	{
	#if 0
		for(i = 0; i < 20; ++i)
		{ 
			//stepmotor_cw_two_phase_excitation();
			stepmotor_cw_one_two_phase_excitation_quick();
		}
	#endif

		return;
	}

	//mutex_lock(&stepmotor_ops_mutex);
	//spin_lock_irq(&g_stepmotorSMPLock);
	spin_lock(&g_stepmotorSMPLock);

	//while(cw_enable)
	while(nums > 0)
	{
		if(0 == cw_enable)
			break;

		//cw_table
		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		udelay(SM_DELAY_LOW);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		udelay(SM_DELAY_LOW);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		udelay(SM_DELAY_LOW);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		udelay(SM_DELAY_LOW);
		--nums;
	}

	//mutex_unlock(&stepmotor_ops_mutex);
	//spin_unlock_irq(&g_stepmotorSMPLock);
	spin_unlock(&g_stepmotorSMPLock);

	printk("cw_two_step: cw_enable=%d  end....\n", cw_enable);
	printk("cw_two_step: nums=%d  end....\n", nums);

	stepmotor_break();
	udelay(SM_DELAY_LOW);

#if 1
	if(0 == cw_enable)
	{
		//solve quick rotation,can't go to bottom
		for(i = 0; i < 20; ++i)
		{ 
			//stepmotor_cw_two_phase_excitation();
			stepmotor_cw_one_two_phase_excitation();
		}
		printk("cw_two_step: cw_enable=%d  i=%d  end continule rotation....\n", cw_enable, i);
	}
#endif

}
void stepmotor_ccw_two_phase_excitation_step(int nums)//total steps:537
{
	int i = 0;

	printk("ccw_two_step: ccw_enable=%d  begin....\n", ccw_enable);
	
	if(0 == ccw_enable)
	{
		#if 0
		for(i = 0; i < 20; ++i)
		{ 
			//stepmotor_ccw_two_phase_excitation();
			stepmotor_ccw_one_two_phase_excitation_quick();
		}
		#endif

		return;
	}

	//mutex_lock(&stepmotor_ops_mutex);
	//spin_lock_irq(&g_stepmotorSMPLock);
	spin_lock(&g_stepmotorSMPLock);

	//while(ccw_enable)
	while(nums > 0)
	{
		if(0 == ccw_enable)
			break;

		//ccw_table
		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		udelay(SM_DELAY_LOW);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		udelay(SM_DELAY_LOW);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		udelay(SM_DELAY_LOW);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		udelay(SM_DELAY_LOW);
		--nums;
	}
	//mutex_unlock(&stepmotor_ops_mutex);
	//spin_unlock_irq(&g_stepmotorSMPLock);
	spin_unlock(&g_stepmotorSMPLock);
	
	printk("ccw_two_step: ccw_enable=%d end....\n", ccw_enable);
	printk("ccw_two_step: nums=%d  end....\n", nums);
	
	stepmotor_break();
	udelay(SM_DELAY_LOW);

#if 1
	if(0 == ccw_enable)
	{
		//solve quick rotation,can't go to bottom
		for(i = 0; i < 20; ++i)
		{ 
			//stepmotor_ccw_two_phase_excitation();
			stepmotor_ccw_one_two_phase_excitation();
		}
		printk("ccw_two_step: ccw_enable=%d  i=%d  end continule rotation....\n", ccw_enable, i);
	}
#endif
}

void stepmotor_cw_two_phase_excitation_206(void)//134x72+34(18x2) = 206x47
{
	int i = 0;

	stepmotor_start();

	printk("stepmotor_cw_one_two_phase_excitation_206... \n");

	stepmotor_cw_two_phase_excitation_step(550);//537

	stepmotor_off();
	//stepmotor_break();
}

void stepmotor_ccw_two_phase_excitation_206(void)//134x72+34(18x2) = 206x47
{
	int i = 0;

	stepmotor_start();
	
	printk("stepmotor_ccw_one_two_phase_excitation_206... \n");

	stepmotor_ccw_two_phase_excitation_step(550);//537

	stepmotor_off();
	//stepmotor_break();
}
////////////////////////////////////////////////////////////////////////////////
void stepmotor_cw_two_phase_excitation_fmtest(void)//206x47
{
	int i = 0;

	stepmotor_start();

	printk("stepmotor_ccw_two_phase_excitation_fmtest... \n");

	for(i = 0; i < 135; ++i)
	{
		if(0 == fmenable)
			break; 
		stepmotor_cw_two_phase_excitation();
	}
	
	//fmenable = 0;

	stepmotor_off();
}

void stepmotor_ccw_two_phase_excitation_fmtest(void)//206x47
{
	int i = 0;

	stepmotor_start();
	
	printk("stepmotor_ccw_two_phase_excitation_fmtest... \n");
	
	for(i = 0; i < 135; ++i)
	{
		if(0 == fmenable)
			break; 
		stepmotor_ccw_two_phase_excitation();
	}
	
	//fmenable = 0;

	stepmotor_off();
}

//////////////////////////////////////////////////////////////////////////////////////
void stepmotor_one_two_phase_excitation_parse(int dir, int nums)//dir:1-cw 2-ccw nums:phases
{
	printk("stepmotor_one_two_phase_excitation_parse  nums = %d  ....\n", nums);

	switch(nums)
	{
		case 7:
		if(1 == dir)
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);
		}
		else
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		break;

		case 6:
		if(1 == dir)
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		else
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		break;

		case 5:
		if(1 == dir)
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		else
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		break;

		case 4:
		if(1 == dir)
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		else
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		break;

		case 3:
		if(1 == dir)
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		else
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		break;

		case 2:
		if(1 == dir)
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
			mdelay(SM_DELAY_HIGH);
		}
		else
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);

			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);
		}
		break;

		case 1:
		if(1 == dir)
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);
		}
		else
		{
			mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
			mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
			mdelay(SM_DELAY_HIGH);
		}
		break;

		case 0:
		break;

		default:
		break;
	}
}

void stepmotor_one_two_phase_excitation_parse_cw(int nums)
{
	printk("stepmotor_one_two_phase_excitation_parse_cw  nums = %d  ....\n", nums);

	while(nums)
	{
		if(0 == stepmotor_enable)
		{
			//stepmotor_off();
			//stepmotor_enable = 1;
			break;
		}

		//cw_table
		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		mdelay(SM_DELAY_HIGH);
		--nums;
	}

#if 0
	stepmotor_break();
	udelay(SM_DELAY_LOW);

	if(0 == cw_enable)
	{
		//solve quick rotation,can't go to bottom
		for(i = 0; i < 20; ++i)
		{ 
			//stepmotor_cw_two_phase_excitation();
			stepmotor_cw_one_two_phase_excitation();
		}
		stepmotor_enable = 0;
		printk("stepmotor_one_two_phase_excitation_parse_cw: cw_enable=%d  end continule rotation....\n", cw_enable);
	}
#endif
}

void stepmotor_one_two_phase_excitation_parse_ccw(int nums)
{
	printk("stepmotor_one_two_phase_excitation_parse_ccw  nums = %d  ....\n", nums);

	while(nums)
	{
		if(0 == stepmotor_enable)
		{
			//stepmotor_off();
			//stepmotor_enable = 1;
			break;
		}

		//ccw_table
		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 0);
		mdelay(SM_DELAY_HIGH);
		--nums;

		mt_set_gpio_out(A_PLUS_PIN_GPIO, 1);
		mt_set_gpio_out(A_MINUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_PLUS_PIN_GPIO, 0);
		mt_set_gpio_out(B_MINUS_PIN_GPIO, 1);
		mdelay(SM_DELAY_HIGH);
		--nums;
	}

#if 0
	stepmotor_break();
	udelay(SM_DELAY_LOW);

	if(0 == ccw_enable)
	{
		for(i = 0; i < 2; ++i)
		{ 
			stepmotor_ccw_one_two_phase_excitation();
		}
		stepmotor_enable = 0;
		printk("stepmotor_one_two_phase_excitation_parse_ccw: ccw_enable=%d end continule rotation....\n", ccw_enable);
	}
#endif
}

void stepmotor_cw_one_two_phase_excitation_slow(int nums)
{
	int tMod = 0;
	int tCir = 0;
	int i = 0;

	stepmotor_start();

	printk("stepmotor_cw_one_two_phase_excitation_slow   nums = %d  ... \n", nums);

#if 1
	if(nums >= 8)
	{
		tCir = nums/8;
		tMod = nums%8;
		
		for(i = 0; i < tCir; ++i)
		{
			if(0 == stepmotor_enable)
				break; 
			stepmotor_cw_one_two_phase_excitation();
		}

		if(0 != tMod)
		{
			stepmotor_one_two_phase_excitation_parse(1,tMod);
		}

	}
	else
	{
		stepmotor_one_two_phase_excitation_parse(1,nums);
	}
	stepmotor_enable = 1;
#else
	stepmotor_one_two_phase_excitation_parse_cw(nums);
	stepmotor_enable = 1;
#endif

	//stepmotor_break();
	stepmotor_off();
}

void stepmotor_ccw_one_two_phase_excitation_slow(int nums)
{
	int tMod = 0;
	int tCir = 0;
	int i = 0;

	stepmotor_start();

	printk("stepmotor_ccw_one_two_phase_excitation_slow   nums = %d  ... \n", nums);

#if 1
	if(nums >= 8)
	{
		tCir = nums/8;
		tMod = nums%8;

		for(i = 0; i < tCir; ++i)
		{ 
			if(0 == stepmotor_enable)
				break; 
			stepmotor_ccw_one_two_phase_excitation();
		}

		if(0 != tMod)
		{
			stepmotor_one_two_phase_excitation_parse(2,tMod);
		}

	}
	else
	{
		stepmotor_one_two_phase_excitation_parse(2,nums);
	}

	stepmotor_enable = 1;
#else
	stepmotor_one_two_phase_excitation_parse_ccw(nums);
	stepmotor_enable = 1;
#endif

	//stepmotor_break();
	stepmotor_off();
}


