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
	/* Input Waveforms */
	uint32_t time_stamp;
	int8_t rssi;
	float w1;
	float w2;
	int16_t data[ADC_SAMPLE_CHANNEL*SAMPLES_FRAME];
};

struct data_header{
	uint8_t version;
	uint16_t total;
	uint8_t flag;
	float igain;
	float vgain;
	uint32_t start_time;
};

#pragma pack()
/* =================================== API ======================================= */
uint8_t * GenerateWaveform(char * file,int *len ,int totals);
ple_uint8_t* ple_decode(struct waveform *waveform_t,int sub_index,uint16_t ucFramesPerGroup ,int *size);
#endif
