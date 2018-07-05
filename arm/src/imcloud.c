#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h> //sleep include
#include <linux/input.h>  
#include <fcntl.h>  
#include <linux/rtc.h>
#include <time.h>
#include <getopt.h>

#include <json-c/json.h>

#include "imcloud.h"
#include "thpool.h"
#include "openssl_tool.h"
#include "wifi_status.h"
#include "http_tool.h"
#include "im_file.h"
#include "im_dataform.h"


char access_key[128]={0};
struct waveform 	global_waveform;
struct otherform	global_otherform;
threadpool thpool;

int ImCloudActivate( uint8_t * data,int len){
	char buffer[1024] = {0x0};
	unsigned char Signature[128]= {0x0};
	char chunkstr[384]= {0x0};
	char mac_addr[MAC_LEN+1]= {0x0};
	int ret = -1;
	int retry = HTTP_RETRY_MAX;
	struct json_object *infor_object = NULL;
	
	if(getLocalMac(mac_addr)<0){
		printf("Network Error.\n");
		return ret;
	}
	
	GenerateSignature(mac_addr,Signature);
	
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac_addr,Signature);
    printf("chunkstr: %s\n",  chunkstr);

	ret = ImHttpPost(ACTIVATE_URL,chunkstr,data,len,buffer);

	if(ret < 0){
		while(retry > 0){
			printf("ImHttpPost retry: %d,wait 5 sec.\n",  retry);
			sleep(5);
			ret = ImHttpPost(ACTIVATE_URL,chunkstr,data,len,buffer);
			if(ret == 0){
				struct json_object *result_object = NULL; 
				infor_object = json_tokener_parse(buffer);
				json_object_object_get_ex(infor_object, "status",&result_object);        
				printf("status:%s\n", json_object_to_json_string(result_object));    
				json_object_put(result_object);//free
				json_object_object_get_ex(infor_object, "access_key",&result_object);        
				printf("access_key:%s\n", json_object_to_json_string(result_object));    
				strcpy(access_key,json_object_to_json_string(result_object)); 
				json_object_put(result_object);//free
				json_object_object_get_ex(infor_object, "time",&result_object);    
				printf("time:%s\n", json_object_to_json_string(result_object));    
				json_object_put(result_object);//free
				if(strlen(access_key)>0){
					ret = 0;
					//break;
				}
				json_object_put(infor_object);//free
			}
			retry--;
		}
	}
	else{
		struct json_object *result_object = NULL; 
		infor_object = json_tokener_parse(buffer);
		printf("RECV:%s\n",buffer);
		json_object_object_get_ex(infor_object, "access_key",&result_object);        
		//printf("access_key:%s\n", json_object_to_json_string(result_object));    
		strcpy(access_key,json_object_to_json_string(result_object)); 
		json_object_put(result_object);//free
		if(strlen(access_key)>0){
			ret = 0;
		}
		json_object_put(infor_object);//free


	}
	return ret;
}

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
				vc = waveform_t.data[index][V_channel*SAMPLES_FRAME+sample];
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
				//printf("wave_size:%d,otherform_t:%d \n",sizeof(waveform_t),sizeof(otherform_t));
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
	uint8_t *postdata = NULL;
	int len;
	
	printf("task().\n");
	fd = im_openfile("1.dat");
	if(fd > 0){
		im_savebuff(fd,(char *)&global_waveform,sizeof(global_waveform));
		im_savebuff(fd,(char *)&global_otherform,sizeof(global_otherform));
	}
	im_close(fd);
	
	memset(access_key,0,128);
	GenerateWaveform("1.dat",&postdata,&len);
	if(postdata!=NULL){
		printf("post len = %d \n",len);
	}
	if(ImCloudActivate(postdata,len)==0){
		printf("access_key:%s\n", access_key);
	}else{
		printf( "ImCloud Activate Error.\n");
	}
	free(postdata);
	
	if(strlen(access_key)==0){
		im_backfile("1.dat"); 
	}else{
		im_delfile("1.dat");
	}
	im_scanDir();
	return NULL;
}

void *task2(void *arg)
{
	uint8_t *postdata = NULL;
	int len;
	
	printf("task2().\n");

	memset(access_key,0,128);
	GenerateWaveform("1.dat",&postdata,&len);
	if(postdata!=NULL){
		printf("post len = %d \n",len);
	}

	free(postdata);
	
	return NULL;
}

int main(int arg, char *arc[])
{
	int ret = 0;
	
	thpool = thpool_init(10);
	//thpool_add_work(thpool, (void*)task2, NULL);
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
