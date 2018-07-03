
/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  File creat read write.
 *               For usage, check the im_file.h file
 *
 *//** @file im_file.h *//*
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

#include "im_file.h"

#define MAX_DIRPATH_LEN 512
#define DEFAULT_DIRPATH "./data"
#define SAVE_DIRPATH "./save"

#define MAX_BACK_FILE 10

int im_getfilenum()
{	
	char src_path[MAX_DIRPATH_LEN]={0x0};
	int i = 0;
	int fd;
	int ret = 0;
	for(i=0;i<MAX_BACK_FILE;i++){
		sprintf(src_path,"%s/%d.bak",SAVE_DIRPATH,i);
		fd = open(src_path,O_RDWR);
		if(fd < 0){
			ret = i;
			break;
		}
	}
	return ret;
}

int im_openfile(char* filename)
{
	struct stat file_stat;
    int ret;
	char dirpath[MAX_DIRPATH_LEN]={0x0};
	char src_path[MAX_DIRPATH_LEN]={0x0};
	int fd;
	
    //下面语句是建立默认文件夹的路径
    strcpy(dirpath, DEFAULT_DIRPATH);//默认的路径为data
   
    ret = stat(dirpath, &file_stat);//检查文件夹状态
    if(ret<0)
    {
        if(errno == ENOENT)//是否已经存在该文件夹
        {
            ret = mkdir(dirpath, 0775);//创建文件夹
            printf("creat dir %s \n", dirpath);
            if(ret < 0)
            {
                printf("Could not create directory %s \n",
					dirpath);
				return EXIT_FAILURE;
            }
 
        }
        else
        {
            printf("bad file path\n");
            return EXIT_FAILURE;
        }
    }
    sprintf(src_path,"%s/%s",dirpath,filename);
    fd = open(src_path,O_RDWR|O_CREAT,0744);
    if(fd > 0 ){
		return fd;
	}
	return -1;
}

int im_savebuff(int fd,char * buf,int len)
{
	int sub_len = len;
	int count = 0;
	while(sub_len>0){
		count = write(fd,buf,sub_len);
		printf("savebuff count:%d\n",count);
		sub_len =sub_len - count;
	}
	return sub_len;
}

void im_close(int fd)
{
	close(fd);
}


int im_savefile(char* filename,char * buf,int len)
{
    struct stat file_stat;
    int ret;
	char dirpath[MAX_DIRPATH_LEN]={0x0};
	char src_path[MAX_DIRPATH_LEN]={0x0};
	int fd;
	
    //下面语句是建立默认文件夹的路径
    strcpy(dirpath, DEFAULT_DIRPATH);//默认的路径为data
   
    ret = stat(dirpath, &file_stat);//检查文件夹状态
    if(ret<0)
    {
        if(errno == ENOENT)//是否已经存在该文件夹
        {
            ret = mkdir(dirpath, 0775);//创建文件夹
            printf("creat dir %s \n", dirpath);
            if(ret < 0)
            {
                printf("Could not create directory %s \n",
					dirpath);
				return EXIT_FAILURE;
            }
 
        }
        else
        {
            printf("bad file path\n");
            return EXIT_FAILURE;
        }
    }
    sprintf(src_path,"%s/%s",dirpath,filename);
    fd = open(src_path,O_RDWR|O_CREAT|O_APPEND,0744);
    if(fd>0){
		int sub_len = len;
		int count = 0;
		while(sub_len>0){
			count = write(fd,buf,sub_len);
			printf("write file:%s ,count:%d\n",src_path,count);
			sub_len =sub_len - count;
		}
	}
    close(fd);
    
	return 0;
}

int im_backfile(char* filename)
{
	char src_path[MAX_DIRPATH_LEN]={0x0};
	char des_path[MAX_DIRPATH_LEN]={0x0};
	struct stat file_stat;
    int ret;
	char dirpath[MAX_DIRPATH_LEN]={0x0};
	int file_num=0;
	
    //下面语句是建立默认文件夹的路径
    strcpy(dirpath, SAVE_DIRPATH);//默认的路径为data
   
    ret = stat(dirpath, &file_stat);//检查文件夹状态
    if(ret<0)
    {
        if(errno == ENOENT)//是否已经存在该文件夹
        {
            ret = mkdir(dirpath, 0775);//创建文件夹
            printf("creat dir :%s \n", dirpath);
            if(ret < 0)
            {
                printf("Could not create directory %s \n",
					dirpath);
				return EXIT_FAILURE;
            }
 
        }
        else
        {
            printf("bad file path\n");
            return EXIT_FAILURE;
        }
    }
	
	file_num = im_getfilenum();
	printf("get free file num %d\n",file_num);
	sprintf(src_path,"%s/%s",DEFAULT_DIRPATH,filename);
	sprintf(des_path,"%s/%d.bak",SAVE_DIRPATH,file_num);
	
	ret = im_copyfile(src_path,des_path);
	remove(src_path);
	
	return ret;
}

int im_scanDir()  
{  
  
	DIR *dp;  
	struct dirent *entry;  
	struct stat statbuf;
	char dirpath[MAX_DIRPATH_LEN]={0x0};
	char dirback[MAX_DIRPATH_LEN]={0x0};
	int ret;
	char buf[MAX_DIRPATH_LEN];
	
printf("chdir %s\n",getcwd(dirback,MAX_DIRPATH_LEN));
	
    strcpy(dirpath, SAVE_DIRPATH);
    
	if ((dp = opendir(dirpath)) == NULL)  
	{  
		fprintf(stderr, "cannot open directory: %s\n", dirpath);  
		return -1;  
	}

	ret = chdir (dirpath);
	if(ret == -1){
		printf("chdir Save Error.\n");
		return -1;
	}
 printf("chdir %s\n",getcwd(buf,MAX_DIRPATH_LEN));
	while ((entry = readdir(dp)) != NULL)  
	{  
		lstat(entry->d_name, &statbuf);  
		if (S_ISREG(statbuf.st_mode))  
		{  
			//remove(entry->d_name); 
			printf("ScanDir:%s\n",entry->d_name);
		}  
	}  
	
	ret = chdir (dirback);
	if(ret == -1){
		printf("chdir Home Error.\n");
		return -1;
	}

printf("chdir %s\n",getcwd(buf,MAX_DIRPATH_LEN));
	return 0;  
}  
    


int im_copyfile(char const *src_path, char const *des_path)
{
	char buff[1024];
	int fd,fd2;
	int len,w_len;
	fd = open(src_path,O_RDWR);
	fd2 = open(des_path,O_RDWR|O_CREAT,0744);
	while((len = read(fd,buff,1024)))
	{
		w_len = write(fd2,buff,len);
		if(w_len!=len){
			printf("im_copyfile Error:%d.\n",w_len);
			return -1;
		}
	}
	close(fd);
	close(fd2);
	return 0;
}





