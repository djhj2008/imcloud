/*************************************************
Copyright (C), 2018-2019, Tech. Co., Ltd.
File name: global_var.c
Author:doujun
Version:1.0
Date:2018-07-19
Description: Main Fuction.
* Global vars get/set fuctions
* 1.access_key
* 2.mac_addr
* 3.cloud_url
* 4.ch_flag
* 5.global_totals
* 6.next_totals
Others: NULL
Function List:
* 1.global_setAccesskey
* 2.global_getAccesskey
* 3.global_setMac
* 4.global_getMac
* 5.global_setUrl
* 6.global_getUrl
* 7.global_setIchFlag
* 8.global_setVchFlag
* 9.global_getChFlag
* global_getIchFlag
* global_getIchannelsCount
* global_getIchannelsCount
* global_getVchannelsCount
* global_resetCHFlag
* global_dumpCH
* global_setTotals
* global_getTotals
* global_getNextTotals
* global_startNextTotals
* global_setIgain
* global_setVgain
* global_getIgain
* global_getVgain
* 
* @file global_var.h
*
*************************************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  
#include <math.h>

#include "im_log.h"
#include "config.h"
#include "imcloud_controller.h"
#include "im_dataform.h"
#include "imcloud.h"
#include "global_var.h"
#include "eeprom_tool.h"

static uint16_t global_fw_version;
static int global_fw_size;
static uint32_t global_fw_crc;

static char domain[32]={0};
static char global_sak[64]={0};
/* get from server.needed other server interface */
static char access_key[ACCESS_KEY_SIZE+1]={0};
/*wifi Mac ADDR */
static char mac_addr[MAC_LEN+1]= {0x0};
/*Server URL */
static char cloud_url[10][256]={0};
/*WaveForm Channels Flag */
static uint8_t ch_flag[WAVE_CHANNEL_MAX];
/*WaveForm upload totals */
static int global_totals;
/*WaveForm next upload totals */
static int next_totals;
/*Vgain Igain*/
static uint16_t global_igain;
static uint16_t global_vgain;

static float V_threshol = 0.0;
static float I_threshol = 0.0;

static uint8_t global_adc_frq;

static char global_uuid[UUID_SIZE+1];

static int global_wifimode=1; //1 wifi mode ,0 lte 3g mode

void global_setWifiMode(int mode)
{
	global_wifimode = mode;
}

int global_getWifiMode()
{
	return global_wifimode;
}


void global_setUUID(char * uuid)
{
	strcpy(global_uuid,uuid);
}

char *global_getUUID()
{
	return global_uuid;
}

void global_setFWsize(int size)
{
	global_fw_size = size;
}

int global_getFWsize()
{
	return global_fw_size;
}

void global_setFWChecksum(uint32_t check_sum)
{
	global_fw_crc = check_sum;
}

int global_getFWChecksum()
{
	return global_fw_crc;
}

void global_setdomain(char *url)
{
	strcpy(domain,url);
}

void global_setSAK(char *sak)
{
	strcpy(global_sak,sak);
}

char *global_getSAK()
{
	return global_sak;
}

void global_setFwVersion(uint8_t * version)
{
	global_fw_version = ((version[1]<<8)&0xffff)|(version[0]&0xffff);
	imlogV("global_setFwVersion fw_version=%d",global_fw_version);
}

void global_setFwVersionNormal(uint16_t  version)
{
	global_fw_version = version;
}

uint16_t global_getFWversion()
{
	return global_fw_version;
}

uint16_t global_getFWversionDefault()
{
	uint16_t major=(FW_VERSION_MAJOR<<12)&0xffff;
	uint16_t minor=(FW_VERSION_MINOR<<8)&0xffff;
	uint16_t revision=(FW_VERSION_REVISION<<4)&0xffff;
	uint16_t host=FW_VERSION_HOST&0xffff;
	global_fw_version = major|minor|revision|host;
	return global_fw_version;
}

void global_setAdcFrq(uint8_t hz)
{
	global_adc_frq=hz;
}

uint8_t gloal_getAdcFrq()
{
	return global_adc_frq;
}

void global_setAccesskey(char *key)
{
	memset(access_key,0,ACCESS_KEY_SIZE+1);
	memcpy(access_key,key,ACCESS_KEY_SIZE);
}

char * global_getAccesskey()
{
	return access_key;
}

void global_setMac(char *mac)
{
	memcpy(mac_addr,mac,MAC_LEN);
}

char * global_getMac()
{
	return mac_addr;
}

void global_setFwUrl(char *fw_domain)
{
	char buf[256]={0x0};
	strcpy(buf,GLOBAL_URL_HEADER);
	strcat(buf,fw_domain);
	strcat(buf,GLOBAL_URL_CONTENT);
	strcat(buf,GLOBAL_URL_FW);
	strcpy(cloud_url[ICLOUD_URL_FW],buf);
}


void global_setUrl(enum ICOULD_URL index)
{
	char buf[256]={0x0};
	
	strcpy(buf,GLOBAL_URL_HEADER);
	
	strcat(buf,domain);

	strcat(buf,GLOBAL_URL_CONTENT);
	if(index == ICLOUD_URL_ACTIVATE){
		strcat(buf,GLOBAL_URL_ACCESSKEY);
	}
	else if(index == ICLOUD_URL_INFO){
		strcat(buf,GLOBAL_URL_INFO);
	}
	else if(index == ICLOUD_URL_DATA){
		strcat(buf,GLOBAL_URL_DATA);
	}
	else if(index == ICLOUD_URL_FW){
		strcat(buf,GLOBAL_URL_FW);
	}
	strcpy(cloud_url[index],buf);
}

char * global_getUrl(enum ICOULD_URL index)
{
	return cloud_url[index];
}

void global_setIchFlag(int ich)
{
	imlogV("setIchFlag %d \n",ich);
	if(ich<I_CHANNELS_1||ich>I_CHANNELS_4)
		return;
	ch_flag[ich-I_CHANNELS_1]=ADC_CH_OPEN;
}

void global_setVchFlag(int vch)
{
	imlogV("setVchFlag %d \n",vch);
	if(vch<V_CHANNELS_1||vch>V_CHANNELS_2)
		return;
	ch_flag[vch-V_CHANNELS_1+ADC_L_CHANNELS]=ADC_CH_OPEN;
}

int global_getChFlag(int ch)
{
	return ch_flag[ch];
}

uint8_t global_getIchFlag()
{
	uint8_t flag = 0;
	int i=0;
	for(i=0;i<I_CHANNELS_4;i++){
		if(ch_flag[i]==ADC_CH_OPEN){
			flag |= L1_CHANNEL_FLAG<<i;
		}
	}
	return flag;	
}

int global_getIchannelsCount()
{
	int i = 0;
	int count = 0;
	for(i=0;i<ADC_L_CHANNELS;i++){
		if(ch_flag[i]==ADC_CH_OPEN){
			count++;
		}
	}
	return count;
}

int global_getVchannelsCount()
{
	int i = 0;
	int count = 0;
	for(i=ADC_L_CHANNELS;i<WAVE_CHANNEL_MAX;i++){
		if(ch_flag[i]==ADC_CH_OPEN){
			count++;
		}
	}
	return count;
}

void global_resetCHFlag()
{
	int i = 0;
	for(i=0;i<WAVE_CHANNEL_MAX;i++){
		ch_flag[i]=ADC_CH_CLOSE;
	}
}

void global_dumpCH()
{
	int i=0;
	for(i=0;i<WAVE_CHANNEL_MAX;i++){
		imlogV("CH[%d] flag=%d \n",i,ch_flag[i]);
	}	
}

void global_setTotals(int totals)
{
	if(totals<5||totals>300)
	 return;
	next_totals = totals;
}

int global_getTotals()
{
	return global_totals;
}

int global_getNextTotals()
{
	return next_totals;
}

void global_startNextTotals()
{
	int totals = global_totals;
	if(next_totals!=global_totals){
		global_totals = next_totals;
		imlogV("Rest Totals %d,pre Totals %d \n",global_totals,totals);
	}
}

/*Vgain Igain*/
float short2float(uint16_t a)
{
  int i=0;
  int max=16;
  float c = 0;

  for(i=0;i<max;i++){
      int t= (a>>i)&0x01;
      if(t==1){
        short x = 0-(max-i);
        float m=0;
        //printf("x=%d\n",x);
		m=pow(2,x);
        c+=m;
		//printf("m=%f,c=%f\n",m,c);
      }
  }
  return c;
}

void global_setIgain(uint16_t igain)
{
	global_igain = igain<<2;
	imlogV("global_igain:%d",global_igain);
}

void global_setVgain(uint16_t vgain)
{
	global_vgain = vgain<<2;
	imlogV("global_vgain:%d",global_vgain);
}

float global_getIgain()
{
	return global_igain;
}

float global_getVgain()
{
	return global_vgain;
}

void global_setIthreshol(float ithreshol)
{
	I_threshol=ithreshol;
}

void global_setVthreshol(float vthreshol)
{
	V_threshol=vthreshol;
}

float global_getIthreshol()
{
	return I_threshol;
}

float global_getVthreshol()
{
	return V_threshol;
}

