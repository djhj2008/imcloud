/*************************************************
Copyright (C), 2018-2019, Tech. Co., Ltd.
File name: imcloud.c
Author:doujun
Version:1.0
Date:2018-07-19
Description: Main Fuction.
* Get data from adc7606.
* Packed data (Use PLE encode)
* Uploading with HTTP  protocol
* Backup data for uploading failed and Re-Upload.
Others: NULL
Function List:
* 1.ImCloudData
* 2.ImCloudInfo
* 3.ImCloudAccessKey
* 4.openInputDev
* 5.sysInputScan
* 6.task
* 7.GetAcessKey
* 8.getConfig
* 9.main
* 
* @file imcloud.h 
* 
*************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h>
#include <linux/input.h>  
#include <fcntl.h>  
#include <linux/rtc.h>
#include <time.h>
#include <getopt.h>

#include <json-c/json.h>

#include "im_log.h"
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
#include "eeprom_tool.h"
#include "led_tool.h"

/*Global wave cache data for uploading info @im_dataform.h */
struct waveform global_waveform[FRAMES_GROUP_MAX];
/*Global Thread pool info @thpool.h */
threadpool thpool;


/*************************************************
Function: ImCloudData
Description: Uploading data (to HTTP Server)
Calls: 
* global_getMac 
* global_getAccesskey
* global_getUrl
* ImHttpPost
* CloudDataHandle
Called By:
* task
Table Accessed: NULL
Table Updated: NULL
Input:
* @param data	uploading data
* @param first_time URL start time
* @param len	uploading data length
* @param try	Http Error retry times (HTTP_RETRY_NONE,HTTP_RETRY_MAX...)	@imcloud.h

Output: 
* @return 0 	uploading success
* @return -1 	uploading failed(net or server errors...)
*************************************************/
int ImCloudData(uint8_t * data,int first_time,int len,int try)
{
	char buffer[HTTP_RECV_BUF_MAX] = {0x0};
	char chunkstr[HTTP_CHUNK_HEAD_LEN]= {0x0};
	int ret = -1;
	int retry = try;
	char *mac = global_getMac();
	char *key = global_getAccesskey();
	char *url = global_getUrl(ICLOUD_URL_DATA);
	int cmd = IMCLOUD_CMD_NONE;
	char data_url[256]={0x0};
	char str_time[16]={0x0};
	
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac,key);
    imlogV("chunkstr: %s\n",  chunkstr);
    sprintf(str_time,"%d",first_time);
    strcpy(data_url,url);
    strcat(data_url,str_time);
	imlogV("Data URL: %s\n",  data_url);

	//ret = ImHttpPostStream(data_url,chunkstr,first_time,len,buffer);
	ret = ImHttpPost(data_url,chunkstr,data,len,buffer);
	if(ret < 0){
		imlogE("CloudDataHandle Error:%s\n",buffer);
		while(retry > 0){
			imlogE("ImHttpPost retry: %d,wait 5 sec.\n",  retry);
			sleep(5);
			ret = ImHttpPostStream(data_url,chunkstr,first_time,len,buffer);
			if(ret == 0){
				cmd = CloudDataHandle(buffer);
				break;
			}
			retry--;
		}
	}
	else{
		cmd = CloudDataHandle(buffer);
	}
	
	if(cmd == IMCLOUD_CMD_FW_UPDATE){
		//TODO down firmware
	}else if(cmd == -1){
		ret = -1;
	}
	
	imlogV("ImCloudData end.\n");
	return ret;
}

/*************************************************
Function: ImCloudInfo
Description: Get WaveForm Info i_channels v_channels
Calls: 
* global_getMac 
* global_getAccesskey
* global_getUrl
* GenerateInfoData
* ImHttpPost
* CloudInfoHandle
Called By:
* GetCHInfo @main 
Table Accessed: NULL
Table Updated: NULL
Input:
* @param NULL
Output: 
* @return 0 	get channels success
* @return -1 	get channels failed(net or server errors...)
*************************************************/
int ImCloudInfo()
{
	char buffer[HTTP_RECV_BUF_MAX] = {0x0};
	char chunkstr[HTTP_CHUNK_HEAD_LEN]= {0x0};
	char postdata[HTTP_NORMAL_POST_BUF_MAX] = {0x0};
	int ret = -1;
	int retry = HTTP_RETRY_MAX;
	char *mac = global_getMac();
	char *key = global_getAccesskey();
	char *url = global_getUrl(ICLOUD_URL_INFO);
	

    ret = GenerateInfoData(postdata);
    if(ret<0){
		imlogE("GenerateInfoData Error.\n");
	}else{
		imlogV("ImCloudInfo Post:%s \n",postdata);
	}
    
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac,key);
    imlogV("chunkstr: %s\n",  chunkstr);

	ret = ImHttpPost(url,chunkstr,(uint8_t *)postdata,strlen(postdata),buffer);

	if(ret < 0){
		while(retry > 0){
			imlogE("ImHttpPost retry: %d,wait 5 sec.\n",retry);
			sleep(5);
			ret = ImHttpPost(url,chunkstr,(uint8_t *)postdata,strlen(postdata),buffer);
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

/*************************************************
Function: ImCloudAccessKey
Description: Get AccessKey Info
Calls: 
* global_getMac 
* global_getAccesskey
* global_getUrl
* ImHttpPost
* CloudAccessKeyHandle
Called By:
* GetAcessKey @main 
Table Accessed: NULL
Table Updated: NULL
Input:
* @param NULL
Output: 
* @return 0 	get access_key success
* @return -1 	get access_key failed(net or server errors...)
*************************************************/
int ImCloudAccessKey()
{
	char buffer[HTTP_RECV_BUF_MAX] = {0x0};
	unsigned char Signature[HTTP_SIGNATURE_LEN]= {0x0};
	char chunkstr[HTTP_CHUNK_HEAD_LEN]= {0x0};
	int ret = -1;
	int retry = HTTP_RETRY_MAX;
	char *mac = global_getMac();
	char *url = global_getUrl(ICLOUD_URL_ACTIVATE);
	char *data="NONE";
	
	GenerateSignature(mac,Signature);
	
	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac,Signature);
    imlogV("chunkstr: %s\n",  chunkstr);

	ret = ImHttpPost(url,chunkstr,(uint8_t *)data,0,buffer);

	if(ret < 0){
		while(retry > 0){
			imlogE("ImHttpPost retry: %d,wait 5 sec.\n",retry);
			sleep(5);
			ret = ImHttpPost(url,chunkstr,(uint8_t *)data,0,buffer);
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


/*************************************************
Function: openInputDev
Description: open dev adc7606
Calls: 
* NULL
Called By:
* main
Table Accessed: NULL
Table Updated: NULL
Input:
* @param NULL
Output: 
* @return 0 	open dev success
* @return -1 	device error
*************************************************/
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

/*************************************************
Function: sysInputScan
Description: Get adc data from dev adc7606
Calls: 
* global_getMac 
* global_getAccesskey
* global_getUrl
* ImHttpPost
* CloudAccessKeyHandle
Called By:
* main
Table Accessed: NULL
Table Updated: NULL
Input:
* @param NULL
Output: 
* @return 0 	get data success
* @return -1 	device error
*************************************************/
int sysInputScan(void)  
{  
    int l_ret = -1;  
    int i = 0;  
    int ch = 0;
    int sample = 0;     
    int dev_fd  = 0;
	int index = 0;
	//int V_channel = WAVE_V1_CHANNEL;
	//int L1_channel = WAVE_L1_CHANNEL;
	//int L2_channel = WAVE_L2_CHANNEL;
    struct ping_buffer_data ping_data;
    struct waveform waveform_t[FRAMES_GROUP_MAX];
	float w_sum[ADC_L_CHANNELS] = {0x0};
	float v_sum=0;
	float ch_adc=0;
    char devName[128];
	float vc=0;
	int totals=0,n_totals=0;
	uint16_t vgain = global_getVgain();
	uint16_t igain = global_getIgain();
	float vgain_f = short2float(vgain);
	float igain_f = short2float(igain);

    imlogV("enter sysInputScan.\n");
    
    imlogV("vgain=%f,igain=%f\n",vgain_f,igain_f);
    
    memset(waveform_t,0,sizeof(struct waveform)*FRAMES_GROUP_MAX);
    
	sprintf(devName, "%s%s", ADC_DEV_PATH_NAME, ADC_DEV_NAME);

    dev_fd = open(devName, O_RDONLY);  
    
    if(dev_fd <= 0)
    {  
        imlogE("adc7606 open devName : %s error\n", devName);  
        return l_ret;  
    }  

    if (ioctl(dev_fd, 1, 1)) {
        imlogE("adc7606 ioctl set enable failed!\n");  
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
				imlogV("start time stamp: %d \n", waveform_t[index].time_stamp);
			}else{
				imlogV("index: %d time stamp: %d \n",index, waveform_t[index].time_stamp);
			}
			for(i = 0; i< 2; i++){
				//imlogV("pnumber[%d]= 0x%x\n", i, ping_data.pnumber[i]);
			}
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
					//imlogV("CH[%d]=%d",ch,val);
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
			
			
			vc = 0;
			ch_adc = 0;
			v_sum = 0;
			memset(w_sum,0,sizeof(w_sum));
			for(sample = 0; sample < SAMPLES_FRAME; sample++){
				vc = waveform_t[index].data[WAVE_V1_CHANNEL*SAMPLES_FRAME+sample]*vgain_f;
				for(ch=WAVE_L1_CHANNEL;ch<=WAVE_L4_CHANNEL;ch++){
					ch_adc = waveform_t[index].data[ch*SAMPLES_FRAME+sample]*igain_f;
					w_sum[ch] += vc*ch_adc;
				}
				v_sum+=vc;
			}
			
			for(ch=WAVE_L1_CHANNEL;ch<=WAVE_L4_CHANNEL;ch++){
				if(w_sum[ch]/64 < LED_ADC_DIRECTION_POWER){
					led_ctrl_ADC7606_ct_direction(ch,ADC7606_LED_RED);
				}else{
					led_ctrl_ADC7606_ct_direction(ch,ADC7606_LED_BLUE);
				}
			}
			
			waveform_t[index].w1 = w_sum[WAVE_L1_CHANNEL]/SAMPLES_FRAME;
			waveform_t[index].w2 = w_sum[WAVE_L2_CHANNEL]/SAMPLES_FRAME;
			waveform_t[index].w3 = w_sum[WAVE_L3_CHANNEL]/SAMPLES_FRAME;
			waveform_t[index].w4 = w_sum[WAVE_L4_CHANNEL]/SAMPLES_FRAME;
			
			waveform_t[index].rssi=get_wifi_info();
			
			
			totals = global_getTotals();
			n_totals = global_getNextTotals();
			imlogV("global_totals = %d next_totals =%d rssi = %d v=%f w1 = %f w2 = %f w3 = %f w4 = %f\n",totals, n_totals,waveform_t[index].rssi,v_sum/SAMPLES_FRAME,waveform_t[index].w1,waveform_t[index].w2,waveform_t[index].w3,waveform_t[index].w4);
			
			//global_startNextTotals(); //change when next package
			
			index++;
			if(index == totals){
				//imlogV("wave_size:%d,otherform_t:%d \n",sizeof(waveform_t),sizeof(otherform_t));
				memcpy(global_waveform,waveform_t,sizeof(struct waveform)*totals);
				memset(waveform_t,0,sizeof(struct waveform)*FRAMES_GROUP_MAX);
				thpool_add_work(thpool, (void*)task, NULL); 
				index = 0;

			}else if(index > totals){
				int remaining = index-totals;
				imlogV("remaining:%d \n",remaining);
				memcpy(global_waveform,waveform_t,sizeof(struct waveform)*totals);
				memcpy(waveform_t,&waveform_t[totals],sizeof(struct waveform)*remaining);
				thpool_add_work(thpool, (void*)task, NULL);
				index -= totals;
			}
		}
  
    }  

    if (ioctl(dev_fd, 1, 0)) {
        imlogE("adc7606 ioctl set disable failed!\n");  
        return -1;
    }
    close(dev_fd);  
      
    return l_ret;  
      
}  


/*************************************************
Function: task
Description: Uploading data thread fuction 
Calls: 
* global_getTotals 
* global_getIchannelsCount
* global_getVchannelsCount
* im_redis_get_backup_len
* GenerateWaveform
* ImCloudData
* im_delfile
* im_redis_pop_head
Called By:
* main
Table Accessed: NULL
Table Updated: NULL
Input:
* @param NULL
Output: 
* @return NULL 	
*************************************************/
void *task(void *arg)
{
	int fd;
	uint8_t *postdata = NULL;
	uint8_t flag;
	int len;
	int totals = global_getTotals();
	int icount = global_getIchannelsCount();
	int vcount = global_getVchannelsCount();
	char filename[CONFIG_FILENAME_LEN]={0x0};
	char filepath[CONFIG_FILEPATH_LEN]={0x0};
	int backup_len = 0;
	int i=0;
	int first_time=0;
	
	get_filename(filename);
	
	global_startNextTotals();
	
	flag = global_getIchFlag();
	
	imlogV("task().\n");
	 
	fd = im_openfile(filename);
	if(fd > 0){
		im_savebuff(fd,(char *)&global_waveform,sizeof(struct waveform)*totals);
	}else{
		imlogE("Error Open TMP File.");
		return NULL;
	}
	im_close(fd);
	
	backup_len = im_redis_get_backup_len();
	
	
	if(backup_len>0){
		for(i=0;i<backup_len;i++){
			char name[CONFIG_FILENAME_LEN]={0x0};
			char file[CONFIG_FILEPATH_LEN]={0x0};
			
			len = 0;
			im_redis_get_list_head(name);
			sprintf(file,"%s/%s.bak",SAVE_DIRPATH,name);
			imlogE("backup file:%s \n",file);
			postdata = GenerateWaveform(file,&len,&first_time,icount,vcount,totals,flag);
			
			if(postdata!=NULL){
				imlogV("post len = %d \n",len);
				imlogV("post first_time = %d \n",first_time);
				if(ImCloudData(postdata,first_time,len,HTTP_RETRY_NONE)==0){
					im_delfile(file);
					im_redis_pop_head();
					imlogE("ImCloud Send Backup OK.\n");
				}else{
					imlogE("ImCloud Send Backup Error.\n");
				}
				free(postdata);
			}else{
				imlogE("postdata NULL \n");
			}
		}
	}
	
	len = 0;
	sprintf(filepath,"%s/%s",DEFAULT_DIRPATH,filename);
	postdata = GenerateWaveform(filepath,&len,&first_time,icount,vcount,totals,flag);
	if(postdata!=NULL){
		//im_delfile(filepath);
		imlogV("post len = %d \n",len);
	}else{
		imlogE("postdata NULL \n");
		return NULL;
	}
	
	//sprintf(filepath,"%s/%d.bin",DEFAULT_DIRPATH,first_time);
	if(ImCloudData(postdata,first_time,len,HTTP_RETRY_MAX)==0){
		im_delfile(filepath);
	}else{
		imlogE("ImCloud Data Error.\n");
		im_backfile(filepath); 
	}
	/*
	len = 0;
	sprintf(filepath,"%s/%s",DEFAULT_DIRPATH,filename);
	int result = GenerateWaveFile(filepath,&len,&first_time,icount,vcount,totals,flag);
	if(result==0){
		im_delfile(filepath);
		imlogV("post len = %d \n",len);
	}else{
		imlogE("postdata NULL \n");
		return NULL;
	}
	
	//im_savefile("upload.bin",(char * )postdata,len);
	sprintf(filepath,"%s/%d.bin",DEFAULT_DIRPATH,first_time);
	if(ImCloudData(postdata,first_time,len,HTTP_RETRY_MAX)==0){
		//im_delfile(filepath);
	}else{
		imlogE("ImCloud Data Error.\n");
		im_backfile(filepath); 
	}
	*/
	im_redis_backup_dump();
	free(postdata);

	return NULL;
}

/*************************************************
Function: GetAcessKey
Description: @ImCloudAccessKey
Calls: 
* ImCloudAccessKey
Called By:
* main
Table Accessed: NULL
Table Updated: NULL
Input:
* @param NULL
Output: 
* @return @ImCloudAccessKey
*************************************************/
int GetAcessKey(){
	int ret = -1;
	imlogV("GetAcessKey().\n");
	
	ret = ImCloudAccessKey();
	if(ret < 0){
		imlogE("Error Get Access Key.");
	}
	
	return ret;
}

/*************************************************
Function: GetAcessKey
Description: @ImCloudAccessKey
Calls: 
* ImCloudInfo
Called By:
* main
Table Accessed: NULL
Table Updated: NULL
Input:
* @param NULL
Output: 
* @return @ImCloudAccessKey
*************************************************/
int GetCHInfo()
{
	int ret = -1;
	imlogV("GetInfo().\n");
	ret = ImCloudInfo();
	if(ret < 0){
		imlogE("Error GetInfo.");
	}
	
	return ret;
}

/*************************************************
Function: getConfig
Description: get URL TOTALS form config file.
Calls: 
* global_setTotals
* global_startNextTotals
* global_setUrl
Called By:
* main
Table Accessed: NULL
Table Updated: NULL
Input:
* @param NULL
Output: 
* @return @ImCloudAccessKey
*************************************************/
int getConfig()
{
	int ret =-1;
	char * buf = NULL;
/*	
	int fd;
	int count;
	int size;
	struct json_object *result_object = NULL;
	struct json_object *infor_object = NULL;
	int totals,debug_on;
	int i=0;
	
	size = get_file_size(CONFIG_FILE_PATH);
	
	if(size>0){
		fd = open(CONFIG_FILE_PATH,O_RDWR);
		if(fd<0){
			imlogE("Config open error use default.\n");
		}
		buf = (char *)malloc(size);
		
		count = read(fd,buf,size);
		
		if(count<0){
			imlogE("file read error.\n");
		}
		close(fd);
		
		infor_object = json_tokener_parse(buf);
		imlogV("Config:%s\n",buf);
		json_object_object_get_ex(infor_object, "interval",&result_object);
		totals = json_object_get_int(result_object);
		json_object_put(result_object);//free
		
		json_object_object_get_ex(infor_object, "debug",&result_object);
		debug_on = json_object_get_int(result_object);
		json_object_put(result_object);//free
		setDebugOnOff(debug_on);
		
		
		if(totals >=GLOBAL_TOTALS_MIN && totals <= GLOBAL_TOTALS_MAX){
			global_setTotals(totals);
			global_startNextTotals();
		}else{
			global_setTotals(GLOBAL_TOTALS_DEFAULT);
			global_startNextTotals();
		}
		
		json_object_object_get_ex(infor_object, "imcloud_activate",&result_object);
		global_setUrl(json_object_get_string(result_object),ICLOUD_URL_ACTIVATE);

		json_object_object_get_ex(infor_object, "imcloud_info",&result_object);
		global_setUrl(json_object_get_string(result_object),ICLOUD_URL_INFO);
		
		json_object_object_get_ex(infor_object, "imcloud_data",&result_object);
		global_setUrl(json_object_get_string(result_object),ICLOUD_URL_DATA);
		
		json_object_object_get_ex(infor_object, "imcloud_fw",&result_object);
		global_setUrl(json_object_get_string(result_object),ICLOUD_URL_FW);
		
		json_object_put(infor_object);//free
		
		for(i=0;i<ICLOUD_URL_MAX;i++){
			if(strlen(global_getUrl(i))==0){
				ret = -1;
				break;
			}
			ret = 0;
		}
	}
*/
	//if(ret<0){
		global_setTotals(GLOBAL_TOTALS_DEFAULT);
		global_startNextTotals();
		global_setUrl(ICLOUD_URL_ACTIVATE);
		global_setUrl(ICLOUD_URL_INFO);
		global_setUrl(ICLOUD_URL_DATA);
		global_setUrl(ICLOUD_URL_FW);
		ret = 0;
	//}

	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_ACTIVATE));
	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_INFO));
	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_DATA));
	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_FW));
	
	if(buf != NULL){
		free(buf);
	}
	return ret;
}


/*************************************************
Function: main
Description: MAIN
Calls: 
* thpool_init
* redis_init
* getLocalMac
* global_setMac
* getConfig
* GetAcessKey
* GetCHInfo
* enum_devices
* sysInputScan
* redis_free
* thpool_wait
* thpool_destroy
Called By:
* main
Table Accessed: NULL
Table Updated: NULL
Input:
* @param NULL
Output: 
* @return <=0 Init Config Error or ADC7606 Devices Error.
*************************************************/
int main(int arg, char *arc[])
{
	int ret = 0;
	char mac[MAC_LEN+1]= {0x0};

	thpool = thpool_init(10);
	
    /* init redis */
    if (redis_init()) {
		sleep(10);
		imlogE("Redis retry...\n");
		if (redis_init()) {
			imlogE("Redis TimeOut...\n");
		}
    }
	
	if(im_init_e2prom_data()<0){
		imlogE("EEPROM Error.\n");
		goto Finish;
	}
	
	if(getLocalMac(mac)<0){
		imlogE("Network Error.\n");
		goto Finish;
	}else{
		global_setMac(mac);
	}
	
	if(getConfig()<0){
		imlogE("Config error\n"); 
		goto Finish;
	}
	
	if(GetAcessKey()<0){
		imlogE("GetAcessKey\n"); 
		goto Finish;
	}
	
	if(GetCHInfo()<0){
		imlogE("GetInfo\n"); 
		goto Finish;
	}
	
	ret = openInputDev(ADC_DEV_NAME);
	if(ret){
		imlogE("adc7606 open device error\n");  
		goto Finish;
	} else {
		imlogV("adc7606 input device dir: %s%s\n", ADC_DEV_PATH_NAME, ADC_DEV_NAME);  
	}
	
	if(enum_devices()!=0){
		imlogE("WIFI error\n");  
		goto Finish;
	}

	while(1)
	{  
		ret = sysInputScan(); 
		if(ret <=0)
			break;
	}


Finish:	
	redis_free();
	thpool_wait(thpool);
	thpool_destroy(thpool);
	return ret;  
}  
