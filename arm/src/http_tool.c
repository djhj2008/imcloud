/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  HTTP Tool
 *               For usage, check the http_tool.h file
 *
 *//** @file http_tool.h *//*
 *
 ********************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <stdint.h>
#include <unistd.h> //sleep include
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

#include <curl/curl.h>  

#include "im_log.h"
#include "http_tool.h"
#include "im_file.h"

size_t readfile_callback(void *ptr, size_t size, size_t nmemb, void *userp)  
{  
	struct WriteFile *pooh = (struct WriteFile *)userp;  
	int fd = pooh->fd;
	imlogV("bytes_remaining %d size %d \n",pooh->bytes_remaining,size*nmemb);
	
	if(size*nmemb < 1)  
		return 0;  

    if(pooh->bytes_remaining>0){
        if(pooh->bytes_remaining > size*nmemb) {
			lseek(fd,pooh->bytes_written,SEEK_SET);
			read(fd,ptr,size*nmemb);
            //memcpy(ptr, pooh->data + pooh->bytes_written, size*nmemb);
            pooh->bytes_written += size*nmemb;
            pooh->bytes_remaining = pooh->body_size -  pooh->bytes_written;
            return size*nmemb;
		}
		else{
			int w_size = pooh->bytes_remaining;
			lseek(fd,pooh->bytes_written,SEEK_SET);
			read(fd,ptr,pooh->bytes_remaining);
			//memcpy(ptr, pooh->data + pooh->bytes_written, pooh->bytes_remaining);
			pooh->bytes_remaining=0;
			close(fd);
			return w_size;
		}
	}
	return 0;
}  

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp)  
{  
	struct WriteThis *pooh = (struct WriteThis *)userp;  
	
	imlogV("bytes_remaining %d size %d \n",pooh->bytes_remaining,size*nmemb);
	
	if(size*nmemb < 1)  
		return 0;  

    if(pooh->bytes_remaining>0){
        if(pooh->bytes_remaining > size*nmemb) {
            memcpy(ptr, pooh->data + pooh->bytes_written, size*nmemb);
            pooh->bytes_written += size*nmemb;
            pooh->bytes_remaining = pooh->body_size -  pooh->bytes_written;
            return size*nmemb;
		}
		else{
			int w_size = pooh->bytes_remaining;
			memcpy(ptr, pooh->data + pooh->bytes_written, pooh->bytes_remaining);
			pooh->bytes_remaining=0;
			return w_size;
		}
	}
	return 0;
}  

size_t copy_data(void *ptr, size_t size, size_t nmemb, void *userp)
{
	struct ReadThis *rooh = (struct ReadThis *)userp;  
	int res_size;
	if(size*nmemb < 1)  
		return 0;  

	res_size = size * nmemb;
	
	imlogV("recv:%d,left:%ld\n",res_size,rooh->sizeleft);
	imlogV("recv:%s\n",(char *)ptr);
	if(rooh->sizeleft+res_size < HTTP_RECV_BUF_MAX)
	{
		memcpy(rooh->readptr + rooh->sizeleft, ptr, res_size);	
		rooh->sizeleft += res_size;
	}
	return res_size;
}

int ImHttpPostStr(char *url,char* header,uint8_t * post_data,int data_len,char *rev_data){
	CURL *curl;  
	CURLcode res;
	long retcode = 0;
	struct WriteThis pooh;
	struct ReadThis rooh;
	int ret = -1;
	struct curl_slist *chunk = NULL; 
	
	imlogV("ImHttpPost enter %s.\n",url);
	
	rooh.readptr = rev_data;
	rooh.sizeleft = 0;

	/* In windows, this will init the winsock stuff */   
	res = curl_global_init(CURL_GLOBAL_DEFAULT);  
	/* Check for errors */   
	if(res != CURLE_OK) {  
		imlogE("curl_global_init() failed: %s\n", curl_easy_strerror(res));  
	 	return ret;  
	}  
	imlogV("POST:%s",post_data);
	/* get a curl handle */   
	curl = curl_easy_init();  
	if(curl) {  
		/* First set the URL that is about to receive our POST. */   
		curl_easy_setopt(curl, CURLOPT_URL, url);  

		if(post_data!=NULL){
			pooh.data = post_data;
			pooh.body_size = data_len;
			pooh.bytes_remaining = data_len;
			pooh.bytes_written = 0;
			
			//imlogV("ImHttpPost data_len = %d .\n",data_len);
			
			/* Now specify we want to POST data */   
			curl_easy_setopt(curl, CURLOPT_POST, 1L);  

			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)post_data);

			/* we want to use our own read function */   
			//curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);  

			/* pointer to pass to our read function */   
			//curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);
		}else{
			curl_easy_setopt(curl, CURLOPT_POST, 1L);
			curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)post_data);
			pooh.body_size = 0;
		}
		/* get verbose debug output please */   
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  

		/* 
		  If you use POST to a HTTP 1.1 server, you can send data without knowing 
		  the size before starting the POST if you use chunked encoding. You 
		  enable this by adding a header like "Transfer-Encoding: chunked" with 
		  CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must 
		  specify the size in the request. 
		*/   
		/* Set the expected POST size. If you want to POST large amounts of data, 
		   consider CURLOPT_POSTFIELDSIZE_LARGE */   
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,pooh.body_size); 

		//#ifdef DISABLE_EXPECT  
		/* 
		  Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue" 
		  header.  You can disable this header with CURLOPT_HTTPHEADER as usual. 
		  NOTE: if you want chunked transfer too, you need to combine these two 
		  since you can only set one list of headers with CURLOPT_HTTPHEADER. */   

		/* A less good option would be to enforce HTTP 1.0, but that might also 
		   have other implications. */   
		if(header!=NULL)
		{  
		  //struct curl_slist *chunk = NULL;  
		  chunk = curl_slist_append(chunk, header);
		  //chunk = curl_slist_append(chunk, "Content-Type: application/binary");
		  chunk = curl_slist_append(chunk, "Expect:");
		  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		  /* use curl_slist_free_all() after the *perform() call to free this 
		     list again */
		}  
		//#endif

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, copy_data); //设置下载数据的回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&rooh);   


		curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0L);
		curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0L);
		/* Perform the request, res will get the return code */   
		res = curl_easy_perform(curl);
		
		curl_slist_free_all(chunk);
		
		/* Check for errors */   
		if(res != CURLE_OK)  
		  imlogE("curl_easy_perform() failed: %s\n",  
			  curl_easy_strerror(res));  

		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode);
		if(res == CURLE_OK){
			ret = 0;
		}else{
			imlogE("Error %ld\n",retcode);
		}
		/* always cleanup */   
		curl_easy_cleanup(curl);  
	}  
	curl_global_cleanup();
	return ret;
}

int ImHttpPost(char *url,char* header,uint8_t * post_data,int data_len,char *rev_data){
	CURL *curl;  
	CURLcode res;
	long retcode = 0;
	struct WriteThis pooh;
	struct ReadThis rooh;
	int ret = -1;
	struct curl_slist *chunk = NULL; 
	
	imlogV("ImHttpPost enter %s.\n",url);
	
	rooh.readptr = rev_data;
	rooh.sizeleft = 0;

	/* In windows, this will init the winsock stuff */   
	res = curl_global_init(CURL_GLOBAL_DEFAULT);  
	/* Check for errors */   
	if(res != CURLE_OK) {  
		imlogE("curl_global_init() failed: %s\n", curl_easy_strerror(res));  
	 	return ret;  
	}  
 
	/* get a curl handle */   
	curl = curl_easy_init();  
	if(curl) {  
		/* First set the URL that is about to receive our POST. */   
		curl_easy_setopt(curl, CURLOPT_URL, url);  

		if(post_data!=NULL){
			pooh.data = post_data;
			pooh.body_size = data_len;
			pooh.bytes_remaining = data_len;
			pooh.bytes_written = 0;
			
			//printf("ImHttpPost data_len = %d .\n",data_len);
			
			/* Now specify we want to POST data */   
			curl_easy_setopt(curl, CURLOPT_POST, 1L);  

			//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)"OK");

			/* we want to use our own read function */   
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, read_callback);  

			/* pointer to pass to our read function */   
			curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);
		}else{
			curl_easy_setopt(curl, CURLOPT_POST, 1L); 
			pooh.body_size = 0;
		}
		/* get verbose debug output please */   
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  

		/* 
		  If you use POST to a HTTP 1.1 server, you can send data without knowing 
		  the size before starting the POST if you use chunked encoding. You 
		  enable this by adding a header like "Transfer-Encoding: chunked" with 
		  CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must 
		  specify the size in the request. 
		*/   
		/* Set the expected POST size. If you want to POST large amounts of data, 
		   consider CURLOPT_POSTFIELDSIZE_LARGE */   
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,pooh.body_size); 

		//#ifdef DISABLE_EXPECT  
		/* 
		  Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue" 
		  header.  You can disable this header with CURLOPT_HTTPHEADER as usual. 
		  NOTE: if you want chunked transfer too, you need to combine these two 
		  since you can only set one list of headers with CURLOPT_HTTPHEADER. */   

		/* A less good option would be to enforce HTTP 1.0, but that might also 
		   have other implications. */   
		if(header!=NULL)
		{  
		  //struct curl_slist *chunk = NULL;  
		  chunk = curl_slist_append(chunk, header);
		  //chunk = curl_slist_append(chunk, "Content-Type: application/binary");
		  //chunk = curl_slist_append(chunk, "Content-Type: audio/aiff");
		  chunk = curl_slist_append(chunk, "Expect:");
		  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		  /* use curl_slist_free_all() after the *perform() call to free this 
		     list again */
		}  
		//#endif

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, copy_data); //设置下载数据的回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&rooh);   


		curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0L);
		curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0L);
		
		/* Perform the request, res will get the return code */   
		res = curl_easy_perform(curl);
		
		curl_slist_free_all(chunk);
		
		/* Check for errors */   
		if(res != CURLE_OK)  
		  imlogE("curl_easy_perform() failed: %s\n",  
			  curl_easy_strerror(res));  

		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode);
		if(res == CURLE_OK){
			ret = 0;
		}else{
			imlogE("Error %ld\n",retcode);
		}
		/* always cleanup */   
		curl_easy_cleanup(curl);  
	}  
	curl_global_cleanup();
	return ret;
}

int ImHttpPostStream(char *url,char* header,int first_time,int data_len,char *rev_data){
	CURL *curl;  
	CURLcode res;
	long retcode = 0;
	struct WriteFile pooh;
	struct ReadThis rooh;
	int ret = -1;
	struct curl_slist *chunk = NULL; 
	char filepath[CONFIG_FILEPATH_LEN]={0x0};
	int fd;
	
	imlogV("ImHttpPost enter %s.\n",url);
	
	rooh.readptr = rev_data;
	rooh.sizeleft = 0;

	/* In windows, this will init the winsock stuff */   
	res = curl_global_init(CURL_GLOBAL_DEFAULT);  
	/* Check for errors */   
	if(res != CURLE_OK) {  
		imlogE("curl_global_init() failed: %s\n", curl_easy_strerror(res));  
	 	return ret;  
	}  
 
	/* get a curl handle */   
	curl = curl_easy_init();  
	if(curl) {  
		/* First set the URL that is about to receive our POST. */   
		curl_easy_setopt(curl, CURLOPT_URL, url);  

		/* enable TCP keep-alive for this transfer */
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPALIVE, 1L);
		/* keep-alive idle time to 120 seconds */
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPIDLE, 120L);
		/* interval time between keep-alive probes: 60 seconds */
		curl_easy_setopt(curl, CURLOPT_TCP_KEEPINTVL, 60L);

		if(first_time>0){
			sprintf(filepath,"%s/%d.bin",DEFAULT_DIRPATH,first_time);
			fd= open(filepath,O_RDWR|O_CREAT|O_APPEND,0644);
			if(fd<0){
				imlogE("file open error.\n");
				return ret;
			}

			pooh.fd = fd;
			pooh.body_size = data_len;
			pooh.bytes_remaining = data_len;
			pooh.bytes_written = 0;
			
			//printf("ImHttpPost data_len = %d .\n",data_len);
			
			/* Now specify we want to POST data */   
			curl_easy_setopt(curl, CURLOPT_POST, 1L);  

			//curl_easy_setopt(curl, CURLOPT_TRANSFER_ENCODING, 1L);
			//curl_easy_setopt(curl, CURLOPT_POSTFIELDS, (void *)"OK");

			/* we want to use our own read function */   
			curl_easy_setopt(curl, CURLOPT_READFUNCTION, readfile_callback);  

			/* pointer to pass to our read function */   
			curl_easy_setopt(curl, CURLOPT_READDATA, &pooh);
		}else{
			curl_easy_setopt(curl, CURLOPT_POST, 1L); 
			pooh.body_size = 0;
		}
		/* get verbose debug output please */   
		curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);  

		/* 
		  If you use POST to a HTTP 1.1 server, you can send data without knowing 
		  the size before starting the POST if you use chunked encoding. You 
		  enable this by adding a header like "Transfer-Encoding: chunked" with 
		  CURLOPT_HTTPHEADER. With HTTP 1.0 or without chunked transfer, you must 
		  specify the size in the request. 
		*/   
		/* Set the expected POST size. If you want to POST large amounts of data, 
		   consider CURLOPT_POSTFIELDSIZE_LARGE */   
		//curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE,pooh.body_size); 

		//#ifdef DISABLE_EXPECT  
		/* 
		  Using POST with HTTP 1.1 implies the use of a "Expect: 100-continue" 
		  header.  You can disable this header with CURLOPT_HTTPHEADER as usual. 
		  NOTE: if you want chunked transfer too, you need to combine these two 
		  since you can only set one list of headers with CURLOPT_HTTPHEADER. */   

		/* A less good option would be to enforce HTTP 1.0, but that might also 
		   have other implications. */   
		if(header!=NULL)
		{  
		  //struct curl_slist *chunk = NULL;  
		  chunk = curl_slist_append(chunk, header);
		  //chunk = curl_slist_append(chunk, "Content-Type: application/binary");
		  chunk = curl_slist_append(chunk, "Connection: Keep-Alive");
		  chunk = curl_slist_append(chunk, "Content-Type: application/octet-stream");
		  chunk = curl_slist_append(chunk,  "Transfer-Encoding: chunked");
		  chunk = curl_slist_append(chunk, "Expect:");
		  res = curl_easy_setopt(curl, CURLOPT_HTTPHEADER, chunk);
		  /* use curl_slist_free_all() after the *perform() call to free this 
		     list again */
		}  
		//#endif

		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, copy_data); //设置下载数据的回调函数
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&rooh);   

  
		curl_easy_setopt(curl,CURLOPT_SSL_VERIFYPEER,0L);
		curl_easy_setopt(curl,CURLOPT_SSL_VERIFYHOST,0L);
		
		/* Perform the request, res will get the return code */   
		res = curl_easy_perform(curl);
		
		curl_slist_free_all(chunk);
		
		/* Check for errors */   
		if(res != CURLE_OK)  
		  imlogE("curl_easy_perform() failed: %s\n",  
			  curl_easy_strerror(res));  

		res = curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE , &retcode);
		if(res == CURLE_OK){
			ret = 0;
		}else{
			imlogE("Error %ld\n",retcode);
		}
		/* always cleanup */   
		curl_easy_cleanup(curl);  
	}  
	curl_global_cleanup();
	return ret;
}

