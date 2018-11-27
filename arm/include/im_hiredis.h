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
int im_redis_backup_dump();
int im_redis_get_backup_len();
int im_redis_pop_head(char * name);
int im_redis_get_list_head(char *file);
int im_redis_backup_push(char * name);
void redis_free();
int redis_init();
#endif
