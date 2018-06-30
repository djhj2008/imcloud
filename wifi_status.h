/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __WIFI_STATUS_H_
#define __WIFI_STATUS_H_

#define MY_VERSION      "1.0.1"
#define PROC_NET_WIRELESS	"/proc/net/wireless"
#define PROC_SIGNAL_LED		"/sys/class/leds/hps_led"
#define NETWORK_CARD_NAME	"enp4s0"

/*
 *	Quality of the link
 */
struct	iw_quality
{
	__u8		qual;		/* link quality (%retries, SNR or better...) */
	__u8		level;		/* signal level */
	__u8		noise;		/* noise level */
	__u8		updated;	/* Flags to know if updated */
};

/*
 *	Packet discarded in the wireless adapter due to
 *	"wireless" specific problems...
 */
struct	iw_discarded
{
	__u32		nwid;		/* Wrong nwid */
	__u32		code;		/* Unable to code/decode */
	__u32		misc;		/* Others cases */
};

struct	iw_statistics
{
	__u16		status;		/* Status
					 * - device dependent for now */

	struct iw_quality	qual;		/* Quality of the link
						 * (instant/mean/max) */
	struct iw_discarded	discard;	/* Packet discarded counts */
};

typedef struct iw_statistics	iwstats;


/* =================================== API ======================================= */

int enum_devices(void);
int getLocalMac(char * mac_addr);
int get_wifi_info();


#endif
