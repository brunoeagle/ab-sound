#include <string.h>
#include "stm32f7xx_hal.h"
#include "peripherals/esp8266.h"
#include "stm32/usart2.h"
#include "wifi.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define	RESPONSE_OK		1
#define RESPONSE_ERROR	2

static void wifi_Setup( void );
static void wifi_ReceiveCallback( uint8_t receivedByte );
static uint8_t wifi_SendATCommand( char *command, uint8_t expectedResponseCode, uint32_t timeout );
uint8_t webPage[] = { "<head><meta http-equiv=\"refresh\" content=\"3\"></head><h1><u>ESP-12 - Web Server</u></h1><h2>Porta Digital 8: 1<h2>Porta Digital 9: 2</h2>" };

volatile char rxBuffer[ 1000 ];
volatile int16_t rxBufferPos;
volatile TickType_t lastByteReceivedTime;
volatile uint8_t responseCode;
static const char *responses[] = { "OK\r\n", "ERROR\r\n" };

void wifi_Task( void *pvParameters ) {
	uint8_t pageSize = strlen( webPage );
	wifi_Setup();
	for( ;; ) {
		// some tests
		//wifi_SendATCommand( "AT+CWLAP\r\n", RESPONSE_OK, 5000 );
		wifi_SendATCommand( "AT+CWJAP=\"AARB\",\"saidowhatsapp\"\r\n", RESPONSE_OK, 10000 );
		wifi_SendATCommand( "AT+CWMODE=1\r\n", RESPONSE_OK, 1000 );
		wifi_SendATCommand( "AT+CIFSR\r\n", RESPONSE_OK, 1000 );
		wifi_SendATCommand( "AT+CIPMUX=1\r\n", RESPONSE_OK, 2000 );
		wifi_SendATCommand( "AT+CIPSERVER=1,80\r\n", RESPONSE_OK, 2000 );
		//wifi_SendATCommand( "AT+CIPSTATUS\r\n", RESPONSE_OK, 2000 );
		wifi_SendATCommand( "AT+CIPSEND=136\r\n", RESPONSE_OK, 2000 );
		wifi_SendATCommand( webPage, RESPONSE_OK, 5000 );
		while( 1 ) {
			pageSize++;
		}
	}
}

static void wifi_Setup( void ) {
	usart2_SetupCallback( wifi_ReceiveCallback );
	rxBufferPos = 0;
	usart2_EnableRxInterrupt();

	esp8266_Setup( ESP8266_BOOT_NORMAL );
	vTaskDelay( 500 / portTICK_PERIOD_MS );
	esp8266_SetReset( 0 );
	vTaskDelay( 2000 / portTICK_PERIOD_MS );
}

static void wifi_ReceiveCallback( uint8_t receivedByte ) {
	static uint8_t responseState = 0;
	static char *responseExpectedPtr;
	static uint8_t responseExpectedSize;
	static uint8_t preResponseCode;
	rxBuffer[ rxBufferPos++ ] = receivedByte;
	lastByteReceivedTime = xTaskGetTickCount();
	if( rxBufferPos >= 1000 )
		rxBufferPos = 0;
	switch( responseState ) {
	case 0:
		if( receivedByte == 'O' ) {
			responseExpectedPtr = responses[ 0 ] + 1;
			responseExpectedSize = 3;
			preResponseCode = RESPONSE_OK;
			responseState = 1;
		}
		else if( receivedByte == 'E' ) {
			responseExpectedPtr = responses[ 1 ] + 1;
			responseExpectedSize = 6;
			preResponseCode = RESPONSE_ERROR;
			responseState = 1;
		}
		else {
			responseState = 0;
		}
		break;
	case 1:
		if( *responseExpectedPtr++ == receivedByte ) {
			responseExpectedSize--;
			if( !responseExpectedSize ) {
				responseCode = preResponseCode;
				responseState = 0;
			}
		}
		else {
			responseState = 0;
		}
		break;
	}
}

static uint8_t wifi_SendATCommand( char *command, uint8_t expectedResponseCode, uint32_t timeout ) {
	rxBufferPos = 0;
	responseCode = 0;
	volatile TickType_t t = xTaskGetTickCount();
	usart2_WriteString( command );
	while( !responseCode && ( ( xTaskGetTickCount() - t ) < timeout ) );	// wait for the first byte
	return ( responseCode == expectedResponseCode ) ? 1 : 0;
}
