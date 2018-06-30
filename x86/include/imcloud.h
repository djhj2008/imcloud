/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IMCLOUD_H__
#define __IMCLOUD_H__

#define ACTIVATE_URL "http://iot.xunrun.com.cn/base/index.php/Home/time/gettime"

#define MAC_LEN 12
#define HTTP_RETRY_MAX 3

/* =================================== API ======================================= */
int ImHttpPost(char *url,char* header,char * post_data,char * res_data);

#endif





