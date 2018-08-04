#ifndef __DISPLAY_H
#define __DISPLAY_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"
#include "semphr.h"

SemaphoreHandle_t displayMutex;

void display_Setup( void );
void display_Task( void *pvParameters );

#endif
