#include <linux/input.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <linux/rtc.h>
#include <unistd.h>
#include <string.h>
#include <time.h>
#include <getopt.h>
#include <syslog.h>
#include "data_config.h"


int open_data_file(FILE **out_fd){
	FILE* fd = NULL;
	int ret = 0;
	char devname[128];

	sprintf(devname, "%s", DATA_FILE);
	fd = fopen(devname, "rt");
	if (fd ==NULL)
	{
		printf("data file:can not open device %s\n", devname);
		ret = -1;
	}

	*out_fd = fd;
	return ret;
}

int read_data_info(FILE *fd, struct data_config_info *data)
{
	int read_count = 0;
	char  buf[256];
	char *	  bp;
	int   t;

	if(fd == NULL)
		return -1;

	while(fgets(buf, 255, fd))
	{
		bp = buf;

    bp = strstr(bp,"	{");
		if (bp != NULL){

			if(strncmp(bp,"	{",strlen("	{")) == 0)
			{
				bp += strlen("	{");

				/* -- afe reg -- */
				sscanf(bp, "%X", &t);
				data->reg = (unsigned char) t;

				/* -- eeprom offset -- */
		    bp = strstr(bp,",    ");
				bp += strlen(",    ");
				sscanf(bp, "%X", &t);
				data->offset = (unsigned short) t;
				/* -- data len -- */
		    bp = strstr(bp,",    ");
				bp += strlen(",    ");
				sscanf(bp, "%d", &t);
				data->len = (unsigned char) t;

				read_count++;
				data++;

			}
		}
	}
	return read_count;
}

int init_data_config(struct data_config_info *data){

	FILE* data_fd;
	int ret = 0;

	ret = open_data_file(&data_fd);
	if(ret){
			return ret;
	}
	ret = read_data_info(data_fd, data);
	if(ret){
			return ret;
	}
	fclose(data_fd);

	return ret;
}

