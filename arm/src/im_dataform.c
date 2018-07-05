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

int GenerateWaveform(char * file,uint8_t **postdata,int *len)
{
	uint16_t	ucFramesPerGroup  = FRAMES_GROUP;  //300

	int fd;
    char dirpath[MAX_DIRPATH_LEN]={0x0};
	int h_count,w_count,o_count,plc_count;
	struct waveform 	*waveform_t=NULL;
	struct otherform 	*otherform_t=NULL;
	struct data_header  data_header_t;
	int result_size = 0 ,full_size = 0;
	int index = 0;
	int remaining = 0;
	int i = 0;
	
	printf("ucFramesPerGroup  : %d\n", ucFramesPerGroup);


	sprintf(dirpath,"./data/%s",file);
	
	fd = open(dirpath,O_RDWR);
	if(fd<0){
		printf("file open error.\n");
		return -1;
	}

	waveform_t = (struct waveform *)malloc(sizeof(struct waveform));
	
	otherform_t = (struct otherform *)malloc(sizeof(struct otherform));
	
	w_count = read(fd,waveform_t,sizeof(struct waveform));
	
	printf("w_count  : %d\n", w_count);
	
	lseek(fd,SEEK_SET,w_count);
	
	o_count = read(fd,otherform_t,sizeof(struct otherform));
	
	if(o_count<0){
		printf("file read error.\n");
		return -1;
	}
	close(fd);
	//printf("waveform_t time_stamp=%ld\n",waveform_t->time_stamp);

	data_header_t.version 	= DATA_FORMAT_VERSION;
	data_header_t.total 	= ucFramesPerGroup;
	data_header_t.flag 		= WATTAGE_FLAG|RSSI_FLAG|ALL_CHANNEL_FLAG;
	data_header_t.igain		= 0;
	data_header_t.vgain		= 0;
	data_header_t.start_time= (uint32_t)waveform_t->time_stamp;
	
	printf("version  : %d\n", data_header_t.version);
	printf("total : %d\n", data_header_t.total);
	printf("flag : %d\n", data_header_t.flag );
	printf("start_time : %d\n", data_header_t.start_time);
	
	for(i=0;i<FRAMES_GROUP;i++){
		printf("index = %d rssi = %d w1 = %f w2 = %f \n", i,otherform_t->rssi[i],otherform_t->wat[i].w1,otherform_t->wat[i].w2);
		printf(" data %d \n",waveform_t->data[i][0]);
	}

	h_count = sizeof(data_header_t);
	
	plc_count = (FRAMES_GROUP + PLE_FRAME_MAX - 1) / PLE_FRAME_MAX;
	
	full_size = PLC_HEADER_SIZE*plc_count+(ADC_L_CHANNEL*PLC_L_DATA_SIZE+ADC_V_CHANNEL*PLC_V_DATA_SIZE)*FRAMES_GROUP;
	
	*postdata = (uint8_t *)malloc(h_count+full_size+o_count);
	
	memcpy(*postdata,&data_header_t,h_count);
	
	index += h_count;
	
	printf("index : %d all : %d\n", index,h_count+full_size+o_count);
	
	remaining = FRAMES_GROUP;
	
	while(remaining > 0){
		
		if(remaining > PLE_FRAME_MAX){
			
			ple_uint8_t	*wp= ple_decode(waveform_t,i*FRAMES_GROUP,PLE_FRAME_MAX);
		
			result_size = PLC_HEADER_SIZE+(ADC_L_CHANNEL*PLC_L_DATA_SIZE+ADC_V_CHANNEL*PLC_V_DATA_SIZE)*PLE_FRAME_MAX;
		
			memcpy(*postdata+index,wp,result_size);
			
			index += result_size;

			remaining -= PLE_FRAME_MAX;
			
			if(wp != NULL){
				free(wp);
			}
			
			printf("index  : %d remaining = %d\n", index , remaining);
						
		}else{
			ple_uint8_t	*wp= ple_decode(waveform_t,i*FRAMES_GROUP,remaining);
		
			result_size = PLC_HEADER_SIZE+(ADC_L_CHANNEL*PLC_L_DATA_SIZE+ADC_V_CHANNEL*PLC_V_DATA_SIZE)*remaining;
		
			memcpy(*postdata+index,wp,result_size);
			
			index += result_size;

			remaining = 0;
			
			if(wp != NULL){
				free(wp);
			}
			
			printf("index  : %d\n", index);
			
			break;
		}
		i++;
	
	}
	
	memcpy(*postdata+index,otherform_t,o_count);
	
	index += o_count;
	
	printf("index  end: %d\n", index);	
	
	*len = index;
	
	if(*postdata!=NULL){
		printf("len  : %d\n", *len);
	}

	if(waveform_t!=NULL){
		free(waveform_t);
	}
	
	printf("free  : waveform_t\n");
	if(otherform_t!=NULL){
		free(otherform_t);
	}
	
	printf("free  : otherform_t\n");
	/* Ending */
	return(0);
}


ple_uint8_t* ple_decode(struct waveform *waveform_t,int sub_index,uint16_t ucFramesPerGroup){
	HPLE		hPle = NULL;   /* Encoder */
	plc_uint8_t	*pucItem = NULL;  /* Encoded data */
	plc_uint8_t	*rp;
	plc_size_t	nItemSize;
	ple_uint16_t	**ppusWave = NULL;  /* Input */
	ple_uint8_t	*encoded_result = NULL; /* output */
	ple_uint8_t	*wp=NULL;
	
	/* Input Attribute */
	uint8_t		ucFundamentalFrq  = AC_LINE_FREQUENCY; //60
//	uint8_t		ucCurrentChannels = 4;
	uint8_t		ucCurrentChannels = ADC_L_CHANNEL; //4
	uint8_t		ucVoltageChannels = ADC_V_CHANNEL; //1
	uint8_t		ucSamplesPerFrame = SAMPLES_FRAME; //64
	//uint16_t	ucFramesPerGroup  = FRAMES_GROUP;  //300
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


	printf("ple_decode  000\n");
	
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

	printf("ple_decode  111\n");

	/* Loop */
	for(j=0;j<ucFramesPerGroup;j++){
		
		printf("encode j= %d \n",j);
		/* preparing wave data (1 sec) */
		for(l=0;l<nChannels;l++){
			for(i=0;i<ucSamplesPerFrame;i++){
				if(l<ucCurrentChannels){
					ppusWave[l][i] = 0;//waveform_t->data[j][i+(l+1)*ucSamplesPerFrame];
				}
				else {
					ppusWave[l][i] = waveform_t->data[j][i];
				}
			}
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
