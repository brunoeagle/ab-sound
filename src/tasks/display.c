#include "peripherals/lcd.h"
#include "display.h"
#include "lcd.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

void display_Task( void *pvParameters ) {
	uint8_t number = 0;
	lcd_Init();
	for( ;; ) {
		//lcd_WriteNumber( number++, 0, 0 );
		//if( number > 9 ) number = 0;
		vTaskDelay( ( ( TickType_t ) 200 / portTICK_PERIOD_MS ) );
	}
}
