#include "stm32f7xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOSConfig.h"

#include "peripherals/lcd.h"
#include "tasks/display.h"

static void hw_Setup( void );


int main( void ) {
	hw_Setup();

	xTaskCreate( display_Task, "Display Task", 1000, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
	vTaskStartScheduler();

	while( 1 );
	return 1;
}

static void hw_Setup( void ) {
	SCB_EnableICache();
	SCB_EnableDCache();
	HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );
	__HAL_RCC_GPIOG_CLK_ENABLE();
	__HAL_RCC_GPIOK_CLK_ENABLE();

	lcd_Setup();
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
