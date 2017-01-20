#include "stm32f7xx_hal.h"
#include "volume_control.h"
#include "digital_trimpots.h"
#include "lcd.h"
#include "spi1.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// Digital trimpot goes from 0x00 to 0xFF, where 0x00 is minimum gain, and 0xFF the maximum in our circuit
#define DEFAULT_GAIN_LOW		0x00
#define	DEFAULT_GAIN_HIGH		0x00
#define DEFAULT_GAIN_MASTER		0x00

volatile uint8_t lowChannelGain;
volatile uint8_t highChannelGain;

static void volumeControl_StateMachine( uint8_t channel, volatile uint8_t *stateMachine, uint16_t pinUp,
		uint16_t pinDown, volatile TickType_t *lastActivity,
		volatile uint8_t *counter );
static void volumeControl_SetVolume( uint8_t channel, uint8_t gain );
static void volumeControl_IncreaseVolume( uint8_t channel );
static void volumeControl_DecreaseVolume( uint8_t channel );
static void volumeControl_WriteHexa( uint8_t value, uint8_t line, uint8_t col );

void volumeControl_Setup( void ) {
	GPIO_InitTypeDef GPIO_InitStruct;

	__HAL_RCC_GPIOI_CLK_ENABLE();
	// Setup the I/O of the volume trimpots
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
	GPIO_InitStruct.Pin = GPIO_PIN_4 | GPIO_PIN_5 |
			GPIO_PIN_6 | GPIO_PIN_7 | GPIO_PIN_14 |
			GPIO_PIN_15;
	HAL_GPIO_Init( GPIOI, &GPIO_InitStruct );
}

void volumeControl_Task( void *pvParameters ) {
	volatile uint8_t masterStateMachine = 0;
	volatile uint8_t highStateMachine = 0;
	volatile uint8_t lowStateMachine = 0;
	volatile uint8_t lowCounter, highCounter, masterCounter;
	volatile TickType_t lastLowActivity;
	volatile TickType_t lastHighActivity;
	volatile TickType_t lastMasterActivity;

	lowChannelGain = DEFAULT_GAIN_LOW;
	highChannelGain = DEFAULT_GAIN_HIGH;

	/*volumeControl_SetIndividual( LEFT_CHANNEL, leftChannelGain );
	volumeControl_SetIndividual( RIGHT_CHANNEL, rightChannelGain );
	volumeControl_SetIndividual( SUBWOOFER, subWooferGain );*/

	vTaskDelay( 2000 / portTICK_PERIOD_MS );
	lcd_WriteNumber( 0, 0, 0 );
	for( ;; ) {
		volumeControl_StateMachine( LOW_ENCODER, &lowStateMachine, GPIO_PIN_14, GPIO_PIN_15, &lastLowActivity, &lowCounter );
		volumeControl_StateMachine( HIGH_ENCODER, &highStateMachine, GPIO_PIN_6, GPIO_PIN_7, &lastHighActivity, &highCounter );
		volumeControl_StateMachine( MASTER_ENCODER, &masterStateMachine, GPIO_PIN_4, GPIO_PIN_5, &lastMasterActivity, &masterCounter );
	}
}

static void volumeControl_StateMachine( uint8_t channel, volatile uint8_t *stateMachine,
		uint16_t pinUp, uint16_t pinDown, volatile TickType_t *lastActivity,
		volatile uint8_t *counter ) {
	switch( *stateMachine ) {
		case 0:
			if( !HAL_GPIO_ReadPin( GPIOI, pinUp ) )
				*stateMachine = 1;
			else if( !HAL_GPIO_ReadPin( GPIOI, pinDown ) )
				*stateMachine = 2;
			*lastActivity = xTaskGetTickCount();
			*counter = 0;
			break;
		case 1:		// increase	volume
			if( !HAL_GPIO_ReadPin( GPIOI, pinUp ) && !HAL_GPIO_ReadPin( GPIOI, pinDown ) )
				(*counter)++;
			if( *counter >= 20 ) {
				// increase volume on channel
				volumeControl_IncreaseVolume( channel );
				*stateMachine = 3;
				*counter = 0;
			}
			else if( ( xTaskGetTickCount() - *lastActivity ) > 20 )
				*stateMachine = 0;
			break;
		case 2:		// decrease volume
			if( !HAL_GPIO_ReadPin( GPIOI, pinUp ) && !HAL_GPIO_ReadPin( GPIOI, pinDown ) )
				(*counter)++;
			if( *counter >= 20 ) {
				// decrease volume on channel
				volumeControl_DecreaseVolume( channel );
				*stateMachine = 3;
				*counter = 0;
			}
			else if( ( xTaskGetTickCount() - *lastActivity ) > 20 )
				*stateMachine = 0;
			break;
		case 3:
			if( HAL_GPIO_ReadPin( GPIOI, pinUp ) && HAL_GPIO_ReadPin( GPIOI, pinDown ) )
				(*counter)++;
			if( *counter >= 30 )
				*stateMachine = 0;
			break;
		default:
			*stateMachine = 0;
			break;
		}
}

static void volumeControl_SetVolume( uint8_t channel, uint8_t gain ) {

}

static void volumeControl_IncreaseVolume( uint8_t channel ) {
	uint8_t volumeSet, res;
	switch( channel ) {
		case LOW_ENCODER:
			HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_RESET );
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			spi1_WriteByte( 0x06 );
			res = spi1_ReturnReceivedByte();
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_SET );
			vTaskDelay( 5 / portTICK_PERIOD_MS );

			HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_RESET );
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			spi1_WriteByte( 0x0E );
			res = spi1_ReturnReceivedByte();
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			spi1_WriteByte( 0xFF );
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_SET );
			volumeSet = spi1_ReturnReceivedByte();
			volumeControl_WriteHexa( volumeSet, 0, 0 );
			volumeControl_WriteHexa( res, 0, 6 );
			break;
		case HIGH_ENCODER:
			// increase volume in both right and left trimpot
			highChannelGain++;
			volumeControl_WriteHexa( highChannelGain, 0, 6 );
			break;
		case MASTER_ENCODER:
			// increase volume in right, left and low trimpot
			lowChannelGain++;
			volumeControl_WriteHexa( lowChannelGain, 0, 0 );
			highChannelGain++;
			volumeControl_WriteHexa( highChannelGain, 0, 6 );
			break;
		default:
			break;
	}
}

static void volumeControl_DecreaseVolume( uint8_t channel ) {
	uint8_t res;
	uint8_t volumeSet;
	switch( channel ) {
		case LOW_ENCODER:
			HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_RESET );
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			spi1_WriteByte( 0x08 );
			res = spi1_ReturnReceivedByte();
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_SET );
			vTaskDelay( 5 / portTICK_PERIOD_MS );

			HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_RESET );
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			spi1_WriteByte( 0x0C );
			res = spi1_ReturnReceivedByte();
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			spi1_WriteByte( 0xFF );
			vTaskDelay( 1 / portTICK_PERIOD_MS );
			HAL_GPIO_WritePin( GPIOA, GPIO_PIN_2, GPIO_PIN_SET );
			volumeSet = spi1_ReturnReceivedByte();
			volumeControl_WriteHexa( volumeSet, 0, 0 );
			volumeControl_WriteHexa( res, 0, 6 );
			break;
		case HIGH_ENCODER:
			// increase volume in both right and left trimpot
			highChannelGain--;
			volumeControl_WriteHexa( highChannelGain, 0, 6 );
			break;
		case MASTER_ENCODER:
			// increase volume in right, left and low trimpot
			lowChannelGain--;
			volumeControl_WriteHexa( lowChannelGain, 0, 0 );
			highChannelGain--;
			volumeControl_WriteHexa( highChannelGain, 0, 6 );
			break;
		default:
			break;
	}
}

static void volumeControl_WriteHexa( uint8_t value, uint8_t line, uint8_t col ) {
	lcd_WriteNumber( ( ( value >> 4 ) & 0x0F ), line, col );
	lcd_WriteNumber( ( value & 0x0F ), line, col + 2 );
}
