/**********************************
 * @author      DJ
 * License:     NULL
 *
 **********************************/

#ifndef __LED_TOOL__
#define __LED_TOOL__

#define WIFI_LED_ON             0
#define WIFI_LED_OFF            255
#define WIFI_LINK_LED_RED       0
#define WIFI_LINK_LED_BLUE      255
#define ADC7606_LED_BLUE        255
#define ADC7606_LED_RED         0
#define HT7036_LED_BLUE         1
#define HT7036_LED_RED          0

#define ADC7606_SKU_LED_COUNT   4

#define WIFI_SIG_LED_S  1
#define WIFI_SIG_LED_E  3
#define ADC7606_SKU_LED_S   WIFI_SIG_LED_E + 1
#define ADC7606_SKU_LED_E   7
#define HT7036_SKU_LED_S    1
#define HT7036_SKU_LED_E    4

#define LED_ADC_DIRECTION_POWER  0 //BLUE or RED LED

#define SYS_WIFI_SIGNAL_LED        "/sys/class/leds/hps_led"
#define SYS_WIFI_LINK_LED          "/sys/class/leds/phy0-rx"
#define SYS_WIFI_LINK_LED_RED      "/sys/class/leds/phy0-assoc"
#define SYS_HT7036_CT_LED          "/sys/bus/spi/devices/spi0.0/ledtest"


/* =================================== API ======================================= */
int led_ctrl_ADC7606_ct_direction(int index,int on_off);
#endif
