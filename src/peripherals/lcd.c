#include "stm32f7xx_hal.h"
#include "lcd.h"
#include "lcd_lib.h"

#include "FreeRTOS.h"
#include "semphr.h"
#include "task.h"

#define	RD_HIGH			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_SET )		// DC high
#define	RD_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_RESET )		// DC low
#define	DC_HIGH			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_11, GPIO_PIN_SET )		// DC high
#define	DC_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_11, GPIO_PIN_RESET )		// DC low
#define CS_HIGH			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_13, GPIO_PIN_SET )		// CS high
#define CS_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_13, GPIO_PIN_RESET )		// CS low
#define WR_HIGH			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_10, GPIO_PIN_SET )		// WR high
#define WR_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_10, GPIO_PIN_RESET )		// WR low
#define RST_HIGH		HAL_GPIO_WritePin( GPIOG, GPIO_PIN_12, GPIO_PIN_SET )		// RST high
#define RST_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_12, GPIO_PIN_RESET )		// RST low

uint8_t Contrast_level=0x30;
volatile uint32_t timerCounter;
TIM_HandleTypeDef TIM_HandleStruct;

void lcd_ClearRam( void );
void lcd_ShowContrastLevel( uint8_t number );
void lcd_WriteData( uint8_t dat );
void lcd_WriteInstruction( uint8_t cmd );
void lcd_SetRowAddress( uint8_t add );
void lcd_SetColumnAddress( uint8_t add );
void lcd_SetContrast( uint8_t mod );
void lcd_DisplayChess( uint8_t  value1, uint8_t value2);
void lcd_DisplayPicture(uint8_t image[]);
void lcd_DrawSingleAscii( uint16_t x, uint16_t y, char *pAscii );
void lcd_SendGrayScaleData( uint8_t temp );

/*static void delay_ms( volatile uint32_t t ) {
	timerCounter = 0;
	while( timerCounter < t );
}*/

/*void TIM7_IRQHandler( void ) {
	__HAL_TIM_CLEAR_IT( &TIM_HandleStruct, TIM_IT_UPDATE );
	timerCounter++;
}*/

void lcd_Setup( void ) {
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOK_CLK_ENABLE();

	// Setup the I/O to the module
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 |
			GPIO_PIN_2 | GPIO_PIN_3 | GPIO_PIN_4 |
			GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	HAL_GPIO_Init( GPIOK, &GPIO_InitStruct );

	GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10 |
				GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
	HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );

	// Setup the timer
	/*__HAL_RCC_TIM7_CLK_ENABLE();
	HAL_NVIC_SetPriority( TIM7_IRQn, 0, 0 );
	HAL_NVIC_EnableIRQ( TIM7_IRQn );

	TIM_HandleStruct.Instance = TIM7;
	TIM_HandleStruct.Init.ClockDivision = 0;
	TIM_HandleStruct.Init.Prescaler = 216;		// 108MHz of input clock
	TIM_HandleStruct.Init.Period = 1000;		// Interrupt each 1ms
	TIM_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
	if( HAL_TIM_Base_Init( &TIM_HandleStruct ) != HAL_OK )
		while( 1 );
	HAL_TIM_Base_Start_IT( &TIM_HandleStruct );*/
	lcdMutex = xSemaphoreCreateMutex();
	while( lcdMutex == NULL );
}

void lcd_WriteHexa( uint8_t value, uint8_t line, uint8_t col ) {
	if( xSemaphoreTake( lcdMutex, 1000 / portTICK_PERIOD_MS ) == pdTRUE ) {
		lcd_WriteNumber( ( ( value >> 4 ) & 0x0F ), line, col );
		lcd_WriteNumber( ( value & 0x0F ), line, col + 2 );
		xSemaphoreGive( lcdMutex );
	}
}

void lcd_WriteNumber( uint8_t number, uint8_t line, uint8_t col ) {
	uint8_t i;
	for( i = line; i < line+16; i++) {
		lcd_SetRowAddress(i);
		lcd_SetColumnAddress(col);
		lcd_WriteInstruction(0x5C);
		lcd_SendGrayScaleData(*(numHexa+16*number+i));
	}
}

void lcd_ShowContrastLevel(uint8_t number) {
	uint8_t number1, number2, number3;
	number1 = number / 100;
	number2 = number % 100 / 10;
	number3 = number % 100 % 10;
    Write_number( numHexa, number1, 0);
	Write_number( numHexa, number2, 2);
	Write_number( numHexa, number3, 4);
}

void lcd_WriteData(uint8_t dat) {
	DC_HIGH;
	CS_LOW;
    WR_LOW;
    GPIOK->ODR = dat;
	WR_HIGH;
	CS_HIGH;
	return;
}

void lcd_WriteInstruction(uint8_t cmd)
{
	DC_LOW;
	CS_LOW;
    WR_LOW;
    GPIOK->ODR = cmd;
    WR_HIGH;
	CS_HIGH;
	return;
}

void lcd_SendGrayScaleData( uint8_t temp ) {  //turns 1byte B/W data to 4 bye gray data
	uint8_t temp1,temp2,temp3,temp4,temp5,temp6,temp7,temp8;
	uint8_t h11,h12,h13,h14,h15,h16,h17,h18,d1,d2,d3,d4;

	temp1 = temp & 0x80;
	temp2 = ( temp & 0x40 ) >> 3;
	temp3 = ( temp & 0x20 ) << 2;
	temp4 = ( temp & 0x10 ) >> 1;
	temp5 = ( temp & 0x08 ) << 4;
	temp6 = ( temp & 0x04 ) << 1;
	temp7 = ( temp & 0x02 ) << 6;
	temp8 = ( temp & 0x01 ) << 3;
	h11 = temp1 | temp1 >> 1 | temp1 >> 2 | temp1 >> 3;
	h12 = temp2 | temp2 >> 1 | temp2 >> 2 | temp2 >> 3;
	h13 = temp3 | temp3 >> 1 | temp3 >> 2 | temp3 >> 3;
	h14 = temp4 | temp4 >> 1 | temp4 >> 2 | temp4 >> 3;
	h15 = temp5 | temp5 >> 1 | temp5 >> 2 | temp5 >> 3;
	h16 = temp6 | temp6 >> 1 | temp6 >> 2 | temp6 >> 3;
	h17 = temp7 | temp7 >> 1 | temp7 >> 2 | temp7 >> 3;
	h18 = temp8 | temp8 >> 1 | temp8 >> 2 | temp8 >> 3;
	d1 = h11 | h12;
	d2 = h13 | h14;
	d3 = h15 | h16;
	d4 = h17 | h18;
	lcd_WriteData( d1 );
	lcd_WriteData( d2 );
	lcd_WriteData( d3 );
	lcd_WriteData( d4 );
}

// Set row address 0~32
void lcd_SetRowAddress( uint8_t add ) {
    lcd_WriteInstruction( 0x75 );	/*SET SECOND PRE-CHARGE PERIOD*/
    add = 0x3F & add;
	lcd_WriteData( add );
	lcd_WriteData( 0x3F );
	return;
}

// Set row address 0~64  for Gray mode)
void lcd_SetColumnAddress( uint8_t add ) {
	add = 0x3F & add;
    lcd_WriteInstruction( 0x15 );	/*SET SECOND PRE-CHARGE PERIOD*/
	lcd_WriteData( 0x1C + add );
	lcd_WriteData( 0x5B );
}

// Set Contrast
void lcd_SetContrast( uint8_t mod ) {
    lcd_WriteInstruction( 0xC1 );
	lcd_WriteData( mod );
	return;
}

void lcd_Init( void ) {
	vTaskDelay( 50 / portTICK_PERIOD_MS );
	RST_HIGH;
	RD_HIGH;
	vTaskDelay( 50 / portTICK_PERIOD_MS );
	RST_LOW;
	vTaskDelay( 1 / portTICK_PERIOD_MS );
	RST_HIGH;
	vTaskDelay( 1 / portTICK_PERIOD_MS );

	lcd_WriteInstruction( 0xFD );		/*SET COMMAND LOCK*/
	lcd_WriteData( 0x12 );				/*UNLOCK*/
	lcd_WriteInstruction( 0xAE );		/*DISPLAY OFF*/
	lcd_WriteInstruction( 0xB3 );		/*DISPLAYDIVIDE CLOCKRADIO/OSCILLATAR FREQUANCY*/
	lcd_WriteData( 0x91 );				/*MULTIPLEX RATIO*/
	lcd_WriteInstruction( 0xCA );
	lcd_WriteData( 0x3F );				/*DUTY = 1/64*/
	lcd_WriteInstruction( 0xA2 );		/*SET OFFSET*/
	lcd_WriteData( 0x00 );
	lcd_WriteInstruction( 0xA1 );		/*START LINE*/
	lcd_WriteData( 0x00 );
	lcd_WriteInstruction( 0xA0 );		/*SET REMAP*/
	lcd_WriteData( 0x14 );
	lcd_WriteData( 0x11 );
	lcd_WriteInstruction( 0xAB );		/*FUNCTION REFLECION*/
	lcd_WriteData( 0x01 );				/* SELECTION EXTERNAL VDD */
	lcd_WriteInstruction( 0xB4 );
	lcd_WriteData( 0xA0 );
	lcd_WriteData( 0xFD );
	lcd_WriteInstruction( 0xC1 );		/*SET CONTRAST CURRENT*/
	lcd_WriteData( Contrast_level );
	lcd_WriteInstruction( 0xC7 );		/*MASTER CONTRAST CURRENT CONTROL*/
	lcd_WriteData( 0x0F );
	lcd_WriteInstruction( 0xB1 );		/*SET PHASE LENGTH*/
	lcd_WriteData( 0xE2 );
	lcd_WriteInstruction( 0xD1 );
	lcd_WriteData( 0x82 );
	lcd_WriteData( 0x20 );
	lcd_WriteInstruction( 0xBB );		/*SET PRE-CHANGE VOLTAGE*/
	lcd_WriteData( 0x1F );
	lcd_WriteInstruction( 0xB6 );		/*SET SECOND PRE-CHARGE PERIOD*/
	lcd_WriteData( 0x08 );
	lcd_WriteInstruction( 0xBE );		/*SET VCOMH*/
	lcd_WriteData( 0x07 );
	lcd_WriteInstruction( 0xA6 );		/*NORMAL DISPLAY*/
	lcd_ClearRam();
	lcd_WriteInstruction( 0xAF );		/*DISPLAY ON*/
}

void lcd_ClearRam( void ){
	uint8_t x,y;

	lcd_WriteInstruction( 0x15 );
	lcd_WriteData( 0x00 );
	lcd_WriteData( 0x77 );
	lcd_WriteInstruction( 0x75 );
	lcd_WriteData( 0x00 );
	lcd_WriteData( 0x7F );
	lcd_WriteInstruction( 0x5C );
	for( y = 0; y < 128; y++ ) {
		for( x = 0; x < 120; x++ ) {
			lcd_WriteData( 0x00 );
		}
	}
}



void lcd_DisplayChess( uint8_t value1,uint8_t value2 ) {
    uint8_t i,k;

	lcd_SetRowAddress( 0 );
	lcd_SetColumnAddress( 0 );
	lcd_WriteInstruction( 0x5C );

    for( i = 0; i < 32; i++ ) {
    	for( k = 0; k < 32; k++ )
    		lcd_SendGrayScaleData( value1 );
    	for( k = 0; k < 32; k++ )
    		lcd_SendGrayScaleData( value2 );
	}
    return;
}

//DISPLAY ASCII
void lcd_DrawSingleAscii( uint16_t x, uint16_t y, char *pAscii ) {
    uint8_t i;
    uint8_t str;
    uint16_t OffSet;

    OffSet = ( *pAscii - 32 ) * 16;
    for( i = 0; i < 16; i++ ) {
  		lcd_SetRowAddress( y + i );
    	lcd_SetColumnAddress( x );
 	    lcd_WriteInstruction( 0x5C );
        str = *( AsciiLib + OffSet + i );
        lcd_SendGrayScaleData( str );
    }
}

void lcd_DisplayPicture( uint8_t image[] ) {
	uint8_t i,j;

	lcd_SetRowAddress( 0 );
	lcd_SetColumnAddress( 0 );
	lcd_WriteInstruction( 0x5C );
	for( i = 0; i < 64; i++ ) {
		for( j = 0; j < 32; j++ ) {
			lcd_SendGrayScaleData( image[ i * 32 + j ] );
		}
	}
    return;
}

void lcd_GrayTest( void ) {
	uint8_t i,k,m,j;

	j=0;
  	lcd_SetRowAddress( 0 );
    lcd_SetColumnAddress( 0 );
    lcd_WriteInstruction( 0x5C );
    for( m = 0; m < 32; m++ ) {
		for( k = 0; k < 16; k++ ) {
			for( i = 0; i < 8; i++ ) {
				lcd_WriteData( j );
			}
			j += 0x11;
		}
		j = 0;
	}
    j = 255;
    for( m = 0; m < 32; m++ ) {
		for( k = 0; k < 16; k++ ) {
			for( i = 0; i < 8; i++ ) {
				lcd_WriteData( j );
			}
			j -= 0x11;
		}
		j = 255;
	}
}

void lcd_Test( void ) {
	//static uint8_t number = 0;
	/*if( number == 0 ) {
		lcd_Init();
		lcd_WriteInstruction( 0xA6 );	//--set normal display
	}*/
	/*lcd_WriteInstruction( 0xA5 );	//--all display on
	delay_ms( 500 );
	lcd_WriteInstruction( 0xA4 );	//--all Display off
	delay_ms( 500 );*/

	//lcd_DisplayChess( 0x01, 0x00 );
	//Write_number( num, number++, 0, 0 );
	//if( number > 9 ) number = 0;

	/*lcd_DisplayPicture( pic );
	delay_ms( 3000 );
	lcd_WriteInstruction( 0xA7 );	//--set Inverse Display
	lcd_DisplayPicture( pic );
	delay_ms( 500 );
	lcd_WriteInstruction( 0xA6);	//--set normal display
	lcd_DisplayPicture( pic1 );
	delay_ms( 500 );
	lcd_WriteInstruction( 0xA7 );	//--set Inverse Display
	lcd_DisplayPicture( pic1 );
	delay_ms( 500 );

	lcd_WriteInstruction( 0xA6 );	//--set normal display

	lcd_DisplayChess( 0x00, 0x00 );	//clear display

	lcd_GrayTest();	// gray test
	delay_ms( 500 );

	lcd_DisplayChess( 0x55, 0xAA );
	delay_ms( 500 );
	lcd_DisplayChess( 0xAA, 0x55 );
	delay_ms( 500 );

	lcd_DisplayChess( 0x55, 0x55 );
	delay_ms( 500 );
	lcd_DisplayChess( 0xAA,0xAA );
	delay_ms( 500 );

	lcd_DisplayChess( 0xFF,0x00 );
	delay_ms( 500 );
	lcd_DisplayChess( 0x00,0xFF );
	delay_ms( 500 );
	lcd_DisplayChess( 0x00, 0x00 );	//clear display*/

}
