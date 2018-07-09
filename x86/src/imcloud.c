/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  Main APP.
 *               For usage, check the imcloud.h file
 *
 *//** @file imcloud.h *//*
 *
 ********************************/

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
#include "imcloud_controller.h"


char access_key[128]={0};
struct waveform global_waveform[FRAMES_GROUP_MAX];
threadpool thpool;

char mac_addr[MAC_LEN+1]= {0x0};
char cloud_url[10][256]={0};
int global_totals;
int next_totals;

void setGlobalTotals(int totals){
	if(totals<5||totals>300)
	 return;
	next_totals = totals;
}

void setAccessKey(char * key){
	strcpy(access_key,key);
}

int ImCloudData( uint8_t * data,int len){
	char buffer[1024] = {0x0};
	char chunkstr[384]= {0x0};
	int ret = -1;
	int retry = HTTP_RETRY_MAX;
	
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac_addr,access_key);
    printf("chunkstr: %s\n",  chunkstr);

	ret = ImHttpPost(cloud_url[ICLOUD_DATA],chunkstr,data,len,buffer);

	if(ret < 0){
		while(retry > 0){
			printf("ImHttpPost retry: %d,wait 5 sec.\n",  retry);
			sleep(5);
			ret = ImHttpPost(cloud_url[ICLOUD_ACTIVATE],chunkstr,data,len,buffer);
			if(ret == 0){
				CloudDataHandle(buffer);
			}
			retry--;
		}
	}
	else{
		CloudDataHandle(buffer);
	}
	return ret;
}

int ImCloudInfo(){
	char buffer[1024] = {0x0};
	char chunkstr[384]= {0x0};
	int ret = -1;
	int retry = HTTP_RETRY_MAX;
	
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac_addr,access_key);
    printf("chunkstr: %s\n",  chunkstr);

	ret = ImHttpPost(cloud_url[ICLOUD_ACTIVATE],chunkstr,NULL,0,buffer);

	if(ret < 0){
		while(retry > 0){
			printf("ImHttpPost retry: %d,wait 5 sec.\n",retry);
			sleep(5);
			ret = ImHttpPost(cloud_url[ICLOUD_DATA],chunkstr,NULL,0,buffer);
			if(ret == 0){
				
			}
			retry--;
		}
	}
	else{

	}
	return ret;
}

int ImCloudAccessKey(){
	char buffer[1024] = {0x0};
	unsigned char Signature[128]= {0x0};
	char chunkstr[384]= {0x0};
	int ret = -1;
	int retry = HTTP_RETRY_MAX;
	
	GenerateSignature(mac_addr,Signature);
	
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac_addr,Signature);
    printf("chunkstr: %s\n",  chunkstr);

	ret = ImHttpPost(cloud_url[ICLOUD_ACTIVATE],chunkstr,NULL,0,buffer);

	if(ret < 0){
		while(retry > 0){
			printf("ImHttpPost retry: %d,wait 5 sec.\n",retry);
			sleep(5);
			ret = ImHttpPost(cloud_url[ICLOUD_DATA],chunkstr,NULL,0,buffer);
			if(ret == 0){
				ret = CloudAccessKeyHandle(buffer);
				break;
			}
			retry--;
		}
	}
	else{
		ret = CloudAccessKeyHandle(buffer);
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
    struct waveform waveform_t[FRAMES_GROUP_MAX];
	float w1_sum = 0;
	float w2_sum = 0;
    char devName[128];
	int vc = 0,ch1 = 0,ch2 = 0;

    
    printf("enter sysInputScan.\n");
    
    memset(waveform_t,0,sizeof(struct waveform)*FRAMES_GROUP_MAX);
    
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
			waveform_t[index].time_stamp=(uint32_t)ping_data.time_stamp;
			if(index==0){
				printf("start time stamp: %d \n", waveform_t[index].time_stamp);
			}else{
				printf("index: %d time stamp: %d \n",index, waveform_t[index].time_stamp);
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
					waveform_t[index].data[ch*SAMPLES_FRAME+sample]=val;
				}
			}
			
			w1_sum = 0;
			w2_sum = 0;
			vc = 0;
			ch1 = 0;
			ch2 = 0;
			for(sample = 0; sample < SAMPLES_FRAME; sample++){
				vc = waveform_t[index].data[V_channel*SAMPLES_FRAME+sample];
				ch1 = waveform_t[index].data[L1_channel*SAMPLES_FRAME+sample];
				ch2 = waveform_t[index].data[L2_channel*SAMPLES_FRAME+sample];
				if(sample==0){
					printf("CH[v] = %d CH[L1] = %d CH[L2] = %d\n", vc, ch1,ch2);
				}
				w1_sum += vc*ch1;
				w2_sum += vc*ch2;
			}
			waveform_t[index].w1 = w1_sum/SAMPLES_FRAME;
			waveform_t[index].w2 = w2_sum/SAMPLES_FRAME;
			
			waveform_t[index].rssi=get_wifi_info();
			printf("global_totals = %d next_totals =%d rssi = %d w1 = %f w2 = %f\n",global_totals, next_totals,waveform_t[index].rssi,waveform_t[index].w1,waveform_t[index].w2);
			global_totals = next_totals;
			index++;
			if(index == global_totals){
				//printf("wave_size:%d,otherform_t:%d \n",sizeof(waveform_t),sizeof(otherform_t));
				memcpy(global_waveform,waveform_t,sizeof(struct waveform)*global_totals);

				memset(waveform_t,0,sizeof(struct waveform)*FRAMES_GROUP_MAX);
				
				thpool_add_work(thpool, (void*)task, NULL); 
				index = 0;
			}	
			if(index > global_totals){
				int remaining = index-global_totals;
				printf("remaining:%d \n",remaining);
				memcpy(global_waveform,waveform_t,sizeof(struct waveform)*global_totals);
				
				memcpy(waveform_t,&waveform_t[global_totals],sizeof(struct waveform)*remaining);
  
				thpool_add_work(thpool, (void*)task, NULL);
				index -= global_totals;
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
	int totals = global_totals;
	
	if(next_totals!=global_totals){
		global_totals = next_totals;
		printf("Rest Totals %d,pre Totals %d",global_totals,totals);
	}
	
	printf("task().\n");
	
	if((access(ADC_TMP_FILE_NAME,F_OK))!=-1)   
    {   
		printf("TMP File Del!\n"); 
		im_delfile(ADC_TMP_FILE_NAME); 
    }  
    
	fd = im_openfile(ADC_TMP_FILE_NAME);
	if(fd > 0){
		im_savebuff(fd,(char *)&global_waveform,sizeof(struct waveform)*totals);
	}else{
		printf("Error Open TMP File.");
		return NULL;
	}
	im_close(fd);
	
	postdata = GenerateWaveform(ADC_TMP_FILE_NAME,&len,totals);
	if(postdata!=NULL){
		printf("post len = %d \n",len);
	}else{
		printf("postdata NULL \n");
		return NULL;
	}
	
	if(strlen(access_key)<0){
		int ret = ImCloudAccessKey();
		if(ret < 0)
		{
			printf("Error Get Access Key more.");
			return NULL;
		}
	}

	if(ImCloudData(postdata,len)==0){
		printf("access_key:%s\n", access_key);
		im_delfile(ADC_TMP_FILE_NAME);
	}else{
		printf( "ImCloud Activate Error.\n");
		im_backfile(ADC_TMP_FILE_NAME); 
	}
	
	free(postdata);

	return NULL;
}

void *GetAcessKey(void *arg)
{
	int ret = -1;
	printf("GetAcessKey().\n");
	memset(access_key,0,128);
	ret = ImCloudAccessKey();
	if(ret < 0){
		printf("Error Get Access Key.");
	}
	
	return NULL;
}

int getConfig()
{
	int fd;
	int count;
	int size;
	char * buf = NULL;
	struct json_object *result_object = NULL;
	struct json_object *infor_object = NULL;
	int totals;
	int ret;
	
	size = get_file_size(CONFIG_FILE_PATH);
	
	if(size <= 0){
		printf("Config File Error.\n");
		return ret;
	}
	
	fd = open(CONFIG_FILE_PATH,O_RDWR);
	if(fd<0){
		printf("file open error.\n");
		return ret;
	}
	
	buf = (char *)malloc(size);
	
	count = read(fd,buf,size);
	if(count<0){
		printf("file read error.\n");
		return ret;
	}
	close(fd);
	
	infor_object = json_tokener_parse(buf);
	printf("Config:%s\n",buf);
	json_object_object_get_ex(infor_object, "interval",&result_object);
	totals = json_object_get_int(result_object);
	json_object_put(result_object);//free
	
	if(totals >=5 && totals <= 300){
		global_totals = totals;
		next_totals = totals;
	}else{
		global_totals = 30;
		next_totals = 30;
	}
	
	json_object_object_get_ex(infor_object, "icloud_activate",&result_object);
	strcpy(cloud_url[ICLOUD_ACTIVATE],json_object_get_string(result_object));
	json_object_put(result_object);//free
	
	json_object_object_get_ex(infor_object, "icloud_data",&result_object);
	strcpy(cloud_url[ICLOUD_DATA],json_object_get_string(result_object));
	json_object_put(result_object);//free
	
	json_object_put(infor_object);//free
	
	if(strlen(cloud_url[ICLOUD_ACTIVATE])>0 && strlen(cloud_url[ICLOUD_DATA])>0){
		ret = 0;
	}
	printf("URL = %s\n",cloud_url[ICLOUD_ACTIVATE]);
	printf("URL = %s\n",cloud_url[ICLOUD_DATA]);
	
	if(buf != NULL){
		free(buf);
	}
	return ret;
}


int main(int arg, char *arc[])
{
	int ret = 0;
	
	if(getLocalMac(mac_addr)<0){
		printf("Network Error.\n");
		return ret;
	}
	
	if(getConfig()<0){
		printf("Config error\n"); 
		return 0;
	}
	
	thpool = thpool_init(10);
	thpool_add_work(thpool, (void*)GetAcessKey, NULL);
	thpool_wait(thpool);
	
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
