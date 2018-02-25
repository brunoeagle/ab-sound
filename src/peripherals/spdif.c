#include "stm32f7xx_hal.h"
#include "spdif.h"

RCC_PeriphCLKInitTypeDef RCC_InitStruct;
GPIO_InitTypeDef GPIO_InitStruct;
SAI_InitTypeDef SAI_InitStruct;
DMA_InitTypeDef DMA_InitStruct;
SPDIFRX_InitTypeDef SPDIFRX_InitStruct;
SPDIFRX_HandleTypeDef SPDIFRX_HandleStruct;

void spdif_Setup( uint8_t input ) {
	__HAL_RCC_SPDIFRX_CLK_ENABLE();

	RCC_InitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPDIFRX;
	RCC_InitStruct.I2sClockSelection = RCC_I2SCLKSOURCE_PLLI2S;
	RCC_InitStruct.PLLI2S.PLLI2SN = 400;
	RCC_InitStruct.PLLI2S.PLLI2SP = RCC_PLLI2SP_DIV4;
	HAL_RCCEx_PeriphCLKConfig( &RCC_InitStruct );

	/* Enable and set SPDIF Interrupt */
	HAL_NVIC_SetPriority( SPDIF_RX_IRQn,  0, 1);
	HAL_NVIC_EnableIRQ( SPDIF_RX_IRQn );

	/*configure SPDIFRX_IN2 PC4 pin */

	/* Enable SPDIF GPIO IN */
	__HAL_RCC_GPIOC_CLK_ENABLE();

	if( input == 1 ) {
		GPIO_InitStruct.Pin = GPIO_PIN_4;
		SPDIFRX_HandleStruct.Init.InputSelection = SPDIFRX_INPUT_IN2;
	}
	else if( input == 2 ) {
		GPIO_InitStruct.Pin = GPIO_PIN_5;
		SPDIFRX_HandleStruct.Init.InputSelection = SPDIFRX_INPUT_IN3;
	}
	else
		return;

	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF8_SPDIFRX;
	HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

	/* Configure SPDIFRX Peripheral */
	SPDIFRX_HandleStruct.Instance = SPDIFRX;
	HAL_SPDIFRX_DeInit( &SPDIFRX_HandleStruct );

	SPDIFRX_HandleStruct.Init.Retries = SPDIFRX_MAXRETRIES_15;
	SPDIFRX_HandleStruct.Init.WaitForActivity = SPDIFRX_WAITFORACTIVITY_ON;
	SPDIFRX_HandleStruct.Init.ChannelSelection = SPDIFRX_CHANNEL_A;
	SPDIFRX_HandleStruct.Init.DataFormat = SPDIFRX_DATAFORMAT_MSB;
	SPDIFRX_HandleStruct.Init.StereoMode = SPDIFRX_STEREOMODE_ENABLE;
	SPDIFRX_HandleStruct.Init.PreambleTypeMask = SPDIFRX_PREAMBLETYPEMASK_ON;
	SPDIFRX_HandleStruct.Init.ChannelStatusMask = SPDIFRX_CHANNELSTATUS_ON;
	if( HAL_SPDIFRX_Init( &SPDIFRX_HandleStruct ) != HAL_OK ) {
		while( 1 );
	}
}

void SPDIF_RX_IRQHandler(void) {
	HAL_SPDIFRX_IRQHandler( &SPDIFRX_HandleStruct );
}
