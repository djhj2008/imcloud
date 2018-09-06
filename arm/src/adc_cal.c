#include <linux/types.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <linux/rtc.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <string.h>

#include "im_log.h"
#include "data_config.h"
#include "eeprom_tool.h"

#define ADC_READY_DELAY 		500
#define LIST_MAX 				1024

#define  ADC_CAL_DEV_PATH         "/sys/bus/platform/devices/ff228000.register/"

struct adc_cal_reg {
	unsigned char   reg;
	unsigned short  offset;
	unsigned char   len;
	unsigned int    val;
};

static struct adc_cal_reg cal_list[LIST_MAX];

int setRegVal(char addr, char bit, int val)
{
	FILE* fd;
	char devname[128];
	char reg_value[32];

	if(val != 0 && val != 1){
		imlogE("adc7606 setRegVal val invalid : %d\n", val);
		return 0;
	}

	sprintf(devname, "%s%s", ADC_CAL_DEV_PATH, "register_regs");
	sprintf(reg_value,"%X %X %X", addr, bit, val);

	fd = fopen(devname, "w+");
	if(fd != NULL){
		fwrite(reg_value, 1, strlen(reg_value), fd);
	}else{
		imlogE("adc7606 setRegVal error : %s\n", devname);
	}

	fclose(fd);

	return 0;
}

int open_reg_file(FILE **out_fd){
	FILE* fd = NULL;
	int ret = 0;
	char devname[128];

	sprintf(devname, "%s%s", ADC_CAL_DEV_PATH, "cal_regs");
	fd = fopen(devname, "w+");
	if (fd ==NULL)
	{
		printf("adc_cal:can not open device %s\n", devname);
		ret = -1;
	}

	*out_fd = fd;
	return ret;
}

int writeRegVal(FILE* reg_fd, char addr, int val)
{
	char reg_value[32];
	int ret = 0;

	sprintf(reg_value,"%02X %08X", addr, val);

	if(reg_fd != NULL){
		ret = fwrite(reg_value, 1, strlen(reg_value), reg_fd);
		if(ret != strlen(reg_value))
		{
			imlogE("adc_cal writeRegVal fwrite error ret = %d\n", ret);
			ret = -1;
		}
    fseek(reg_fd,0,SEEK_SET);
    usleep(ADC_READY_DELAY);
	}else{
		imlogE("adc_cal writeRegVal error!\n");
		ret = -1;
	}

	return ret;
}

unsigned int U8StoU32(unsigned char* buffer, char len){
	int i;
	unsigned int data = buffer[0];

	for(i = 1; i< len; i++){
		data <<= 8;
		data |= buffer[i];
	}
	return data;
}

void adc_calibration(){

	FILE* reg_fd;
	int ret = 0;
	int i;
	struct data_config_info adc_data[LIST_MAX];
	int data_len = 0;

	data_len = init_data_config(adc_data);
	imlogE("adc_cal: data_len = %d\n", data_len);
	if(data_len <= 0 )
		return;

	memset(cal_list, 0, sizeof(cal_list));

	for(i = 0; i< data_len; i++)
	{
		unsigned int  offset = cal_list[i].offset = adc_data[i].offset;
		int  len = cal_list[i].len = adc_data[i].len;
		unsigned char buffer[len];

		cal_list[i].reg = adc_data[i].reg;
		read_eeprom(offset, (unsigned char*)buffer, len);
		cal_list[i].val = U8StoU32(buffer, len);
		//imlogE("adc_cal: cal_list[%d] = 0x%X , reg = 0x%X, offset = 0x%X\n", i, cal_list[i].val, cal_list[i].reg, cal_list[i].offset);

	}

	/*open ADC reg device*/
	ret = open_reg_file(&reg_fd);
	if(ret){
			return;
	}

	/*enable ADC calibration*/
	setRegVal(0, 1, 1);

	/*write calibration data to adc*/
	for(i = 0; i< data_len; i++)
	{
		unsigned int reg = cal_list[i].reg;
		unsigned int val = cal_list[i].val;

		imlogE("adc_cal: cal_list[%d] reg: %02X, val: 0x%08X \n", i, reg, val);
		ret = writeRegVal(reg_fd, reg, val);
		if(ret < 0){
				return;
		}

	}
	/*disable ADC calibration*/
	setRegVal(0, 1, 0);

	fclose(reg_fd);

}

