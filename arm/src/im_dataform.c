/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  File creat read write.
 *               For usage, check the dataform.h file
 *
 *//** @file im_dataform.h *//*
 *
 ********************************/
 
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
#include <sys/types.h>
#include <errno.h>
#include <unistd.h>

#include <fcntl.h>
#include <dirent.h>

#include "plchead.h"
#include "plehead.h"

#include "config.h"
#include "im_dataform.h"
#include "im_file.h"
/* Expected data */

uint8_t * GenerateWaveform(char * file,int *len ,int ichannels,int vchannels,int totals,uint8_t flag)
{
	int fd;
    char dirpath[MAX_DIRPATH_LEN]={0x0};
	int h_count,w_count,o_count,plc_count;
	struct waveform 	waveform_t[FRAMES_GROUP_MAX];
	struct data_header  data_header_t;
	int result_size = 0 ,full_size = 0;
	int index = 0;
	int remaining = 0;
	int i=0,j=0;
	uint8_t *postdata;
	uint8_t	ucCurrentChannels = ichannels; //4
	uint8_t	ucVoltageChannels = vchannels; //1
	uint16_t ucFramesPerGroup  = totals;  //300
	
	printf("ucFramesPerGroup  : %d\n", ucFramesPerGroup);


	sprintf(dirpath,"./data/%s",file);
	
	fd = open(dirpath,O_RDWR);
	if(fd<0){
		printf("file open error.\n");
		return NULL;
	}

	w_count = read(fd,waveform_t,sizeof(struct waveform)*totals);
	
	if(w_count<0){
		printf("file read error.\n");
		return NULL;
	}
	close(fd);
	//printf("waveform_t time_stamp=%ld\n",waveform_t->time_stamp);

	data_header_t.version 	= DATA_FORMAT_VERSION;
	data_header_t.total 	= ucFramesPerGroup;
	data_header_t.flag 		= WATTAGE_FLAG|RSSI_FLAG|flag;
	data_header_t.igain		= 0;
	data_header_t.vgain		= 0;
	data_header_t.start_time= (uint32_t)waveform_t[0].time_stamp;
	
	printf("version  : %d\n", data_header_t.version);
	printf("total : %d\n", data_header_t.total);
	printf("flag : %d\n", data_header_t.flag );
	printf("start_time : %d\n", data_header_t.start_time);
	
	for(i=0;i<totals;i++){
		//printf("index = %d rssi = %d w1 = %f w2 = %f \n", i,waveform_t[i].rssi,waveform_t[i].w1,waveform_t[i].w2);
		//printf(" data v=%d l1=%d l2=%d\n",waveform_t[i].data[0],waveform_t[i].data[64],waveform_t[i].data[128]);
		
	}

	h_count = sizeof(data_header_t);
	
	plc_count = (totals + PLC_FRAMES_PER_GROUP_MAX - 1) / PLC_FRAMES_PER_GROUP_MAX;
	
	full_size = PLC_HEADER_SIZE*plc_count+(ucCurrentChannels*PLC_L_DATA_SIZE+ucVoltageChannels*PLC_V_DATA_SIZE)*totals;
	
	o_count = totals*(sizeof(int8_t)+sizeof(float)+sizeof(float));
	
	printf("o_count : %d",o_count);
	
	postdata = (uint8_t *)malloc(h_count+full_size+o_count);
	
	memcpy(postdata,&data_header_t,h_count);
	
	index += h_count;
	
	printf("index : %d all : %d\n", index,h_count+full_size+o_count);
	
	remaining = totals;
	
	i=0;
	while(remaining > 0){
		
		if(remaining > PLC_FRAMES_PER_GROUP_MAX){
			
			ple_uint8_t	*wp= ple_decode(waveform_t,i*PLC_FRAMES_PER_GROUP_MAX,ucCurrentChannels,ucVoltageChannels,PLC_FRAMES_PER_GROUP_MAX,&result_size);
		
			memcpy(postdata+index,wp,result_size);
			
			index += result_size;

			remaining -= PLE_FRAME_MAX;
			
			if(wp != NULL){
				free(wp);
			}
			
			printf("index  : %d remaining = %d\n", index , remaining);
			
			for(j=0;j<PLC_FRAMES_PER_GROUP_MAX;j++){
				memcpy(postdata+index,&(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].rssi),sizeof(int8_t));
				index += sizeof(int8_t);
			}
			printf("times : %d index  rssi end: %d\n", i,index);
			
			for(j=0;j<PLC_FRAMES_PER_GROUP_MAX;j=j+2){
				memcpy(postdata+index,&(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w1),sizeof(float));
				index += sizeof(float);
				memcpy(postdata+index,&(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w2),sizeof(float));
				index += sizeof(float);	
			}

			printf("index wat end: %d\n", index);
						
		}else{
			ple_uint8_t	*wp= ple_decode(waveform_t,i*PLC_FRAMES_PER_GROUP_MAX,ucCurrentChannels,ucVoltageChannels,remaining,&result_size);
		
			memcpy(postdata+index,wp,result_size);
			
			index += result_size;

			remaining = 0;
			
			if(wp != NULL){
				free(wp);
			}
			
			printf("index  : %d\n", index);
			
			for(j=0;j<remaining;j++){
				memcpy(postdata+index,&(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].rssi),sizeof(int8_t));
				index += sizeof(int8_t);
			}
			printf("times : %d index  rssi end: %d\n", i,index);
			
			for(j=0;j<remaining;j=j+2){
				memcpy(postdata+index,&(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w1),sizeof(float));
				index += sizeof(float);
				memcpy(postdata+index,&(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w2),sizeof(float));
				index += sizeof(float);	
			}

			printf("index wat end: %d\n", index);
			
			break;
		}
		i++;
	
	}

	
	*len = index;

	printf("free  : waveform_t\n");

	/* Ending */
	return postdata;
}


ple_uint8_t* ple_decode(struct waveform *waveform_t,
									int sub_index,
									uint8_t ucCurrentChannels,
									uint8_t ucVoltageChannels,
									uint16_t ucFramesPerGroup ,
									int *size)
{
	HPLE		hPle = NULL;   /* Encoder */
	plc_uint8_t	*pucItem = NULL;  /* Encoded data */
	plc_uint8_t	*rp;
	plc_size_t	nItemSize;
	ple_uint16_t	**ppusWave = NULL;  /* Input */
	ple_uint8_t	*encoded_result = NULL; /* output */
	ple_uint8_t	*wp=NULL;
	
	/* Input Attribute */
	uint8_t		ucFundamentalFrq  = AC_LINE_FREQUENCY; //60
	uint8_t		ucSamplesPerFrame = SAMPLES_FRAME; //64
	int		nChannels;
	int		i, j, k, l, result_size;
	ple_code_t	ier_e;

	printf("ucFundamentalFrq  : %d\n", ucFundamentalFrq);
	printf("ucCurrentChannels : %d\n", ucCurrentChannels);
	printf("ucVoltageChannels : %d\n", ucVoltageChannels);
	printf("ucSamplesPerFrame : %d\n", ucSamplesPerFrame);
	printf("ucFramesPerGroup  : %d\n", ucFramesPerGroup);
	printf("\n");
	printf("==================================================\n");
	printf("                PLE check\n");
	printf("==================================================\n");

	printf("ple_decode sub_index=%d ucFramesPerGroup=%d \n",sub_index,ucFramesPerGroup);
	
	/* Initialize */
	PLEInitialize(&hPle, NULL, 0);

	/* Alloc */
	/* array of pointer to each channel of single waveform buffer */
	nChannels = ucCurrentChannels + ucVoltageChannels;
	ppusWave = (ple_uint16_t **)malloc((size_t)nChannels*sizeof(ple_uint16_t *));
	if(ppusWave == NULL) {
		fprintf(stderr, " ### Error: Memory Allocation ###\n");
		goto Finish;
	}
	memset(ppusWave, 0, (size_t)nChannels*sizeof(ple_uint16_t *));

	/* buffer of each channel */
	/* To reduce working memory, you should modify PLC_SAMPLES_PER_FRAME_MAX to the appropriate value for your system */
	for(i=0; i<nChannels; i++) {
		ppusWave[i] = (ple_uint16_t *)malloc(PLC_SAMPLES_PER_FRAME_MAX*sizeof(ple_uint16_t));
		if(ppusWave[i] == NULL) {
			fprintf(stderr, " ### Error: Memory Allocation ###\n");
			goto Finish;
		}
	}

	/* buffer of encoded data */
	result_size = PLC_HEADER_SIZE+(ucCurrentChannels*PLC_L_DATA_SIZE+ucVoltageChannels*PLC_V_DATA_SIZE)*ucFramesPerGroup;
	encoded_result = (ple_uint8_t *)malloc(result_size);
	wp = encoded_result;
	/* PreProcess */
	ier_e = PLEPreProcess(hPle,
			(ple_uint8_t)ucFundamentalFrq,
			(ple_uint8_t)ucCurrentChannels,
			(ple_uint8_t)ucVoltageChannels,
			(ple_uint8_t)ucSamplesPerFrame,
			(ple_uint8_t)ucFramesPerGroup);
	if(ier_e < 0) {
		fprintf(stderr, " ### Error: PLEPreProcess: %d ###\n", ier_e);
		goto Finish;
	}

	printf("Start Decode.\n");

	/* Loop */
	for(j=0;j<ucFramesPerGroup;j++){
		float w1_sum = 0;
		float w2_sum = 0;
		int vc = 0;
		int ch1 = 0;
		int ch2 = 0;
		float w1 = 0;
		float w2 = 0;
		/* preparing wave data (1 sec) */
		//printf("index=%d v=%d l1=%d l2=%d\n",j,waveform_t[j+sub_index].data[0],waveform_t[j+sub_index].data[64],waveform_t[j+sub_index].data[128]);
		for(l=0;l<nChannels;l++){
			for(i=0;i<ucSamplesPerFrame;i++){
				if(l<ucCurrentChannels){
					ppusWave[l][i] = waveform_t[j+sub_index].data[i+l*ucSamplesPerFrame];
				}
				else {
					ppusWave[l][i] = waveform_t[j+sub_index].data[i+(l-ucCurrentChannels+WAVE_V1_CHANNEL)*ucSamplesPerFrame];
				}
			}
		}
		
		for(i = 0; i < ucSamplesPerFrame; i++){
			vc = (int16_t)ppusWave[ucCurrentChannels][i];
			ch1 = (int16_t)ppusWave[0][i];
			ch2 = (int16_t)ppusWave[1][i];
			w1_sum += vc*ch1;
			w2_sum += vc*ch2;
		}
		w1 = w1_sum/ucSamplesPerFrame;
		w2 = w2_sum/ucSamplesPerFrame;
		
		if(w1 != waveform_t[j+sub_index].w1||w2 != waveform_t[j+sub_index].w2){
			printf("w1 = %f pw1 = %f ,w2 = %f pw2 = %f\n",w1,waveform_t[j+sub_index].w1,w2,waveform_t[j+sub_index].w2);
		}

		/* Put data to PLE */
		ier_e = PLEPutData(hPle, (const ple_uint16_t **)ppusWave);
		if(ier_e < 0) {
			fprintf(stderr, " ### Error: PLEPutData: %d ###\n", ier_e);
			goto Finish;
		}

		/* Get result from PLE */
		while(1) {

			/* Get result size */
			ier_e = PLEGetResultSize(hPle, &nItemSize);
			if(ier_e == PLE_RESULT_NONE) {
				break;
			}
			else if(ier_e < 0) {
				fprintf(stderr, " ### Error: PLEGetResultSize: %d ###\n", ier_e);
				goto Finish;
			}

			/* Alloc */
			pucItem = (plc_uint8_t *)realloc((void *)pucItem, nItemSize);
			if(pucItem == NULL) {
				fprintf(stderr, " ### Error: Memory Allocation ###\n");
				goto Finish;
			}
			else {
				rp = pucItem;
			}

			/* Get result */
			ier_e = PLEGetResult(hPle, pucItem);
			if(ier_e == PLE_RESULT_NONE) {
				break;
			}
			else if(ier_e < 0) {
				fprintf(stderr, " ### Error: PLEGetResult: %d ###\n", ier_e);
				goto Finish;
			}
      
			/* Save result */
			for(k=0;k<nItemSize;k++){
				*wp++ = *rp++;
			}
		}
	}

	/* CHECK RESULT */
	wp = encoded_result;
	*size = result_size;
	printf("==================  ENCODE OK ====================\n");

Finish:
	printf("Finish \n");
	/* Release */
	PLEFinalize(&hPle);
	printf("PLEFinalize \n");
	if(pucItem != NULL) {
		free(pucItem);
	}
	printf("free  : pucItem\n");
	if(ppusWave != NULL) {
		for(i=0; i<nChannels; i++) {
			if(ppusWave[i] != NULL) {
				free(ppusWave[i]);
			}
		}
		free(ppusWave);
	}
	printf("free  : ppusWave\n");
	//if(encoded_result != NULL) {
	//	free(encoded_result);
	//}
	
	/* Ending */
	return encoded_result;	

}
