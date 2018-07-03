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

int GenerateWaveform(char * file)
{
	HPLE		hPle = NULL;   /* Encoder */
	plc_uint8_t	*pucItem = NULL;  /* Encoded data */
	plc_uint8_t	*rp;
	plc_size_t	nItemSize;
	ple_uint16_t	**ppusWave = NULL;  /* Input */
	ple_uint8_t	*encoded_result = NULL; /* output */
	ple_uint8_t	*wp=NULL, *expdata=NULL;

	/* Input Attribute */
	uint8_t		ucFundamentalFrq  = AC_LINE_FREQUENCY; //60
//	uint8_t		ucCurrentChannels = 4;
	uint8_t		ucCurrentChannels = ADC_L_CHANNEL; //4
	uint8_t		ucVoltageChannels = ADC_V_CHANNEL; //1
	uint8_t		ucSamplesPerFrame = SAMPLES_FRAME; //64
	uint16_t	ucFramesPerGroup  = FRAMES_GROUP;  //300
	int		nChannels;
	int		i, j, k, l, result_size;
	ple_code_t	ier_e;
	int fd;
    char dirpath[MAX_DIRPATH_LEN]={0x0};
	int w_count,o_count;
	struct waveform 	*waveform_t=NULL;
	struct otherform 	*otherform_t=NULL;
	struct data_header  data_header_t;

	printf("ucFundamentalFrq  : %d\n", ucFundamentalFrq);
	printf("ucCurrentChannels : %d\n", ucCurrentChannels);
	printf("ucVoltageChannels : %d\n", ucVoltageChannels);
	printf("ucSamplesPerFrame : %d\n", ucSamplesPerFrame);
	printf("ucFramesPerGroup  : %d\n", ucFramesPerGroup);
	printf("\n");
	printf("==================================================\n");
	printf("                PLE check\n");
	printf("==================================================\n");


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
	wp = encoded_result = (ple_uint8_t *)malloc(result_size);

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


	sprintf(dirpath,"./data/%s",file);
	
	fd = open(dirpath,O_RDWR);
	if(fd<0){
		printf("file open error.\n");
		goto Finish;;
	}

	waveform_t = (struct waveform *)malloc(sizeof(struct waveform));
	
	otherform_t = (struct otherform *)malloc(sizeof(struct otherform));
	
	w_count = read(fd,waveform_t,sizeof(struct waveform));
	
	lseek(fd,SEEK_SET,w_count);
	
	o_count = read(fd,otherform_t,sizeof(struct otherform));
	
	if(o_count<0){
		printf("file read error.\n");
		goto Finish;
	}
	close(fd);
	printf("waveform_t time_stamp=%ld\n",waveform_t->time_stamp);


	data_header_t.version 	= DATA_FORMAT_VERSION;
	data_header_t.total 	= ucFramesPerGroup;
	data_header_t.flag 		=  WATTAGE_FLAG|RSSI_FLAG|ALL_CHANNEL_FLAG;
	data_header_t.igain		= 0;
	data_header_t.vgain		= 0;
	data_header_t.start_time= (uint32_t)waveform_t->time_stamp;
	
	printf("version  : %d\n", data_header_t.version);
	printf("total : %d\n", data_header_t.total);
	printf("flag : %d\n", data_header_t.flag );
	printf("start_time : %d\n", data_header_t.start_time);
	
	for(i=0;i<FRAMES_GROUP;i++){
		printf("index = %d rssi = %d w1 = %f w2 = %f \n", i,otherform_t->rssi[i],otherform_t->wat[i].w1,otherform_t->wat[i].w2);
	}
	
	if(data_header_t.start_time>0){
		goto Finish;
	}
	

	/* Loop */
	for(j=0;j<ucFramesPerGroup;j++){
      
		/* preparing wave data (1 sec) */
		for(l=0;l<nChannels;l++){
			for(i=0;i<ucSamplesPerFrame;i++){
				if(l<ucCurrentChannels){
					ppusWave[l][i] = waveform_t->data[l+1][i+j*ucSamplesPerFrame];
				}
				else {
					ppusWave[l][i] = waveform_t->data[0][i+j*ucSamplesPerFrame];
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
	if(ucCurrentChannels == 2){
		
	}
	else {
		
	}

	i = 0;
	while(result_size--){
		if(*wp++ != *expdata++){
			fprintf(stderr, " !!! ENCODE ERROR !!!\n");
			goto Finish;
		}
	}
	fprintf(stderr, "==================  ENCODE OK ====================\n");

Finish:
	/* Release */
	PLEFinalize(&hPle);
	if(pucItem != NULL) {
		free(pucItem);
	}
	if(ppusWave != NULL) {
		for(i=0; i<nChannels; i++) {
			if(ppusWave[i] != NULL) {
				free(ppusWave[i]);
			}
		}
		free(ppusWave);
	}
	if(encoded_result != NULL) {
		free(encoded_result);
	}
	free(waveform_t);
	free(otherform_t);
	/* Ending */
	return(0);
}
