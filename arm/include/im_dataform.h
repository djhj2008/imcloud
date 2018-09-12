/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IM_DATAFORM_H__
#define __IM_DATAFORM_H__

#include "config.h"
#include "plehead.h"

/* Constants */
#define	PLC_HEADER_SIZE         16  /* bytes */
#define PLC_L_DATA_SIZE         28  /* bytes */
#define	PLC_V_DATA_SIZE         4   /* bytes */

#define DATA_FORMAT_VERSION		0x04

#define WATTAGE_FLAG  	0x01<<7
#define RSSI_FLAG 		0x01<<6
#define L4_CHANNEL_FLAG 0x01<<3
#define L3_CHANNEL_FLAG 0x01<<2
#define L2_CHANNEL_FLAG 0x01<<1
#define L1_CHANNEL_FLAG 0x01
#define ALL_CHANNEL_FLAG L1_CHANNEL_FLAG|L2_CHANNEL_FLAG|L3_CHANNEL_FLAG|L4_CHANNEL_FLAG

#pragma pack(1) 


/* Input Waveforms */
struct waveform{
	uint32_t time_stamp;	//data time stamp
	int8_t rssi;			//wifi signal intensity
	float w1;				//line L1 capacity.
	float w2;				//line L2 capacity.
	float w3;
	float w4;
	int16_t data[ADC_SAMPLE_CHANNEL*SAMPLES_FRAME];	//Data for six channels. info @config.h
};

#define WAVE_FORM_HEAD_LEN	 	12
#define WAVE_FORM_VERSION 		1
#define WAVE_FORM_TOTAL 		2
#define WAVE_FORM_FLAG 			1
#define WAVE_FORM_IGAIN 		2
#define WAVE_FORM_VGAIN			2
#define WAVE_FORM_TIME 			4

/* Upload data head */
struct data_header{
	uint8_t version;	//version always
	uint16_t total;
	uint8_t flag;
	uint16_t igain;
	uint16_t vgain;
	uint32_t start_time;
};

#pragma pack()
/* =================================== API ======================================= */
uint8_t * GenerateBackupWaveform(char * file,int *len ,uint32_t * first_time);
uint8_t * GenerateWaveform(char * file,int *len ,uint32_t * first_time,int ichannels,int vchannels,int totals,uint8_t flag);
ple_uint8_t* ple_decode(struct waveform *waveform_t,
									int sub_index,
									uint8_t ucCurrentChannels,
									uint8_t ucVoltageChannels,
									uint16_t ucFramesPerGroup ,
									int *size);
//int GenerateWaveFile(char * file,int *len ,uint32_t * first_time,int ichannels,int vchannels,int totals,uint8_t flag);
#endif
