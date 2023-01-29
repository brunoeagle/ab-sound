#include "stm32f7xx_hal.h"
#include "exti.h"
#include "input_selector.h"
#include "display.h"
#include "u8g2.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define	DISABLE_P2_INPUT	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_0, GPIO_PIN_SET )
#define	DISABLE_RCA_INPUT	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_1, GPIO_PIN_SET )
#define	DISABLE_DAC_INPUT	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_2, GPIO_PIN_SET )
#define	DISABLE_BT_INPUT	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_3, GPIO_PIN_SET )
#define	ENABLE_P2_INPUT		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_0, GPIO_PIN_RESET )
#define	ENABLE_RCA_INPUT	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_1, GPIO_PIN_RESET )
#define	ENABLE_DAC_INPUT	HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_2, GPIO_PIN_RESET )
#define	ENABLE_BT_INPUT		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_3, GPIO_PIN_RESET )

#define	DISABLE_L_FILTER		HAL_GPIO_WritePin( GPIOC, GPIO_PIN_15, GPIO_PIN_SET )
#define	DISABLE_L_NO_FILTER		HAL_GPIO_WritePin( GPIOC, GPIO_PIN_14, GPIO_PIN_SET )
#define	DISABLE_R_FILTER		HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, GPIO_PIN_SET )
#define	DISABLE_R_NO_FILTER		HAL_GPIO_WritePin( GPIOI, GPIO_PIN_8, GPIO_PIN_SET )
#define	ENABLE_L_FILTER			HAL_GPIO_WritePin( GPIOC, GPIO_PIN_15, GPIO_PIN_RESET )
#define	ENABLE_L_NO_FILTER		HAL_GPIO_WritePin( GPIOC, GPIO_PIN_14, GPIO_PIN_RESET )
#define	ENABLE_R_FILTER			HAL_GPIO_WritePin( GPIOC, GPIO_PIN_13, GPIO_PIN_RESET )
#define	ENABLE_R_NO_FILTER		HAL_GPIO_WritePin( GPIOI, GPIO_PIN_8, GPIO_PIN_RESET )

#define	MUTE_ON					HAL_GPIO_WritePin( GPIOE, GPIO_PIN_5, GPIO_PIN_RESET )
#define	MUTE_OFF				HAL_GPIO_WritePin( GPIOE, GPIO_PIN_5, GPIO_PIN_SET )
#define	STANDBY_ON				HAL_GPIO_WritePin( GPIOE, GPIO_PIN_4, GPIO_PIN_RESET )
#define	STANDBY_OFF				HAL_GPIO_WritePin( GPIOE, GPIO_PIN_4, GPIO_PIN_SET )

static void inputSelector_DisableAll( void );
static void inputSelector_Mute( uint8_t mute );
static void inputSelectorP2Interrupt( void );
static void inputSelectorRCAInterrupt( void );
static void inputSelectorBTInterrupt( void );
static void inputSelectorSPDIFInterrupt( void );
static void inputSelectorOpticInterrupt( void );

static volatile int8_t inputSelected = INPUT_NONE;
static QueueHandle_t selectionQueue;

void inputSelector_Setup( void ) {
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	// Input selector buttons init
	// Optic button
	exti_InitInput( 9, GPIOI, GPIO_MODE_IT_FALLING, GPIO_PULLUP );
	exti_AddCallback( 9, &inputSelectorOpticInterrupt );
	// SPDIF button
	exti_InitInput( 10, GPIOI, GPIO_MODE_IT_FALLING, GPIO_PULLUP );
	exti_AddCallback( 10, &inputSelectorSPDIFInterrupt );
	// BT button
	exti_InitInput( 11, GPIOI, GPIO_MODE_IT_FALLING, GPIO_PULLUP );
	exti_AddCallback( 11, &inputSelectorBTInterrupt );
	// RCA button
	exti_InitInput( 12, GPIOI, GPIO_MODE_IT_FALLING, GPIO_PULLUP );
	exti_AddCallback( 12, &inputSelectorRCAInterrupt );
	// P2
	exti_InitInput( 13, GPIOI, GPIO_MODE_IT_FALLING, GPIO_PULLUP );
	exti_AddCallback( 13, &inputSelectorP2Interrupt );

	// Input selector init
	DISABLE_P2_INPUT;
	DISABLE_RCA_INPUT;
	DISABLE_DAC_INPUT;
	DISABLE_BT_INPUT;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_LOW;
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 |
			GPIO_PIN_2 | GPIO_PIN_3;
	HAL_GPIO_Init( GPIOJ, &GPIO_InitStruct );

	// Filter/no filter init
	DISABLE_L_FILTER;
	DISABLE_L_NO_FILTER;
	DISABLE_R_FILTER;
	DISABLE_R_NO_FILTER;
	ENABLE_L_NO_FILTER;
	ENABLE_R_NO_FILTER;
	GPIO_InitStruct.Pin = GPIO_PIN_13 | GPIO_PIN_14 |
			GPIO_PIN_15;
	HAL_GPIO_Init( GPIOC, &GPIO_InitStruct );
	GPIO_InitStruct.Pin = GPIO_PIN_8;
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );

	// Mute/stand-by init
	MUTE_ON;
	STANDBY_ON;
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_4;
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );

	// Selection interrupt queue
	selectionQueue = xQueueCreate( 10, sizeof( int8_t ) );
	while( selectionQueue == NULL );
}

void inputSelector_Task( void *pvParameters ) {
	uint8_t selectionReceived;

	for( ;; ) {
        if( xQueueReceive( selectionQueue, &selectionReceived, portMAX_DELAY ) != pdPASS )
            continue;
		if( inputSelected == selectionReceived )
			continue;
		inputSelected = selectionReceived;
		switch( inputSelected ) {
			case INPUT_P2:
				inputSelector_DisableAll();
				ENABLE_P2_INPUT;
				inputSelector_Mute( 0 );
			break;
			case INPUT_RCA:
				inputSelector_DisableAll();
				ENABLE_RCA_INPUT;
				inputSelector_Mute( 0 );
			break;
			case INPUT_BT:
				inputSelector_DisableAll();
				ENABLE_BT_INPUT;
				inputSelector_Mute( 0 );
			break;
			case INPUT_SPDIF:
				inputSelector_DisableAll();
				ENABLE_DAC_INPUT;
				inputSelector_Mute( 0 );
				// init spdif task
			break;
			case INPUT_OPTIC:
				inputSelector_DisableAll();
				ENABLE_DAC_INPUT;
				inputSelector_Mute( 0 );
				// init optic task
			break;
			default:
				inputSelected = INPUT_NONE;
				inputSelector_DisableAll();
				inputSelector_Mute( 1 );
			break;
		}
	}
}

static void inputSelector_DisableAll( void ) {
	DISABLE_P2_INPUT;
	DISABLE_RCA_INPUT;
	DISABLE_DAC_INPUT;
	DISABLE_BT_INPUT;
}

static void inputSelector_Mute( uint8_t mute ) {
	if( mute ) {
		MUTE_ON;
		STANDBY_ON;
	}
	else {
		MUTE_OFF;
		STANDBY_OFF;
	}
}

int8_t inputSelector_GetCurrentInput( void ) {
	return inputSelected;
}

static void inputSelectorP2Interrupt( void ) {
    int8_t evt = INPUT_P2;
    xQueueSendFromISR( selectionQueue, &evt, NULL );
}

static void inputSelectorRCAInterrupt( void ) {
    int8_t evt = INPUT_RCA;
    xQueueSendFromISR( selectionQueue, &evt, NULL );
}

static void inputSelectorBTInterrupt( void ) {
    int8_t evt = INPUT_BT;
    xQueueSendFromISR( selectionQueue, &evt, NULL );
}

static void inputSelectorSPDIFInterrupt( void ) {
    int8_t evt = INPUT_SPDIF;
    xQueueSendFromISR( selectionQueue, &evt, NULL );
}

static void inputSelectorOpticInterrupt( void ) {
    int8_t evt = INPUT_OPTIC;
    xQueueSendFromISR( selectionQueue, &evt, NULL );
}