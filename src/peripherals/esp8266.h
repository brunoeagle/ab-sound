#ifndef __ESP8266_H
#define __ESP8266_H

#define	ESP8266_BOOT_NORMAL			1
#define	ESP8266_BOOT_DOWNLOAD		2
#define	ESP8266_BOOT_SDCARD			3

void esp8266_Setup( uint8_t bootMode );
void esp8266_DeInit( void );
void esp8266_SetReset( uint8_t reset );

#endif
