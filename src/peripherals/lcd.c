#include "stm32f7xx_hal.h"
#include "lcd.h"
#include "test_lcd.h"

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

void Clear_ram( void );
void Initial( void );
void Write_number( const uint8_t *n, uint8_t k, uint8_t station_dot );
void display_Contrast_level( uint8_t number );
void Write_Data( uint8_t dat );
void Write_Instruction( uint8_t cmd );
void Set_Row_Address( uint8_t add );
void Set_Column_Address( uint8_t add );
void Set_Contrast_Control_Register( uint8_t mod );
void Display_Chess( uint8_t  value1, uint8_t value2);
void Display_Picture(uint8_t image[]);
void DrawSingleAscii( uint16_t x, uint16_t y, char *pAscii );
void Gray_test( void );
void Data_processing( uint8_t temp );

static void delay_ms( volatile uint32_t t ) {
	timerCounter = 0;
	while( timerCounter < t );
}

void TIM7_IRQHandler( void ) {
	__HAL_TIM_CLEAR_IT( &TIM_HandleStruct, TIM_IT_UPDATE );
	timerCounter++;
}

void lcd_Setup( void ) {
	GPIO_InitTypeDef GPIO_InitStruct;

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
	__HAL_RCC_TIM7_CLK_ENABLE();
	HAL_NVIC_SetPriority( TIM7_IRQn, 0, 0 );
	HAL_NVIC_EnableIRQ( TIM7_IRQn );

	TIM_HandleStruct.Instance = TIM7;
	TIM_HandleStruct.Init.ClockDivision = 0;
	TIM_HandleStruct.Init.Prescaler = 216;		// 108MHz of input clock
	TIM_HandleStruct.Init.Period = 1000;		// Interrupt each 1ms
	TIM_HandleStruct.Init.CounterMode = TIM_COUNTERMODE_UP;
	if( HAL_TIM_Base_Init( &TIM_HandleStruct ) != HAL_OK )
		while( 1 );
	HAL_TIM_Base_Start_IT( &TIM_HandleStruct );
}

void Write_number( const uint8_t *n, uint8_t k, uint8_t col ) {
	uint8_t i;
	for(i=0;i<16;i++) {
		Set_Row_Address(i);
		Set_Column_Address(col);
		Write_Instruction(0x5C);
		Data_processing(*(n+16*k+i));
	}
}

void display_Contrast_level(uint8_t number) {
	uint8_t number1,number2,number3;
	number1=number/100;number2=number%100/10;number3=number%100%10;
    Write_number(num,number1,0);
	Write_number(num,number2,2);
	Write_number(num,number3,4);
}

void Write_Data(uint8_t dat) {
	DC_HIGH;
	CS_LOW;
    WR_LOW;
    GPIOK->ODR = dat;
	WR_HIGH;
	CS_HIGH;
	return;
}

void Write_Instruction(uint8_t cmd)
{
	DC_LOW;
	CS_LOW;
    WR_LOW;
    GPIOK->ODR = cmd;
    WR_HIGH;
	CS_HIGH;
	return;
}

void Data_processing( uint8_t temp ) {  //turns 1byte B/W data to 4 bye gray data
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
	Write_Data( d1 );
	Write_Data( d2 );
	Write_Data( d3 );
	Write_Data( d4 );
}

// Set row address 0~32
void Set_Row_Address( uint8_t add ) {
    Write_Instruction( 0x75 );	/*SET SECOND PRE-CHARGE PERIOD*/
    add = 0x3F & add;
	Write_Data( add );
	Write_Data( 0x3F );
	return;
}

// Set row address 0~64  for Gray mode)
void Set_Column_Address( uint8_t add ) {
	add = 0x3F & add;
    Write_Instruction( 0x15 );	/*SET SECOND PRE-CHARGE PERIOD*/
	Write_Data( 0x1C + add );
	Write_Data( 0x5B );
}

// Set Contrast
void Set_Contrast_Control_Register( uint8_t mod ) {
    Write_Instruction( 0xC1 );
	Write_Data( mod );
	return;
}

void Initial( void ) {
	RST_HIGH;
	RD_HIGH;
	delay_ms( 200 );
	RST_LOW;
	delay_ms( 200 );
	RST_HIGH;
	delay_ms( 200 );

	Write_Instruction( 0xFD ); //Set Command Lock

	Write_Instruction( 0xFD );		/*SET COMMAND LOCK*/
	Write_Data( 0x12 );				/*UNLOCK*/
	Write_Instruction( 0xAE );		/*DISPLAY OFF*/
	Write_Instruction( 0xB3 );		/*DISPLAYDIVIDE CLOCKRADIO/OSCILLATAR FREQUANCY*/
	Write_Data( 0x91 );				/*MULTIPLEX RATIO*/
	Write_Instruction( 0xCA );
	Write_Data( 0x3F );				/*DUTY = 1/64*/
	Write_Instruction( 0xA2 );		/*SET OFFSET*/
	Write_Data( 0x00 );
	Write_Instruction( 0xA1 );		/*START LINE*/
	Write_Data( 0x00 );
	Write_Instruction( 0xA0 );		/*SET REMAP*/
	Write_Data( 0x14 );
	Write_Data( 0x11 );
	Write_Instruction( 0xAB );		/*FUNCTION REFLECION*/
	Write_Data( 0x01 );				/* SELECTION EXTERNAL VDD */
	Write_Instruction( 0xB4 );
	Write_Data( 0xA0 );
	Write_Data( 0xFD );
	Write_Instruction( 0xC1 );		/*SET CONTRAST CURRENT*/
	Write_Data( Contrast_level );
	Write_Instruction( 0xC7 );		/*MASTER CONTRAST CURRENT CONTROL*/
	Write_Data( 0x0F );
	Write_Instruction( 0xB1 );		/*SET PHASE LENGTH*/
	Write_Data( 0xE2 );
	Write_Instruction( 0xD1 );
	Write_Data( 0x82 );
	Write_Data( 0x20 );
	Write_Instruction( 0xBB );		/*SET PRE-CHANGE VOLTAGE*/
	Write_Data( 0x1F );
	Write_Instruction( 0xB6 );		/*SET SECOND PRE-CHARGE PERIOD*/
	Write_Data( 0x08 );
	Write_Instruction( 0xBE );		/*SET VCOMH*/
	Write_Data( 0x07 );
	Write_Instruction( 0xA6 );		/*NORMAL DISPLAY*/
	Clear_ram();
	Write_Instruction( 0xAF );		/*DISPLAY ON*/
}

void Clear_ram( void ){
	uint8_t x,y;

	Write_Instruction( 0x15 );
	Write_Data( 0x00 );
	Write_Data( 0x77 );
	Write_Instruction( 0x75 );
	Write_Data( 0x00 );
	Write_Data( 0x7F );
	Write_Instruction( 0x5C );
	for( y = 0; y < 128; y++ ) {
		for( x = 0; x < 120; x++ ) {
			Write_Data( 0x00 );
		}
	}
}



void Display_Chess( uint8_t value1,uint8_t value2 ) {
    uint8_t i,k;

	Set_Row_Address( 0 );
	Set_Column_Address( 0 );
	Write_Instruction( 0x5C );

    for( i = 0; i < 32; i++ ) {
    	for( k = 0; k < 32; k++ )
    		Data_processing( value1 );
    	for( k = 0; k < 32; k++ )
    		Data_processing( value2 );
	}
    return;
}

//DISPLAY ASCII
void DrawSingleAscii( uint16_t x, uint16_t y, char *pAscii ) {
    uint8_t i;
    uint8_t str;
    uint16_t OffSet;

    OffSet = ( *pAscii - 32 ) * 16;
    for( i = 0; i < 16; i++ ) {
  		Set_Row_Address( y + i );
    	Set_Column_Address( x );
 	    Write_Instruction( 0x5C );
        str = *( AsciiLib + OffSet + i );
        Data_processing( str );
    }
}

void Display_Picture( uint8_t image[] ) {
	uint8_t i,j;

	Set_Row_Address( 0 );
	Set_Column_Address( 0 );
	Write_Instruction( 0x5C );
	for( i = 0; i < 64; i++ ) {
		for( j = 0; j < 32; j++ ) {
			Data_processing( image[ i * 32 + j ] );
		}
	}
    return;
}

void Gray_test( void ) {
	uint8_t i,k,m,j;

	j=0;
  	Set_Row_Address( 0 );
    Set_Column_Address( 0 );
    Write_Instruction( 0x5C );
    for( m = 0; m < 32; m++ ) {
		for( k = 0; k < 16; k++ ) {
			for( i = 0; i < 8; i++ ) {
				Write_Data( j );
			}
			j += 0x11;
		}
		j = 0;
	}
    j = 255;
    for( m = 0; m < 32; m++ ) {
		for( k = 0; k < 16; k++ ) {
			for( i = 0; i < 8; i++ ) {
				Write_Data( j );
			}
			j -= 0x11;
		}
		j = 255;
	}
}

void lcd_Test( void ) {
	Initial();
	Write_Instruction( 0xA5 );	//--all display on
	delay_ms( 500 );
	Write_Instruction( 0xA4 );	//--all Display off
	delay_ms( 500 );

	Write_Instruction( 0xA6 );	//--set normal display

	Display_Picture( pic );
	delay_ms( 3000 );
	Write_Instruction( 0xA7 );	//--set Inverse Display
	Display_Picture( pic );
	delay_ms( 500 );
	Write_Instruction( 0xA6);	//--set normal display
	Display_Picture( pic1 );
	delay_ms( 500 );
	Write_Instruction( 0xA7 );	//--set Inverse Display
	Display_Picture( pic1 );
	delay_ms( 500 );

	Write_Instruction( 0xA6 );	//--set normal display

	Display_Chess( 0x00, 0x00 );	//clear display

	Gray_test();	// gray test
	delay_ms( 500 );

	Display_Chess( 0x55, 0xAA );
	delay_ms( 500 );
	Display_Chess( 0xAA, 0x55 );
	delay_ms( 500 );

	Display_Chess( 0x55, 0x55 );
	delay_ms( 500 );
	Display_Chess( 0xAA,0xAA );
	delay_ms( 500 );

	Display_Chess( 0xFF,0x00 );
	delay_ms( 500 );
	Display_Chess( 0x00,0xFF );
	delay_ms( 500 );
	Display_Chess( 0x00, 0x00 );	//clear display

}
