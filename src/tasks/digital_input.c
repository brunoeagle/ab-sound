#include "stm32f7xx_hal.h"
#include "digital_input.h"
#include "peripherals/spdif.h"
#include "peripherals/dac.h"
#include "spdif.h"
#include "sai1.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

static void digitalInput_ConfigureInput( uint8_t input );

uint8_t digitalChannelSelected;
extern SPDIFRX_HandleTypeDef SPDIFRX_HandleStruct;
extern uint32_t rxBuffer[ 15360 ];
extern SAI_HandleTypeDef SAI_HandleStruct;

void digitalInput_Setup( void ) {
	digitalChannelSelected = 0;
}

void digitalInput_Task( void *pvParameters ) {
	spdif_Setup( 1 );
	sai1_Setup();
	for( ;; ) {
		/*if( !digitalChannelSelected ) {
			portYIELD();
		}
		else {
			digitalInput_ConfigureInput( digitalChannelSelected );
		}*/


		if(	HAL_SPDIFRX_ReceiveDataFlow_IT( &SPDIFRX_HandleStruct, rxBuffer, 15360 ) == HAL_OK ) {
			while ( SPDIFRX_HandleStruct.RxXferCount > 15360 / 2) ;
			HAL_GPIO_TogglePin( GPIOC, GPIO_PIN_5 );
		}
		if( HAL_SAI_Transmit( &SAI_HandleStruct, (uint8_t*)rxBuffer, 5000, 1000 ) != HAL_OK ) {
			digitalChannelSelected = 1;
		}
		digitalChannelSelected = 2;

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
