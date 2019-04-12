#ifndef __CHIPS_COMMON_H__
#define __CHIPS_COMMON_H__

struct param {
	unsigned char cmd;
	unsigned short addr;
	unsigned short data;	
};

struct config {
	struct param *p_param;
	int num;
};


int chips_sfr_read(struct chips_data *chips_data,unsigned short addr,unsigned char *data,unsigned short len);
int chips_sfr_write(struct chips_data *chips_data,unsigned short addr,unsigned char *data,unsigned short len);
int chips_sram_read(struct chips_data *chips_data,unsigned short addr,unsigned char *data,unsigned short len);
int chips_sram_write(struct chips_data *chips_data,unsigned short addr,unsigned char *data,unsigned short len);
int chips_spi_send_cmd(struct chips_data *chips_data,unsigned char *cmd,unsigned short len);
int chips_write_configs(struct chips_data *chips_data,struct param *p_param, int num);

#endif