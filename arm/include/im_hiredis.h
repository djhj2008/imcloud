/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __IM_HIREDIS_H__
#define __IM_HIREDIS_H__



#define REDIS_HOST        "127.0.0.1"
#define REDIS_PORT        6379

#define IM_BACKUP_WAVE_MAX	1000
#define IM_BACKUP_KEY_NAME  "wave_backup"





/* =================================== API ======================================= */
int redis_test(const char *cmd);
int redis_init();
void redis_free();
int im_backup_push(char * name);
int im_get_backup_len();
int im_backup_dump();


#endif
