/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IM_FILE_H__
#define __IM_FILE_H__

#define MAX_DIRPATH_LEN 512
#define MAX_BACK_FILE 10
#define DEFAULT_DIRPATH "./data"
#define SAVE_DIRPATH "./save"

/* =================================== API ======================================= */
int im_scanDir() ;
int im_copyfile(char const *src_path, char const *des_path);
int im_backfile(char* filename);
int im_savefile(char* filename,char * buf,int len);
#endif
