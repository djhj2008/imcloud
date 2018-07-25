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

#include "im_log.h"
#include "config.h"
#include "imcloud_controller.h"
#include "im_dataform.h"
#include "imcloud.h"

/* get from server.needed other server interface */
char access_key[128]={0};
/*wifi Mac ADDR */
char mac_addr[MAC_LEN+1]= {0x0};
/*Server URL */
char cloud_url[10][256]={0};
/*WaveForm Channels Flag */
uint8_t ch_flag[WAVE_CHANNEL_MAX];
/*WaveForm upload totals */
int global_totals;
/*WaveForm next upload totals */
int next_totals;
/*Vgain Igain*/
static float global_igain;
static float global_vgain;

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

void global_setUrl(const char *url,enum ICOULD_URL index)
{
	strcpy(cloud_url[index],url);
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
void global_setIgain(uint16_t igain)
{
	global_igain = igain;
	imlogV("global_igain:%f",global_igain);
}

void global_setVgain(uint16_t vgain)
{
	global_vgain = vgain;
	imlogV("global_vgain:%f",global_vgain);
}

float global_getIgain()
{
	return global_igain;
}

float global_getVgain()
{
	return global_vgain;
}



