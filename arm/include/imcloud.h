/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IMCLOUD_H__
#define __IMCLOUD_H__

#define ACTIVATE_URL "http://iot.xunrun.com.cn/base/index.php/Home/time/gettime"

#define MAC_LEN 12
#define HTTP_RETRY_MAX 3

#define  ADC_DEV_NAME    "adc7606_regs"  
#define  ADC_DEV_PATH_NAME    "/dev/"  

#define ADC_SAMPLE_SIZE 192

#define FRAMES_GROUP 300
#define SAMPLES_FRAME 64
#define ADC_SAMPLE_CHANNEL 6

#define DEFAULT_DIRPATH "./data"
#define SAVE_DIRPATH "./save"

struct ping_buffer_data{
	//struct rtc_time tm;
	unsigned long long time_stamp;
	unsigned int pnumber[2];
	unsigned int sample[ADC_SAMPLE_SIZE];
};

/* =================================== API ======================================= */
void *task(void *arg);

#endif





