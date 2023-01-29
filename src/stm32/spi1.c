#include "stm32f7xx_hal.h"
#include "spi1.h"

SPI_HandleTypeDef SPI_HandleStruct;
uint8_t lastReceivedByte;

SemaphoreHandle_t spi1Mutex;

void spi1_Setup( void ) {
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_SPI1_CLK_ENABLE();

	// Setup the I/O to the module
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF5_SPI1;
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_6 | GPIO_PIN_7;
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

	SPI_HandleStruct.Instance = SPI1;
	SPI_HandleStruct.Init.Mode = SPI_MODE_MASTER;
	SPI_HandleStruct.Init.BaudRatePrescaler = SPI_BAUDRATEPRESCALER_8;	// APB2 prescaler = 1, fPCLK = 80MHz, SPI cock 80MHz/8 = 10MHz
	SPI_HandleStruct.Init.Direction = SPI_DIRECTION_2LINES;
	SPI_HandleStruct.Init.CLKPhase = SPI_PHASE_1EDGE;
	SPI_HandleStruct.Init.CLKPolarity = SPI_POLARITY_LOW;
	SPI_HandleStruct.Init.DataSize = SPI_DATASIZE_8BIT;
	SPI_HandleStruct.Init.FirstBit = SPI_FIRSTBIT_MSB;
	SPI_HandleStruct.Init.TIMode = SPI_TIMODE_DISABLE;
	SPI_HandleStruct.Init.CRCCalculation = SPI_CRCCALCULATION_DISABLE;
	SPI_HandleStruct.Init.CRCPolynomial = 7;
	SPI_HandleStruct.Init.NSS = SPI_NSS_SOFT;

	if( HAL_SPI_Init( &SPI_HandleStruct ) != HAL_OK )
		while( 1 );
	__HAL_SPI_ENABLE( &SPI_HandleStruct );

	spi1Mutex = xSemaphoreCreateMutex();
	while( spi1Mutex == NULL );
}

uint8_t spi1_WriteByte( uint8_t byteToWrite ) {
	if( HAL_SPI_TransmitReceive( &SPI_HandleStruct, &byteToWrite, &lastReceivedByte, 1, 1000 ) != HAL_OK )
		return 0;
	return 1;
}

uint8_t spi1_ReturnReceivedByte( void ) {
	return lastReceivedByte;
}
