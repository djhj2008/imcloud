/*************************************************
Copyright (C), 2018-2019, Tech. Co., Ltd.
File name: imcloud.c
Author:doujun
Version:1.0
Date:2018-07-19
Description: Controller for Server response.
* process the data returned from the server
Others: NULL
Function List:
* CloudAccessKeyHandle
* parseICHStr
* GenerateInfoData
* CloudInfoHandle
* CloudDataHandle
* CloudGetCMD
* 
* @file imcloud_controller.h 
* 
*************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h> //sleep include

#include <json-c/json.h>

#include "im_log.h"
#include "imcloud.h"
#include "imcloud_controller.h"
#include "global_var.h"

/*Data interface calback command*/
char *imcloud_cmd_t[IMCLOUD_CMD_MAX]={
	IMCOULD_DATA_NONE_CMD,
	IMCOULD_DATA_FW_UPDATE_CMD,
	IMCOULD_DATA_REBOOT_CMD,
	IMCOULD_DATA_INTERVAL_CMD,
	IMCOULD_DATA_LOGLEVEL_CMD,
	IMCOULD_DATA_WAVEUPLOAD_CMD,
		
};

/*************************************************
Function: CloudAccessKeyHandle
Description: Parsing the data from server response
Calls: 
* global_setAccesskey 
Called By:
* ImCloudAccessKey
Table Accessed: NULL
Table Updated: NULL
Input:
* @param buf	server response json data

Output: 
* @return 0 	parse success
* @return -1 	data format error(net or server errors...)
*************************************************/
int CloudAccessKeyHandle(char * buf){
	struct json_object *infor_object = NULL;
	struct json_object *status_object = NULL; 
	char status[4]={0};
	char key[128]={0};
	int ret = -1;
	
	infor_object = json_tokener_parse(buf);
	imlogV("CloudAccessKeyHandle RECV:%s\n",buf);
	
	json_object_object_get_ex(infor_object, IMCLOUD_ACCESSKEY_TITLE,&status_object);     
	strcpy(status,json_object_get_string(status_object));
	
	if(strcmp(status,IMCLOUD_ACCESSKEY_RESULT)==0){
		struct json_object *result_object = NULL;
		json_object_object_get_ex(infor_object, IMCLOUD_ACCESSKEY_CONTENT,&result_object);        
		strcpy(key,json_object_get_string(result_object));
		global_setAccesskey(key);
		json_object_put(result_object);//free
		ret = 0;
	}
	
	json_object_put(status_object);//free
	json_object_put(infor_object);//free
	
	return ret;
}

/*************************************************
 * parse Channels string
 * i_channels:1,2,3,4
*************************************************/
void parseICHStr(char * str){
	char*token=strtok(str,",");
	int ch=0;
	while(token!=NULL){
		ch=atoi(token);
		if(ch>=I_CHANNELS_1&&ch<=I_CHANNELS_4){
			global_setIchFlag(ch);
		}
		token=strtok(NULL,",");
	}
}

/*************************************************
 * parse Channels string
 * v_channels:1
*************************************************/
void parseVCHStr(char * str){
	char*token=strtok(str,",");
	int ch=0;
	while(token!=NULL){
		ch=atoi(token);
		if(ch>=V_CHANNELS_1&&ch<=V_CHANNELS_2){
			global_setVchFlag(ch);
		}
		token=strtok(NULL,",");
	}
}

/*************************************************
Function: GenerateInfoData
Description: Generate json data for server request
Calls: NULL
Called By:
* ImCloudInfo
Table Accessed: NULL
Table Updated: NULL
Input:
* @param postdata	data for server request

Output: 
* @return 0 	Generate success
* @return -1 	json object error
*************************************************/
int GenerateInfoData(char * postdata)
{
	struct json_object *infor_object = NULL;
	infor_object = json_object_new_object();
	if (NULL == infor_object)
	{
		imlogE("ImCloudInfo new json object failed.\n");
		return -1;
	}
	
	struct json_object *array_object = NULL;
    array_object = json_object_new_array();
    if (NULL == array_object)
    {
        json_object_put(infor_object);//free
        imlogE("new json object failed.\n");
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
	
	json_object_put(array_object);//free
    json_object_put(infor_object);//free	
	
	return 0;
}

/*************************************************
Function: CloudInfoHandle
Description: Parsing the data from server response
Calls: 
* global_resetCHFlag 
* parseICHStr
* parseVCHStr
Called By:
* ImCloudInfo
Table Accessed: NULL
Table Updated: NULL
Input:
* @param buf	server response json data

Output: 
* @return 0 	parse success
* @return -1 	data format error(net or server errors...)
*************************************************/
int CloudInfoHandle(char * buf){
	struct json_object *infor_object = NULL;
	struct json_object *status_object = NULL; 
	char status[4]={0};
	int ret = -1;
	
	infor_object = json_tokener_parse(buf);
	imlogV("CloudInfoHandle RECV:%s\n",buf);
	
	json_object_object_get_ex(infor_object, IMCLOUD_INFO_TITLE,&status_object);     
	strcpy(status,json_object_get_string(status_object));
	
	if(strcmp(status,IMCLOUD_ACCESSKEY_RESULT)==0){
		struct json_object *i_channels_object = NULL;
		struct json_object *v_channels_object = NULL;
		global_resetCHFlag();
		
		json_object_object_get_ex(infor_object, IMCLOUD_INFO_ICH_CONTENT,&i_channels_object);      
		if(i_channels_object!=NULL){
			imlogV("%s \n",json_object_get_string(i_channels_object));
			parseICHStr((char *)json_object_get_string(i_channels_object));
		}
		
		json_object_object_get_ex(infor_object, IMCLOUD_INFO_VCH_CONTENT,&v_channels_object);
		if(v_channels_object!=NULL){
			imlogV("%s \n",json_object_get_string(v_channels_object));
			parseVCHStr((char *)json_object_get_string(v_channels_object));
		}
		
		json_object_put(i_channels_object);//free
		json_object_put(v_channels_object);//free
		ret = 0;
	}
	
	json_object_put(status_object);//free
	json_object_put(infor_object);//free
	
	return ret;
}

/*************************************************
Function: CloudDataHandle
Description: Parsing the data from server response
Calls: 
* CloudGetCMD 
* global_setTotals
Called By:
* ImCloudData
Table Accessed: NULL
Table Updated: NULL
Input:
* @param buf	server response json data

Output: 
* @return 0 	parse success
* @return -1 	data format error(net or server errors...)
*************************************************/
void CloudDataHandle(char * buf){
	struct json_object *infor_object = NULL;
	struct json_object *request_object = NULL;
	char req[128]={0};
	int cmd = 0;
	
	infor_object = json_tokener_parse(buf);
	imlogV("CloudDataHandle RECV:%s\n",buf);
	
	json_object_object_get_ex(infor_object, IMCLOUD_DATA_TITLE,&request_object);        
	strcpy(req,json_object_get_string(request_object));
	imlogV("request:%s\n", req);
	cmd = CloudGetCMD(req);
	if(cmd==IMCLOUD_CMD_NONE){
		imlogV("CMD:None.\n");
	}else if(cmd==IMCLOUD_CMD_FW_UPDATE){
		imlogV("CMD:FW Update.\n");
	}else if(cmd==IMCLOUD_CMD_REBOOT){
		char *exec_argv[] = { "imcloud", "0", 0 };
		execv("/proc/self/exe", exec_argv);
		imlogV("CMD:Reboot.\n");
	}else if(cmd==IMCLOUD_CMD_INTERVAL_CHANGE){
		struct json_object *result_object = NULL; 
		int totals = 0;
		json_object_object_get_ex(infor_object, IMCLOUD_DATA_INTERVAL_CONTENT,&result_object);
		totals = json_object_get_int(result_object);
		imlogV("interval:%d\n", totals);    
		global_setTotals(totals);
		json_object_put(result_object);//free
	}else if(cmd==IMCLOUD_CMD_LOGLEVEL_CHANGE){
		imlogV("CMD:Loglevel.\n");
	}else if(cmd==IMCLOUD_CMD_WAVEUPLOAD_CHANGE){
		imlogV("CMD:Wave Upload.\n");
	}else if(cmd==IMCLOUD_CMD_SSID_CHANGE){
		imlogV("CMD:SSID Change.\n");
	}else{
		imlogE("Error CMD:%d\n", cmd);
	}
	
	json_object_put(request_object);//free
	json_object_put(infor_object);//free
}

int CloudGetCMD(char * req){
	int ret = -1;
	int i = 0;
	for(i=0;i<IMCLOUD_CMD_MAX;i++){
		if(strcmp(req,imcloud_cmd_t[i])==0){
			ret = i;
			break;
		}
	}
	return ret;
}
