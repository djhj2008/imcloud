/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __HTTP_TOOL_H_
#define __HTTP_TOOL_H_

#define HTTP_RECV_BUF_MAX 2048
#define HTTP_SIGNATURE_LEN 128
#define HTTP_CHUNK_HEAD_LEN 384
#define HTTP_NORMAL_POST_BUF_MAX 1024

struct WriteThis {  
	uint8_t *data;
	int body_size;
	int bytes_remaining;
	int bytes_written;
}; 

struct ReadThis {  
	char *readptr;  
	long sizeleft;  
};

size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp);
size_t copy_data(void *ptr, size_t size, size_t nmemb, void *stream);
int ImHttpPost(char *url,char* header,uint8_t * post_data,int data_len,char *rev_data);

#endif
