/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  File creat read write.
 *               For usage, check the im_file.h file
 *
 *//** @file im_file.h *//*
 *
 ********************************/
#include <stdint.h>
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

#include "im_log.h"
#include "im_file.h"
#include "im_hiredis.h"

int get_file_size(const char *path)  
{  
    int filesize = -1;
    struct stat statbuff;  
    if(stat(path, &statbuff) < 0){  
        return filesize;  
    }else{  
        filesize = (int)statbuff.st_size;  
    }  
    return filesize;  
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
            imlogE("creat dir %s \n", dirpath);
            if(ret < 0)
            {
                imlogE("Could not create directory %s \n",
					dirpath);
				return EXIT_FAILURE;
            }
 
        }
        else
        {
            imlogE("bad file path\n");
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
		imlogV("savebuff count:%d\n",count);
		sub_len =sub_len - count;
	}
	return sub_len;
}

void im_close(int fd)
{
	close(fd);
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
	imlogV("get_filename:%s\n",filename);
	
}

FILE* im_getfile(char* filename)
{
	char dirpath[MAX_DIRPATH_LEN]={0x0};
	char src_path[MAX_DIRPATH_LEN]={0x0};
	FILE *fd;
    
    strcpy(dirpath, DEFAULT_DIRPATH);//默认的路径为data
    sprintf(src_path,"%s/%s",dirpath,filename);
    fd = fopen(src_path,"r");
    
	return fd;
}


int im_savefile(char* filename,char *buf,int len)
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
            imlogE("creat dir %s \n", dirpath);
            if(ret < 0)
            {
                imlogE("Could not create directory %s \n",
					dirpath);
				return EXIT_FAILURE;
            }
 
        }
        else
        {
            imlogE("bad file path\n");
            return EXIT_FAILURE;
        }
    }
    sprintf(src_path,"%s/%s",dirpath,filename);
    fd = open(src_path,O_RDWR|O_CREAT|O_APPEND,0644);
    if(fd>0){
		int sub_len = len;
		int count = 0;
		while(sub_len>0){
			count = write(fd,buf+(len-sub_len),sub_len);
			imlogV("write file:%s ,count:%d\n",src_path,count);
			sub_len =sub_len - count;
		}
	}
    close(fd);
    
	return 0;
}

int im_delfile(char *filename){
	//char src_path[MAX_DIRPATH_LEN]={0x0};
	
	//sprintf(src_path,"%s/%s",DEFAULT_DIRPATH,filename);
	
	imlogV("im_delfile : %s\n",filename);
	
	remove(filename);
	
	return 0;
	
}

int im_save_postdata(uint8_t *postdata,int len)
{
	char des_path[MAX_DIRPATH_LEN]={0x0};
	struct stat file_stat;
    int ret;
	char dirpath[MAX_DIRPATH_LEN]={0x0};
	char file_back[64]={0x0};
	int fd;
	
    //下面语句是建立默认文件夹的路径
    strcpy(dirpath, SAVE_DIRPATH);//默认的路径为data
   
    ret = stat(dirpath, &file_stat);//检查文件夹状态
    if(ret<0)
    {
        if(errno == ENOENT)//是否已经存在该文件夹
        {
            ret = mkdir(dirpath, 0775);//创建文件夹
            imlogE("creat dir :%s \n", dirpath);
            if(ret < 0)
            {
                imlogE("Could not create directory %s \n",
					dirpath);
				return EXIT_FAILURE;
            }
 
        }
        else
        {
            imlogE("bad file path\n");
            return EXIT_FAILURE;
        }
    }
	
	get_filename(file_back);
	
	sprintf(des_path,"%s/%s.bak",SAVE_DIRPATH,file_back);
	
	fd = open(des_path,O_RDWR|O_CREAT|O_APPEND,0644);
    if(fd>0){
		int sub_len = len;
		int count = 0;
		while(sub_len>0){
			count = write(fd,postdata+(len-sub_len),sub_len);
			imlogV("write file:%s ,count:%d\n",des_path,count);
			sub_len =sub_len - count;
			ret = 0;
		}
	}
	
	if(ret == 0 ){
		im_redis_backup_push(file_back);
	}
	return ret;
}


int im_backfile(char* filename)
{
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
            imlogE("creat dir :%s \n", dirpath);
            if(ret < 0)
            {
                imlogE("Could not create directory %s \n",
					dirpath);
				return EXIT_FAILURE;
            }
 
        }
        else
        {
            imlogE("bad file path\n");
            return EXIT_FAILURE;
        }
    }
	
	get_filename(file_back);
	
	sprintf(des_path,"%s/%s.bak",SAVE_DIRPATH,file_back);
	
	ret = im_copyfile(filename,des_path);
	if(ret == 0 ){
		im_redis_backup_push(file_back);
		im_delfile(filename);
	}
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
	
	imlogV("chdir %s\n",getcwd(dirback,MAX_DIRPATH_LEN));
	
    strcpy(dirpath, SAVE_DIRPATH);
    
	if ((dp = opendir(dirpath)) == NULL)  
	{  
		imlogE("cannot open directory: %s\n", dirpath);  
		return -1;  
	}

	ret = chdir (dirpath);
	if(ret == -1){
		imlogE("chdir Save Error.\n");
		return -1;
	}
	imlogV("chdir %s\n",getcwd(buf,MAX_DIRPATH_LEN));
	while ((entry = readdir(dp)) != NULL)  
	{  
		lstat(entry->d_name, &statbuf);  
		if (S_ISREG(statbuf.st_mode))  
		{  
			//remove(entry->d_name); 
			imlogV("ScanDir:%s\n",entry->d_name);
		}  
	}  
	
	ret = chdir (dirback);
	if(ret == -1){
		imlogE("chdir Home Error.\n");
		return -1;
	}

	imlogV("chdir %s\n",getcwd(buf,MAX_DIRPATH_LEN));
	return 0;  
}  
    


int im_copyfile(char const *src_path, char const *des_path)
{
	char buff[1024];
	int fd,fd2;
	int len,w_len;
	fd = open(src_path,O_RDWR);
	fd2 = open(des_path,O_RDWR|O_CREAT,0644);
	
	if(fd < 0|| fd2 < 0)
	{
		imlogE("im_copyfile open Error.\n");
		return -1;
	}
	
	while((len = read(fd,buff,1024)))
	{
		w_len = write(fd2,buff,len);
		if(w_len!=len){
			imlogE("im_copyfile Error:%d.\n",w_len);
			return -1;
		}
	}
	close(fd);
	close(fd2);
	return 0;
}





