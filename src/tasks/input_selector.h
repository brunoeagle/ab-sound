#ifndef __INPUT_SELECTOR_H
#define __INPUT_SELECTOR_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

#define INPUT_NONE  0
#define INPUT_P2    1
#define INPUT_RCA   2
#define INPUT_BT    3
#define INPUT_SPDIF 4
#define INPUT_OPTIC 5

void inputSelector_Task( void *pvParameters );
void inputSelector_Setup( void );
uint8_t inputSelector_GetCurrentInput( void );

#endif
