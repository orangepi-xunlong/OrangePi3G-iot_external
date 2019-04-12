#ifndef _LV8413GP_H_
#define _LV8413GP_H_

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

#include <mach/mt_gpio.h>

struct stepmotor_data 
{
	struct mutex	mutex;
};

void stepmotor_pwr_enable(int flag);

void stepmotor_start(void);
void stepmotor_off(void);
void stepmotor_break(void);
void stepmotor_hw_init(void);

void stepmotor_cw_one_phase_excitation(void);
void stepmotor_ccw_one_phase_excitation(void);
void stepmotor_cw_two_phase_excitation(void);
void stepmotor_ccw_two_phase_excitation(void);
void stepmotor_cw_one_two_phase_excitation(void);
void stepmotor_ccw_one_two_phase_excitation(void);

void stepmotor_cw_two_phase_excitation_206(void);
void stepmotor_ccw_two_phase_excitation_206(void);
void stepmotor_cw_one_two_phase_excitation_slow(int nums);
void stepmotor_ccw_one_two_phase_excitation_slow(int nums);
void stepmotor_cw_two_phase_excitation_fmtest(void);
void stepmotor_ccw_two_phase_excitation_fmtest(void);

#endif /* _LV8413GP_H_ */
