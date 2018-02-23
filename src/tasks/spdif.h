#ifndef __SPDIF_H
#define __SPDIF_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

void spdif_Task( void *pvParameters );
void spdif_Setup( void );

#endif
