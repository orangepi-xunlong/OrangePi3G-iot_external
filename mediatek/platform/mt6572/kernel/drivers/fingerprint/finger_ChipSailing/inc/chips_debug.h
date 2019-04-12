#ifndef __CHIPS_DEBUG_H__
#define __CHIPS_DEBUG_H__


enum mode{
	IDLE = 0,
	NORMAL = 1,
	SLEEP = 2,
	DEEP_SLEEP = 6,	
};

int chips_probe_sensorID(unsigned short *sensorid);
int read_SFR(unsigned short addr,unsigned char *data);
int write_SFR(unsigned short addr,unsigned char data);
int read_SRAM(unsigned short addr,unsigned short *data);
int write_SRAM(unsigned short addr,unsigned short data);
void chips_sensor_config(void);
int chips_scan_one_image(unsigned short addr, unsigned char *buffer, unsigned short len);
int chips_set_sensor_mode(int mode);
int chips_hw_reset(struct chips_data *chips_data, unsigned int delay_ms);
int chips_esd_reset(struct chips_data *chips_data,unsigned int delay_ms);

#endif