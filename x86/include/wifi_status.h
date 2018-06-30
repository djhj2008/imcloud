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
	uint8_t		qual;		/* link quality (%retries, SNR or better...) */
	uint8_t		level;		/* signal level */
	uint8_t		noise;		/* noise level */
	uint8_t		updated;	/* Flags to know if updated */
};

/*
 *	Packet discarded in the wireless adapter due to
 *	"wireless" specific problems...
 */
struct	iw_discarded
{
	uint32_t		nwid;		/* Wrong nwid */
	uint32_t		code;		/* Unable to code/decode */
	uint32_t		misc;		/* Others cases */
};

struct	iw_statistics
{
	uint16_t		status;		/* Status
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
