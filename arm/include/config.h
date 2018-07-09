/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IM_CONFIG_H__
#define __IM_CONFIG_H__

#define FRAMES_GROUP_MAX 300
#define SAMPLES_FRAME 64
#define ADC_SAMPLE_CHANNEL 6 //driver adc channel count

enum ADC_HW_CHANNEL{
	ADC_V1_CHANNEL=0,
	ADC_L1_CHANNEL,
	ADC_L2_CHANNEL,
	ADC_L3_CHANNEL,
	ADC_L4_CHANNEL,
	ADC_V2_CHANNEL
};

enum WAVE_SW_CHANNEL{
	WAVE_L1_CHANNEL=0,
	WAVE_L2_CHANNEL,
	WAVE_L3_CHANNEL,
	WAVE_L4_CHANNEL,
	WAVE_V1_CHANNEL,
	WAVE_V2_CHANNEL
};

#define ADC_CH_OPEN 	1
#define ADC_CH_CLOSE 	0

#define ADC_V_CHANNELS 2 //upload server V channel count
#define ADC_L_CHANNELS 4 //upload server I channel count

#define AC_LINE_FREQUENCY 60 //50Hz or 60Hz

#define PLE_FRAME_MAX 255
	
/* =================================== API ======================================= */

#endif

