/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __WIFI_STATUS_H_
#define __WIFI_STATUS_H_

struct WriteThis {  
  const char *readptr;  
  long sizeleft;  
}; 

struct ReadThis {  
  char *readptr;  
  long sizeleft;  
};

static size_t read_callback(void *ptr, size_t size, size_t nmemb, void *userp);
size_t copy_data(void *ptr, size_t size, size_t nmemb, void *stream);
int ImHttpPost(char *url,char* header,char * post_data,char * rev_data);

#endif
