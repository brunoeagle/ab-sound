#ifndef __USART3_H
#define __USART3_H

#include "FreeRTOS.h"
#include "semphr.h"

typedef void ( *usartRxCallbackFunction )( uint8_t b );

void usart3_Setup( void );
void usart3_EnableRxInterrupt( void );
void usart3_SetupCallback( usartRxCallbackFunction );
void usart3_EnableRxInterrupt( void );
uint8_t usart3_WriteArray( uint8_t *arrayToWrite, uint16_t size );
uint8_t usart3_WriteString( char *string );

#endif
