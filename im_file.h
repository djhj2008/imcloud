/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IM_FILE_H__
#define __IM_FILE_H__

/* =================================== API ======================================= */
int im_scanDir() ;
int im_copyfile(char const *src_path, char const *des_path);
int im_backfile(char* filename);
int im_savefile(char* filename,char * buf,int len);

int im_openfile(char* filename);
int im_savebuff(int fd,char * buf,int len);
void im_close(int fd);

#endif
