/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/
#ifndef __IMCLOUD_CONTROLLER_H__
#define __IMCLOUD_CONTROLLER_H__

#define IMCLOUD_SERVER_MESSAGE		"message"
#define IMCLOUD_SERVER_CODE			"code"

#define SERVER_BAD_REQUEST			"BadRequest"
#define SERVER_NOT_ALLOWED			"ActivationNotAllowed"
#define SERVER_SIG_NOT_MATCH		"SignatureDoesNotMatch"
#define SERVER_CONNECT_ERROR		"ConnectionError"
#define SERVER_INVALID_KEY			"InvalidAccessKey"

enum SERVER_CODE_ERROR{
	STATUS_OK = 0,
	HTTP_ERROR = -1,
	NORMAL_ERROR = -2,
	INVALID_KEY = -3,
}; 

/*Interface Get Access Key*/
#define IMCLOUD_ACCESSKEY_TITLE			"status"
#define IMCLOUD_ACCESSKEY_CONTENT		"access_key"
#define IMCLOUD_ACCESSKEY_STATUS_OK		"ok"
#define IMCLOUD_ACCESSKEY_STATUS_FAIL	"fail"
#define IMCLOUD_ACCESSKEY_NOT_ALLOWED	"ActivationNotAllowed"
#define IMCLOUD_ACCESSKEY_SIG_NOTMATCH	"SignatureDoesNotMatch"

/*Interface Info*/
#define IMCLOUD_INFO_TITLE			"status"
#define IMCLOUD_INFO_ICH_CONTENT	"i_channels"
#define IMCLOUD_INFO_VCH_CONTENT	"v_channels"
#define IMCLOUD_INFO_RESULT			"ok"

enum IMCOULD_I_CHANNELS {
	I_CHANNELS_1 = 1,
	I_CHANNELS_2,
	I_CHANNELS_3,
	I_CHANNELS_4
};

enum IMCOULD_V_CHANNELS {
	V_CHANNELS_1 = 1,
	V_CHANNELS_2
};

/*Interface Data Upload*/
#define IMCLOUD_DATA_TITLE					"request"

#define IMCOULD_DATA_NONE_CMD				"none"
#define IMCOULD_DATA_FW_UPDATE_CMD			"fw_update"
#define IMCOULD_DATA_REBOOT_CMD				"reboot"
#define IMCOULD_DATA_INTERVAL_CMD			"interval_change"
#define IMCOULD_DATA_LOGLEVEL_CMD			"loglevel_change"
#define IMCOULD_DATA_WAVEUPLOAD_CMD			"waveupload_change"
#define IMCOULD_DATA_SSID_CMD				"ssid_change"

#define IMCLOUD_DATA_FW_VERSION_CONTENT		"fw_version"
#define IMCLOUD_DATA_FW_DOMAIN_CONTENT		"domain"
#define IMCLOUD_DATA_FW_CHECKSUM_CONTENT	"checksum"
#define IMCLOUD_DATA_FW_SIZE_CONTENT		"size"

#define IMCLOUD_DATA_INTERVAL_CONTENT	"interval"

#define IMCLOUD_DATA_LOGLEVEL_CONTENT	"loglevel"

#define IMCLOUD_DATA_WAVEUPLOAD_CONTENT	"upload"

#define IMCLOUD_DATA_SSID_CUR_SSID_CONTENT	"current_ssid"
#define IMCLOUD_DATA_SSID_SECURITY_CONTENT	"new_security"
#define IMCLOUD_DATA_SSID_NEW_SSID_CONTENT	"new_ssid"
#define IMCLOUD_DATA_SSID_NEW_KEY_CONTENT	"new_key"

enum IMCOULD_CMD {
	IMCLOUD_CMD_NONE=0,
	IMCLOUD_CMD_FW_UPDATE,
	IMCLOUD_CMD_REBOOT,
	IMCLOUD_CMD_INTERVAL_CHANGE,
	IMCLOUD_CMD_LOGLEVEL_CHANGE,
	IMCLOUD_CMD_WAVEUPLOAD_CHANGE,
	IMCLOUD_CMD_SSID_CHANGE,
	IMCLOUD_CMD_MAX,
};

/* =================================== API ======================================= */
int CloudAccessKeyHandle(char * buf);
int GenerateInfoData(char * postdata);
int CloudInfoHandle(char * buf);
int CloudDataHandle(char * buf);
int CloudGetCMD(char * req);
uint32_t HEX2int(char *pcBCD);
#endif
