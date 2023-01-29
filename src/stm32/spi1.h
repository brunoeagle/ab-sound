#ifndef __SPI1_H
#define __SPI1_H

#include "FreeRTOS.h"
#include "semphr.h"

void spi1_Setup( void );
uint8_t spi1_WriteByte( uint8_t byteToWrite );
uint8_t spi1_ReturnReceivedByte( void );

#endif
