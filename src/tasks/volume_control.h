#ifndef __VOLUME_CONTROL_H
#define __VOLUME_CONTROL_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void volumeControl_Task( void *pvParameters );
void volumeControl_Setup( void );
uint8_t volumeControl_GetCurrentVolume( uint8_t trimpot );

#endif
