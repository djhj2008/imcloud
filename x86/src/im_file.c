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
#include <time.h>
#include <sys/time.h>

#include "im_file.h"

void dumpconfig()
{
	char filename[MAX_BACK_FILE][MAX_DIRPATH_LEN]={0x0};
	int fd;
	int count;
	int i=0;
	char dirpath[MAX_DIRPATH_LEN]={0x0};
	
	sprintf(dirpath,"%s/config.dat",SAVE_DIRPATH);
	fd = open(dirpath,O_RDWR|O_CREAT,0644);
	if(fd<0){
		printf("file open error.\n");
		return;
	}
	
	
	count = read(fd,filename,MAX_BACK_FILE*MAX_DIRPATH_LEN);
	if(count<0){
		printf("file read error.\n");
		return;
	}
	
	for(i=0;i<MAX_BACK_FILE;i++){
		
			printf("file:%d name is %s.\n",i,filename[i]);
		
	}
	close(fd);
}


int im_saveconfig(char * file)
{	
	char filename[MAX_BACK_FILE][MAX_DIRPATH_LEN]={0x0};
	char temp[MAX_BACK_FILE][MAX_DIRPATH_LEN]={0x0};
	char dirpath[MAX_DIRPATH_LEN]={0x0};
	int i=0;
	int flag = 0;
	int fd;
	int count;
	
	printf("saveconfig file :%s \n",file);
	
	sprintf(dirpath,"%s/config.dat",SAVE_DIRPATH);
	fd = open(dirpath,O_RDWR|O_CREAT,0644);
	if(fd<0){
		printf("file open error.\n");
		return -1;
	}
	
	
	count = read(fd,filename,MAX_BACK_FILE*MAX_DIRPATH_LEN);
	if(count<0){
		printf("file read error.\n");
		return -1;
	}
	close(fd);
	
	printf("file read %d.\n",count);
	
	fd = open(dirpath,O_RDWR|O_CREAT,0644);
	for(i=0;i<MAX_BACK_FILE;i++){
		if(strlen((const char *)filename[i])==0){
			strcpy(filename[i],file);
			printf("save file:%d\n",i);
			flag = 1;
			break;
		}else{
			//printf("save file:%d %s\n",i,filename[i]);
		}
	}
	if(flag == 0){
		memcpy(temp,filename[1],MAX_DIRPATH_LEN*(MAX_BACK_FILE-1));
		strcpy(temp[MAX_BACK_FILE-1],file);
		count = write(fd,temp,MAX_BACK_FILE*MAX_DIRPATH_LEN);
		if(count < 0)
		{
			printf("file write error.\n");
			return -1;	
		}
	}else{
		count = write(fd,filename,MAX_BACK_FILE*MAX_DIRPATH_LEN);
		if(count < 0)
		{
			printf("file write error.\n");
			return -1;	
		}
	}
	close(fd);
 
	dumpconfig();
	return -1;
	
}

void get_filename(char * filename)
{
	struct tm *t;
	time_t tt;
	int random;
	struct timeval tpstart;
	
	gettimeofday(&tpstart,NULL);
	srand(tpstart.tv_usec);
    random=1+(int)(999.0*rand()/(RAND_MAX+1.0));
    
	time(&tt);
	t = localtime(&tt);
	sprintf(filename,"%4d%02d%02d_%02d%02d%02d_%03d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday, t->tm_hour, t->tm_min, t->tm_sec,random);
	printf("get_filename:%s\n",filename);
	
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
    fd = open(src_path,O_RDWR|O_CREAT|O_APPEND,0644);
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
	char file_back[64]={0x0};
	
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
	
	get_filename(file_back);
	
	sprintf(src_path,"%s/%s",DEFAULT_DIRPATH,filename);
	sprintf(des_path,"%s/%s.bak",SAVE_DIRPATH,file_back);
	
	ret = im_copyfile(src_path,des_path);
	im_saveconfig(file_back);
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
	fd2 = open(des_path,O_RDWR|O_CREAT,0644);
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





