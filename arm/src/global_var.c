/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  Global vars.
 *               For usage, check the global_var.h file
 *
 *//** @file global_var.h *//*
 *
 ********************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>  

#include "config.h"
#include "imcloud_controller.h"
#include "im_dataform.h"
#include "imcloud.h"

uint8_t ch_flag[WAVE_CHANNEL_MAX];

char access_key[128]={0};
char mac_addr[MAC_LEN+1]= {0x0};
char cloud_url[10][256]={0};

int global_totals;
int next_totals;

void global_setAccesskey(char *key)
{
	memset(access_key,0,128);
	strcpy(access_key,key);
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

char * global_getUrl(enum ICOULD_URL index)
{
	return cloud_url[index];
}

void global_setUrl(const char *url,enum ICOULD_URL index)
{
	strcpy(cloud_url[index],url);
}

void global_setIchFlag(int ich)
{
	printf("setIchFlag %d \n",ich);
	if(ich<I_CHANNELS_1||ich>I_CHANNELS_4)
		return;
	ch_flag[ich-I_CHANNELS_1]=ADC_CH_OPEN;
}

void global_setVchFlag(int vch)
{
	printf("setVchFlag %d \n",vch);
	if(vch<V_CHANNELS_1||vch>V_CHANNELS_2)
		return;
	ch_flag[vch-V_CHANNELS_1+ADC_L_CHANNELS]=ADC_CH_OPEN;
}

void global_dumpCH()
{
	int i=0;
	for(i=0;i<WAVE_CHANNEL_MAX;i++){
		printf("CH[%d] flag=%d \n",i,ch_flag[i]);
	}	
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
		printf("Rest Totals %d,pre Totals %d \n",global_totals,totals);
	}
}
