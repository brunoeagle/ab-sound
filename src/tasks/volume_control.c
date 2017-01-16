#include "volume_control.h"
#include "digital_trimpots.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

// Digital trimpot goes from 0x00 to 0xFF, where 0x00 is minimum gain, and 0xFF the maximum in our circuit
#define	DEFAULT_GAIN_CHANNELS	0x00
#define DEFAULT_GAIN_SUBWOOFER	0x00

#define LEFT_CHANNEL	LEFT_TRIMPOT
#define	RIGHT_CHANNEL	RIGHT_TRIMPOT
#define	SUBWOOFER		SUBWOOFER_TRIMPOT

volatile uint8_t leftChannelGain;
volatile uint8_t rightChannelGain;
volatile uint8_t subWooferGain;

static void volumeControl_SetIndividual( uint8_t channel, uint8_t gain );

void volumeControl_Task( void *pvParameters ) {

	leftChannelGain = DEFAULT_GAIN_CHANNELS;
	rightChannelGain = DEFAULT_GAIN_CHANNELS;
	subWooferGain = DEFAULT_GAIN_SUBWOOFER;

	volumeControl_SetIndividual( LEFT_CHANNEL, leftChannelGain );
	volumeControl_SetIndividual( RIGHT_CHANNEL, rightChannelGain );
	volumeControl_SetIndividual( SUBWOOFER, subWooferGain );

	for( ;; ) {
		vTaskDelay( ( ( TickType_t ) 1000 / portTICK_PERIOD_MS ) );
	}
}

static void volumeControl_SetIndividual( uint8_t channel, uint8_t gain ) {
	// send the code
}
