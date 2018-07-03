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

#include "im_file.h"
#include "thpool.h"
#include "wifi_status.h"

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

struct waveform{
	/* Input Waveforms */
	uint64_t time_stamp;
	int16_t data[FRAMES_GROUP][ADC_SAMPLE_CHANNEL*SAMPLES_FRAME];
};


struct wattage{
	/* Input Waveforms */
	float w1;
	float w2
};

struct otherform{
	int8_t rssi[FRAMES_GROUP];
	struct wattage wat[FRAMES_GROUP]
};

struct waveform 	global_waveform;
struct otherform	global_otherform;
threadpool thpool;

void *task(void *arg);

int openInputDev(const char* inputName)
{
    int fd = -1;
    char devname[128];
    int i;

	for(i = 0; i< 5; i++){
		sprintf(devname, "%s%s", ADC_DEV_PATH_NAME, inputName);

		fd = open(devname, O_RDONLY);
		if(fd >= 0){
			break;
		}
	}
	if(i >= 5)
		return -1;
	close(fd);

    return 0;
}

int sysInputScan(void)  
{  
    int l_ret = -1;  
    int i = 0;  
    int ch = 0;
    int sample = 0;     
    int dev_fd  = 0;
	int index = 0;
	int V_channel = 0;
	int L1_channel = 1;
	int L2_channel = 2;
    struct ping_buffer_data ping_data;
    struct waveform waveform_t;
    struct otherform otherform_t;
	float w1_sum = 0;
	float w2_sum = 0;
    char devName[128];
	int vc = 0,ch1 = 0,ch2 = 0;

    
    printf("enter sysInputScan.\n");
    
    memset(&waveform_t,0,sizeof(waveform_t));
    
	sprintf(devName, "%s%s", ADC_DEV_PATH_NAME, ADC_DEV_NAME);

    dev_fd = open(devName, O_RDONLY);  
    
    if(dev_fd <= 0)
    {  
        printf("adc7606 open devName : %s error\n", devName);  
        return l_ret;  
    }  

    if (ioctl(dev_fd, 1, 1)) {
        printf("adc7606 ioctl set enable failed!\n");  
        return -1;
    }

	

    while(1)  
    {
        l_ret = lseek(dev_fd, 0, SEEK_SET);  
        l_ret = read(dev_fd, &ping_data, sizeof(ping_data));  
          
        if(l_ret)  
        {
			if(index==0){
				waveform_t.time_stamp=ping_data.time_stamp;
				printf("start time stamp: %lld \n", ping_data.time_stamp);
			}else{
				printf("index: %d time stamp: %lld \n",index, ping_data.time_stamp);
			}
			for(i = 0; i< 2; i++)
				printf("pnumber[%d]= 0x%x\n", i, ping_data.pnumber[i]);  
			for(ch = 0; ch < ADC_SAMPLE_CHANNEL; ch++){
				int flag = ch % 2;

				for(sample = 0; sample < SAMPLES_FRAME; sample++){
					unsigned short old_val;
					short val;
					if(flag == 0){
						old_val = ping_data.sample[(sample * 3) + (ch/2)] & 0xffff;
					}
					else{
						old_val = (ping_data.sample[(sample * 3) + (ch/2)] >> 16) & 0xffff;
					}
					val = old_val;
					//printf("CH[%d] = %d\n", ch, val);
					waveform_t.data[index][ch*SAMPLES_FRAME+sample]=val;
				}
			}
			
			w1_sum = 0;
			w2_sum = 0;
			vc = 0;
			ch1 = 0;
			ch2 = 0;
			for(sample = 0; sample < SAMPLES_FRAME; sample++){
				vc = waveform_t.data[index][sample];
				ch1 = waveform_t.data[index][L1_channel*SAMPLES_FRAME+sample];
				ch2 = waveform_t.data[index][L2_channel*SAMPLES_FRAME+sample];
				//printf("CH[v] = %d CH[L1] = %d CH[L2] = %d\n", vc, ch1,ch2);
				w1_sum += vc*ch1;
				w2_sum += vc*ch2;
			}
			otherform_t.wat[index].w1 = w1_sum/SAMPLES_FRAME;
			otherform_t.wat[index].w2 = w2_sum/SAMPLES_FRAME;
			
			otherform_t.rssi[index]=get_wifi_info();
			printf("rssi = %d w1 = %f w2 = %f\n", otherform_t.rssi[index],otherform_t.wat[index].w1,otherform_t.wat[index].w2);
			index++;
			if(index == FRAMES_GROUP){
				printf("wave_size:%d,otherform_t:%d \n",sizeof(waveform_t),sizeof(otherform_t));
				memcpy(&global_waveform,&waveform_t,sizeof(waveform_t));
				memcpy(&global_otherform,&otherform_t,sizeof(otherform_t));
				
				memset(&waveform_t,0,sizeof(waveform_t));
				memset(&otherform_t,0,sizeof(otherform_t));
				
				thpool_add_work(thpool, (void*)task, NULL);
				//im_backfile("1.dat");
				index = 0;
			}	
		}
  
    }  

    if (ioctl(dev_fd, 1, 0)) {
        printf("adc7606 ioctl set disable failed!\n");  
        return -1;
    }
    close(dev_fd);  
      
    return l_ret;  
      
}  

void *task(void *arg)
{
	int fd;
	printf("task().\n");
	fd = im_openfile("1.dat");
	if(fd > 0){
		im_savebuff(fd,(char *)&global_waveform,sizeof(global_waveform));
		im_savebuff(fd,(char *)&global_otherform,sizeof(global_otherform));
	}
	im_close(fd);
	im_backfile("1.dat"); 
	im_scanDir();
	return NULL;
}

int main(int arg, char *arc[])  
{
	int ret = 0;
	
	thpool = thpool_init(10);
	
	ret = openInputDev(ADC_DEV_NAME);
	if(ret){
		printf("adc7606 open device error\n");  
		return 0;
	} else {
		printf("adc7606 input device dir: %s%s\n", ADC_DEV_PATH_NAME, ADC_DEV_NAME);  
	}
	
	if(enum_devices()!=0){
		printf("WIFI error\n");  
		return 0;
	}

	while(1)
	{  
		ret = sysInputScan(); 
		if(ret <=0)
			break;
	}
	
	thpool_wait(thpool);
	thpool_destroy(thpool);
	return ret;  
}  
