/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  EEPROM interface
 *               For usage, check the eeprom_tool.h file
 *
 *//** @file eeprom_tool.h *//*
 *
 ********************************/
 
 
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

#include "eeprom_tool.h"
#include "global_var.h"
#include "im_log.h"

unsigned bcd2bin(unsigned char val)
{       
	return (val & 0x0f) + (val >> 4) * 10;  
}

unsigned char bin2bcd(unsigned val)
{       
	return ((val / 10) << 4) + val % 10;
} 

static int __i2c_set(int fd, uint32_t timeout, uint32_t retry)
{
	if (fd == 0 )
		return -1;

	ioctl(fd, I2C_TIMEOUT, timeout ? timeout : I2C_DEFAULT_TIMEOUT);
	ioctl(fd, I2C_RETRIES, retry ? retry : I2C_DEFAULT_RETRY);
	
	return 0;
}

int i2c_open(char* dev, uint32_t timeout, uint32_t retry)
{
	int fd=0;
	if ((fd = open(dev, O_RDWR)) < 0)
		return fd;
	
	__i2c_set(fd, timeout, retry);

	return fd;
}

static int __i2c_send(int fd, struct i2c_rdwr_ioctl_data *data)
{
	if (fd < 0)
		return -1;

	if (data == NULL)
		return -1;

	if (data->msgs == NULL || data->nmsgs == 0)
		return -1;
	
	return ioctl(fd, I2C_RDWR, (unsigned long)data) ;
}

int i2c_path(char* argv){

	int fd;
	
	fd = i2c_open(argv,TIMEOUT,RETRY);
	if( fd < 0 ){	
		imlogE("i2c_open error!\n");
		exit(-1);
	}
	return fd;	
}

int i2c_read_data(int fd,uint16_t addr, uint16_t offset, uint8_t *buf, int len)
{
	//int i
	int ret = 0;
	uint8_t w_buf[2] = {0};

	struct i2c_rdwr_ioctl_data data;
    struct i2c_msg             i2cmsg[2];


    data.msgs = i2cmsg;
	data.nmsgs = 2;
	w_buf[0] = (uint8_t)(offset >> 8);
	w_buf[1] = (uint8_t)(offset & 0xFF);
	//printf("i2c_read_data offset = %x \n", offset);
	//printf("i2c_read_data w_buf[0] = %x ,w_buf[1] = %x \n", w_buf[0],w_buf[1]);

	i2cmsg[0].addr = addr;
	i2cmsg[0].flags = 0;
	i2cmsg[0].len = 2;
	i2cmsg[0].buf = w_buf;

	i2cmsg[1].addr = addr;
	i2cmsg[1].flags = I2C_M_RD;
	i2cmsg[1].len = len;			//original data is 1
	i2cmsg[1].buf = buf;

	if ((ret = __i2c_send(fd, &data)) < 0)
		goto errexit0;

	//for(i = 0 ;i < data.msgs[1].len; i++)
	//	printf("read success, val  = 0x%x\n",data.msgs[1].buf[i]);

errexit0:

	return ret;
}

int i2c_write_data(int fd,uint16_t addr, uint16_t offset, char *buf, int len)
{
	int ret = 0;
	uint8_t my_buf [PAGE_SIZE + 2];

	struct i2c_rdwr_ioctl_data data;
    struct i2c_msg             i2cmsg;

	//printf("i2c_write_data start addr : 0x%x, offset = 0x%x, len = %d\n",addr, offset, len );

    data.msgs = &i2cmsg;
	data.nmsgs = 1;
	my_buf[0] = (uint8_t)(offset >> 8);
	my_buf[1] = (uint8_t)(offset & 0xFF);
	memcpy(my_buf + 2, buf, len);

	i2cmsg.addr = addr;
	i2cmsg.flags = 0;
	i2cmsg.len = len + 2;
	i2cmsg.buf = my_buf;

	if ((ret = __i2c_send(fd, &data)) < 0)
		goto errexit0;

	//printf("i2c_write_data stop addr : 0x%x\n",addr );

errexit0:

	return ret;
}

/*
* read_data();
* addr:   i2c slave address;
* offset: eeprom address;
* buf:     read data;
* len:     length of data read;
*/

int read_data(int fd,uint16_t addr, uint16_t offset, uint8_t *buf, int len){
	int ret;

	ret = i2c_read_data(fd,addr, offset, buf, len);
	if(ret < 0){
		imlogE("%s error!\n",__FUNCTION__);
		exit(-1);
	}
	//printf("read_data : %s\n", buf);
	return 0;
}

/*
* write_data();
* addr:   i2c slave address;
* offset: eeprom address;
* buf:     write data;
* len:     length of data write;
*/
int write_data(int fd,uint16_t addr, unsigned short offset, char *buf, int len){
		
	int ret;
	
	ret = i2c_write_data(fd,addr, offset, buf, len);
	if(ret < 0){
		imlogE("%s error!\n",__FUNCTION__);
		exit(-1);
	}

	imlogV("write success!\n");
	usleep(10000);   
	return 0;
}

void i2c_close(int fd)
{
	if (fd < 0)
		return;

	close(fd);
}
 
int im_get_Igain_Vgain(){
	//uint8_t buffer[VIGAIN_SIZE]={0x0};
	uint16_t igain=0;
	uint16_t vgain=0; 
	int fd;
	int ret = -1;

	fd=i2c_path("/dev/i2c-0");
	read_data(fd,EEPROM_SLAVER_ADDR, VGAIN_ADDR, (uint8_t *)&vgain, VIGAIN_SIZE);
	imlogV("vgain=%d",vgain);
	global_setVgain(vgain);
	read_data(fd,EEPROM_SLAVER_ADDR, IGAIN_ADDR, (uint8_t *)&igain, VIGAIN_SIZE);
	imlogV("igain=%d",igain);
	global_setIgain(igain);
	i2c_close(fd);
	if(vgain>1||igain>1){
		global_setIgain(1);
		global_setVgain(1);
		ret = 0;
	}

	return ret;
}

