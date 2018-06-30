#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <unistd.h> //sleep include

#include <json-c/json.h>

#include "imcloud.h"
#include "thpool.h"
#include "openssl_tool.h"
#include "wifi_status.h"
#include "http_tool.h"
#include "im_file.h"
#include "im_dataform.h"

char access_key[128]={0};


int ImCloudActivate( char * data){
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

	ret = ImHttpPost(ACTIVATE_URL,chunkstr,NULL,buffer);

	if(ret < 0){
		while(retry > 0){
			printf("ImHttpPost retry: %d,wait 5 sec.\n",  retry);
			sleep(5);
			ret = ImHttpPost(ACTIVATE_URL,chunkstr,NULL,buffer);
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

void *task(void *arg)
{
	printf("task().\n");
	char *data = "test.";
	if(ImCloudActivate(data)==0){
		printf("access_key:%s\n", access_key);
	}else{
		printf( "ImCloud Activate Error.\n");
	}
	GenerateWaveform("1.dat");
	//im_backfile("1.dat");
	im_scanDir();
	return NULL;
}


int main(void)  
{  
	threadpool thpool = thpool_init(10);
	thpool_add_work(thpool, (void*)task, NULL);
	thpool_wait(thpool);
	fprintf(stderr, "exit.\n");
	return 0;  
}


