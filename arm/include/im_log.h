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

#define IM_DEBUG_ON 	1
#define IM_DEBUG_OFF 	0

/* =================================== API ======================================= */
void imlogV(char *format, ...);
void imlogE(char *format, ...);
void setDebugOnOff(int debug_on);

#endif
