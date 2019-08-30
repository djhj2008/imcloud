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
#include <math.h>
#include <netinet/in.h> 

#include "plchead.h"
#include "plehead.h"

#include "im_log.h"
#include "config.h"
#include "im_dataform.h"
#include "global_var.h"
#include "im_file.h"
/* Expected data */

float htomf(float inFloat)
{
	//float retVal;
	//uint8_t *floatToConvert = (uint8_t*)&inFloat;
	//uint8_t *returnFloat = (uint8_t* )&retVal;
	//returnFloat[0] = floatToConvert[3];
	//returnFloat[1] = floatToConvert[2];
	//returnFloat[2] = floatToConvert[1];
	//returnFloat[3] = floatToConvert[0];
	return inFloat*1000;
} 

uint16_t htomId(uint16_t i)
{
	
	//imlogV("i %d",(int16_t)i);
	int temp = (int16_t)i+8192;
	uint16_t value=(uint16_t)temp;
	//imlogV("value %d",value);
	
	/*
	int16_t temp = (int16_t)i;
	int16_t value = 0;
	imlogV("temp %d",temp);
	if(temp<0){
		value = fabs(temp);
	}else{
		value = 0-temp;
	}
	imlogV("value %d",value);
	*/
	
	return value;
} 


void waveHead2Buf(struct data_header  *data_header_s,struct data_header  *data_header_d)
{
	data_header_d->version=data_header_s->version;
	data_header_d->total = htons(data_header_s->total);
	data_header_d->flag=data_header_s->flag;
	data_header_d->igain = htons(data_header_s->igain);
	data_header_d->vgain = htons(data_header_s->vgain);
	data_header_d->start_time = (uint32_t)htonl(data_header_s->start_time);
}

uint8_t * GenerateBackupWaveform(char * file,int *len ,uint32_t * first_time)
{
	int fd;
	struct data_header header_buf;
	uint8_t *postdata=NULL;
	int file_size;
	int time;
	
	file_size = get_file_size(file);
	imlogV("backup file size:%d",file_size);
	fd = open(file,O_RDWR);
	if(fd<0){
		imlogE("file open error.\n");
		return NULL;
	}
	read(fd,&header_buf,sizeof(struct data_header));
	*first_time = (uint32_t)htonl(header_buf.start_time);
	time = *first_time;
	imlogV("backup file time:%d",time);
	if((time<4102416000)&&(time>1514736000)){
		lseek(fd,0,SEEK_SET);
		postdata = (uint8_t *)malloc(file_size);
		*len = read(fd,postdata,file_size);
	}
	close(fd);
	return postdata;
	
}


uint8_t * GenerateWaveform(char * file,int *len ,uint32_t * first_time,int ichannels,int vchannels,int totals,uint8_t flag)
{
	int fd;
	int h_count,w_count,o_count,plc_count;
	struct waveform 	waveform_t[FRAMES_GROUP_MAX];
	struct data_header  data_header_t;
	int result_size = 0 ,full_size = 0 ,file_size = 0;
	int index = 0;
	int remaining = 0;
	int i=0,j=0;
	uint8_t *postdata;
	uint8_t	ucCurrentChannels = ichannels; //4
	uint8_t	ucVoltageChannels = vchannels; //1
	uint16_t ucFramesPerGroup  = totals;   //300
	int file_totals=0;
	struct data_header header_buf;
	
	imlogV("ucFramesPerGroup  : %d\n", ucFramesPerGroup);

	file_size = get_file_size(file);
	
	if(file_size > 0){
		if(file_size%sizeof(struct waveform)==0){
			file_totals = file_size/sizeof(struct waveform);
		}else{
			imlogE("file format error.filesize=%d,block=%d.\n",file_size,sizeof(struct waveform));
			return NULL;
		}
	
	}else{
		imlogE("file size error.\n");
		return NULL;	
	}

	if(file_totals!=totals){
		ucFramesPerGroup = file_totals;
		imlogE("ChangeTotals File totals = %d curTotals=%d.\n",file_totals,totals);
	}
	
	fd = open(file,O_RDWR);
	if(fd<0){
		imlogE("file open error.\n");
		return NULL;
	}
	w_count = read(fd,waveform_t,sizeof(struct waveform)*ucFramesPerGroup);
	close(fd);
	
	if(w_count<0){
		imlogE("file read error.\n");
		return NULL;
	}
	//imlogV("waveform_t time_stamp=%ld\n",waveform_t->time_stamp);

	data_header_t.version 	= DATA_FORMAT_VERSION;
	data_header_t.total 	= ucFramesPerGroup;
	data_header_t.flag 		= WATTAGE_FLAG|RSSI_FLAG|flag;
	data_header_t.igain		= global_getIgain();
	data_header_t.vgain		= global_getVgain();
	data_header_t.start_time= (uint32_t)waveform_t[0].time_stamp;
	*first_time = data_header_t.start_time;
	
	imlogV("version  : %d\n", data_header_t.version);
	imlogV("total : %d\n", data_header_t.total);
	imlogV("flag : %d\n", data_header_t.flag );
	imlogV("start_time : %d\n", data_header_t.start_time);
	
	/*
	for(i=0;i<ucFramesPerGroup;i++){
		imlogV("index = %d rssi = %d w1 = %f w2 = %f \n", i,waveform_t[i].rssi,waveform_t[i].w1,waveform_t[i].w2);
		imlogV(" data v=%d l1=%d l2=%d\n",waveform_t[i].data[0],waveform_t[i].data[64],waveform_t[i].data[128]);	
	}
	*/

	h_count = WAVE_FORM_HEAD_LEN;
	
	plc_count = (ucFramesPerGroup + PLC_FRAMES_PER_GROUP_MAX - 1) / PLC_FRAMES_PER_GROUP_MAX;
	
	full_size = PLC_HEADER_SIZE*plc_count+(ucCurrentChannels*PLC_L_DATA_SIZE+ucVoltageChannels*PLC_V_DATA_SIZE)*ucFramesPerGroup;
	
	o_count = ucFramesPerGroup*(sizeof(int8_t)+sizeof(float)*ucCurrentChannels);
	
	imlogV("o_count : %d",o_count);
	
	postdata = (uint8_t *)malloc(h_count+full_size+o_count);

	waveHead2Buf(&data_header_t,&header_buf);
	
	memcpy(postdata+index,&header_buf,h_count);
	
	index += h_count;
	
	imlogV("index : %d all : %d\n", index,h_count+full_size+o_count);
	
	remaining = ucFramesPerGroup;
	
	i=0;
	while(remaining > 0){
		
		if(remaining > PLC_FRAMES_PER_GROUP_MAX){
			
			ple_uint8_t	*wp= ple_decode(waveform_t,i*PLC_FRAMES_PER_GROUP_MAX,ucCurrentChannels,ucVoltageChannels,PLC_FRAMES_PER_GROUP_MAX,&result_size);
		
			memcpy(postdata+index,wp,result_size);
			
			index += result_size;
			
			if(wp != NULL){
				free(wp);
			}
			
			imlogV("index  : %d remaining = %d\n", index , remaining);
			
			for(j=0;j<PLC_FRAMES_PER_GROUP_MAX;j++){
				memcpy(postdata+index,&(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].rssi),sizeof(int8_t));
				index += sizeof(int8_t);
			}
			imlogV("times : %d index  rssi end: %d\n", i,index);
			
			for(j=0;j<PLC_FRAMES_PER_GROUP_MAX;j++){
				int w1=0,w2=0,w3=0,w4=0;
				//imlogV("w1=%f w2=%f w3=%f w4=%f\n",
				//waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w1,
				//waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w2,
				//waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w3,
				//waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w4);
				if((flag&0x01)==0x01){
					w1 = htonl((int)htomf(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w1));
					memcpy(postdata+index,&w1,sizeof(uint32_t));
					index += sizeof(float);
				}
				if((flag&0x02)==0x02){
					w2 = htonl((int)htomf(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w2));
					memcpy(postdata+index,&w2,sizeof(uint32_t));
					index += sizeof(float);
				}
				if((flag&0x04)==0x04){
					w3 = htonl((int)htomf(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w3));
					memcpy(postdata+index,&w3,sizeof(uint32_t));
					index += sizeof(float);	
				}
				if((flag&0x08)==0x08){
					w4 = htonl((int)htomf(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w4));
					memcpy(postdata+index,&w4,sizeof(uint32_t));
					index += sizeof(float);	
				}
				imlogV("w1=%d w2=%d w3=%d w4=%d\n",htonl(w1),htonl(w2),htonl(w3),htonl(w4));
			}

			imlogV("index wat end: %d\n", index);
			
			remaining -= PLE_FRAME_MAX;
						
		}else{
			ple_uint8_t	*wp= ple_decode(waveform_t,i*PLC_FRAMES_PER_GROUP_MAX,ucCurrentChannels,ucVoltageChannels,remaining,&result_size);
		
			memcpy(postdata+index,wp,result_size);
			
			index += result_size;
			
			if(wp != NULL){
				free(wp);
			}
			
			imlogV("index  : %d\n", index);
			
			for(j=0;j<remaining;j++){
				memcpy(postdata+index,&(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].rssi),sizeof(int8_t));
				index += sizeof(int8_t);
			}
			imlogV("times : %d index  rssi end: %d\n", i,index);
			
			for(j=0;j<remaining;j++){
				int w1=0,w2=0,w3=0,w4=0;
				//imlogV("w1=%f w2=%f w3=%f w4=%f\n",
				//waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w1,
				//waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w2,
				//waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w3,
				//waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w4);
				if((flag&0x01)==0x01){
					w1 = htonl((int)htomf(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w1));
					memcpy(postdata+index,&w1,sizeof(uint32_t));
					index += sizeof(float);
				}
				if((flag&0x02)==0x02){
					w2 = htonl((int)htomf(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w2));
					memcpy(postdata+index,&w2,sizeof(uint32_t));
					index += sizeof(float);
				}
				if((flag&0x04)==0x04){
					w3 = htonl((int)htomf(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w3));
					memcpy(postdata+index,&w3,sizeof(uint32_t));
					index += sizeof(float);	
				}
				if((flag&0x08)==0x08){
					w4 = htonl((int)htomf(waveform_t[j+i*PLC_FRAMES_PER_GROUP_MAX].w4));
					memcpy(postdata+index,&w4,sizeof(uint32_t));
					index += sizeof(float);	
				}
				//imlogV("w1=%d w2=%d w3=%d w4=%d\n",htonl(w1),htonl(w2),htonl(w3),htonl(w4));
			}
		
			imlogV("index wat end: %d\n", index);
			
			remaining = 0;
			
			break;
		}
		i++;
	
	}

/*
	memcpy(postdata+index,log_header,4);
    index+=4;
    
    memcpy(postdata+index,first_time,4);
    index+=4;
	
	
	memcpy(postdata+index,"TEST",4);
    index+=5;
*/    
	*len = index;

	imlogV("free  : waveform_t\n");

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
	uint8_t		ucFundamentalFrq  = gloal_getAdcFrq(); //60
	uint8_t		ucSamplesPerFrame = SAMPLES_FRAME; //64
	int		nChannels;
	int		i, j, k, l, result_size;
	ple_code_t	ier_e;

	//uint16_t vgain = global_getVgain();
	//uint16_t igain = global_getIgain();
	//float vgain_f = short2float(vgain);
	//float igain_f = short2float(igain);
	
	
	imlogV("ucFundamentalFrq  : %d\n", ucFundamentalFrq);
	imlogV("ucCurrentChannels : %d\n", ucCurrentChannels);
	imlogV("ucVoltageChannels : %d\n", ucVoltageChannels);
	imlogV("ucSamplesPerFrame : %d\n", ucSamplesPerFrame);
	imlogV("ucFramesPerGroup  : %d\n", ucFramesPerGroup);
	imlogV("\n");
	imlogV("==================================================\n");
	imlogV("                PLE check\n");
	imlogV("==================================================\n");

	imlogV("ple_decode sub_index=%d ucFramesPerGroup=%d \n",sub_index,ucFramesPerGroup);
	
	/* Initialize */
	PLEInitialize(&hPle, NULL, 0);

	/* Alloc */
	/* array of pointer to each channel of single waveform buffer */
	nChannels = ucCurrentChannels + ucVoltageChannels;
	ppusWave = (ple_uint16_t **)malloc((size_t)nChannels*sizeof(ple_uint16_t *));
	if(ppusWave == NULL) {
		imlogE("### Error: Memory Allocation ###\n");
		goto Finish;
	}
	memset(ppusWave, 0, (size_t)nChannels*sizeof(ple_uint16_t *));

	/* buffer of each channel */
	/* To reduce working memory, you should modify PLC_SAMPLES_PER_FRAME_MAX to the appropriate value for your system */
	for(i=0; i<nChannels; i++) {
		ppusWave[i] = (ple_uint16_t *)malloc(PLC_SAMPLES_PER_FRAME_MAX*sizeof(ple_uint16_t));
		if(ppusWave[i] == NULL) {
			imlogE("### Error: Memory Allocation ###\n");
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
		imlogE("### Error: PLEPreProcess: %d ###\n", ier_e);
		goto Finish;
	}

	imlogV("Start Decode.\n");

	global_dumpCH();
	/* Loop */
	for(j=0;j<ucFramesPerGroup;j++){
		/*
		float w1_sum = 0;
		float w2_sum = 0;
		float w3_sum = 0;
		float w4_sum = 0;
		float v_sum = 0;		
		float vc = 0;
		float ch1 = 0;
		float ch2 = 0;
		float ch3 = 0;
		float ch4 = 0;		
		float w1 = 0;
		float w2 = 0;
		float w3 = 0;
		float w4 = 0;
		*/
		l = 0;
		/* preparing wave data (1 sec) */
		//imlogV("index=%d v=%d l1=%d l2=%d\n",j,waveform_t[j+sub_index].data[0],waveform_t[j+sub_index].data[64],waveform_t[j+sub_index].data[128]);
		for(k=0;k<WAVE_CHANNEL_MAX;k++){
			int iflag = global_getChFlag(k);
			if(iflag == ADC_CH_OPEN){
				for(i=0;i<ucSamplesPerFrame;i++){
					if(k<WAVE_V1_CHANNEL){
						uint16_t itemp = htomId(waveform_t[j+sub_index].data[i+k*ucSamplesPerFrame]);
						ppusWave[l][i] = (uint16_t)itemp;
					}else{
						ppusWave[l][i] = waveform_t[j+sub_index].data[i+k*ucSamplesPerFrame];
					}
				}
				l++;
			}
			if(l>nChannels){
				imlogV("Channels Out :%d\n",nChannels);
				break;
			}
		}
		
/*		
		for(i = 0; i < ucSamplesPerFrame; i++){
			vc = ((short)ppusWave[ucCurrentChannels][i])*vgain_f;
			//imlogV("vc=%f vgain_f=%f",vc,vgain_f);
			ch1 = ((short)ppusWave[0][i])*igain_f;
			ch2 = ((short)ppusWave[1][i])*igain_f;
			ch3 = ((short)ppusWave[2][i])*igain_f;
			ch4 = ((short)ppusWave[3][i])*igain_f;
			w1_sum += vc*ch1;
			w2_sum += vc*ch2;
			w3_sum += vc*ch3;
			w4_sum += vc*ch4;
			v_sum +=vc;		
		}
		
		w1 = w1_sum/ucSamplesPerFrame;
		w2 = w2_sum/ucSamplesPerFrame;
		w3 = w3_sum/ucSamplesPerFrame;
		w4 = w4_sum/ucSamplesPerFrame;
		
		imlogV("v=%f",v_sum/ucSamplesPerFrame);
		if(w1 != waveform_t[j+sub_index].w1||w2 != waveform_t[j+sub_index].w2){
			imlogV("w1 = %f pw1 = %f ,w2 = %f pw2 = %f\n",w1,waveform_t[j+sub_index].w1,w2,waveform_t[j+sub_index].w2);
			imlogV("w3 = %f pw3 = %f ,w4 = %f pw4 = %f\n",w3,waveform_t[j+sub_index].w3,w4,waveform_t[j+sub_index].w4);
		}
*/
		/* Put data to PLE */
		ier_e = PLEPutData(hPle, (const ple_uint16_t **)ppusWave);
		if(ier_e < 0) {
			imlogE("### Error: PLEPutData: %d ###\n", ier_e);
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
				imlogE("### Error: PLEGetResultSize: %d ###\n", ier_e);
				goto Finish;
			}

			/* Alloc */
			pucItem = (plc_uint8_t *)realloc((void *)pucItem, nItemSize);
			if(pucItem == NULL) {
				imlogE("### Error: Memory Allocation ###\n");
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
				imlogE(" ### Error: PLEGetResult: %d ###\n", ier_e);
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
	imlogV("==================  ENCODE OK ====================\n");

Finish:
	imlogV("Finish \n");
	/* Release */
	PLEFinalize(&hPle);
	imlogV("PLEFinalize \n");
	if(pucItem != NULL) {
		free(pucItem);
	}
	imlogV("free  : pucItem\n");
	if(ppusWave != NULL) {
		for(i=0; i<nChannels; i++) {
			if(ppusWave[i] != NULL) {
				free(ppusWave[i]);
			}
		}
		free(ppusWave);
	}
	imlogV("free  : ppusWave\n");
	//if(encoded_result != NULL) {
	//	free(encoded_result);
	//}
	
	/* Ending */
	return encoded_result;	

}
