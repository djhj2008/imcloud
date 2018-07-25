/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IMCLOUD_H__
#define __IMCLOUD_H__

#define ACCESS_KEY_SIZE 64

#define MAC_LEN 12
#define HTTP_RETRY_NONE 0
#define HTTP_RETRY_MAX 3

#define  ADC_DEV_NAME    "adc7606_regs"  
#define  ADC_DEV_PATH_NAME    "/dev/"  

#define ADC_SAMPLE_SIZE 192

#define ADC_TMP_FILE_NAME "tmp.dat"
#define CONFIG_FILE_PATH "./config/imcloud.json"

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
	ICLOUD_URL_MAX,
};

/* =================================== API ======================================= */
int ImCloudData(uint8_t * data,int len,int try);
int ImCloudInfo();
int ImCloudAccessKey();
int openInputDev(const char* inputName);
int sysInputScan(void);
void *task(void *arg);
int GetAcessKey();
int getConfig();
#endif





