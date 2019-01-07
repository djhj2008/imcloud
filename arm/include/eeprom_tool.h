/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __EEPROM_TOOL__
#define __EEPROM_TOOL__

#define PAGE_SIZE 128
#define TIMEOUT	3
#define RETRY	3
#define I2C_DEFAULT_TIMEOUT		1
#define I2C_DEFAULT_RETRY		3
/*IC 24LC1025 eeprom Block0 I2C slave addr: 0x51*/
#define I2C_ADDERSS_B0 0x51
/*IC 24LC1025 eeprom Block1 I2C slave addr: 0x55*/
#define I2C_ADDERSS_B1 0x55
#define BLOCK_MAX 0xffff
#define BLOCK2_MAX 0x1ffff
#define ADDER_LENS 2

#define DATA_LEN 512 * 2
#define SET_DATA 0xFF

typedef unsigned char           u8;
typedef unsigned short          u16;
typedef unsigned int            u32;
typedef unsigned long long      u64;
typedef signed char             s8;
typedef short                   s16;
typedef int                     s32;
typedef long long               s64;

#define MODEL_NAME_ADDR		0x00
#define MODEL_NAME_SIZE		5
#define MANUFACTURE_ADDR	0X05
#define MANUFACTURE_SIZE	10

#define ADC_VERSION_SIZE	5
#define HW_VERSION_ADDR		0x0f
#define FPGA_VERSION_ADDR	0x14
#define FW_VERSION_ADDR		0x19

#define ADC_FREQUENCY_ADDR	0x76
#define ADC_FREQUENCY_SIZE	1

#define ADC_V_THRESHOL		0x77
#define ADC_I_THRESHOL		0x79
#define ADC_THRESHOL_SIZE	2

#define VGAIN_ADDR 			0x3c
#define VGAIN_SIZE			2
#define IGAIN_ADDR 			0x3e
#define IGAIN_SIZE			2	

#define ACCESS_KEY_ADDR		0x161

#define AC_LINE_FREQUENCY_50 50 //50Hz or 60Hz
#define AC_LINE_FREQUENCY_60 60 //50Hz or 60Hz

/* =================================== API ======================================= */
void eeprom_set_fw_version(uint16_t version);
int im_init_e2prom_data();
void eeprom_set_accesskey(char * access_key);
void read_eeprom(unsigned int offset, unsigned char* buffer, int len);
#endif
