/*************************************************
Copyright (C), 2018-2019, Tech. Co., Ltd.
File name: im_log.c
Author:doujun
Version:1.0
Date:2018-07-19
Description: Main Fuction.
* print debug log times
Others: NULL
Function List:
* 1.imlogV
* 2.imlogE
* 
* @file im_log.h 
* 
*************************************************/
#include <string.h>  
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>

#include "im_log.h"

int debug_level = 1;

void setDebugOnOff(int debug_on)
{
	debug_level = debug_on;
	printf("setDebugOnOff:%d",debug_on);
}


void imlogV(char *format, ...)
{
    int iLen = 0;
    int iFlag = 0;  //释放内存标志
    char *cBuffer = NULL;
    char *pcParameter = NULL;
    time_t timep;
    struct tm *tmp = NULL;
 
	if(IM_DEBUG_OFF==debug_level){
		return;
	}
 
    timep = time(NULL);
    tmp = localtime(&timep); //获取当地时间
    
    printf("[%04d-%02d-%02d ", tmp->tm_year-100+2000,tmp->tm_mon+1,tmp->tm_mday); //输出日期
    printf("%02d:%02d:%02d]", tmp->tm_hour,tmp->tm_min,tmp->tm_sec);  //输出时间
    printf("Debug ");
 
    pcParameter = format;
    
    iLen = strlen(format);
    if('\n' != format[iLen-1]) //判断字符串最后一个字符是不是\n
    {
        iFlag = 1; 
        cBuffer = (char*)malloc(iLen+2);
        strncpy(cBuffer, format, iLen);
        cBuffer[iLen] = '\n';  //倒数第二位写为\n
        cBuffer[iLen+1] = '\0'; //倒数第一位写为\0
        pcParameter = cBuffer; 
    }
         
    va_list ap;
    va_start(ap, format);    
    vprintf(pcParameter, ap);    
    va_end(ap);
 
    if(1 == iFlag)
    {
        free(cBuffer);
    }
}

void imlogE(char *format, ...)
{
    int iLen = 0;
    int iFlag = 0;  //释放内存标志
    char *cBuffer = NULL;
    char *pcParameter = NULL;
    time_t timep;
    struct tm *tmp = NULL;
 
    timep = time(NULL);
    tmp = localtime(&timep); //获取当地时间
    
    printf("[%d-%d-%d ", tmp->tm_year-100+2000,tmp->tm_mon+1,tmp->tm_mday); //输出日期
    printf("%d:%d:%d]", tmp->tm_hour,tmp->tm_min,tmp->tm_sec);  //输出时间
    printf("Error ");
    
    pcParameter = format;
    
    iLen = strlen(format);
    if('\n' != format[iLen-1]) //判断字符串最后一个字符是不是\n
    {
        iFlag = 1; 
        cBuffer = (char*)malloc(iLen+2);
        strncpy(cBuffer, format, iLen);
        cBuffer[iLen] = '\n';  //倒数第二位写为\n
        cBuffer[iLen+1] = '\0'; //倒数第一位写为\0
        pcParameter = cBuffer; 
    }
         
    va_list ap;
    va_start(ap, format);    
    vprintf(pcParameter, ap);    
    va_end(ap);
 
    if(1 == iFlag)
    {
        free(cBuffer);
    }
}
