#include "stm32f7xx_hal.h"
#include "usart2.h"

UART_HandleTypeDef UART_HandleStruct;
RCC_PeriphCLKInitTypeDef RCC_InitStruct;
usartRxCallbackFunction usart2_ReceiveCallback;

void usart2_Setup( void ) {
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOD_CLK_ENABLE();

	RCC_InitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART2;
	RCC_InitStruct.Usart2ClockSelection = RCC_USART2CLKSOURCE_SYSCLK;	// 216MHz
	HAL_RCCEx_PeriphCLKConfig( &RCC_InitStruct );
	__HAL_RCC_USART2_CLK_ENABLE();

	// Setup the I/O to the module
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF7_USART2;
	GPIO_InitStruct.Pin = GPIO_PIN_5;
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );
	GPIO_InitStruct.Pin = GPIO_PIN_6;
	HAL_GPIO_Init( GPIOD, &GPIO_InitStruct );

	UART_HandleStruct.Instance = USART2;
	UART_HandleStruct.Init.BaudRate   = 115200;
	UART_HandleStruct.Init.WordLength = UART_WORDLENGTH_8B;
	UART_HandleStruct.Init.StopBits   = UART_STOPBITS_1;
	UART_HandleStruct.Init.Parity     = UART_PARITY_NONE;
	UART_HandleStruct.Init.Mode       = UART_MODE_TX_RX;
	UART_HandleStruct.Init.OverSampling = UART_OVERSAMPLING_16;
	UART_HandleStruct.Init.HwFlowCtl = UART_HWCONTROL_NONE;
	if( HAL_UART_Init( &UART_HandleStruct ) != HAL_OK )
		while( 1 );
	__HAL_UART_ENABLE( &UART_HandleStruct );

	usart2Mutex = xSemaphoreCreateMutex();
	while( usart2Mutex == NULL );
}

void usart2_EnableRxInterrupt( void ) {
	// Setup the USART2 interrupt
	HAL_NVIC_SetPriority( USART2_IRQn, 0, 1 );
	HAL_NVIC_EnableIRQ( USART2_IRQn );
	__HAL_UART_ENABLE_IT( &UART_HandleStruct, UART_IT_RXNE );
}

void usart2_SetupCallback( usartRxCallbackFunction callback ) {
	usart2_ReceiveCallback = callback;
}

uint8_t usart2_WriteArray( uint8_t *arrayToWrite, uint16_t size ) {
	if( HAL_UART_Transmit( &UART_HandleStruct, arrayToWrite, size, 1000 ) != HAL_OK )
		return 0;
	return 1;
}

uint8_t usart2_WriteString( char *string ) {
	uint16_t size = 1000;	// maximum size
	char *p = string;
	while( *p && --size ) {
		if( HAL_UART_Transmit( &UART_HandleStruct, ( uint8_t* )p++, 1, 1000 ) != HAL_OK )
			break;
	}
	return ( size > 0 ) ? 1 : 0;
}

void USART2_IRQHandler( void ) {
	uint8_t rec;
	if( __HAL_UART_GET_FLAG( &UART_HandleStruct, UART_FLAG_RXNE ) ) {
		HAL_UART_Receive( &UART_HandleStruct, &rec, 1, 1000 );
		__HAL_UART_CLEAR_IT( &UART_HandleStruct, UART_FLAG_RXNE );
	}
	if( __HAL_UART_GET_FLAG( &UART_HandleStruct, UART_FLAG_ORE ) ) {
		__HAL_UART_CLEAR_IT( &UART_HandleStruct, UART_FLAG_ORE );
	}
	usart2_ReceiveCallback( rec );
}
