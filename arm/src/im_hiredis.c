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

#include "im_hiredis.h"

redisContext *g_ctx = NULL;

int redis_init()
{    
    redisContext *c = NULL;
    c = redisConnect(REDIS_HOST, REDIS_PORT);
    if (NULL == c || c->err) {
        if(c) {
            printf("Redis [%s:%d], Error:[%s]\n", REDIS_HOST, REDIS_PORT, c->errstr);
            redisFree(c);
        } else {
            printf("Redis [%s:%d] failure\n", REDIS_HOST, REDIS_PORT);
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

    printf("%s\n", cmd);

    r = (redisReply *)redisCommand(g_ctx, cmd);
    if (NULL == r) {
        printf("Error[%d:%s]", g_ctx->err, g_ctx->errstr);
        return -1;
    }

    printf("type: %d\n", r->type); 
    switch(r->type) {
    case REDIS_REPLY_STATUS:
        printf("type:%s, reply->len:%d reply->str:%s\n", "REDIS_REPLY_STATUS", r->len, r->str);
        break;
    case REDIS_REPLY_ERROR:
        printf("type:%s, reply->len:%d reply->str:%s\n", "REDIS_REPLY_ERROR", r->len, r->str);
        break;
    case REDIS_REPLY_INTEGER:
        printf("type:%s, reply->integer:%lld\n", "REDIS_REPLY_INTEGER", r->integer);
        break;
    case REDIS_REPLY_NIL:
        printf("type:%s, no data\n", "REDIS_REPLY_NIL");
        break;
    case REDIS_REPLY_STRING:
        printf("type:%s, reply->len:%d reply->str:%s\n", "REDIS_REPLY_STRING", r->len, r->str);
        break;
    case REDIS_REPLY_ARRAY:
        printf("type:%s, reply->elements:%d\n", "REDIS_REPLY_ARRAY", r->elements);
        for (i = 0; i < r->elements; i++) {
            printf("%d: %s\n", i, r->element[i]->str);
        }
        break;
    default:
        printf("unkonwn type:%d\n", r->type);
        break;
    }
 
    /*release reply and context */
    freeReplyObject(r); 
    return 0; 
}

/* push to head*/
int im_backup_push(char * name)
{
	char cmd[128]={0x0};
	int ret=-1;
	
	int len = im_get_backup_len();
	if(len >= IM_BACKUP_WAVE_MAX){
		/* del last one*/
		sprintf(cmd,"RPOP %s",IM_BACKUP_KEY_NAME);
	}
	sprintf(cmd,"LPUSH %s %s",IM_BACKUP_KEY_NAME,name);
	ret = redis_Cmd(cmd);
	return ret;
}

int im_get_backup_len()
{
	char cmd[128]={0x0};
	int len = 0;
	
	redisReply *r = NULL;
	sprintf(cmd,"LLEN %s",IM_BACKUP_KEY_NAME);
    r = (redisReply *)redisCommand(g_ctx, cmd);
    if (NULL == r) {
        printf("Error[%d:%s]", g_ctx->err, g_ctx->errstr);
        return -1;
    }
    printf("type: %d\n", r->type); 
    if(r->type == REDIS_REPLY_INTEGER){
        printf("reply->integer:%lld\n", r->integer);
        len = r->integer;
	}
    /*release reply and context */
    freeReplyObject(r); 
    return len; 	
}

int im_backup_dump()
{
	char cmd[128]={0x0};
	int len = im_get_backup_len();
	int ret=-1;
	
	sprintf(cmd,"LRANGE %s 0 %d",IM_BACKUP_KEY_NAME,len);
	ret = redis_Cmd(cmd);
	return ret;	
}




