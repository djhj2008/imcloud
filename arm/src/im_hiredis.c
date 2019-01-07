/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  Backup Wave List.
 *               For usage, check the im_redis.h file
 *
 *//** @file im_redis.h *//*
 *
 ********************************/


#include <stdio.h>
#include <string.h>
#include <stddef.h>  
#include <stdarg.h>  
#include <string.h>  
#include <assert.h>  
#include <hiredis/hiredis.h> //redis C接口库

#include "im_log.h"
#include "im_hiredis.h"
#include "im_file.h"

redisContext *g_ctx = NULL;

int redis_init()
{    
    redisContext *c = NULL;
    c = redisConnect(REDIS_HOST, REDIS_PORT);
    if (NULL == c || c->err) {
        if(c) {
            imlogV("Redis [%s:%d], Error:[%s]\n", REDIS_HOST, REDIS_PORT, c->errstr);
            redisFree(c);
        } else {
            imlogV("Redis [%s:%d] failure\n", REDIS_HOST, REDIS_PORT);
        }
        return -1;
    }
    g_ctx = c;
    
    return 0;
}

void redis_free()
{
    if (g_ctx) {
        redisFree(g_ctx);
    }
    g_ctx = NULL;
}

int redis_Cmd(const char *cmd)
{
    int i = 0;
    redisReply *r = NULL;
    if (NULL == cmd) {
        return -1;
    }

    imlogV("%s\n", cmd);

    r = (redisReply *)redisCommand(g_ctx, cmd);
    if (NULL == r) {
        imlogE("Error[%d:%s]", g_ctx->err, g_ctx->errstr);
        return -1;
    }

    imlogV("type: %d\n", r->type); 
    switch(r->type) {
    case REDIS_REPLY_STATUS:
        imlogV("type:%s, reply->len:%d reply->str:%s\n", "REDIS_REPLY_STATUS", r->len, r->str);
        break;
    case REDIS_REPLY_ERROR:
        imlogV("type:%s, reply->len:%d reply->str:%s\n", "REDIS_REPLY_ERROR", r->len, r->str);
        break;
    case REDIS_REPLY_INTEGER:
        imlogV("type:%s, reply->integer:%lld\n", "REDIS_REPLY_INTEGER", r->integer);
        break;
    case REDIS_REPLY_NIL:
        imlogV("type:%s, no data\n", "REDIS_REPLY_NIL");
        break;
    case REDIS_REPLY_STRING:
        imlogV("type:%s, reply->len:%d reply->str:%s\n", "REDIS_REPLY_STRING", r->len, r->str);
        break;
    case REDIS_REPLY_ARRAY:
        imlogV("type:%s, reply->elements:%d\n", "REDIS_REPLY_ARRAY", r->elements);
        for (i = 0; i < r->elements; i++) {
            printf("%d: %s\n", i, r->element[i]->str);
        }
        break;
    default:
        imlogE("unkonwn type:%d\n", r->type);
        break;
    }
 
    /*release reply and context */
    freeReplyObject(r); 
    return 0; 
}

/* push to end*/
int im_redis_backup_push(char * name)
{
	char cmd[128]={0x0};
	int ret=-1;
	int len = im_redis_get_backup_len();
	char tmp_name[CONFIG_FILENAME_LEN]={0x0};
	char file[CONFIG_FILEPATH_LEN]={0x0};
	
	imlogV("im_redis_backup_push:%s",name);
	
	if(len >= IM_BACKUP_WAVE_MAX){
		/* del first one*/
		if(im_redis_get_list_head(tmp_name)<0){
			imlogE("im_redis_backup_push Error.");
			return ret;
		}
		sprintf(file,"%s/%s.bak",SAVE_DIRPATH,tmp_name);
		im_delfile(file);
		im_redis_pop_head(tmp_name);
	}
	sprintf(cmd,"RPUSH %s %s",IM_BACKUP_KEY_NAME,name);
	ret = redis_Cmd(cmd);
	
	return ret;
}

int im_redis_get_list_head(char *file)
{
	char cmd[128]={0x0};
	redisReply *r = NULL;
	int ret = -1;
	
	sprintf(cmd,"LINDEX %s 0",IM_BACKUP_KEY_NAME);
    r = (redisReply *)redisCommand(g_ctx, cmd);
    if (NULL == r) {
        imlogE("Error[%d:%s]", g_ctx->err, g_ctx->errstr);
        return ret;
    }
    imlogV("type: %d\n", r->type); 
    if(r->type == REDIS_REPLY_STRING){
        imlogV("reply->str:%s\n", r->str);
        strncpy(file,r->str,r->len);
        ret = 0;
	}
    /*release reply and context */
    freeReplyObject(r);
    
	return ret;
}

int im_redis_pop_head(char * name)
{
	char cmd[128]={0x0};
	int ret=-1;
	
	sprintf(cmd,"LREM %s %d %s",IM_BACKUP_KEY_NAME,IM_BACKUP_WAVE_MAX,name);
	ret = redis_Cmd(cmd);
	
	return ret;	
}

int im_redis_get_backup_len()
{
	char cmd[128]={0x0};
	int len = 0;
	redisReply *r = NULL;
	
	imlogV("im_redis_get_backup_len.");
	
	sprintf(cmd,"LLEN %s",IM_BACKUP_KEY_NAME);
    r = (redisReply *)redisCommand(g_ctx, cmd);
    if (NULL == r) {
        imlogE("Error[%d:%s]", g_ctx->err, g_ctx->errstr);
        return -1;
    }
    imlogV("type: %d\n", r->type); 
    if(r->type == REDIS_REPLY_INTEGER){
        imlogV("reply->integer:%lld\n", r->integer);
        len = r->integer;
	}
    /*release reply and context */
    freeReplyObject(r); 
    
    imlogV("im_redis_get_backup_len End.");
    
    return len; 	
}

int im_redis_backup_dump()
{
	char cmd[128]={0x0};
	int len = im_redis_get_backup_len();
	int ret=-1;
	
	if(len == 0 )
		return ret;	
	
	sprintf(cmd,"LRANGE %s 0 %d",IM_BACKUP_KEY_NAME,len);
	ret = redis_Cmd(cmd);
	return ret;	
}




