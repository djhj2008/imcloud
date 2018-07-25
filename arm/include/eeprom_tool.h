/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __EEPROM_TOOL__
#define __EEPROM_TOOL__

#define PAGE_SIZE 64
#define TIMEOUT	3
#define RETRY	3
#define I2C_DEFAULT_TIMEOUT		1
#define I2C_DEFAULT_RETRY		3

#define EEPROM_SLAVER_ADDR 0x51
#define VIGAIN_SIZE 2
#define VGAIN_ADDR 0x19
#define IGAIN_ADDR 0x1b

/* =================================== API ======================================= */
int im_get_Igain_Vgain();
#endif
