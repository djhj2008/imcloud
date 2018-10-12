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
#include <sys/time.h>

#include <json-c/json.h>

#include "im_log.h"
#include "imcloud.h"
#include "imcloud_controller.h"
#include "global_var.h"
#include "eeprom_tool.h"

/*Data interface calback command*/
char *imcloud_cmd_t[IMCLOUD_CMD_MAX]={
	IMCOULD_DATA_NONE_CMD,
	IMCOULD_DATA_FW_UPDATE_CMD,
	IMCOULD_DATA_REBOOT_CMD,
	IMCOULD_DATA_INTERVAL_CMD,
	IMCOULD_DATA_LOGLEVEL_CMD,
	IMCOULD_DATA_WAVEUPLOAD_CMD,
	IMCOULD_DATA_SSID_CMD,
		
};

/*hex string*/
uint32_t HEX2int(char *pcBCD)
{
	char acBCD[8] = {0};
	uint32_t nReval = 0;
	int nPower = 1;
	int nStrlen = strlen(pcBCD);
	
	memcpy(acBCD, pcBCD, nStrlen);
	while(nStrlen--)
	{
		if(acBCD[nStrlen] >= 'A' && acBCD[nStrlen] <= 'F')
		{
			nReval += (acBCD[nStrlen] - 55)*nPower;
		}
		else if(acBCD[nStrlen] >= 'a' && acBCD[nStrlen] <= 'f')
		{
			nReval += (acBCD[nStrlen] - 87)*nPower;
		}
		else
		{
			nReval += (acBCD[nStrlen] - '0')*nPower;
		}
		nPower *= 16;
	}
	return nReval;
}


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
	int ret = NORMAL_ERROR;
	
	infor_object = json_tokener_parse(buf);
	//imlogV("CloudAccessKeyHandle RECV:%s\n",buf);
	
	json_object_object_get_ex(infor_object, IMCLOUD_ACCESSKEY_TITLE,&status_object);
	
	if(status_object!=NULL){
		strcpy(status,json_object_get_string(status_object));
		if(strcmp(status,IMCLOUD_ACCESSKEY_STATUS_OK)==0){
			struct json_object *result_object = NULL;
			json_object_object_get_ex(infor_object, IMCLOUD_ACCESSKEY_CONTENT,&result_object);        
			strcpy(key,json_object_get_string(result_object));
			global_setAccesskey(key);
			eeprom_set_accesskey(key);
			json_object_put(result_object);//free
			ret = STATUS_OK;
		}
	}else{
		struct json_object *result_object = NULL;
		json_object_object_get_ex(infor_object, IMCLOUD_SERVER_CODE,&result_object);        
		imlogE("%s",json_object_get_string(result_object));
		json_object_object_get_ex(infor_object, IMCLOUD_SERVER_MESSAGE,&result_object);        
		imlogE("%s",json_object_get_string(result_object));
		json_object_put(result_object);//free
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
	struct timeval tv;
	int fw_version=global_getFWversion();
	
    gettimeofday(&tv,NULL);
    
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
    
	json_object_object_add(infor_object, "manufacturer", json_object_new_string("INTEL"));
	json_object_object_add(infor_object, "model_number", json_object_new_string("FM3JP"));
	json_object_object_add(infor_object, "fw_version", json_object_new_int(fw_version));
	json_object_object_add(infor_object, "booted_at", json_object_new_int(tv.tv_sec));
	json_object_object_add(infor_object, "hw_version", json_object_new_int(4096));
	//strcpy(postdata,json_object_to_json_string(infor_object));
	sprintf(postdata,"data=%s",json_object_to_json_string(infor_object));
	
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
	int ret = NORMAL_ERROR;
	
	infor_object = json_tokener_parse(buf);
	//imlogV("CloudInfoHandle RECV:%s\n",buf);
	
	json_object_object_get_ex(infor_object, IMCLOUD_INFO_TITLE,&status_object);     
	if(status_object!=NULL){
		strcpy(status,json_object_get_string(status_object));
		if(strcmp(status,IMCLOUD_INFO_RESULT)==0){
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
			ret = STATUS_OK;
		}else{
			struct json_object *result_object = NULL;
			char code[64]={0x0};
			json_object_object_get_ex(infor_object, IMCLOUD_SERVER_CODE,&result_object); 
			strcpy(code,json_object_get_string(result_object));     
			imlogE("%s",code);
			if(strcmp(code,SERVER_INVALID_KEY)==0){
				ret = INVALID_KEY;
			}
			
			json_object_object_get_ex(infor_object, IMCLOUD_SERVER_MESSAGE,&result_object);        
			imlogE("%s",json_object_get_string(result_object));
			json_object_put(result_object);//free	
		}
	}
	else{
		struct json_object *result_object = NULL;
		char code[64]={0x0};
		json_object_object_get_ex(infor_object, IMCLOUD_SERVER_CODE,&result_object); 
		if(result_object!=NULL){
			strcpy(code,json_object_get_string(result_object));     
			imlogE("%s",code);
			if(strcmp(code,SERVER_INVALID_KEY)==0){
				ret = INVALID_KEY;
			}
			
			json_object_object_get_ex(infor_object, IMCLOUD_SERVER_MESSAGE,&result_object);        
			imlogE("%s",json_object_get_string(result_object));
		}
		json_object_put(result_object);//free
		
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
int CloudDataHandle(char * buf){
	struct json_object *infor_object = NULL;
	struct json_object *request_object = NULL;
	char req[128]={0};
	int cmd = 0;
	
	infor_object = json_tokener_parse(buf);
	//imlogV("CloudDataHandle RECV:%s\n",buf);
	
	if(infor_object==NULL){
		imlogE("Error Server data.");
		return NORMAL_ERROR;
	}
	
	json_object_object_get_ex(infor_object, IMCLOUD_DATA_TITLE,&request_object);   
	if(request_object!=NULL){
		strcpy(req,json_object_get_string(request_object));
		imlogV("request:%s", req);
		cmd = CloudGetCMD(req);
		if(cmd==IMCLOUD_CMD_NONE){
			imlogV("CMD:None.");
		}else if(cmd==IMCLOUD_CMD_FW_UPDATE){
			struct json_object *result_object = NULL; 
			int version;
			char url[32]={0x0};
			char tmp[16]={0x0};
			//int checksum=0;
			int size=0;
			
			json_object_object_get_ex(infor_object, IMCLOUD_DATA_FW_VERSION_CONTENT,&result_object);
			if(result_object!=NULL){
				version = json_object_get_int(result_object);
				json_object_put(result_object);//free
				imlogV("fw version=%d",version);
			}
			
			json_object_object_get_ex(infor_object, IMCLOUD_DATA_FW_DOMAIN_CONTENT,&result_object);	
			if(result_object!=NULL){
				strcpy(url,json_object_get_string(request_object));
				json_object_put(result_object);//free
				global_setFwUrl(url);
			}
			
			json_object_object_get_ex(infor_object, IMCLOUD_DATA_FW_CHECKSUM_CONTENT,&result_object);
			if(result_object!=NULL){
				int sum=0;
				strcpy(tmp,json_object_get_string(request_object));
				json_object_put(result_object);//free
				sum = HEX2int(tmp);
				global_setFWChecksum(sum);
			}
			
			json_object_object_get_ex(infor_object, IMCLOUD_DATA_FW_SIZE_CONTENT,&result_object);
			if(result_object!=NULL){
				size = json_object_get_int(result_object);
				json_object_put(result_object);//free 
				global_setFWsize(size);
			}
			json_object_put(result_object);//free 
			imlogV("fw size=%d",size);
			
			imlogV("CMD:FW Update.\n");
			
		}else if(cmd==IMCLOUD_CMD_REBOOT){
			char *exec_argv[] = { "restart", "imcloud", 0 };
			execv("/bin/systemctl", exec_argv);
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
	}else{
		struct json_object *result_object = NULL;
		char code[64]={0x0};
		json_object_object_get_ex(infor_object, IMCLOUD_SERVER_CODE,&result_object); 
		if(result_object!=NULL){
			strcpy(code,json_object_get_string(result_object));     
			imlogE("%s",code);
			if(strcmp(code,SERVER_INVALID_KEY)==0){
				cmd = INVALID_KEY;
			}
			
			json_object_object_get_ex(infor_object, IMCLOUD_SERVER_MESSAGE,&result_object);        
			imlogE("%s",json_object_get_string(result_object));
		}
		json_object_put(result_object);//free
		
	}
	
	json_object_put(request_object);//free
	json_object_put(infor_object);//free
	
	return cmd;
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
