#ifndef __DIGITAL_INPUT_H
#define __DIGITAL_INPUT_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void digitalInput_Setup( void );
void digitalInput_Task( void *pvParameters );

#endif
