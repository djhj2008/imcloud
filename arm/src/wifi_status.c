/* ********************************
 * Author:       DJ
 * License:	     NULL
 * Description:  Get Wifi info (MAC RSSI etc)
 *               For usage, check the wifi_status.h file
 *
 *//** @file wifi_status.h *//*
 *
 ********************************/
 
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <netdb.h>		/* gethostbyname, getnetbyname */
#include <net/ethernet.h>	/* struct ether_addr */
#include <net/if.h>			/* for IFNAMSIZ and co... */

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <syslog.h>

#include <linux/ioctl.h>
#include <linux/sockios.h>

#include <signal.h>
#include <fcntl.h>
#include <errno.h>



#include "wifi_status.h"

char  card_name[IFNAMSIZ + 1];

int iw_get_stats(const char *ifname, iwstats *stats)
{
	FILE *	  f = fopen(PROC_NET_WIRELESS, "r");
	char  buf[256];
	char *	  bp;
	int   t;

	if(f==NULL)
		return -1;
	/* Loop on all devices */
	while(fgets(buf,255,f))
	{
		bp=buf;
		while(*bp&&isspace(*bp))
		  bp++;
		/* Is it the good device ? */
		if(strncmp(bp,ifname,strlen(ifname))==0 && bp[strlen(ifname)]==':')
		  {
			/* Skip ethX: */
			bp=strchr(bp,':');
			bp++;
			/* -- status -- */
			bp = strtok(bp, " ");
			sscanf(bp, "%X", &t);
			stats->status = (unsigned short) t;
			/* -- link quality -- */
			bp = strtok(NULL, " ");
			if(strchr(bp,'.') != NULL)
			stats->qual.updated |= 1;
			sscanf(bp, "%d", &t);
			stats->qual.qual = (unsigned char) t;
			/* -- signal level -- */
			bp = strtok(NULL, " ");
			if(strchr(bp,'.') != NULL)
			stats->qual.updated |= 2;
			sscanf(bp, "%d", &t);
			stats->qual.level = (unsigned char) t;
			/* -- noise level -- */
			bp = strtok(NULL, " ");
			if(strchr(bp,'.') != NULL)
			stats->qual.updated += 4;
			sscanf(bp, "%d", &t);
			stats->qual.noise = (unsigned char) t;
			/* -- discarded packets -- */
			bp = strtok(NULL, " ");
			sscanf(bp, "%d", &stats->discard.nwid);
			bp = strtok(NULL, " ");
			sscanf(bp, "%d", &stats->discard.code);
			bp = strtok(NULL, " ");
			sscanf(bp, "%d", &stats->discard.misc);
			fclose(f);
			/* No conversion needed */
			return 0;
		  }
	}
	fclose(f);
	return -1;
}

int get_wifi_info()
{
	iwstats	stats;
	int8_t	dblevel = 0;
	int rc;

	/* Get stats */
	rc = iw_get_stats(card_name, &stats);
	if(rc){
		syslog(LOG_ERR|LOG_DAEMON,"Wifi signal reading error : %d\n",rc);
		dblevel = 0;
	} else {
		dblevel = stats.qual.level;
		if(stats.qual.level >= 64)
			dblevel -= 0x100;
	}
	return dblevel;

}

static inline char *
get_ifname(char *	name,	/* Where to store the name */
	      int	nsize,	/* Size of name buffer */
	      char *	buf)	/* Current position in buffer */
{
	char *	end;

	/* Skip leading spaces */
	while(isspace(*buf))
		buf++;

	/* Get name up to ": "
	* Note : we compare to ": " to make sure to process aliased interfaces
	* properly. Doesn't work on /proc/net/dev, because it doesn't guarantee
	* a ' ' after the ':'*/
	end = strstr(buf, ": ");

	/* Not found ??? To big ??? */
	if((end == NULL) || (((end - buf) + 1) > nsize))
		return(NULL);

	/* Copy */
	memcpy(name, buf, (end - buf));
	name[end - buf] = '\0';

	/* Return value currently unused, just make sure it's non-NULL */
	return(end);
}

int enum_devices(void)
{
	char		buff[1024];
	FILE *	fh;
	int		ret = -1;
	//int		i;
	int		flag = 0;
    	char*   p_get_data;
	/* Check if /proc/net/wireless is available */
	fh = fopen(PROC_NET_WIRELESS, "r");

	if(fh != NULL)
	{
		/* Success : use data from /proc/net/wireless */

		/* Eat 2 lines of header */
		p_get_data = fgets(buff, sizeof(buff), fh);
		if(p_get_data == NULL)
		{

		}

		p_get_data = fgets(buff, sizeof(buff), fh);
		if(p_get_data == NULL)
		{

		}

		/* Read each device line */
		while(fgets(buff, sizeof(buff), fh))
		{
			//char	name[IFNAMSIZ + 1];
			char *s;

			/* Skip empty or almost empty lines. It seems that in some
			* cases fgets return a line with only a newline. */
			if((buff[0] == '\0') || (buff[1] == '\0'))
				continue;

			/* Extract interface name */
			s = get_ifname(card_name, sizeof(card_name), buff);
			//printf("WiFi Card_Name = %s\n", card_name);
			flag = 1;

			if(!s)
			{
				ret = -1;
				goto done;
			} else {
				ret = 0;
				goto done;
			}

		}

		if(!flag){
			ret = -1;
			goto done;
		}
	}else {
		return (-1);
	}
	done:
	fclose(fh);
	return ret;
	
}

int getLocalMac(char * mac_addr)  
{  

    int sock_mac;  
    struct ifreq ifr_mac;  
    
    sock_mac = socket(AF_INET, SOCK_STREAM, 0);  
    if (sock_mac == -1)  
    {  
        perror("create socket falise...mac\n");  
        return -1;  
    }  
  
    memset(&ifr_mac, 0, sizeof(ifr_mac));  
    strncpy(ifr_mac.ifr_name, NETWORK_CARD_NAME, sizeof(ifr_mac.ifr_name) - 1);  
  
    if ((ioctl(sock_mac, SIOCGIFHWADDR, &ifr_mac)) < 0)  
    {  
        printf("mac ioctl error\n");  
        close(sock_mac);  
        return -1;  
    }
  
    close(sock_mac);

    if(ifr_mac.ifr_flags & IFF_RUNNING)
	return -1;
    else{

        sprintf(mac_addr, "%02X%02X%02X%02X%02X%02X",  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[0],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[1],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[2],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[3],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[4],  
            (unsigned char)ifr_mac.ifr_hwaddr.sa_data[5]  
        );  
    	return 0;
    }
} 


