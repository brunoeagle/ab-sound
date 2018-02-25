#include "stm32f7xx_hal.h"
#include "digital_input.h"
#include "peripherals/spdif.h"
#include "peripherals/dac.h"
#include "lcd.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

static void digitalInput_ConfigureInput( uint8_t input );

uint32_t received_data_flow[ 64 ] ;
uint8_t digitalChannelSelected;

void digitalInput_Setup( void ) {
	digitalChannelSelected = 0;
}

void digitalInput_Task( void *pvParameters ) {
	for( ;; ) {
		if( !digitalChannelSelected ) {
			portYIELD();
		}
		else {
			digitalInput_ConfigureInput( digitalChannelSelected );
		}

		/*HAL_SPDIFRX_ReceiveDataFlow( &SPDIFRX_HandleStruct, ( uint32_t * )received_data_flow, 64, 0xFFF );
		if( SPDIFRX_HandleStruct.ErrorCode != HAL_SPDIFRX_ERROR_NONE ) {
			while( 1 );
		}*/
	}
}

static void digitalInput_ConfigureInput( uint8_t input ) {
	static uint8_t coaxStateMachine = 0;
	switch( coaxStateMachine ) {
	case 0:	// configure the spdif for the SPDIF channel 'input' and DMA to send the data to DAC
		spdif_Setup( input );
		coaxStateMachine = 1;
		break;
	case 1:	// check if the spdif packets are being received
		// if receiving ok, if not coaxStateMachine = 0;
		break;
	}
}
