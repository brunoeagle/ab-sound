#ifndef __SAI1_H
#define __SAI1_H

#include "FreeRTOS.h"
#include "semphr.h"

SemaphoreHandle_t sai1Mutex;

void sai1_Setup( void );

#endif
