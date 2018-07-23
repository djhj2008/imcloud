/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __GLOBAL_VAR_H__
#define __GLOBAL_VAR_H__

#include "imcloud.h"

#define GLOBAL_TOTALS_MIN 5
#define GLOBAL_TOTALS_MAX 300

#define GLOBAL_TOTALS_DEFAULT 10
#define GLOBAL_URL_ACCESSKEY "https://iot.xunrun.com.cn/base/index.php/Home/time/accesskey"
#define GLOBAL_URL_INFO "https://iot.xunrun.com.cn/base/index.php/Home/time/info"
#define GLOBAL_URL_DATA "https://iot.xunrun.com.cn/base/test.php"

/* =================================== API ======================================= */
void global_setAccessKey(char * key);
int global_getNextTotals();
int global_getTotals();
void global_setTotals(int totals);
void global_resetCHFlag();
int global_getVchannelsCount();
int global_getIchannelsCount();
uint8_t global_getIchFlag();
void global_setIchFlag(int ich);
void global_setVchFlag(int vch);
int global_getChFlag(int ch);
void global_dumpCH();
void global_setUrl(const char *url,enum ICOULD_URL index);
char * global_getUrl(enum ICOULD_URL index);
char * global_getMac();
void global_setMac(char *mac);
char * global_getAccesskey();
void global_setAccesskey(char *key);
void global_startNextTotals();
#endif
