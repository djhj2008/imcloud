/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __GLOBAL_VAR_H__
#define __GLOBAL_VAR_H__

#include "imcloud.h"

#define FW_VERSION_MAJOR 		VERSION_MAJOR
#define FW_VERSION_MINOR 		VERSION_MINOR
#define FW_VERSION_REVISION 	VERSION_REVISION
#define FW_VERSION_HOST 		VERSION_HOST
#define FW_BUILD_DATE 			BUILD_DATE

#define GLOBAL_TOTALS_MIN 5
#define GLOBAL_TOTALS_MAX 300
#define GLOBAL_TOTALS_DEFAULT 15

#define GLOBAL_DOMAIN_DEFAULT "api-asia.informetis.com"
//#define GLOBAL_DOMAIN_DEFAULT "35.229.162.114"
//#define GLOBAL_DOMAIN_DEFAULT "35.229.161.137"
#define GLOBAL_URL_HEADER "https://"
#define GLOBAL_URL_CONTENT "/imcloud/meter"
#define GLOBAL_URL_ACCESSKEY "/activate/"
#define GLOBAL_URL_INFO "/info/"
#define GLOBAL_URL_DATA "/data/"
#define GLOBAL_URL_FW "/fw/"

/* =================================== API ======================================= */
int global_getFWChecksum();
void global_setFWChecksum(uint32_t check_sum);
int global_getFWsize();
void global_setFWsize(int size);
void global_setFwVersion(uint8_t * version);
uint16_t global_getFWversion();
void global_setFwVersionNormal(uint16_t  version);
uint16_t global_getFWversionDefault();
void global_setAdcFrq(uint8_t hz);
uint8_t gloal_getAdcFrq();
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
void global_setUrl(enum ICOULD_URL index);
char * global_getUrl(enum ICOULD_URL index);
char * global_getMac();
void global_setMac(char *mac);
char * global_getAccesskey();
void global_setAccesskey(char *key);
void global_startNextTotals();
float short2float(uint16_t a);
void global_setIgain(uint16_t igain);
void global_setVgain(uint16_t vgain);
float global_getIgain();
float global_getVgain();
float global_getVthreshol();
float global_getIthreshol();
void global_setVthreshol(float vthreshol);
void global_setIthreshol(float ithreshol);
void global_setdomain(char *url);
void global_setFwUrl(char *fw_domain);
char *global_getSAK();
void global_setSAK(char *sak);
char *global_getUUID();
void global_setUUID(char * uuid);
int global_getWifiMode();
void global_setWifiMode(int mode);
#endif
