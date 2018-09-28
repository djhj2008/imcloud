/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  ADC Led interface
 *               For usage, check the led_tool.h file
 *
 *//** @file led_tool.h *//*
 *
 ********************************/
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include "led_tool.h"
#include "im_log.h"

uint8_t adc_led[ADC7606_SKU_LED_COUNT] = {
	ADC7606_LED_RED,
	ADC7606_LED_RED,
	ADC7606_LED_RED,
	ADC7606_LED_RED
	};

void led_init()
{
	int i =0;
    
	for(i=0;i<ADC7606_SKU_LED_COUNT;i++){
		led_ctrl_ADC7606_ct(i,adc_led[i]);
	}
}

int led_ctrl_ADC7606_ct(int index,int on_off)
{
    FILE *f;
    char path[128];
    
	adc_led[index]=on_off;
	sprintf(path, "%s%d/%s", SYS_WIFI_SIGNAL_LED, index+ADC7606_SKU_LED_S, "brightness");

	f = fopen(path, "w");
	if(f != NULL)
	{
		fprintf(f, "%d",on_off);
		fclose(f);
	}
	else
	{
		imlogE("path is error:%s\n",path);
		return 1;
	}
    

    return 0;
}

int led_ctrl_ADC7606_ct_direction(int index,int on_off)
{
	imlogV("LED %d : %d %d",index,adc_led[index],on_off);
    
    /*
    if(on_off==adc_led[index]){
		return 0;
	}
	*/

	led_ctrl_ADC7606_ct(index,on_off);
	
	return 0;
}
