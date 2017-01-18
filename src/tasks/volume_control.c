#include "stm32f7xx_hal.h"
#include "volume_control.h"
#include "digital_trimpots.h"
#include "lcd.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// Digital trimpot goes from 0x00 to 0xFF, where 0x00 is minimum gain, and 0xFF the maximum in our circuit
#define	DEFAULT_GAIN_CHANNELS	0x00
#define DEFAULT_GAIN_SUBWOOFER	0x00

#define	LOW_CHANNEL		0
#define	HIGH_CHANNEL	1
#define	MASTER_CHANNEL	2

volatile uint8_t leftChannelGain;
volatile uint8_t rightChannelGain;
volatile uint8_t subWooferGain;

static void volumeControl_StateMachine( uint8_t channel, volatile uint8_t *stateMachine, uint16_t pinUp, uint16_t pinDown, volatile TickType_t *lastActivity, volatile uint8_t *counter);
static void volumeControl_SetVolume( uint8_t channel, uint8_t gain );
static void volumeControl_IncreaseVolume( uint8_t channel );
static void volumeControl_DecreaseVolume( uint8_t channel );

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
	volatile TickType_t lastMasterActivity;
	volatile TickType_t lastHighActivity;
	volatile TickType_t lastLowActivity;

	/*leftChannelGain = DEFAULT_GAIN_CHANNELS;
	rightChannelGain = DEFAULT_GAIN_CHANNELS;
	subWooferGain = DEFAULT_GAIN_SUBWOOFER;

	volumeControl_SetIndividual( LEFT_CHANNEL, leftChannelGain );
	volumeControl_SetIndividual( RIGHT_CHANNEL, rightChannelGain );
	volumeControl_SetIndividual( SUBWOOFER, subWooferGain );*/

	vTaskDelay( 2000 / portTICK_PERIOD_MS );
	lcd_WriteNumber( 0, 0, 0 );
	for( ;; ) {
		volumeControl_StateMachine( LOW_CHANNEL, &lowStateMachine, GPIO_PIN_14, GPIO_PIN_15, &lastLowActivity, &lowCounter );
		volumeControl_StateMachine( HIGH_CHANNEL, &highStateMachine, GPIO_PIN_6, GPIO_PIN_7, &lastHighActivity, &highCounter );
		volumeControl_StateMachine( MASTER_CHANNEL, &masterStateMachine, GPIO_PIN_4, GPIO_PIN_5, &lastMasterActivity, &masterCounter );
	}
}

static void volumeControl_StateMachine( uint8_t channel, volatile uint8_t *stateMachine, uint16_t pinUp, uint16_t pinDown, volatile TickType_t *lastActivity, volatile uint8_t *counter) {
	static int8_t volume = 0;
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
				volume++;
				if( volume > 9 ) volume = 0;
				lcd_WriteNumber( volume, 0, 0 );
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
				volume--;
				if( volume < 0 ) volume = 9;
				lcd_WriteNumber( volume, 0, 0 );
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

}

static void volumeControl_DecreaseVolume( uint8_t channel ) {

}
