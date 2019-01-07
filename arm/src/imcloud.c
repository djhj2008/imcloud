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
#include <math.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <errno.h>
#include <dirent.h>
#include <sys/time.h>

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
#include "data_config.h"

/*Global wave cache data for uploading info @im_dataform.h */
struct waveform global_waveform[FRAMES_GROUP_MAX];
/*Global Thread pool info @thpool.h */
threadpool thpool;
int imcloud_status = IMCOULD_ACTIVATE;
int adc_status = ADC_IDLE;
int key_status = KEY_INVALID;
int resend_status = RESEND_IDLE;

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
	int ret = HTTP_ERROR;
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

	ret = ImHttpPost(data_url,chunkstr,data,len,buffer);
	if(ret < 0){
		imlogE("CloudDataHandle Error:%s\n",buffer);
		while(retry > 0){
			imlogE("ImHttpPost retry: %d,wait 5 sec.\n",  retry);
			sleep(5);
			ret = ImHttpPost(data_url,chunkstr,data,len,buffer);
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
		//thpool_add_work(thpool, (void*)testDownloadfw, NULL);
	}else if(cmd < 0){
		ret = cmd;
	}
	
	imlogV("ImCloudData end %d.\n",ret);
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
	int ret = HTTP_ERROR;
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
	int ret = HTTP_ERROR;
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
* @return NULL
*************************************************/

int16_t htomI(int16_t i)
{
	/*
	uint16_t stam= 0x8000&i;
	uint16_t value = i&0x7fff;
	int16_t ret = 0;
	imlogV("stam=0x%x",stam);
	imlogV("value=0x%x",value);
	imlogV("i=0x%x",i);
	ret = stam|((value>>2)&0x3fff);
	imlogV("ret=0x%d",ret);
	*/
	int16_t ret=i/4;
	return ret;
} 


void *sysInputScan(void *arg)
{  
    int l_ret = -1;  
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
	float i_sum[ADC_L_CHANNELS] = {0x0};
	float v_sum=0;
	float ch_adc=0;
    char devName[128];
	float vc=0;
	int totals=0;
	uint16_t vgain = global_getVgain();
	uint16_t igain = global_getIgain();
	float vgain_f = short2float(vgain);
	float igain_f = short2float(igain);
	float V_threshol = global_getVthreshol();
	float I_threshol = global_getIthreshol();

	if(adc_status!=ADC_START){
		imlogE("sysInputScan adc_status=%d \n",adc_status);
		return NULL;
	}

	adc_status = ADC_RUNNING;
	
    imlogV("enter sysInputScan.\n");
    
    imlogV("vgain=%f,igain=%f\n",vgain_f,igain_f);
    
    memset(waveform_t,0,sizeof(struct waveform)*FRAMES_GROUP_MAX);
    
	sprintf(devName, "%s%s", ADC_DEV_PATH_NAME, ADC_DEV_NAME);

    dev_fd = open(devName, O_RDONLY);  
    
    if(dev_fd <= 0)
    {  
		adc_status = ADC_IDLE;
        imlogE("adc7606 open devName : %s error\n", devName);  
        return NULL;  
    }  

    if (ioctl(dev_fd, 1, 1)) {
		adc_status = ADC_IDLE;
        imlogE("adc7606 ioctl set enable failed!\n");  
        return NULL;
    }

    while(imcloud_status==IMCOULD_DATA)  
    {
        l_ret = lseek(dev_fd, 0, SEEK_SET);  
        l_ret = read(dev_fd, &ping_data, sizeof(ping_data));  
          
        if(l_ret)  
        {
			adc_status = ADC_REV_DATA;
			
			waveform_t[index].time_stamp=(uint32_t)ping_data.time_stamp;
			if(index==0){
				imlogV("start time stamp: %d \n", waveform_t[index].time_stamp);
			}else{
				imlogV("index: %d time stamp: %d \n",index, waveform_t[index].time_stamp);
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
					val = htomI((int16_t)old_val);
					
					if(ch == ADC_V1_CHANNEL){
						waveform_t[index].data[WAVE_V1_CHANNEL*SAMPLES_FRAME+sample]=val;
					}else if(ch == ADC_V2_CHANNEL){
						waveform_t[index].data[WAVE_V2_CHANNEL*SAMPLES_FRAME+sample]=val;
					}else if(ch == ADC_L1_CHANNEL){
						uint16_t itemp = val;
						waveform_t[index].data[WAVE_L1_CHANNEL*SAMPLES_FRAME+sample]=itemp;
						//imlogV("CH[%d]=%d",ch,val);
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
			memset(i_sum,0,sizeof(i_sum));
			for(sample = 0; sample < SAMPLES_FRAME; sample++){
				vc = waveform_t[index].data[WAVE_V1_CHANNEL*SAMPLES_FRAME+sample]*vgain_f;
				//imlogV("vc[%d]:%f",sample,vc);
				for(ch=WAVE_L1_CHANNEL;ch<=WAVE_L4_CHANNEL;ch++){
					ch_adc = waveform_t[index].data[ch*SAMPLES_FRAME+sample]*igain_f;
					//if(ch==0)
					//imlogV("I[%d][%d]:%f",ch,sample,ch_adc);
					w_sum[ch] += vc*ch_adc;
					i_sum[ch] += fabs(ch_adc);
				}
				v_sum+=fabs(vc);
			}
			

			for(ch=WAVE_L1_CHANNEL;ch<=WAVE_L4_CHANNEL;ch++){
				//imlogV("ia[%d] =%f ",ch,i_sum[ch]/SAMPLES_FRAME);
				if((v_sum/SAMPLES_FRAME>=V_threshol)
						&&(i_sum[ch]/SAMPLES_FRAME>=I_threshol)
						&&(w_sum[ch]/SAMPLES_FRAME > LED_ADC_DIRECTION_POWER)
				){
					led_ctrl_ADC7606_ct_direction(ch,ADC7606_LED_BLUE);
				}else{
					led_ctrl_ADC7606_ct_direction(ch,ADC7606_LED_RED);
				}
			}
			
			waveform_t[index].w1 = w_sum[WAVE_L1_CHANNEL]/SAMPLES_FRAME;
			waveform_t[index].w2 = w_sum[WAVE_L2_CHANNEL]/SAMPLES_FRAME;
			waveform_t[index].w3 = w_sum[WAVE_L3_CHANNEL]/SAMPLES_FRAME;
			waveform_t[index].w4 = w_sum[WAVE_L4_CHANNEL]/SAMPLES_FRAME;
			
			waveform_t[index].rssi=get_wifi_info();
			
			totals = global_getTotals();
			//n_totals = global_getNextTotals();
			//imlogV("global_totals = %d next_totals =%d rssi = %d v=%f w1 = %f w2 = %f w3 = %f w4 = %f\n",totals, n_totals,waveform_t[index].rssi,v_sum/SAMPLES_FRAME,waveform_t[index].w1,waveform_t[index].w2,waveform_t[index].w3,waveform_t[index].w4);
			
			//global_startNextTotals(); //change when next package
			
			index++;
			if(index == totals){
				int th_num = thpool_num_threads_working(thpool);
				memcpy(global_waveform,waveform_t,sizeof(struct waveform)*totals);
				memset(waveform_t,0,sizeof(struct waveform)*FRAMES_GROUP_MAX);
				if(th_num<IM_SEND_THREAD_MAX){
					thpool_add_work(thpool, (void*)senddata, NULL); 
				}else{
					//error service restart.
					char *exec_argv[] = { "restart", "imcloud", 0 };
					execv("/bin/systemctl", exec_argv);
					imlogV("CMD:Reboot.\n");
				}
				index = 0;

			}else if(index > totals){
				int remaining = index-totals;
				int th_num = thpool_num_threads_working(thpool);
				imlogV("remaining:%d \n",remaining);
				memcpy(global_waveform,waveform_t,sizeof(struct waveform)*totals);
				memcpy(waveform_t,&waveform_t[totals],sizeof(struct waveform)*remaining);
				if(th_num<IM_SEND_THREAD_MAX){
					thpool_add_work(thpool, (void*)senddata, NULL); 
				}else{
					//error service restart.
					char *exec_argv[] = { "restart", "imcloud", 0 };
					execv("/bin/systemctl", exec_argv);
					imlogV("CMD:Reboot.\n");
				}
				index -= totals;
			}
		}
  
    }  
	adc_status = ADC_IDLE;
	
    if (ioctl(dev_fd, 1, 0)) {
        imlogE("adc7606 ioctl set disable failed!\n");  
        return NULL;
    }
    close(dev_fd); 
      
	imlogV("exit sysInputScan.\n");      
    return NULL;  
      
}  


/*************************************************
Function: senddata
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
void *senddata(void *arg)
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
	uint32_t first_time=0;
	int ret;
	
	imlogV("senddata task().\n");
	
	get_filename(filename);
	
	global_startNextTotals();
	
	flag = global_getIchFlag();
	 
	fd = im_openfile(filename);
	if(fd > 0){
		im_savebuff(fd,(char *)&global_waveform,sizeof(struct waveform)*totals);
	}else{
		imlogE("Error Open TMP File.");
		return NULL;
	}
	im_close(fd);

	sprintf(filepath,"%s/%s",DEFAULT_DIRPATH,filename);
	len = 0;

	postdata = GenerateWaveform(filepath,&len,&first_time,icount,vcount,totals,flag);
	if(postdata!=NULL){
		im_delfile(filepath);
		imlogV("post len = %d \n",len);
	}else{
		imlogE("postdata NULL \n");
		return NULL;
	}
	
	if(key_status == KEY_INVALID){
		im_save_postdata(postdata,len);
		free(postdata);
		imlogE("senddata KEY_INVALID.\n");
		return NULL;
	}
	
	ret = ImCloudData(postdata,first_time,len,HTTP_RETRY_NONE);
	if(ret!=STATUS_OK){
		if(ret == INVALID_KEY){
			key_status = KEY_INVALID;
			imcloud_status = IMCOULD_ACTIVATE;
		}
		imlogE("ImCloud Data Error.\n");
		im_save_postdata(postdata,len);
		im_redis_backup_dump();
		free(postdata);
		return NULL;
		//im_backfile(filepath); 
	}

	im_redis_backup_dump();
	free(postdata);

	if(resenddata() == INVALID_KEY){
		imlogE("resenddata KEY_INVALID.\n");
		return NULL;
	}

	return NULL;
}

int resenddata()
{
	int backup_len = 0;
	int i=0;
	uint8_t *postdata = NULL;
	int len;
	uint32_t first_time=0;
	int ret = 0;
	
	if(resend_status == RESEND_RESEND){
		imlogE("resenddata busy.");
		return ret;
	}
	
	resend_status = RESEND_RESEND;
	
	backup_len = im_redis_get_backup_len();
	
	imlogV("resenddata = %d \n",backup_len);
	
	if(backup_len>0){
		for(i=0;i<backup_len;i++){
			char name[CONFIG_FILENAME_LEN]={0x0};
			char file[CONFIG_FILEPATH_LEN]={0x0};
			
			len = 0;
			if(im_redis_get_list_head(name)<0){
				imlogE("im_redis_get_list_head Error.");
				break;
			}
			sprintf(file,"%s/%s.bak",SAVE_DIRPATH,name);
			imlogV("resenddata backup file:%s \n",file);
			postdata = GenerateBackupWaveform(file,&len,&first_time);
			if(postdata!=NULL){
				imlogV("resenddata post len = %d \n",len);
				imlogV("resenddata post first_time = %d \n",first_time);
				imlogV("resenddata postdata = %x \n",postdata[0]);
				ret = ImCloudData(postdata,first_time,len,HTTP_RETRY_NONE);
				if(ret==STATUS_OK){
					im_delfile(file);
					im_redis_pop_head(name);
					imlogE("ImCloud RESend Backup OK.\n");
				}else{
					if(ret == INVALID_KEY){
						key_status = KEY_INVALID;
						imcloud_status = IMCOULD_ACTIVATE;
						free(postdata);
						break;
					}else{
						free(postdata);
						break;
					}
					imlogE("ImCloud RESend Backup Error.\n");
				}
				free(postdata);
			}else{
				im_delfile(file);
				im_redis_pop_head(name);
				imlogE("ImCloud backup File Error.\n");
			}
		}
	}
	
	resend_status = RESEND_IDLE;
	
	return ret;
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
	if(ret<0){
		imlogE("Error Get Access Key");
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
	if(ret<0){
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
	int fd;
	struct json_object *result_object = NULL;
	struct json_object *infor_object = NULL; 
	char buf[1024];

	fd = open(CONFIG_FILE_PATH,O_RDWR);
	if(fd<0){
		imlogE("Config file open error.");
	}
	read(fd,buf,1024);
	close(fd);

	infor_object = json_tokener_parse(buf);
	
	if(infor_object!=NULL){
		json_object_object_get_ex(infor_object, "debug",&result_object);
		if(result_object!=NULL){
			setDebugOnOff(json_object_get_int(result_object));
		}

		json_object_object_get_ex(infor_object, "interval",&result_object);
		if(result_object!=NULL){
			global_setTotals(json_object_get_int(result_object));
		}else{
			global_setTotals(GLOBAL_TOTALS_DEFAULT);
		}  
			
		json_object_object_get_ex(infor_object, "domain",&result_object);
		if(result_object!=NULL){
			global_setdomain((char *)json_object_get_string(result_object));
		}else{
			global_setdomain(GLOBAL_DOMAIN_DEFAULT);
		}
	}else{
		global_setTotals(GLOBAL_TOTALS_DEFAULT);
		global_setdomain(GLOBAL_DOMAIN_DEFAULT);
	}
	json_object_put(result_object);//free
	json_object_put(infor_object);//free
	
	//global_setTotals(GLOBAL_TOTALS_DEFAULT);
	global_startNextTotals();
	global_setUrl(ICLOUD_URL_ACTIVATE);
	global_setUrl(ICLOUD_URL_INFO);
	global_setUrl(ICLOUD_URL_DATA);
	global_setUrl(ICLOUD_URL_FW);

	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_ACTIVATE));
	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_INFO));
	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_DATA));
	imlogV("URL = %s\n",global_getUrl(ICLOUD_URL_FW));
	
	return 0;
}

void *testDownloadfw(void *arg)
{
	int version = global_getFWversion();
	char chunkstr[HTTP_CHUNK_HEAD_LEN]= {0x0};
	char *mac = global_getMac();
	char *key = global_getAccesskey();
	int sum = global_getFWChecksum();
	//char *url = "https://iot.xunrun.com.cn/app/Elook2.apk";
	char *url = global_getUrl(ICLOUD_URL_FW);
	struct stat file_stat;
    int ret;
	char dirpath[MAX_DIRPATH_LEN]={0x0};
	char src_path[MAX_DIRPATH_LEN]={0x0};
	int fd;
	int size=0,file_size=global_getFWsize();
	int retry = HTTP_RETRY_NONE;
    uint32_t img_crc;  
    int download_flag = 0;

	sprintf(chunkstr, "Authorization:imAuth %s:%s",  mac,key);
    imlogV("chunkstr: %s\n",  chunkstr);
	
    //下面语句是建立默认文件夹的路径
    strcpy(dirpath, FW_DIRPATH);//默认的路径为data
   
    ret = stat(dirpath, &file_stat);//检查文件夹状态
    if(ret<0)
    {
        if(errno == ENOENT)//是否已经存在该文件夹
        {
            ret = mkdir(dirpath, 0775);//创建文件夹
            imlogV("creat dir %s \n", dirpath);
            if(ret < 0)
            {
                imlogE("Could not create directory %s \n",
					dirpath);
				return NULL;
            }
 
        }
        else
        {
            imlogE("bad file path\n");
            return NULL;
        }
    }
    sprintf(src_path,"%s/fw_%d.tar",dirpath,version);
    
    im_delfile(src_path);
    
    while(retry>0){
		fd = open(src_path,O_RDWR|O_CREAT,0744);
		if(fd>0){
			ret = ImHttpDownLoadFile(url,chunkstr,fd,file_size);
		}
		close(fd);
		
		if(ret ==0 ){
			imcloud_status = IMCOULD_FW;	
		}else{
		   	//im_delfile(src_path);
		   	break;
		}
		size = get_file_size(src_path);
		
		init_crc_table();  
		  
		ret = calc_img_crc(src_path, &img_crc);  
		if (ret < 0) {  
			imlogE("CRC Error.");
		}  
		imlogV("down load size=%d,crc=%x",size,img_crc);
		imlogV("down load file_size=%d,crc=%x",file_size,sum);
		if(size == file_size||img_crc==sum){
			download_flag = 1;
			retry=0;
			break;
		}
		break;
		retry--;
	}
	if(download_flag == 1){
		char *exec_argv[] = { "restart", "fw_update", 0 };
		execv("/bin/systemctl", exec_argv);
		imlogV("CMD:Reboot.\n");
	}
	
	imlogV("testdownloadfw end.");
	return NULL; 
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
	char *access_key=global_getAccesskey();
	uint16_t version = global_getFWversionDefault();
	uint32_t date = FW_BUILD_DATE;
	
	imlogV("MAIN:VERAION:%d",global_getFWversion());
	imlogV("BUILD DATE:%d",date);
	eeprom_set_fw_version(version);
	thpool = thpool_init(IM_SEND_THREAD_MAX);
		
	led_init();
	
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
	
	if(enum_devices()!=0){
		imlogE("WIFI error\n");  
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
	
	adc_calibration();
	
	ret = openInputDev(ADC_DEV_NAME);
	if(ret){
		imlogE("adc7606 open device error\n");  
		goto Finish;
	} else {
		imlogV("adc7606 input device dir: %s%s\n", ADC_DEV_PATH_NAME, ADC_DEV_NAME);  
	}
	
	imlogE("access_key:%x",access_key[0]);
	if(access_key[0]!=0){
		key_status = KEY_STATUS_OK;
		imcloud_status = IMCOULD_INFO;
	}else{
		imcloud_status = IMCOULD_ACTIVATE;
	}
	adc_status = ADC_IDLE;

	while(1)
	{
		if(imcloud_status==IMCOULD_ACTIVATE){
			if(GetAcessKey()<STATUS_OK){
				imlogE("GetAcessKey\n"); 
				//sleep(delay);
			}else{
				key_status = KEY_STATUS_OK;
				imcloud_status = IMCOULD_INFO;
			}
		}else if(imcloud_status==IMCOULD_INFO){
			int ret = GetCHInfo();
			if(ret == STATUS_OK){
				imcloud_status = IMCOULD_DATA;
				//sleep(delay);
			}else if(ret == INVALID_KEY){
				imcloud_status = IMCOULD_ACTIVATE;
				//sleep(delay);
			}else{
				imlogE("GetCHInfo\n");
				//sleep(delay);
			}
		}else if(imcloud_status==IMCOULD_DATA){
			if(adc_status==ADC_IDLE){
				adc_status = ADC_START;
				sysInputScan(NULL);
				//thpool_add_work(thpool, (void*)sysInputScan, NULL);
			}
		}
	}

Finish:	
	redis_free();
	thpool_wait(thpool);
	thpool_destroy(thpool);
	return ret;  
}  
