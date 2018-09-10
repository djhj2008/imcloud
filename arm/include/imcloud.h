/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IMCLOUD_H__
#define __IMCLOUD_H__


#define EEPROM_WRITE_OFFSET 	2
#define ACCESS_KEY_SIZE 		64

#define MAC_LEN 16
#define HTTP_RETRY_NONE 0
#define HTTP_RETRY_MAX 3

#define  ADC_DEV_NAME    "adc7606_regs"  
#define  ADC_DEV_PATH_NAME    "/dev/"  

#define ADC_SAMPLE_SIZE 192

#define ADC_TMP_FILE_NAME "tmp.dat"
#define CONFIG_FILE_PATH "/etc/imcloud/imcloud.json"

struct ping_buffer_data{
	//struct rtc_time tm;
	unsigned long long time_stamp;
	unsigned int pnumber[2];
	unsigned int sample[ADC_SAMPLE_SIZE];
};

enum ICOULD_URL {
	ICLOUD_URL_ACTIVATE=0,
	ICLOUD_URL_INFO,
	ICLOUD_URL_DATA,
	ICLOUD_URL_FW,
	ICLOUD_URL_MAX,
};

enum IMCOULD_STATUS {
	IMCOULD_ACTIVATE=0,
	IMCOULD_INFO,
	IMCOULD_DATA,
	IMCOULD_FW,
	IMCOULD_MAX,
};

enum ADC_STATUS {
	ADC_IDLE = 0,
	ADC_START,
	ADC_RUNNING,
	ADC_REV_DATA,
};

enum KEY_STATUS {
	KEY_STATUS_OK= 0,
	KEY_INVALID,
};

/* =================================== API ======================================= */
int ImCloudData(uint8_t * data,int first_time,int len,int try);
int ImCloudInfo();
int ImCloudAccessKey();
int openInputDev(const char* inputName);
void *sysInputScan(void *arg);
void *senddata(void *arg);
int resenddata();
int GetAcessKey();
int getConfig();
#endif





