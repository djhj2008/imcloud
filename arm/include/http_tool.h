/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __HTTP_TOOL_H_
#define __HTTP_TOOL_H_

#define HTTP_RECV_BUF_MAX 4096
#define HTTP_SIGNATURE_LEN 128
#define HTTP_CHUNK_HEAD_LEN 384
#define HTTP_NORMAL_POST_BUF_MAX 1024

struct WriteThis {  
	uint8_t *data;
	int body_size;
	int bytes_remaining;
	int bytes_written;
}; 

struct WriteFile {  
	int fd;
	int body_size;
	int bytes_remaining;
	int bytes_written;
}; 

struct ReadThis {  
	char *readptr;  
	long sizeleft;  
};

struct ReadFile {  
	int fd;  
	long sizeleft;
	uint32_t check_sum; 
};

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp);
size_t copy_data(void *ptr, size_t size, size_t nmemb, void *stream);
int ImHttpPost(char *url,char* header,uint8_t * post_data,int data_len,char *rev_data);
int ImHttpPostStr(char *url,char* header,uint8_t * post_data,int data_len,char *rev_data);
int ImHttpDownLoadFile(char *url,char* header,int fd,int file_size);
#endif
