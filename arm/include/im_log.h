/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/
#ifndef __IM_LOG_H__
#define __IM_LOG_H__

#ifndef LOG_LEVEL
#define LOG_LEVEL 1
#endif

/* =================================== API ======================================= */
void imlogV(char *format, ...);
void imlogE(char *format, ...);

#endif
