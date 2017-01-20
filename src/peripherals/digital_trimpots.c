#include "stm32f7xx_hal.h"
#include "digital_trimpots.h"
#include "spi1.h"

static void digitalTrimpots_SelectTrimpot( uint8_t channel, GPIO_PinState state );

void digitalTrimpots_Setup( void ) {
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOA_CLK_ENABLE();

	// Setup the I/O to the module
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Pin = GPIO_PIN_1 | GPIO_PIN_2 |
			GPIO_PIN_3 | GPIO_PIN_4;
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );

	digitalTrimpots_SelectTrimpot( LEFT_TRIMPOT, GPIO_PIN_SET );
	digitalTrimpots_SelectTrimpot( RIGHT_TRIMPOT, GPIO_PIN_SET );
	digitalTrimpots_SelectTrimpot( SUBWOOFER_TRIMPOT, GPIO_PIN_SET );
	HAL_GPIO_WritePin( GPIOA, GPIO_PIN_1, GPIO_PIN_RESET );
}

void digitalTrimpots_Command( uint8_t channel, uint8_t command ) {
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_RESET );
	spi1_WriteByte( command );
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_SET );
}

uint8_t digitalTrimpots_ReadWiper( uint8_t channel ) {
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_RESET );
	spi1_WriteByte( 0x0C );
	spi1_WriteByte( 0xFF );
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_SET );
	return spi1_ReturnReceivedByte();
}

static void digitalTrimpots_SelectTrimpot( uint8_t channel, GPIO_PinState state ) {
	switch (channel) {
	case LEFT_TRIMPOT:
		HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, state );
		break;
	case RIGHT_TRIMPOT:
		HAL_GPIO_WritePin( GPIOA, GPIO_PIN_3, state );
		break;
	case SUBWOOFER_TRIMPOT:
		HAL_GPIO_WritePin( GPIOA, GPIO_PIN_4, state );
		break;
	default:
		break;
	}
}
