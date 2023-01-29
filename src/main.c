#include "stm32f7xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOSConfig.h"

#include "peripherals/lcd.h"
#include "peripherals/digital_trimpots.h"
#include "peripherals/spdif.h"
#include "peripherals/dac.h"
#include "tasks/display.h"
#include "tasks/volume_control.h"
#include "tasks/input_selector.h"
#include "tasks/digital_input.h"
#include "tasks/wifi.h"
#include "stm32/spi1.h"
#include "stm32/usart2.h"
#include "stm32/clock.h"

static void hw_Setup( void );


int main( void ) {
	hw_Setup();

	HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );

	//xTaskCreate( wifi_Task, "WiFi Task", 1000, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
	xTaskCreate( display_Task, "Display Task", 1000, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
	xTaskCreate( volumeControl_Task, "Volume Task", 1000, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
	xTaskCreate( inputSelector_Task, "Input Selector Task", 1000, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
	//xTaskCreate( digitalInput_Task, "SPDIF Task", 1000, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
	vTaskStartScheduler();

	while( 1 );
	return 1;
}

static void hw_Setup( void ) {
	SCB_EnableICache();
	SCB_EnableDCache();
	HAL_Init();
	clock_InitMainClock();
	SystemCoreClockUpdate();
	digitalTrimpots_Setup();
	volumeControl_Setup();
	inputSelector_Setup();
	display_Setup();
	dac_Setup();
	spi1_Setup();
	usart2_Setup();
}

void vAssertCalled( uint32_t ulLine, const char *pcFile )
{
volatile unsigned long ul = 0;

	( void ) pcFile;
	( void ) ulLine;

	taskENTER_CRITICAL();
	{
		/* Set ul to a non-zero value using the debugger to step out of this
		function. */
		while( ul == 0 )
		{
			__NOP();
		}
	}
	taskEXIT_CRITICAL();
}
/*-----------------------------------------------------------*/

void vApplicationTickHook( void )
{
}

void vApplicationStackOverflowHook( xTaskHandle pxTask, char *pcTaskName ) {
	( void ) pcTaskName;
	( void ) pxTask;

	/* Run time stack overflow checking is performed if
	configconfigCHECK_FOR_STACK_OVERFLOW is defined to 1 or 2.  This hook
	function is called if a stack overflow is detected. */
	for( ;; );
}

void vApplicationMallocFailedHook( void )
{
	/* Called if a call to pvPortMalloc() fails because there is insufficient
	free memory available in the FreeRTOS heap.  pvPortMalloc() is called
	internally by FreeRTOS API functions that create tasks, queues, software
	timers, and semaphores.  The size of the FreeRTOS heap is set by the
	configTOTAL_HEAP_SIZE configuration constant in FreeRTOSConfig.h. */

	/* Force an assert. */
	configASSERT( ( volatile void * ) NULL );
}
