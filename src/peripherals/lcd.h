#ifndef __LCD_H
#define __LCD_H

#include "stm32f7xx_hal.h"

void lcd_Setup( void );
void lcd_Init( void );
void lcd_Test( void );
void lcd_WriteNumber( uint8_t number, uint8_t line, uint8_t col );

#endif
