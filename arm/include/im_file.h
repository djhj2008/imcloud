/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IM_FILE_H__
#define __IM_FILE_H__

#define MAX_DIRPATH_LEN 512
#define MAX_BACK_FILE 100
#define CONFIG_FILENAME_LEN 32
#define CONFIG_FILEPATH_LEN 64
#define DEFAULT_DIRPATH "/data"
#define SAVE_DIRPATH "/save"
#define FW_DIRPATH "/fw_update"

#define BUFSIZE     1024*4   

/* =================================== API ======================================= */
int im_scanDir() ;
int im_copyfile(char const *src_path, char const *des_path);
int im_backfile(char* filename);
int im_save_postdata(uint8_t *postdata,int len);
int im_savefile(char* filename,char * buf,int len);
FILE* im_getfile(char* filename);

int im_openfile(char* filename);
int im_savebuff(int fd,char * buf,int len);
void im_close(int fd);
int im_delfile(char *filename);
void get_filename(char * filename);
int get_file_size(const char *path);

void init_crc_table(void);
unsigned int crc32(unsigned int crc, unsigned char * buffer, unsigned int size);  
int calc_img_crc(const char * in_file, unsigned int * img_crc);  
#endif
