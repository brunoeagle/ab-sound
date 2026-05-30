#ifndef __BT_H
#define __BT_H

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

// external notification values
#define BT_TURN_ON              0x01
#define BT_TURN_OFF             0x02
#define BT_ENTER_PAIRING_MODE   0x03

extern TaskHandle_t btTaskHandle;

void btTask( void *pvParameters );

#endif
