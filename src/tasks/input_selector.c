#include "stm32f7xx_hal.h"
#include "input_selector.h"
#include "lcd.h"

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

static void inputSelector_DisableAll( void );

void inputSelector_Setup( void ) {
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOI_CLK_ENABLE();
	__HAL_RCC_GPIOJ_CLK_ENABLE();
	// buttons initialization
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10 |
			GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );

	// mux initialization
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
}

void inputSelector_Task( void *pvParameters ) {
	static uint8_t inputSelected = 0xFF;	// no input selected at first
	vTaskDelay( 3000 / portTICK_PERIOD_MS );
	lcd_WriteHexa( 0xFF, 0, 30 );
	for( ;; ) {
		if( SELECTOR_P2 && inputSelected != 1 ) {
			inputSelected = 1;
			inputSelector_DisableAll();
			ENABLE_IN1;
			lcd_WriteHexa( 1, 0, 30 );
		}
		if( SELECTOR_RCA && inputSelected != 2 ) {
			inputSelected = 2;
			inputSelector_DisableAll();
			ENABLE_IN2;
			lcd_WriteHexa( 2, 0, 30 );
		}
		if( SELECTOR_BT && inputSelected != 3) {
			inputSelected = 3;
			inputSelector_DisableAll();
			ENABLE_IN3;
			lcd_WriteHexa( 3, 0, 30 );
		}
		if( SELECTOR_SPDIF && inputSelected != 4 ) {
			inputSelected = 4;
			inputSelector_DisableAll();
			ENABLE_IN4;
			lcd_WriteHexa( 4, 0, 30 );
			// wake-up the spdif task here
		}
		if( SELECTOR_OPTIC && inputSelected != 5 ) {
			inputSelected = 5;
			inputSelector_DisableAll();
			ENABLE_IN4;
			lcd_WriteHexa( 5, 0, 30 );
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
