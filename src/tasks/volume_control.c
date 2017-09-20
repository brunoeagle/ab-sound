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
volatile uint8_t trimpotWiper[ 3 ];	// left, right, sub
//volatile uint8_t trimpotLeftTCON, trimpotRightTCON, trimpotLowTCON;

static void volumeControl_StateMachine( uint8_t channel, volatile uint8_t *stateMachine, uint16_t pinUp,
		uint16_t pinDown, volatile TickType_t *lastActivity,
		volatile uint8_t *counter );
static void volumeControl_VolumeCommand( uint8_t channel, uint8_t command );
static uint8_t volumeControl_TrimpotCommand( uint8_t trimpot, uint8_t command );

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
	uint8_t volumeSet;

	lowChannelGain = DEFAULT_GAIN_LOW;
	highChannelGain = DEFAULT_GAIN_HIGH;

	/*volumeControl_SetIndividual( LEFT_CHANNEL, leftChannelGain );
	volumeControl_SetIndividual( RIGHT_CHANNEL, rightChannelGain );
	volumeControl_SetIndividual( SUBWOOFER, subWooferGain );*/

	vTaskDelay( 2000 / portTICK_PERIOD_MS );
	digitalTrimpots_WriteWiper( LEFT_TRIMPOT, 0x05 );
	if( digitalTrimpots_ReadWiper( LEFT_TRIMPOT, &volumeSet ) )
		lcd_WriteHexa( volumeSet, 0, 0 );
	digitalTrimpots_WriteWiper( RIGHT_TRIMPOT, 0x05 );
	if( digitalTrimpots_ReadWiper( RIGHT_TRIMPOT, &volumeSet ) )
		lcd_WriteHexa( volumeSet, 0, 6 );
	digitalTrimpots_WriteWiper( SUBWOOFER_TRIMPOT, 0x05 );
	if( digitalTrimpots_ReadWiper( SUBWOOFER_TRIMPOT, &volumeSet ) )
		lcd_WriteHexa( volumeSet, 0, 12 );

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
				volumeControl_VolumeCommand( channel, INCREMENT_COMMAND );
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
				volumeControl_VolumeCommand( channel, DECREMENT_COMMAND );
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

static void volumeControl_VolumeCommand( uint8_t channel, uint8_t command ) {
	uint8_t volumeSet;
	switch( channel ) {
		case LOW_ENCODER:
			volumeControl_TrimpotCommand( SUBWOOFER_TRIMPOT, command );
			lcd_WriteHexa( trimpotWiper[ 2 ], 0, 12 );
			break;
		case HIGH_ENCODER:
			volumeControl_TrimpotCommand( LEFT_TRIMPOT, command );
			lcd_WriteHexa( trimpotWiper[ 0 ], 0, 0 );
			volumeControl_TrimpotCommand( RIGHT_TRIMPOT, command );
			lcd_WriteHexa( trimpotWiper[ 1 ], 0, 6 );
			break;
		case MASTER_ENCODER:
			volumeControl_TrimpotCommand( LEFT_TRIMPOT, command );
			lcd_WriteHexa( trimpotWiper[ 0 ], 0, 0 );
			volumeControl_TrimpotCommand( RIGHT_TRIMPOT, command );
			lcd_WriteHexa( trimpotWiper[ 1 ], 0, 6 );
			volumeControl_TrimpotCommand( SUBWOOFER_TRIMPOT, command );
			lcd_WriteHexa( trimpotWiper[ 2 ], 0, 12 );
			break;
		default:
			break;
	}
}

static uint8_t volumeControl_TrimpotCommand( uint8_t trimpot, uint8_t command ) {
	volatile uint8_t volumeSet, wiper, tcon;
	if( !digitalTrimpots_ReadWiper( trimpot, &wiper ) )
		return 0;
	if( command == INCREMENT_COMMAND ) {
		if( wiper >= 0x00 && wiper < 0x05 ) {
			// activate TCON and increment by 1
			if( !digitalTrimpots_ReadTCON( trimpot, &tcon ) )
				return 0;
			if( ( tcon & 0x04 ) )	// increase wiper only if TCON is already activated
				wiper++;
			if( !digitalTrimpots_WriteTCON( trimpot, tcon | 0x04 ) )
				return 0;
		}
		else if( wiper >= 0x05 && wiper < 0xF0 ) {
			// increment by 5
			wiper += 5;
		}
		else {
			// increment by 1
			if( wiper < 0xFF )
				wiper++;
		}
	}
	else {
		if( wiper <= 0xFF && wiper > 0xF0 ) {
			// decrement by 1
			wiper--;
		}
		else if( wiper <= 0xF0 && wiper >= 0x0A ) {
			// decrement by 5
			wiper -= 5;
		}
		else {
			// decrement by 1, in case of zero, deactivate TCON
			if( wiper > 0x00 )
				wiper--;
			else {
				if( !digitalTrimpots_ReadTCON( trimpot, &tcon ) )
					return 0;
				if( !digitalTrimpots_WriteTCON( trimpot, tcon & 0xFB ) )
					return 0;
			}
		}
	}
	if( !digitalTrimpots_WriteWiper( trimpot, wiper ) )
		return 0;
	if( !digitalTrimpots_ReadWiper( trimpot, &volumeSet ) )
		return 0;
	trimpotWiper[ trimpot ] = volumeSet;
	//lcd_WriteHexa( volumeSet, 0, 6 );
	return 1;
}
