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
#endif
