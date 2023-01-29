#include "stm32f7xx.h"
#include "stdlib.h"
#include "display.h"
#include "string.h"
#include "digital_trimpots.h"
#include "input_selector.h"
#include "volume_control.h"
#include "u8g2.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#define	RD_HIGH			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_SET )		// RD high
#define	RD_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_9, GPIO_PIN_RESET )		// RD low
#define WR_HIGH			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_10, GPIO_PIN_SET )		// WR high
#define WR_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_10, GPIO_PIN_RESET )		// WR low
#define	DC_HIGH			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_11, GPIO_PIN_SET )		// DC high
#define	DC_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_11, GPIO_PIN_RESET )		// DC low
#define RST_HIGH		HAL_GPIO_WritePin( GPIOG, GPIO_PIN_12, GPIO_PIN_SET )		// RST high
#define RST_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_12, GPIO_PIN_RESET )		// RST low
#define CS_HIGH			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_13, GPIO_PIN_SET )		// CS high
#define CS_LOW			HAL_GPIO_WritePin( GPIOG, GPIO_PIN_13, GPIO_PIN_RESET )		// CS low

SemaphoreHandle_t displayMutex;

u8g2_t display;
uint8_t cb1(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
uint8_t cb2(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr);
const char STR_INPUTS[] = { "   P2      RCA      BT     SPDIF    OPTIC  " };

void display_Setup( void ) {
	displayMutex = xSemaphoreCreateMutex();
	while( displayMutex == NULL );
}

void display_Task( void *pvParameters ) {
	int8_t volumeHigh = -1, volumeLow = -1;
	uint8_t inputSelected = INPUT_NONE;
	uint8_t inputArrowOffset = -1;
	char volumeHighStr[] = { "HIGH: 000" };
	char volumeLowStr[] = { "LOW: 000" };

	if( xSemaphoreTake( displayMutex, portMAX_DELAY ) == pdTRUE ) {
		u8g2_Setup_ssd1322_nhd_256x64_f( &display, U8G2_R0, cb1, cb2);
		u8g2_InitDisplay( &display ); // send init sequence to the display, display is in sleep mode after this,
		u8g2_ClearBuffer( &display );
		u8g2_ClearDisplay( &display );
		u8g2_SetFlipMode( &display, 1 );
		u8g2_SetPowerSave( &display, 0 ); // wake up display
		u8g2_SetFontMode( &display, 1 );
		u8g2_SetDrawColor( &display, 1 );
		u8g2_SetFont( &display, u8g2_font_mozart_nbp_t_all );
		xSemaphoreGive( displayMutex );
	}
	for( ;; ) {
		if( ( volumeHigh == volumeControl_GetCurrentVolume( LEFT_TRIMPOT ) )  &&
		    ( volumeLow == volumeControl_GetCurrentVolume( SUBWOOFER_TRIMPOT ) ) &&
			( inputSelected == inputSelector_GetCurrentInput() ) ) {
				taskYIELD();
				continue;
		}
		volumeHigh = volumeControl_GetCurrentVolume( LEFT_TRIMPOT );
		volumeLow = volumeControl_GetCurrentVolume( SUBWOOFER_TRIMPOT );
		inputSelected = inputSelector_GetCurrentInput();
		itoa( volumeHigh, &volumeHighStr[ 6 ], 10 );
		itoa( volumeLow, &volumeLowStr[ 5 ], 10 );
		if( inputSelected )
			inputArrowOffset = 20 + ( 51 * ( inputSelected - 1 ) );
		if( xSemaphoreTake( displayMutex, portMAX_DELAY ) == pdTRUE ) {
			u8g2_ClearBuffer( &display );
			u8g2_DrawStr( &display, 110, 20, volumeHighStr );
			u8g2_DrawStr( &display, 116, 28, volumeLowStr );
			u8g2_DrawStr( &display, 0, 57, STR_INPUTS );
			if( inputSelected )
				u8g2_DrawTriangle( &display, inputArrowOffset, 63, inputArrowOffset + 6, 63, inputArrowOffset + 3, 58 );
			u8g2_SendBuffer( &display );
			xSemaphoreGive( displayMutex );
		}
	}
}

uint8_t cb1(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
	uint8_t *data;

	switch(msg){
		case U8X8_MSG_BYTE_SEND:
			data = (uint8_t *)arg_ptr;
			while( arg_int > 0 ) {
				GPIOK->ODR = *data;
				//asm("NOP");
				WR_LOW;
				//asm("NOP");
				WR_HIGH;
				data++;
				arg_int--;
			}
			break;

		case U8X8_MSG_BYTE_INIT:
			CS_HIGH;
			WR_HIGH;
			break;

		case U8X8_MSG_BYTE_SET_DC:
			u8x8_gpio_SetDC( u8x8, arg_int );
			break;

		case U8X8_MSG_BYTE_START_TRANSFER:
			CS_LOW;
			u8x8->gpio_and_delay_cb( u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->post_chip_enable_wait_ns, NULL);
			break;

		case U8X8_MSG_BYTE_END_TRANSFER:
			u8x8->gpio_and_delay_cb( u8x8, U8X8_MSG_DELAY_NANO, u8x8->display_info->pre_chip_disable_wait_ns, NULL );
			CS_HIGH;
			break;

		default:
			return 0;
	}
	return 1;
}

uint8_t cb2(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr) {
	GPIO_InitTypeDef GPIO_InitStruct;
	volatile uint8_t i;
	switch(msg) {
		case U8X8_MSG_GPIO_AND_DELAY_INIT:	// called once during init phase of u8g2/u8x8

			__HAL_RCC_GPIOG_CLK_ENABLE();
			__HAL_RCC_GPIOK_CLK_ENABLE();

			// DB7~DB0 lines
			GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
			GPIO_InitStruct.Pull = GPIO_PULLUP;
			GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
			GPIO_InitStruct.Pin = GPIO_PIN_All;
			HAL_GPIO_Init( GPIOK, &GPIO_InitStruct );

			// RD, WR, DC, RST, CS respectively
			GPIO_InitStruct.Pin = GPIO_PIN_9 | GPIO_PIN_10 |
						GPIO_PIN_11 | GPIO_PIN_12 | GPIO_PIN_13;
			HAL_GPIO_Init( GPIOG, &GPIO_InitStruct );
			RD_HIGH;
			break;

		case U8X8_MSG_DELAY_NANO:			// delay arg_int * 1 nano second
			break;

		case U8X8_MSG_DELAY_100NANO:		// delay arg_int * 100 nano seconds
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			asm("NOP");
			break;

		case U8X8_MSG_DELAY_10MICRO:		// delay arg_int * 10 micro seconds
			for(i = 0; i < 100; i++ ) {
				asm("NOP");
				asm("NOP");
				asm("NOP");
				asm("NOP");
				asm("NOP");
				asm("NOP");
				asm("NOP");
				asm("NOP");
			}
			break;

		case U8X8_MSG_DELAY_MILLI:			// delay arg_int * 1 milli second
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			break;
    case U8X8_MSG_GPIO_E:				// E/WR pin: Output level in arg_int
    	if( arg_int )
    		WR_HIGH;
    	else
    		WR_LOW;
    	break;

    case U8X8_MSG_GPIO_CS:				// CS (chip select) pin: Output level in arg_int
    	if( arg_int )
    		CS_HIGH;
    	else
    		CS_LOW;
    	break;

	case U8X8_MSG_GPIO_DC:				// DC (data/cmd, A0, register select) pin: Output level in arg_int
    	if( arg_int )
    		DC_HIGH;
    	else
    		DC_LOW;
		break;

	case U8X8_MSG_GPIO_RESET:			// Reset pin: Output level in arg_int
    	if( arg_int )
    		RST_HIGH;
    	else
    		RST_LOW;
		break;

	case U8X8_MSG_GPIO_CS1:				// CS1 (chip select) pin: Output level in arg_int
    	if( arg_int )
    		CS_HIGH;
    	else
    		CS_LOW;
		break;

    case U8X8_MSG_GPIO_MENU_SELECT:
      u8x8_SetGPIOResult(u8x8, /* get menu select pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_NEXT:
      u8x8_SetGPIOResult(u8x8, /* get menu next pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_PREV:
      u8x8_SetGPIOResult(u8x8, /* get menu prev pin state */ 0);
      break;
    case U8X8_MSG_GPIO_MENU_HOME:
      u8x8_SetGPIOResult(u8x8, /* get menu home pin state */ 0);
      break;
    default:
      u8x8_SetGPIOResult(u8x8, 1);			// default return value
      break;
  }
  return 1;
}
