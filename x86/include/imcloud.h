/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IMCLOUD_H__
#define __IMCLOUD_H__

#define ACCESS_KEY_SIZE 64

#define MAC_LEN 12
#define HTTP_RETRY_MAX 3

#define  ADC_DEV_NAME    "adc7606_regs"  
#define  ADC_DEV_PATH_NAME    "/dev/"  

#define ADC_SAMPLE_SIZE 192

#define DEFAULT_DIRPATH "./data"
#define SAVE_DIRPATH "./save"
#define ADC_TMP_FILE_NAME "tmp.dat"

#define CONFIG_FILE_PATH "./config/cloud.conf"

struct ping_buffer_data{
	//struct rtc_time tm;
	unsigned long long time_stamp;
	unsigned int pnumber[2];
	unsigned int sample[ADC_SAMPLE_SIZE];
};

enum ICOULD_URL {
	ICLOUD_ACTIVATE=0,
	ICLOUD_INFO,
	ICLOUD_DATA,
};
/* =================================== API ======================================= */
void setGlobalTotals(int totals);
void setAccessKey(char *key);
void *task(void *arg);
int ImCloudData( uint8_t * data,int len);
int ImCloudAccessKey();
#endif





