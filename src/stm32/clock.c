#include "stm32f7xx_hal.h"
#include "clock.h"

RCC_ClkInitTypeDef RCC_ClkInitStruct;
RCC_OscInitTypeDef RCC_OscInitStruct;
RCC_PeriphCLKInitTypeDef RCC_PerInitStruct;

void clock_InitMainClock( void ) {
	__HAL_RCC_PWR_CLK_ENABLE();
	// HSE source, PLL set to 216MHz on P and 48MHz on Q
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 160;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 2;
	if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
		while( 1 );
	// Overdrive activation to achieve 216MHz
	if( HAL_PWREx_EnableOverDrive() != HAL_OK )
		while( 1 );

	// PLL as system clock source. HCLK 80MHz, APB1 40MHz, APB2 80MHz
	RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2) ;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;		// Core will be at 80MHz
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;		// APB1 Peripherals at 40MHz
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;		// APB2 Peripherals at 80MHz
	if( HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK )
		while( 1 );
}
