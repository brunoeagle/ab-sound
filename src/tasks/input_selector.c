#include "stm32f7xx_hal.h"
#include "input_selector.h"
#include "display.h"
#include "u8g2.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define	DISABLE_IN1		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_0, GPIO_PIN_SET )
#define	DISABLE_IN2		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_1, GPIO_PIN_SET )
#define	DISABLE_IN3		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_2, GPIO_PIN_SET )
#define	DISABLE_IN4		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_3, GPIO_PIN_SET )
#define	ENABLE_IN1		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_0, GPIO_PIN_RESET )
#define	ENABLE_IN2		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_1, GPIO_PIN_RESET )
#define	ENABLE_IN3		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_2, GPIO_PIN_RESET )
#define	ENABLE_IN4		HAL_GPIO_WritePin( GPIOJ, GPIO_PIN_3, GPIO_PIN_RESET )
#define	SELECTOR_P2		( HAL_GPIO_ReadPin( GPIOI, GPIO_PIN_13 ) == GPIO_PIN_RESET )
#define	SELECTOR_RCA	( HAL_GPIO_ReadPin( GPIOI, GPIO_PIN_12 ) == GPIO_PIN_RESET )
#define	SELECTOR_BT		( HAL_GPIO_ReadPin( GPIOI, GPIO_PIN_11 ) == GPIO_PIN_RESET )
#define	SELECTOR_SPDIF	( HAL_GPIO_ReadPin( GPIOI, GPIO_PIN_10 ) == GPIO_PIN_RESET )
#define	SELECTOR_OPTIC	( HAL_GPIO_ReadPin( GPIOI, GPIO_PIN_9 ) == GPIO_PIN_RESET )

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

volatile uint8_t inputSelected = 0;

void inputSelector_Setup( void ) {
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	__HAL_RCC_GPIOC_CLK_ENABLE();
	__HAL_RCC_GPIOE_CLK_ENABLE();

	// buttons init
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10 |
			GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );

	// input selection init
	DISABLE_IN1;
	DISABLE_IN2;
	DISABLE_IN3;
	DISABLE_IN4;
	GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStruct.Pin = GPIO_PIN_0 | GPIO_PIN_1 |
			GPIO_PIN_2 | GPIO_PIN_3;
	HAL_GPIO_Init( GPIOJ, &GPIO_InitStruct );

	// filter/no filter selection
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

	// mute/stand-by init
	MUTE_ON;
	STANDBY_ON;
	GPIO_InitStruct.Pin = GPIO_PIN_5 | GPIO_PIN_4;
	HAL_GPIO_Init( GPIOE, &GPIO_InitStruct );

}

void inputSelector_Task( void *pvParameters ) {
	vTaskDelay( 50 / portTICK_PERIOD_MS );
	for( ;; ) {
		if( SELECTOR_P2 && inputSelected != 1 ) {
			inputSelected = 1;
			inputSelector_DisableAll();
			ENABLE_IN1;
			inputSelector_Mute( 0 );
		}
		if( SELECTOR_RCA && inputSelected != 2 ) {
			inputSelected = 2;
			inputSelector_DisableAll();
			ENABLE_IN2;
			inputSelector_Mute( 1 );
		}
		if( SELECTOR_BT && inputSelected != 3) {
			inputSelected = 3;
			inputSelector_DisableAll();
			ENABLE_IN3;
			inputSelector_Mute( 1 );
		}
		if( SELECTOR_SPDIF && inputSelected != 4 ) {
			inputSelected = 4;
			inputSelector_DisableAll();
			ENABLE_IN4;
			inputSelector_Mute( 0 );
			// wake-up the spdif task here
		}
		if( SELECTOR_OPTIC && inputSelected != 5 ) {
			inputSelected = 5;
			inputSelector_DisableAll();
			ENABLE_IN4;
			inputSelector_Mute( 0 );
			// wake-up the spdif task here
		}
	}
}

static void inputSelector_DisableAll( void ) {
	DISABLE_IN1;
	DISABLE_IN2;
	DISABLE_IN3;
	DISABLE_IN4;
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

uint8_t inputSelector_GetCurrentInput( void ) {
	return inputSelected;
}
