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
#include <assert.h>
#include <inttypes.h>
#include <netinet/in.h> 

#include "eeprom_tool.h"
#include "global_var.h"
#include "im_log.h"
#include "imcloud.h"

int i2c_open(char* dev, unsigned int timeout, unsigned int retry);
int i2c_read_data(u32 offset, u8 *buf, int len);
int i2c_write_data(u32 offset, u8 *buf, int len);
void i2c_close_exit(void);
unsigned char bin2bcd(unsigned val);
unsigned bcd2bin(unsigned char val);
static int __i2c_send(int fd, struct i2c_rdwr_ioctl_data *data);
static int __i2c_set(int fd, unsigned int timeout, unsigned int retry);

static int fd;
static u16 addr;

/*
* read_data();
* offset: eeprom address;
* buf:     read data;
* len:     length of data read;
*/
int read_data(u32 offset, u8 *buf, int len){
	int ret;
	int c_len = len;
	int o_len = 0;
	int i = 0;

	while(c_len > 0){

		o_len = PAGE_SIZE * i;
		//printf("%s c_len = %d, o_len = %d\n",__FUNCTION__, c_len, o_len);
		if(c_len > PAGE_SIZE){
			ret = i2c_read_data(offset + o_len, buf + o_len, PAGE_SIZE);
			if(ret < 0){
				printf("%s 1 error offset: 0x%x, ret = %d\n",__FUNCTION__, offset + o_len, ret);
				i2c_close_exit();
				return -1;
			}
			c_len -= PAGE_SIZE;
			i++;
		}else{
			ret = i2c_read_data(offset + o_len, buf + o_len, c_len);
			if(ret < 0){
				printf("%s 2 error offset: 0x%x, ret = %d\n",__FUNCTION__, offset + o_len, ret);
				i2c_close_exit();
				return -1;
			}
#if 0
			hexdump(buf, len);
#endif
			usleep(10000);
			return 0;
		}
		usleep(10000);
	}
	return 0;
}

/*
* write_data();
* offset: eeprom address;
* buf:     write data;
* len:     length of data write;
*/
int write_data(u32 offset, u8 *buf, int len){
	int ret;
	int f_len = 0;        //first length, Length of the current offset before the page node.
	int s_len = 0;        //secondary length, Length of the current offset after the page node.
	int offset_len = 0;   //Length calculation offset.
	int w_len = 0;	      //Current write length.
	//multiple offset addr,Calculate the most recent page node in the current offset.
	u32 m_offset = ((offset / PAGE_SIZE) + ((offset % PAGE_SIZE) ? 1: 0)) * PAGE_SIZE;

	f_len = m_offset - offset;
	f_len = (len <= f_len) ? len : ((f_len <= 0) ? 0 : f_len);
	s_len = (len <= f_len) ? 0 : (len - f_len);

	while(f_len || s_len){

			if(f_len){
				w_len = f_len;
				f_len = 0;
			}else{
				offset_len += w_len;
				if(s_len > PAGE_SIZE){
					w_len = PAGE_SIZE;
					s_len -= PAGE_SIZE;
				}else{
					w_len = s_len;
					s_len = 0;
				}
			}
			printf("%s offset: 0x%x, offset_len = %d, w_len = %d\n",__FUNCTION__, offset, offset_len, w_len);
			ret = i2c_write_data(offset + offset_len, buf + offset_len, w_len);
			if(ret < 0){
				printf("%s error offset: 0x%x, ret = %d\n",__FUNCTION__, offset, ret);
				i2c_close_exit();
				return -1;
			}
			usleep(10000);
	}

	printf("write success!\n");
	return 0;
}


void i2c_path(char* argv){

	fd = i2c_open(argv,TIMEOUT,RETRY);
	if( fd < 0 ){
		printf("i2c_open error!\n");
		exit(-1);
	}
}

void i2c_addr(char * argv){
	char *s = argv;
	addr	= bin2bcd(atoi(s+2));
}

int i2c_read_data(u32 offset, u8 *buf, int len)
{
	int ret = 0;
	u8 w_buf[2] = {0};
	u32 c_max_offset = offset + (len - 1);
	int b1_len = 0;
	int b2_len = 0;

	struct i2c_rdwr_ioctl_data data;
	struct i2c_msg             i2cmsg[2];

	if(c_max_offset <= BLOCK_MAX){
		//printf("i2c_read_data offset = 0x%x, c_max_offset = 0x%x ,len = %d\n", offset, c_max_offset, len);
		data.msgs = i2cmsg;
		data.nmsgs = 2;
		w_buf[0] = (u8)((offset >> 8) & 0xFF);
		w_buf[1] = (u8)(offset & 0xFF);

		i2cmsg[0].addr = I2C_ADDERSS_B0;
		i2cmsg[0].flags = 0;
		i2cmsg[0].len = 2;
		i2cmsg[0].buf = w_buf;

		i2cmsg[1].addr = I2C_ADDERSS_B0;
		i2cmsg[1].flags = I2C_M_RD;
		i2cmsg[1].len = len;
		i2cmsg[1].buf = buf;

		if ((ret = __i2c_send(fd, &data)) < 0)
			goto errexit0;

	}else	if(c_max_offset <= BLOCK2_MAX){

		if(offset > BLOCK_MAX){
			goto block2;
		}

		b1_len = (BLOCK_MAX - offset) + 1;
		b2_len = c_max_offset - BLOCK_MAX;
		//printf("i2c_read_data offset 0x%x, c_max_offset = 0x%x, b1_len = %d , b2_len = %d, len = %d\n", offset, c_max_offset, b1_len, b2_len, len);
		data.msgs = i2cmsg;
		data.nmsgs = 2;
		w_buf[0] = (u8)((offset >> 8) & 0xFF);
		w_buf[1] = (u8)(offset & 0xFF);

		i2cmsg[0].addr = I2C_ADDERSS_B0;
		i2cmsg[0].flags = 0;
		i2cmsg[0].len = 2;
		i2cmsg[0].buf = w_buf;

		i2cmsg[1].addr = I2C_ADDERSS_B0;
		i2cmsg[1].flags = I2C_M_RD;
		i2cmsg[1].len = b1_len;
		i2cmsg[1].buf = buf;

		if ((ret = __i2c_send(fd, &data)) < 0)
			goto errexit0;

		usleep(10000);
		len = b2_len;
		offset += b1_len;

block2:
		//printf("i2c_read_data block2 offset = 0x%x, c_max_offset = 0x%x, len = %d\n", offset, c_max_offset, len);
		offset &= 0xffff;
		data.msgs = i2cmsg;
		data.nmsgs = 2;
		w_buf[0] = (u8)((offset >> 8) & 0xFF);
		w_buf[1] = (u8)(offset & 0xFF);

		i2cmsg[0].addr = I2C_ADDERSS_B1;
		i2cmsg[0].flags = 0;
		i2cmsg[0].len = 2;
		i2cmsg[0].buf = w_buf;

		i2cmsg[1].addr = I2C_ADDERSS_B1;
		i2cmsg[1].flags = I2C_M_RD;
		i2cmsg[1].len = len;
		i2cmsg[1].buf = &buf[b1_len];

		if ((ret = __i2c_send(fd, &data)) < 0)
			goto errexit0;
	}else{
		ret = -1;
	}
errexit0:

	return ret;
}

int i2c_write_data(u32 offset, u8 *buf, int len)
{
	int ret = 0;
	u32 c_max_offset = offset + (len - 1);
	int b1_len = 0;
	int b2_len = 0;

	struct i2c_rdwr_ioctl_data data;
	struct i2c_msg             i2cmsg;

	if(c_max_offset <= BLOCK_MAX){
		//printf("i2c_write_data offset 0x%x, c_max_offset = 0x%x, len = %d\n", offset, c_max_offset, len);
		data.msgs = &i2cmsg;
		data.nmsgs = 1;
		buf[0] = (u8)(offset >> 8);
		buf[1] = (u8)(offset & 0xFF);

		i2cmsg.addr = I2C_ADDERSS_B0;
		i2cmsg.flags = 0;
		i2cmsg.len = len + ADDER_LENS;
		i2cmsg.buf = buf;

		if ((ret = __i2c_send(fd, &data)) < 0)
			goto errexit0;
		}else if(c_max_offset <= BLOCK2_MAX){


		if(offset > BLOCK_MAX){
			goto block2;
		}

		b1_len = (BLOCK_MAX - offset) + 1;
		b2_len = c_max_offset - BLOCK_MAX;
		//printf("i2c_write_data offset 0x%x, c_max_offset = 0x%x, b1_len = %d , b2_len = %d, len = %d\n", offset, c_max_offset, b1_len, b2_len, len);

		data.msgs = &i2cmsg;
		data.nmsgs = 1;
		buf[0] = (u8)(offset >> 8);
		buf[1] = (u8)(offset & 0xFF);

		i2cmsg.addr = I2C_ADDERSS_B0;
		i2cmsg.flags = 0;
		i2cmsg.len = b1_len + ADDER_LENS;
		i2cmsg.buf = buf;

		if ((ret = __i2c_send(fd, &data)) < 0)
			goto errexit0;

		usleep(10000);
		len = b2_len;
		offset += b1_len;

block2:
		//printf("i2c_write_data block2 offset 0x%x, c_max_offset = 0x%x, b1_len = %d , b2_len = %d, len = %d\n", offset, c_max_offset, b1_len, b2_len, len);
		offset &= 0xffff;
		data.msgs = &i2cmsg;
		data.nmsgs = 1;
		buf[b1_len + 0] = (u8)(offset >> 8);
		buf[b1_len + 1] = (u8)(offset & 0xFF);

		i2cmsg.addr = I2C_ADDERSS_B1;
		i2cmsg.len = len + ADDER_LENS;
		i2cmsg.flags = 0;
		i2cmsg.buf = &buf[b1_len];

		if ((ret = __i2c_send(fd, &data)) < 0)
			goto errexit0;

	}else{
		ret = -1;
	}

errexit0:

	return ret;
}

void clear_all_rom(void)
{
	int i;//, j;
	unsigned char buffer[PAGE_SIZE + ADDER_LENS];
	int ret = 0;

	printf("clear all data to 0x%X\n", SET_DATA);

	for(i = 0; i< DATA_LEN; i++)
	{
		memset(&buffer[ADDER_LENS], SET_DATA, sizeof(buffer) - ADDER_LENS);
		ret = i2c_write_data(i * PAGE_SIZE, buffer, PAGE_SIZE);
		if(ret < 0){
			printf("%s error offset 0x%X, ret = %d\n",__FUNCTION__, i * PAGE_SIZE, ret);
			i2c_close_exit();
		}
		usleep(10000);
	}
	printf("clear all data done!\n");

#if 0//test write is ok?
	for(i = 0; i< DATA_LEN; i++)
	{
		memset(buffer, 0x00, sizeof(buffer));
		ret = i2c_read_data(i * PAGE_SIZE, buffer, PAGE_SIZE);
		if(ret < 0){
			printf("%s error ret = %d\n",__FUNCTION__, ret);
			i2c_close_exit();
		}
		for(j = 0; j< PAGE_SIZE; j++)
		{
			if(buffer[j] != SET_DATA)
				printf("clear_all_rom test data fail j = %d\n", j);
		}
		usleep(10000);
	}
#endif
}

int i2c_open(char* dev, unsigned int timeout, unsigned int retry)
{
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

	return ioctl(fd, I2C_RDWR, (unsigned long)data);
}

static int __i2c_set(int fd, unsigned int timeout, unsigned int retry)
{
	if (fd == 0 )
		return -1;

	ioctl(fd, I2C_TIMEOUT, timeout ? timeout : I2C_DEFAULT_TIMEOUT);
	ioctl(fd, I2C_RETRIES, retry ? retry : I2C_DEFAULT_RETRY);

	return 0;
}

void i2c_close_exit(void)
{
	if (fd < 0)
		return;

	close(fd);
	//exit(1);
}

unsigned bcd2bin(unsigned char val)
{
	return (val & 0x0f) + (val >> 4) * 10;
}

unsigned char bin2bcd(unsigned val)
{
	return ((val / 10) << 4) + val % 10;
}

void read_eeprom(unsigned int offset, unsigned char* buffer, int len){

	i2c_path("/dev/i2c-0");
	//printf("Read : addr: 0x%x\n",offset);
	read_data(offset, buffer, len);
	//printf("read_eeprom: cal_list buffer0 0x%X, buffer1 0x%X\n", buffer[0], buffer[1]);
	i2c_close_exit();;

}

unsigned int U8StoU16(unsigned char* buffer){
	int i;
	unsigned int data = buffer[0];

	for(i = 1; i< 2; i++){
		data <<= 8;
		data |= buffer[i];
	}
	return data;
}

void eeprom_set_accesskey(char * access_key)
{
	u8 buf[ACCESS_KEY_SIZE+EEPROM_WRITE_OFFSET]={0x0};
	i2c_path("/dev/i2c-0");
	imlogV("eeprom_set_accesskey :%s",access_key);
	memcpy(buf+EEPROM_WRITE_OFFSET,access_key,ACCESS_KEY_SIZE);
	write_data(ACCESS_KEY_ADDR, (u8 *)buf, ACCESS_KEY_SIZE);
	i2c_close_exit();;
}

void eeprom_set_fw_version(uint16_t version)
{
	u8 buf[ADC_VERSION_SIZE+EEPROM_WRITE_OFFSET]={0x0};
	i2c_path("/dev/i2c-0");
	imlogV("eeprom_set_fw_version :%d",version);
	memcpy(buf+EEPROM_WRITE_OFFSET,&version,sizeof(version));
	write_data(FW_VERSION_ADDR, (u8 *)buf, ADC_VERSION_SIZE);
	i2c_close_exit();
}

int im_init_e2prom_data()
{
	uint8_t  modelname[MODEL_NAME_SIZE+1]={0x0};
	uint8_t  manufacture[MANUFACTURE_SIZE+1]={0x0};
	uint8_t  hw_version[ADC_VERSION_SIZE+1]={0x0};
	uint8_t  fpga_version[ADC_VERSION_SIZE+1]={0x0};
	uint8_t  fw_version[ADC_VERSION_SIZE+1]={0x0};
	uint8_t  threshol[ADC_THRESHOL_SIZE]={0x0};
	uint8_t  uuid[UUID_SIZE+1]={0x0};
	//uint8_t  mac[MAC_SIZE+1]={0x0};
	
	char  keys[ACCESS_KEY_SIZE+1]={0x0};
	//char  sak[SAK_SEED_SIZE+1]={0x0};
	//char  url[DOMAIN_SIZE+1]={0x0};
	float I_threshol,V_threshol;
	uint16_t igain=0;
	uint16_t vgain=0; 
	uint8_t frq=0;
	int ret = 0;

	i2c_path("/dev/i2c-0");
	
	read_data(MODEL_NAME_ADDR, (uint8_t *)modelname, MODEL_NAME_SIZE);
	imlogV("MODEL=%s",modelname);	
	
	read_data(MANUFACTURE_ADDR, (uint8_t *)manufacture, MANUFACTURE_SIZE);
	imlogV("MANUFATURE=%s",manufacture);	
	
	read_data(HW_VERSION_ADDR, (uint8_t *)hw_version, ADC_VERSION_SIZE);
	imlogV("HW=%s",hw_version);	
	
	read_data(FPGA_VERSION_ADDR, (uint8_t *)fpga_version, ADC_VERSION_SIZE);
	imlogV("FPGA=%s",hw_version);	
	
	read_data(FW_VERSION_ADDR, (uint8_t *)fw_version, ADC_VERSION_SIZE);
	global_setFwVersion(fw_version);
	
	/*
	if((fw_version[0]==0xff&&fw_version[1]==0xff)||
		(fw_version[0]==0&&fw_version[1]==0)||
		strcmp((char *)fw_version,"V10")==0){
		u8 buf[ADC_VERSION_SIZE+EEPROM_WRITE_OFFSET]={0x0};
		uint16_t version = 0;
		global_setFWversionDefault();
		version = global_getFWversion();
		memcpy(buf+EEPROM_WRITE_OFFSET,&version,sizeof(version));
		write_data(FW_VERSION_ADDR, (u8 *)buf, ADC_VERSION_SIZE);
	}else{
		global_setFwVersion(fw_version);
	}
	*/
	
	read_data(VGAIN_ADDR, (uint8_t *)&vgain, VGAIN_SIZE);
	imlogV("vgain=%x",vgain);
	global_setVgain(htons(vgain));
	read_data(IGAIN_ADDR, (uint8_t *)&igain, IGAIN_SIZE);
	imlogV("igain=%x",igain);
	global_setIgain(htons(igain));
	read_data(ADC_FREQUENCY_ADDR, (uint8_t *)&frq,ADC_FREQUENCY_SIZE);
	
	read_data(ADC_V_THRESHOL, (uint8_t *)threshol,ADC_THRESHOL_SIZE);
	V_threshol = U8StoU16(threshol) / 1000.0;
	global_setVthreshol(V_threshol);
	imlogV("V_threshol=%f",V_threshol);

	read_data(ADC_I_THRESHOL, (uint8_t *)threshol,ADC_THRESHOL_SIZE);
	I_threshol = U8StoU16(threshol) / 1000.0;
	global_setIthreshol(I_threshol);
	imlogV("I_threshol=%f",I_threshol);
	
	/*
	read_data(SAK_SEED_ADDR,(uint8_t *)sak,SAK_SEED_SIZE);
	imlogV("sak=%s",sak);
	if(sak[0]!=0xff){
		global_setSAK((char *)sak);
	}
	
	read_data(DOMAIN_ADDR,(uint8_t *)url,DOMAIN_SIZE);
	imlogV("url=%s",url);
	if(url[0]!=0xff&&strlen(url)>3){
		global_setdomain((char *)url);
	}else{
		global_setdomain(GLOBAL_DOMAIN_DEFAULT);
	}

	global_setUrl(ICLOUD_URL_ACTIVATE);
	global_setUrl(ICLOUD_URL_INFO);
	global_setUrl(ICLOUD_URL_DATA);
	global_setUrl(ICLOUD_URL_FW);

	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_ACTIVATE));
	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_INFO));
	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_DATA));
	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_FW));
	*/
	
	read_data(UUID_ADDR,(uint8_t *)uuid,UUID_SIZE);
	imlogV("uuid=%s",uuid);
	if(uuid[0]!=0xff&&uuid[0]!=0){
		global_setUUID((char *)uuid);
		global_setMac((char *)uuid);
		if(uuid[0]=='M'){
			imlogV("LTE MODE.");
			global_setWifiMode(0);//3g lte
		}else{
			imlogV("WIFI MODE.");
			global_setWifiMode(1);//wifi
		}
	}
	
	read_data(ACCESS_KEY_ADDR,(uint8_t *)keys,ACCESS_KEY_SIZE);
	imlogV("access_key=%s",keys);
	if(keys[0]!=0xff){
		global_setAccesskey((char *)keys);
	}
	
	i2c_close_exit();

	if(vgain==0||igain==0){
		global_setIgain(0x0190);
		global_setVgain(0x0582);
	}

	if(frq==0){
		global_setAdcFrq(AC_LINE_FREQUENCY_50);
	}else{
		global_setAdcFrq(AC_LINE_FREQUENCY_60);
	}
	return ret;
}


