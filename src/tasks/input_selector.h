#ifndef __INPUT_SELECTOR_H
#define __INPUT_SELECTOR_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void inputSelector_Task( void *pvParameters );
void inputSelector_Setup( void );

#endif
