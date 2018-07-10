/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  Cloud Data Handle.
 *               For usage, check the imcloud.h file
 *
 *//** @file imcloud_controller.h *//*
 *
 ********************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h> //sleep include

#include <json-c/json.h>

#include "imcloud.h"
#include "imcloud_controller.h"
#include "global_var.h"

char *imcloud_cmd_t[IMCLOUD_CMD_MAX]={
	IMCOULD_DATA_NONE_CMD,
	IMCOULD_DATA_FW_UPDATE_CMD,
	IMCOULD_DATA_REBOOT_CMD,
	IMCOULD_DATA_INTERVAL_CMD,
	IMCOULD_DATA_LOGLEVEL_CMD,
	IMCOULD_DATA_WAVEUPLOAD_CMD,
	IMCOULD_DATA_SSID_CMD
};

int CloudAccessKeyHandle(char * buf){
	struct json_object *infor_object = NULL;
	struct json_object *status_object = NULL; 
	char status[4]={0};
	char key[128]={0};
	int ret = -1;
	
	infor_object = json_tokener_parse(buf);
	printf("CloudAccessKeyHandle RECV:%s\n",buf);
	
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

int CloudInfoHandle(char * buf){
	struct json_object *infor_object = NULL;
	struct json_object *status_object = NULL; 
	char status[4]={0};
	int ret = -1;
	
	infor_object = json_tokener_parse(buf);
	printf("CloudInfoHandle RECV:%s\n",buf);
	
	json_object_object_get_ex(infor_object, IMCLOUD_INFO_TITLE,&status_object);     
	strcpy(status,json_object_get_string(status_object));
	
	if(strcmp(status,IMCLOUD_ACCESSKEY_RESULT)==0){
		struct json_object *i_channels_object = NULL;
		struct json_object *v_channels_object = NULL;
		global_resetCHFlag();
		
		json_object_object_get_ex(infor_object, IMCLOUD_INFO_ICH_CONTENT,&i_channels_object);      
		if(i_channels_object!=NULL){
			printf("%s \n",json_object_get_string(i_channels_object));
			parseICHStr((char *)json_object_get_string(i_channels_object));
		}
		
		json_object_object_get_ex(infor_object, IMCLOUD_INFO_VCH_CONTENT,&v_channels_object);
		if(v_channels_object!=NULL){
			printf("%s \n",json_object_get_string(v_channels_object));
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

void CloudDataHandle(char * buf){
	struct json_object *infor_object = NULL;
	struct json_object *request_object = NULL;
	char req[128]={0};
	int cmd = 0;
	
	infor_object = json_tokener_parse(buf);
	printf("CloudDataHandle RECV:%s\n",buf);
	
	json_object_object_get_ex(infor_object, IMCLOUD_DATA_TITLE,&request_object);        
	strcpy(req,json_object_get_string(request_object));
	printf("request:%s\n", req);
	cmd = CloudGetCMD(req);
	if(cmd==IMCLOUD_CMD_NONE){
		printf("CMD:None.\n");
	}else if(cmd==IMCLOUD_CMD_FW_UPDATE){
		printf("CMD:FW Update.\n");
	}else if(cmd==IMCLOUD_CMD_REBOOT){
		char *exec_argv[] = { "imcloud", "0", 0 };
		execv("/proc/self/exe", exec_argv);
		printf("CMD:Reboot.\n");
	}else if(cmd==IMCLOUD_CMD_INTERVAL_CHANGE){
		struct json_object *result_object = NULL; 
		int totals = 0;
		json_object_object_get_ex(infor_object, IMCLOUD_DATA_INTERVAL_CONTENT,&result_object);
		totals = json_object_get_int(result_object);
		printf("interval:%d\n", totals);    
		global_setTotals(totals);
		json_object_put(result_object);//free
	}else if(cmd==IMCLOUD_CMD_LOGLEVEL_CHANGE){
		printf("CMD:Loglevel.\n");
	}else if(cmd==IMCLOUD_CMD_WAVEUPLOAD_CHANGE){
		printf("CMD:Wave Upload.\n");
	}else if(cmd==IMCLOUD_CMD_SSID_CHANGE){
		printf("CMD:SSID Change.\n");
	}else{
		printf("Error CMD:%d\n", cmd);
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
