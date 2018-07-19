
#include <string.h>  
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>
#include <stdarg.h>

void imlogV(char *format, ...)
{
    int iLen = 0;
    int iFlag = 0;  //释放内存标志
    char *cBuffer = NULL;
    char *pcParameter = NULL;
    time_t timep;
    struct tm *tmp = NULL;
 
    timep = time(NULL);
    tmp = localtime(&timep); //获取当地时间
    
    printf("Debug ");
    printf("[%d-%d-%d ", tmp->tm_year-100+2000,tmp->tm_mon+1,tmp->tm_mday); //输出日期
    printf("%d:%d:%d]", tmp->tm_hour,tmp->tm_min,tmp->tm_sec);  //输出时间
 
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
    
    printf("Error ");
    printf("[%d-%d-%d ", tmp->tm_year-100+2000,tmp->tm_mon+1,tmp->tm_mday); //输出日期
    printf("%d:%d:%d]", tmp->tm_hour,tmp->tm_min,tmp->tm_sec);  //输出时间
 
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
