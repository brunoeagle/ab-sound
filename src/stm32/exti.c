#include "stm32f7xx_hal.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"

#include "exti.h"

#define EXTI_QTY        16
#define	CALLBACKS_QTY   3

static extiCallbackFunction *cb[ EXTI_QTY ][ CALLBACKS_QTY ];
volatile uint32_t interruptsEnabled;

static void exti_ExecuteCallbacks( uint8_t exti );

static void exti_EnableNVICInterrupt( uint16_t pin ) {
    IRQn_Type irqn = -50;   // invalid IRQn

    switch( pin ) {
        case 0:
            irqn = EXTI0_IRQn;
        break;
        case 1:
            irqn = EXTI1_IRQn;
        break;
        case 2:
            irqn = EXTI2_IRQn;
        break;
        case 3:
            irqn = EXTI3_IRQn;
        break;
        case 4:
            irqn = EXTI4_IRQn;
        break;
        case 5 ... 9:
            irqn = EXTI9_5_IRQn;
        break;
        case 10 ... 15:
            irqn = EXTI15_10_IRQn;
        break;
    }
    if( irqn != -50 ) {
        HAL_NVIC_SetPriority( irqn, 15, 0 );
        HAL_NVIC_EnableIRQ( irqn );
    }
}

void exti_InitInput( uint8_t pinNumber, GPIO_TypeDef *port, uint32_t mode, uint32_t pull ) {
    GPIO_InitTypeDef init;

    init.Pin = ( uint16_t )0x0001 << pinNumber;
    init.Mode = mode;
    init.Pull = pull;
    HAL_GPIO_Init( port, &init );
    exti_EnableNVICInterrupt( pinNumber );
}

void exti_AddCallback( uint8_t pinNumber, extiCallbackFunction *callback ) {
    uint8_t i;
    for( i = 0; i < CALLBACKS_QTY; i++ ) {
        if( cb[ pinNumber ][ i ] == NULL ) {
            cb[ pinNumber ][ i ] = callback;
            break;
        }
    }
}
/*
void exti_ResetCallbacks( void ) {
    uint8_t i, j;
    for( i = 0; i < EXTI_QTY; i++ )
        for( j = 0; j < CALLBACKS_QTY; j++ )
            cb[ i ][ j ] = NULL;
}

void exti_AddCallback( uint8_t exti, extiCallbackFunction callback ) {
    uint8_t i;
    for( i = 0; i < CALLBACKS_QTY; i++ ) {
        if( cb[ exti ][ i ] == NULL ) {
            cb[ exti ][ i ] = callback;
            break;
        }
    }
}
*/
static void exti_ExecuteCallbacks( uint8_t pinNumber ) {
    uint8_t i;
    for( i = 0; i < CALLBACKS_QTY; i++ ) {
        if( cb[ pinNumber ][ i ] != NULL )
            cb[ pinNumber ][ i ]();
    }
}

/*void exti_DisableInterrupts( void ) {
    interruptsEnabled = EXTI->IMR1;
    LL_EXTI_DisableIT_0_31( interruptsEnabled );
}

void exti_EnableInterrupts( void ) {
    LL_EXTI_EnableIT_0_31( interruptsEnabled );
}
*/

static void exti_callback( void ) {
    uint8_t i;
    for( i = 0; i < EXTI_QTY; i++ ) {
        uint16_t pin = ( uint16_t )0x0001 << i;
        if( __HAL_GPIO_EXTI_GET_IT( pin ) != RESET ) {
            __HAL_GPIO_EXTI_CLEAR_IT( pin );
            exti_ExecuteCallbacks( i );
        }
    }
}

void EXTI0_IRQHandler( void ) {
    exti_callback();
}

void EXTI1_IRQHandler( void ) {
    exti_callback();
}

void EXTI2_IRQHandler( void ) {
    exti_callback();
}

void EXTI3_IRQHandler( void ) {
    exti_callback();
}

void EXTI4_IRQHandler( void ) {
    exti_callback();
}

void EXTI9_5_IRQHandler( void ) {
    exti_callback();
}

void EXTI15_10_IRQHandler( void ) {
    exti_callback();
}
