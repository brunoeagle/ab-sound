#include <string.h>
#include <limits.h>
#include "stm32f7xx_hal.h"
#include "peripherals/bm20.h"
#include "stm32/usart3.h"
#include "bt.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "semphr.h"
#include "task.h"
#include "message_buffer.h"

#define BT_STATE_OFF    1
#define BT_STATE_ON     2

// internal notification values
#define BT_EVT_RECEIVED 0x04

// MessageBuffer buffer size
#define MSG_BUFFER_SIZE_BYTES 256

// serial commands
static const uint8_t BM20_ENTER_PAIRING_CMD[] = { 0xAA, 0x00, 0x03, 0x02, 0x00, 0x50, 0xAB };
static const uint8_t BM20_POWER_OFF_CMD[] = { 0xAA, 0x00, 0x03, 0x02, 0x00, 0x5E, 0x9D };
static const uint8_t BM20_PDL_ERASE[] = { 0x00, 0xAA, 0x00, 0x03, 0x02, 0x00, 0x56, 0xA5 };

TaskHandle_t btTaskHandle;
static uint8_t btState = BT_STATE_OFF;
static uint8_t btRxBuffer[ 200 ];
static uint8_t btRxBufferPos = 0;
static volatile TickType_t btRxLastByteReceivedTime;
StaticMessageBuffer_t xMessageBufferStruct;
uint8_t ucStorageBuffer[ MSG_BUFFER_SIZE_BYTES ];
MessageBufferHandle_t xMessageBuffer;
static volatile uint32_t received_events_counter = 0;

static void btReceiveCallback( uint8_t receivedByte );
static bool btCheckSumMatches( uint8_t *array, uint16_t size ) {
    uint8_t checksum = 0;
    for( uint8_t i = 0; i < ( size - 1); i++) {
        checksum += array[ i ];
    }
    checksum = ( ~checksum ) + 1;
    return ( checksum == array[ size - 1 ] ) ? true : false;
}

static void btSendCommand( uint8_t *cmd, uint8_t size ) {
    bm20_SetMFB( true );
    vTaskDelay( 5 / portTICK_PERIOD_MS );    // power-on hold time: 480ms
    usart3_WriteArray( cmd, size );
    bm20_SetMFB( false );
}

void btTask( void *pvParameters ) {
    uint32_t ulNotificationValue;
    uint8_t receivedEvent[ 50 ];
    uint8_t receivedEventSize;

    xMessageBuffer = xMessageBufferCreate( MSG_BUFFER_SIZE_BYTES );

    usart3_SetupCallback( btReceiveCallback );
    usart3_EnableRxInterrupt();

    for( ;; ) {
        if( xTaskNotifyWait( pdFALSE, ULONG_MAX, &ulNotificationValue, portMAX_DELAY ) != pdPASS ) {
            continue;
        }
        // if input was selected and module is off, initiate it
        if( ( ulNotificationValue & BT_TURN_ON ) && ( btState == BT_STATE_OFF ) ) {
            bm20_SetReset( false );
            vTaskDelay( 500 / portTICK_PERIOD_MS );
            bm20_SetMFB( true );
            vTaskDelay( 1000 / portTICK_PERIOD_MS );    // power-on hold time: 480ms
            bm20_SetMFB( false );
            btState = BT_STATE_ON;
        }
        // enter pairing mode
        if( ( ulNotificationValue & BT_ENTER_PAIRING_MODE ) && ( btState == BT_STATE_ON ) ) {
            //btSendCommand( (uint8_t*)BM20_PDL_ERASE, sizeof( BM20_PDL_ERASE ) );
            //vTaskDelay( 5000 / portTICK_PERIOD_MS );
            btSendCommand( (uint8_t*)BM20_ENTER_PAIRING_CMD, sizeof( BM20_ENTER_PAIRING_CMD ) );
        }
        // power the module off, probably another input selected
        if( ( ulNotificationValue & BT_TURN_OFF ) && ( btState == BT_STATE_ON ) ) {
            btSendCommand( (uint8_t*)BM20_POWER_OFF_CMD, sizeof( BM20_POWER_OFF_CMD ) );
            btState = BT_STATE_OFF;
        }
        // if event was sent by the module
        if( ulNotificationValue & BT_EVT_RECEIVED ) {
            do {
                receivedEventSize = xMessageBufferReceive( xMessageBuffer, receivedEvent, sizeof( receivedEvent ), 0 );
                if( receivedEventSize ) {
                    if( btCheckSumMatches( receivedEvent, receivedEventSize ) ) {
                        received_events_counter++;
                    }
                }
            } while (receivedEventSize);
        }

    }
}

static void btReceiveCallback( uint8_t receivedByte ) {
    BaseType_t xHigherPriorityTaskWoken = pdFALSE;
    static uint8_t uartStateMachine = 0;
    static uint16_t commandSize = 0;
    btRxLastByteReceivedTime = xTaskGetTickCountFromISR();
    if( btRxBufferPos >= sizeof( btRxBuffer) ) {
        btRxBufferPos = 0;
        uartStateMachine = 0;
    }
    switch( uartStateMachine ) {
        case 0:
            // detect start byte 0xAA
            if( receivedByte == 0xAA ) {
                uartStateMachine++;
                btRxBufferPos = 0;
            }
        break;
        case 1:
        case 2:
            // get size of the command
            btRxBuffer[ btRxBufferPos++ ] = receivedByte;
            if( uartStateMachine == 2 ) {
                commandSize = btRxBuffer[ btRxBufferPos - 2 ] << 8;
                commandSize |= btRxBuffer[ btRxBufferPos - 1 ];
                commandSize += 3; // add 2 bytes of the size and 1 of the checksum
            }
            uartStateMachine++;
        break;
        case 3:
            btRxBuffer[ btRxBufferPos++ ] = receivedByte;
            if( ( btRxBufferPos ) >= ( commandSize ) ) { // increase by 1 for the checksum
                xMessageBufferSendFromISR( 
                        xMessageBuffer, 
                        ( void * ) btRxBuffer, 
                        commandSize, 
                        &xHigherPriorityTaskWoken 
                    );
                xTaskNotifyFromISR( btTaskHandle, BT_EVT_RECEIVED, eSetBits, &xHigherPriorityTaskWoken );
                commandSize = uartStateMachine = btRxBufferPos = 0;
            }
        break;
    }
    portYIELD_FROM_ISR( xHigherPriorityTaskWoken );
}