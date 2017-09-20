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

uint8_t digitalTrimpots_Command( uint8_t channel, uint8_t command ) {
	uint8_t ok;
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_RESET );
	ok = spi1_WriteByte( command );
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_SET );
	if( ( spi1_ReturnReceivedByte() & 0x02 ) && ok )
		return 1;
	return 0;
}

uint8_t digitalTrimpots_WriteWiper( uint8_t channel, uint8_t value ) {
	uint8_t ok1, ok2, cmderr;
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_RESET );
	ok1 = spi1_WriteByte( 0x00 );
	cmderr = spi1_ReturnReceivedByte();
	ok2 = spi1_WriteByte( value );
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_SET );
	if( ok1 && ok2 && ( cmderr & 0x02 ) )
		return 1;
	return 0;
}

uint8_t digitalTrimpots_ReadWiper( uint8_t channel, uint8_t *valueRead ) {
	uint8_t ok1, ok2, cmderr;
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_RESET );
	ok1 = spi1_WriteByte( 0x0C );
	cmderr = spi1_ReturnReceivedByte();
	ok2 = spi1_WriteByte( 0xFF );
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_SET );
	*valueRead = spi1_ReturnReceivedByte();
	if( ok1 && ok2 && ( cmderr & 0x02 ) )
		return 1;
	return 0;
}

uint8_t digitalTrimpots_WriteTCON( uint8_t channel, uint8_t value ) {
	uint8_t ok1, ok2, cmderr;
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_RESET );
	ok1 = spi1_WriteByte( 0x40 );
	cmderr = spi1_ReturnReceivedByte();
	ok2 = spi1_WriteByte( value );
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_SET );
	if( ok1 && ok2 && ( cmderr & 0x02 ) )
		return 1;
	return 0;
}

uint8_t digitalTrimpots_ReadTCON( uint8_t channel, uint8_t *valueRead ) {
	uint8_t ok1, ok2, cmderr;
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_RESET );
	ok1 = spi1_WriteByte( 0x4C );
	cmderr = spi1_ReturnReceivedByte();
	ok2 = spi1_WriteByte( 0xFF );
	digitalTrimpots_SelectTrimpot( channel, GPIO_PIN_SET );
	*valueRead = spi1_ReturnReceivedByte();
	if( ok1 && ok2 && ( cmderr & 0x02 ) )
		return 1;
	return 0;
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
