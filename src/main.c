#include "stm32f7xx_hal.h"

#include "peripherals/lcd.h"

static void hw_Setup( void );


int main( void ) {
	hw_Setup();

	lcd_Test();	// just display a image from EastRising for testing

	while( 1 );
	return 1;
}

static void hw_Setup( void ) {
	SCB_EnableICache();
	SCB_EnableDCache();
	HAL_Init();
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOK_CLK_ENABLE();

	lcd_Setup();
}
