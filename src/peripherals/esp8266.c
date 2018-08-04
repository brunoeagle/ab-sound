#include "stm32f7xx_hal.h"
#include "esp8266.h"

#define	RST_LOW		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_12, GPIO_PIN_RESET )
#define	RST_HIGH	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_12, GPIO_PIN_SET )
#define	CHPD_LOW	HAL_GPIO_WritePin( GPIOA, GPIO_PIN_8, GPIO_PIN_RESET )
#define	CHPD_HIGH	HAL_GPIO_WritePin( GPIOA, GPIO_PIN_8, GPIO_PIN_SET )
#define	GPIO0_LOW	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_13, GPIO_PIN_RESET )
#define	GPIO0_HIGH	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_13, GPIO_PIN_SET )
#define	GPIO2_LOW	HAL_GPIO_WritePin( GPIOH, GPIO_PIN_13, GPIO_PIN_RESET )
#define	GPIO2_HIGH	HAL_GPIO_WritePin( GPIOH, GPIO_PIN_13, GPIO_PIN_SET )
#define	GPIO15_LOW	HAL_GPIO_WritePin( GPIOH, GPIO_PIN_14, GPIO_PIN_RESET )
#define	GPIO15_HIGH	HAL_GPIO_WritePin( GPIOH, GPIO_PIN_14, GPIO_PIN_SET )

GPIO_InitTypeDef GPIO_InitStruct;

void esp8266_Setup( uint8_t bootMode ) {
	// configure the I/O
	__HAL_RCC_GPIOA_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_GPIOH_CLK_ENABLE();

	RST_LOW;
	CHPD_HIGH;
	switch( bootMode ) {
	case ESP8266_BOOT_NORMAL:
		GPIO0_HIGH;
		GPIO2_HIGH;
		GPIO15_LOW;
		break;
	case ESP8266_BOOT_DOWNLOAD:
		GPIO0_LOW;
		GPIO2_HIGH;
		GPIO15_LOW;
		break;
	case ESP8266_BOOT_SDCARD:
		GPIO0_LOW;
		GPIO2_LOW;
		GPIO15_HIGH;
		break;
	}

	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;

	// CHPD
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
	// RST
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init( GPIOJ, &GPIO_InitStruct );
	// GPIO0
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init( GPIOJ, &GPIO_InitStruct );
	// GPIO2
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	// GPIO15
	GPIO_InitStruct.Pin = GPIO_PIN_14;
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
}

void esp8266_DeInit( void ) {
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;

	// CHPD
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	HAL_GPIO_Init( GPIOA, &GPIO_InitStruct );
	// RST
	GPIO_InitStruct.Pin = GPIO_PIN_12;
	HAL_GPIO_Init( GPIOJ, &GPIO_InitStruct );
	// GPIO0
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init( GPIOJ, &GPIO_InitStruct );
	// GPIO2
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
	// GPIO15
	GPIO_InitStruct.Pin = GPIO_PIN_14;
	HAL_GPIO_Init( GPIOH, &GPIO_InitStruct );
}

void esp8266_SetReset( uint8_t reset ) {
	if( reset )
		RST_LOW;
	else
		RST_HIGH;
}
