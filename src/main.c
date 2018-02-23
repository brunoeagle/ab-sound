#include "stm32f7xx_hal.h"

#include "FreeRTOS.h"
#include "task.h"
#include "queue.h"
#include "FreeRTOSConfig.h"

#include "peripherals/lcd.h"
#include "peripherals/digital_trimpots.h"
#include "tasks/display.h"
#include "tasks/volume_control.h"
#include "tasks/input_selector.h"
#include "tasks/spdif.h"
#include "stm32/spi1.h"

static void hw_Setup( void );


int main( void ) {
	hw_Setup();

	xTaskCreate( display_Task, "Display Task", 1000, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
	xTaskCreate( volumeControl_Task, "Volume Task", 1000, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
	xTaskCreate( inputSelector_Task, "Input Selector Task", 1000, NULL, ( tskIDLE_PRIORITY + 1 ), NULL );
	vTaskStartScheduler();

	while( 1 );
	return 1;
}

static void hw_Setup( void ) {
	RCC_ClkInitTypeDef RCC_ClkInitStruct;
	RCC_OscInitTypeDef RCC_OscInitStruct;

	SCB_EnableICache();
	SCB_EnableDCache();
	HAL_NVIC_SetPriorityGrouping( NVIC_PRIORITYGROUP_4 );
	__HAL_RCC_PWR_CLK_ENABLE();
	HAL_Init();

	/* Enable HSE Oscillator and activate PLL with HSE as source */
	RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
	RCC_OscInitStruct.HSEState = RCC_HSE_ON;
	RCC_OscInitStruct.PLL.PLLState = RCC_PLL_ON;
	RCC_OscInitStruct.PLL.PLLSource = RCC_PLLSOURCE_HSE;
	RCC_OscInitStruct.PLL.PLLM = 25;
	RCC_OscInitStruct.PLL.PLLN = 432;
	RCC_OscInitStruct.PLL.PLLP = RCC_PLLP_DIV2;
	RCC_OscInitStruct.PLL.PLLQ = 9;

	if( HAL_RCC_OscConfig( &RCC_OscInitStruct ) != HAL_OK )
		while( 1 );

	/* Activate the OverDrive to reach the 216 MHz Frequency */
	if( HAL_PWREx_EnableOverDrive() != HAL_OK )
		while( 1 );

	/* Select PLL as system clock source and configure the HCLK, PCLK1 and PCLK2 clocks dividers */
	RCC_ClkInitStruct.ClockType = ( RCC_CLOCKTYPE_SYSCLK | RCC_CLOCKTYPE_HCLK | RCC_CLOCKTYPE_PCLK1 | RCC_CLOCKTYPE_PCLK2) ;
	RCC_ClkInitStruct.SYSCLKSource = RCC_SYSCLKSOURCE_PLLCLK;
	RCC_ClkInitStruct.AHBCLKDivider = RCC_SYSCLK_DIV1;		// Core will be at 216MHz
	RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV4;		// APB1 Peripherals at 54MHz
	RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV2;		// APB2 Peripherals at 108MHz

	if( HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_7) != HAL_OK )
		while( 1 );

	SystemCoreClockUpdate();
	HAL_DeInit();
	digitalTrimpots_Setup();
	lcd_Setup();
	volumeControl_Setup();
	inputSelector_Setup();
	spdif_Setup();

	spi1_Setup();
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
