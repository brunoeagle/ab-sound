#ifndef __USART2_H
#define __USART2_H

#include "FreeRTOS.h"
#include "semphr.h"

SemaphoreHandle_t usart2Mutex;

typedef void ( *usartRxCallbackFunction )( uint8_t b );

void usart2_Setup( void );
void usart2_EnableRxInterrupt( void );
void usart2_SetupCallback( usartRxCallbackFunction );
void usart2_EnableRxInterrupt( void );
uint8_t usart2_WriteArray( uint8_t *arrayToWrite, uint16_t size );
uint8_t usart2_WriteString( char *string );

#endif
