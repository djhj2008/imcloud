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
#include "global_var.h"
#include "im_hiredis.h"

struct waveform global_waveform[FRAMES_GROUP_MAX];
threadpool thpool;

int ImCloudData( uint8_t * data,int len){
	char buffer[HTTP_RECV_BUF_MAX] = {0x0};
	char chunkstr[HTTP_CHUNK_HEAD_LEN]= {0x0};
	int ret = -1;
	int retry = HTTP_RETRY_MAX;
	char *mac = global_getMac();
	char *key = global_getAccesskey();
	char *url = global_getUrl(ICLOUD_URL_DATA);
	
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac,key);
    printf("chunkstr: %s\n",  chunkstr);

	ret = ImHttpPost(url,chunkstr,data,len,buffer);

	if(ret < 0){
		while(retry > 0){
			printf("ImHttpPost retry: %d,wait 5 sec.\n",  retry);
			sleep(5);
			ret = ImHttpPost(url,chunkstr,data,len,buffer);
			if(ret == 0){
				CloudDataHandle(buffer);
				break;
			}
			retry--;
		}
	}
	else{
		CloudDataHandle(buffer);
	}
	printf("ImCloudData end.");
	return ret;
}

int ImCloudInfo(){
	char buffer[HTTP_RECV_BUF_MAX] = {0x0};
	char chunkstr[HTTP_CHUNK_HEAD_LEN]= {0x0};
	char postdata[HTTP_NORMAL_POST_BUF_MAX] = {0x0};
	int ret = -1;
	int retry = HTTP_RETRY_MAX;
	char *mac = global_getMac();
	char *key = global_getAccesskey();
	char *url = global_getUrl(ICLOUD_URL_INFO);
	
	struct json_object *infor_object = NULL;
	infor_object = json_object_new_object();
	if (NULL == infor_object)
	{
		printf("ImCloudInfo new json object failed.\n");
		return -1;
	}
	
	struct json_object *array_object = NULL;
    array_object = json_object_new_array();
    if (NULL == array_object)
    {
        json_object_put(infor_object);//free
        printf("new json object failed.\n");
        return -1;
    }
    
    json_object_array_add(array_object, json_object_new_int(256));
    json_object_array_add(array_object, json_object_new_int(257));
    json_object_array_add(array_object, json_object_new_int(258));
    json_object_object_add(infor_object, "array", array_object);
    
	json_object_object_add(infor_object, "fw_version", json_object_new_int(4097));
	json_object_object_add(infor_object, "booted_at", json_object_new_int64(1234567890));
	json_object_object_add(infor_object, "manufacturer", json_object_new_string("xxxx"));
	json_object_object_add(infor_object, "model_number", json_object_new_int(0));
	json_object_object_add(infor_object, "hw_version", json_object_new_int(4097));
	strcpy(postdata,json_object_to_json_string(infor_object));
	
	printf("ImCloudInfo Post:%s \n",postdata);
	
	json_object_put(array_object);//free
    json_object_put(infor_object);//free
    
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac,key);
    printf("chunkstr: %s\n",  chunkstr);

	ret = ImHttpPost(url,chunkstr,(uint8_t *)postdata,strlen(postdata),buffer);

	if(ret < 0){
		while(retry > 0){
			printf("ImHttpPost retry: %d,wait 5 sec.\n",retry);
			sleep(5);
			ret = ImHttpPost(url,chunkstr,NULL,0,buffer);
			if(ret == 0){
				ret = CloudInfoHandle(buffer);
				break;
			}
			retry--;
		}
	}
	else{
		ret = CloudInfoHandle(buffer);
	}

	return ret;
}

int ImCloudAccessKey(){
	char buffer[HTTP_RECV_BUF_MAX] = {0x0};
	unsigned char Signature[HTTP_SIGNATURE_LEN]= {0x0};
	char chunkstr[HTTP_CHUNK_HEAD_LEN]= {0x0};
	int ret = -1;
	int retry = HTTP_RETRY_MAX;
	char *mac = global_getMac();
	char *url = global_getUrl(ICLOUD_URL_ACTIVATE);
	
	GenerateSignature(mac,Signature);
	
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac,Signature);
    printf("chunkstr: %s\n",  chunkstr);

	ret = ImHttpPost(url,chunkstr,NULL,0,buffer);

	if(ret < 0){
		while(retry > 0){
			printf("ImHttpPost retry: %d,wait 5 sec.\n",retry);
			sleep(5);
			ret = ImHttpPost(url,chunkstr,NULL,0,buffer);
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
	int V_channel = WAVE_V1_CHANNEL;
	int L1_channel = WAVE_L1_CHANNEL;
	int L2_channel = WAVE_L2_CHANNEL;
    struct ping_buffer_data ping_data;
    struct waveform waveform_t[FRAMES_GROUP_MAX];
	float w1_sum = 0;
	float w2_sum = 0;
    char devName[128];
	int vc = 0,ch1 = 0,ch2 = 0;
	int totals=0,n_totals=0;

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
					if(ch == ADC_V1_CHANNEL){
						waveform_t[index].data[WAVE_V1_CHANNEL*SAMPLES_FRAME+sample]=val;
					}else if(ch == ADC_V2_CHANNEL){
						waveform_t[index].data[WAVE_V2_CHANNEL*SAMPLES_FRAME+sample]=val;
					}else if(ch == ADC_L1_CHANNEL){
						waveform_t[index].data[WAVE_L1_CHANNEL*SAMPLES_FRAME+sample]=val;
					}else if(ch == ADC_L2_CHANNEL){
						waveform_t[index].data[WAVE_L2_CHANNEL*SAMPLES_FRAME+sample]=val;
					}else if(ch == ADC_L3_CHANNEL){
						waveform_t[index].data[WAVE_L3_CHANNEL*SAMPLES_FRAME+sample]=val;
					}else if(ch == ADC_L4_CHANNEL){
						waveform_t[index].data[WAVE_L4_CHANNEL*SAMPLES_FRAME+sample]=val;
					}
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
					//printf("CH[v] = %d CH[L1] = %d CH[L2] = %d\n", vc, ch1,ch2);
				}
				w1_sum += vc*ch1;
				w2_sum += vc*ch2;
			}
			waveform_t[index].w1 = w1_sum/SAMPLES_FRAME;
			waveform_t[index].w2 = w2_sum/SAMPLES_FRAME;
			
			waveform_t[index].rssi=get_wifi_info();
			
			
			totals = global_getTotals();
			n_totals = global_getNextTotals();
			printf("global_totals = %d next_totals =%d rssi = %d w1 = %f w2 = %f\n",totals, n_totals,waveform_t[index].rssi,waveform_t[index].w1,waveform_t[index].w2);
			
			//global_startNextTotals(); //change when next package
			
			index++;
			if(index == totals){
				//printf("wave_size:%d,otherform_t:%d \n",sizeof(waveform_t),sizeof(otherform_t));
				memcpy(global_waveform,waveform_t,sizeof(struct waveform)*totals);

				memset(waveform_t,0,sizeof(struct waveform)*FRAMES_GROUP_MAX);
				
				thpool_add_work(thpool, (void*)task, NULL); 
				index = 0;
				
			}else if(index > totals){
				int remaining = index-totals;
				printf("remaining:%d \n",remaining);
				memcpy(global_waveform,waveform_t,sizeof(struct waveform)*totals);
				
				memcpy(waveform_t,&waveform_t[totals],sizeof(struct waveform)*remaining);
  
				thpool_add_work(thpool, (void*)task, NULL);
				index -= totals;
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
	uint8_t flag;
	int len;
	int totals = global_getTotals();
	int icount = global_getIchannelsCount();
	int vcount = global_getVchannelsCount();
	char * key = global_getAccesskey();
	char filename[32]={0x0};
	
	get_filename(filename);
	
	global_startNextTotals();
	
	flag = global_getIchFlag();
	
	printf("task().\n");
	 
	fd = im_openfile(filename);
	if(fd > 0){
		im_savebuff(fd,(char *)&global_waveform,sizeof(struct waveform)*totals);
	}else{
		printf("Error Open TMP File.");
		return NULL;
	}
	im_close(fd);
	
	postdata = GenerateWaveform(filename,&len,icount,vcount,totals,flag);
	if(postdata!=NULL){
		printf("post len = %d \n",len);
	}else{
		printf("postdata NULL \n");
		return NULL;
	}
	
	if(strlen(key)<0){
		int ret = ImCloudAccessKey();
		if(ret < 0)
		{
			printf("Error Get Access Key more.");
			return NULL;
		}
	}

	if(ImCloudData(postdata,len)==0){
		im_delfile(filename);
	}else{
		printf("ImCloud Activate Error.\n");
		im_backfile(filename); 
	}
	im_backup_dump();
	free(postdata);

	return NULL;
}

int GetAcessKey(){
	int ret = -1;
	printf("GetAcessKey().\n");
	
	ret = ImCloudAccessKey();
	if(ret < 0){
		printf("Error Get Access Key.");
	}
	
	return ret;
}

int GetInfo(){
	int ret = -1;
	printf("GetInfo().\n");
	ret = ImCloudInfo();
	if(ret < 0){
		printf("Error GetInfo.");
	}
	
	return ret;
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
	int ret =-1;
	int i=0;
	
	size = get_file_size(CONFIG_FILE_PATH);
	
	if(size>0){
		fd = open(CONFIG_FILE_PATH,O_RDWR);
		if(fd<0){
			printf("Config open error use default.\n");
		}
		buf = (char *)malloc(size);
		
		count = read(fd,buf,size);
		
		if(count<0){
			printf("file read error.\n");
		}
		close(fd);
		
		infor_object = json_tokener_parse(buf);
		printf("Config:%s\n",buf);
		json_object_object_get_ex(infor_object, "interval",&result_object);
		totals = json_object_get_int(result_object);
		json_object_put(result_object);//free
		
		if(totals >=GLOBAL_TOTALS_MIN && totals <= GLOBAL_TOTALS_MAX){
			global_setTotals(totals);
			global_startNextTotals();
		}else{
			global_setTotals(GLOBAL_TOTALS_DEFAULT);
			global_startNextTotals();
		}
		
		json_object_object_get_ex(infor_object, "icloud_activate",&result_object);
		global_setUrl(json_object_get_string(result_object),ICLOUD_URL_ACTIVATE);

		json_object_object_get_ex(infor_object, "icloud_info",&result_object);
		global_setUrl(json_object_get_string(result_object),ICLOUD_URL_INFO);
		
		json_object_object_get_ex(infor_object, "icloud_data",&result_object);
		global_setUrl(json_object_get_string(result_object),ICLOUD_URL_DATA);
		
		
		json_object_put(infor_object);//free
		
		for(i=0;i<ICLOUD_URL_MAX;i++){
			if(strlen(global_getUrl(i))==0){
				ret = -1;
				break;
			}
			ret = 0;
		}
	}
	if(ret<0){
		global_setTotals(GLOBAL_TOTALS_DEFAULT);
		global_startNextTotals();
		global_setUrl(GLOBAL_URL_ACCESSKEY,ICLOUD_URL_ACTIVATE);
		global_setUrl(GLOBAL_URL_INFO,ICLOUD_URL_INFO);
		global_setUrl(GLOBAL_URL_DATA,ICLOUD_URL_DATA);
		ret = 0;
	}

	printf("URL = %s\n",global_getUrl(ICLOUD_URL_ACTIVATE));
	printf("URL = %s\n",global_getUrl(ICLOUD_URL_INFO));
	printf("URL = %s\n",global_getUrl(ICLOUD_URL_DATA));
	
	if(buf != NULL){
		free(buf);
	}
	return ret;
}


int main(int arg, char *arc[])
{
	int ret = 0;
	char mac[MAC_LEN+1]= {0x0};

    /* init redis */
    if (redis_init()) {
        return -1;
    }
	
	if(getLocalMac(mac)<0){
		printf("Network Error.\n");
		return ret;
	}
	
	global_setMac(mac);
	
	if(getConfig()<0){
		printf("Config error\n"); 
		return 0;
	}
	
	thpool = thpool_init(10);
	
	if(GetAcessKey()<0){
		printf("GetAcessKey\n"); 
		return 0;
	}
	
	if(GetInfo()<0){
		printf("GetInfo\n"); 
		return 0;
	}
	
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
	
	redis_free();
	thpool_wait(thpool);
	thpool_destroy(thpool);
	return ret;  
}  
