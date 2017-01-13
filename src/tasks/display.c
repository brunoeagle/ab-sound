#include "peripherals/lcd.h"
#include "display.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

void display_Task( void *pvParameters ) {
	for( ;; ) {
		lcd_Test();
		vTaskDelay( ( ( TickType_t ) 1000 / portTICK_PERIOD_MS ) );
	}
}
