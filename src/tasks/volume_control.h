#ifndef __VOLUME_CONTROL_H
#define __VOLUME_CONTROL_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void volumeControl_Task( void *pvParameters );

#endif
