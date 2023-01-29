#include "stm32f7xx_hal.h"
#include "sai1.h"

static SAI_HandleTypeDef SAI_HandleStruct;
static RCC_PeriphCLKInitTypeDef RCC_PerInitStruct;
static GPIO_InitTypeDef GPIO_InitStruct;

SemaphoreHandle_t sai1Mutex;

void sai1_Setup( void ) {
	__HAL_RCC_SAI1_CLK_ENABLE();

	HAL_RCCEx_GetPeriphCLKConfig( &RCC_PerInitStruct );

	// AK4430 requires 36,8640MHz for 192KHz audio sampling
	RCC_PerInitStruct.PeriphClockSelection = RCC_PERIPHCLK_SAI1;
	RCC_PerInitStruct.Sai1ClockSelection = RCC_SAI1CLKSOURCE_PLLSAI;	// SAI1 clock comes from PLLSAI
	RCC_PerInitStruct.PLLSAI.PLLSAIN = 258;		// Input to PLLSAI block is 1MHZ, output 258MHz
	RCC_PerInitStruct.PLLSAI.PLLSAIQ = 7;		// 258 � 7 = 36,8571MHz
	RCC_PerInitStruct.PLLSAIDivQ = 1;			// 36,8571 � 1 = 36,8571MHz to SAI1 and SAI2, 0,186% deviation to 36,8640MHz
	if( HAL_RCCEx_PeriphCLKConfig( &RCC_PerInitStruct ) != HAL_OK )
		while( 1 );

	// SD | MCLK | SCK | FS GPIO's
	__HAL_RCC_GPIOF_CLK_ENABLE();
	GPIO_InitStruct.Pin = GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_8 | GPIO_PIN_9;
	GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_HIGH;
	GPIO_InitStruct.Alternate = GPIO_AF6_SAI1;
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	// DIF low, 24-bit MSB format
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_NOPULL;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	HAL_GPIO_Init( GPIOF, &GPIO_InitStruct );
	HAL_GPIO_WritePin( GPIOF, GPIO_PIN_10, GPIO_PIN_RESET );

	__HAL_SAI_RESET_HANDLE_STATE( &SAI_HandleStruct );
	SAI_HandleStruct.Instance = SAI1_Block_B;
	__HAL_SAI_DISABLE( &SAI_HandleStruct );
	SAI_HandleStruct.Init.AudioMode      = SAI_MODEMASTER_TX;
	SAI_HandleStruct.Init.Synchro        = SAI_ASYNCHRONOUS;
	SAI_HandleStruct.Init.OutputDrive    = SAI_OUTPUTDRIVE_DISABLE;
	SAI_HandleStruct.Init.NoDivider      = SAI_MASTERDIVIDER_ENABLE;
	SAI_HandleStruct.Init.FIFOThreshold  = SAI_FIFOTHRESHOLD_1QF;
	SAI_HandleStruct.Init.AudioFrequency = SAI_AUDIO_FREQUENCY_96K;
	SAI_HandleStruct.Init.Protocol       = SAI_FREE_PROTOCOL;
	SAI_HandleStruct.Init.DataSize       = SAI_DATASIZE_32;
	SAI_HandleStruct.Init.FirstBit       = SAI_FIRSTBIT_MSB;
	SAI_HandleStruct.Init.ClockStrobing  = SAI_CLOCKSTROBING_FALLINGEDGE;

	SAI_HandleStruct.FrameInit.FrameLength       = 64;
	SAI_HandleStruct.FrameInit.ActiveFrameLength = 32;
	SAI_HandleStruct.FrameInit.FSDefinition      = SAI_FS_STARTFRAME;
	SAI_HandleStruct.FrameInit.FSPolarity        = SAI_FS_ACTIVE_LOW;
	SAI_HandleStruct.FrameInit.FSOffset          = SAI_FS_FIRSTBIT;

	SAI_HandleStruct.SlotInit.FirstBitOffset = 0;
	SAI_HandleStruct.SlotInit.SlotSize       = SAI_SLOTSIZE_32B;
	SAI_HandleStruct.SlotInit.SlotNumber     = 2;
	SAI_HandleStruct.SlotInit.SlotActive     = (SAI_SLOTACTIVE_0 | SAI_SLOTACTIVE_1);

	if( HAL_SAI_Init( &SAI_HandleStruct ) != HAL_OK ) {
		while( 1 );
	}

	__HAL_SAI_ENABLE( &SAI_HandleStruct );

	sai1Mutex = xSemaphoreCreateMutex();
	while( sai1Mutex == NULL );
}
