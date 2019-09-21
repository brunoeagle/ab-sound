#include "stm32f7xx_hal.h"
#include "spdif.h"

RCC_PeriphCLKInitTypeDef RCC_PerInitStruct;
GPIO_InitTypeDef GPIO_InitStruct;
SAI_InitTypeDef SAI_InitStruct;
DMA_InitTypeDef DMA_InitStruct;
SPDIFRX_InitTypeDef SPDIFRX_InitStruct;
SPDIFRX_HandleTypeDef SPDIFRX_HandleStruct;
uint32_t rxBuffer[ 1000 ];


void spdif_Setup( uint8_t input ) {
	__HAL_RCC_SPDIFRX_CLK_ENABLE();

	HAL_RCCEx_GetPeriphCLKConfig( &RCC_PerInitStruct );

	// SPDIF-RX clock config (requires at least 135.2MHz to detect 192KHz audio rate)
	RCC_PerInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SPDIFRX;
	RCC_PerInitStruct.PLLI2S.PLLI2SN= 300;					// Input to PLLI2S block is 1MHZ
	RCC_PerInitStruct.PLLI2S.PLLI2SP = RCC_PLLI2SP_DIV2;	// 300MHz/2 = 150MHz
	if( HAL_RCCEx_PeriphCLKConfig( &RCC_PerInitStruct ) != HAL_OK )
		while( 1 );

	// SPDIF interrupt config
	HAL_NVIC_SetPriority( SPDIF_RX_IRQn,  0, 1);
	HAL_NVIC_EnableIRQ( SPDIF_RX_IRQn );

	// GPIO config
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
	__HAL_RCC_GPIOC_CLK_ENABLE();
	GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull      = GPIO_NOPULL;
	GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_VERY_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF8_SPDIFRX;
	GPIO_InitStruct.Pin = GPIO_PIN_4;
	HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );

	// SPDIF-RX config
	SPDIFRX_HandleStruct.Instance = SPDIFRX;
	HAL_SPDIFRX_DeInit( &SPDIFRX_HandleStruct );
	SPDIFRX_HandleStruct.Init.InputSelection = SPDIFRX_INPUT_IN2;
	SPDIFRX_HandleStruct.Init.Retries = SPDIFRX_MAXRETRIES_63;
	SPDIFRX_HandleStruct.Init.WaitForActivity = SPDIFRX_WAITFORACTIVITY_ON;
	SPDIFRX_HandleStruct.Init.ChannelSelection = SPDIFRX_CHANNEL_A;
	SPDIFRX_HandleStruct.Init.DataFormat = SPDIFRX_DATAFORMAT_MSB;
	SPDIFRX_HandleStruct.Init.StereoMode = SPDIFRX_STEREOMODE_ENABLE;
	SPDIFRX_HandleStruct.Init.PreambleTypeMask = SPDIFRX_PREAMBLETYPEMASK_ON;
	SPDIFRX_HandleStruct.Init.ChannelStatusMask = SPDIFRX_CHANNELSTATUS_ON;
	SPDIFRX_HandleStruct.Init.ValidityBitMask = SPDIFRX_VALIDITYMASK_ON;
	SPDIFRX_HandleStruct.Init.ParityErrorMask = SPDIFRX_PARITYERRORMASK_ON;

	SPDIFRX_HandleStruct.pRxBuffPtr = rxBuffer;
	SPDIFRX_HandleStruct.RxXferSize = 192;
	SPDIFRX_HandleStruct.RxXferCount = 192;
	SPDIFRX_HandleStruct.CsXferSize = 192;
	SPDIFRX_HandleStruct.CsXferCount = 192;
	SPDIFRX_HandleStruct.hdmaCsRx = NULL;
	SPDIFRX_HandleStruct.hdmaDrRx = NULL;
	SPDIFRX_HandleStruct.Lock = 0;
	SPDIFRX_HandleStruct.State = HAL_SPDIFRX_STATE_RESET;
	SPDIFRX_HandleStruct.ErrorCode = HAL_SPDIFRX_ERROR_NONE;

	if( HAL_SPDIFRX_Init( &SPDIFRX_HandleStruct ) != HAL_OK ) {
		while( 1 );
	}
}

void SPDIF_RX_IRQHandler(void) {
	HAL_SPDIFRX_IRQHandler( &SPDIFRX_HandleStruct );
}
