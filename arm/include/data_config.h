/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __DATA_CONFIG_H__
#define __DATA_CONFIG_H__

#define DATA_FILE "/etc/imcloud/data_config.cfg"

struct data_config_info {
	unsigned char   reg;
	unsigned short  offset;
	unsigned char   len;
};

int init_data_config(struct data_config_info *data);
void adc_calibration();
#endif
