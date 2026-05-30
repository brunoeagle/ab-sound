#include "stm32f7xx_hal.h"
#include "usart3.h"

static UART_HandleTypeDef UART_HandleStruct;
static RCC_PeriphCLKInitTypeDef RCC_InitStruct;
static usartRxCallbackFunction usart3_ReceiveCallback;

static SemaphoreHandle_t usart3Mutex;
static uint8_t usart3Buffer[ 3000 ];
static volatile uint16_t usart3BufferIdx = 0;

void usart3_Setup( void ) {
    GPIO_InitTypeDef GPIO_InitStruct;

    __HAL_RCC_GPIOB_CLK_ENABLE();

    RCC_InitStruct.PeriphClockSelection = RCC_PERIPHCLK_USART3;
    RCC_InitStruct.Usart3ClockSelection = RCC_USART3CLKSOURCE_SYSCLK;    // 216MHz
    HAL_RCCEx_PeriphCLKConfig( &RCC_InitStruct );
    __HAL_RCC_USART3_CLK_ENABLE();

    // Setup the I/O to the module
    GPIO_InitStruct.Mode = GPIO_MODE_AF_PP;
    GPIO_InitStruct.Pull = GPIO_PULLUP;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_VERY_HIGH;
    GPIO_InitStruct.Alternate = GPIO_AF7_USART3;
    GPIO_InitStruct.Pin = GPIO_PIN_10;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );
    GPIO_InitStruct.Pin = GPIO_PIN_11;
    HAL_GPIO_Init( GPIOB, &GPIO_InitStruct );

    UART_HandleStruct.Instance = USART3;
    UART_HandleStruct.Init.BaudRate   = 115200;
    UART_HandleStruct.Init.WordLength = UART_WORDLENGTH_8B;
    UART_HandleStruct.Init.StopBits   = UART_STOPBITS_1;
    UART_HandleStruct.Init.Parity     = UART_PARITY_NONE;
    UART_HandleStruct.Init.Mode       = UART_MODE_TX_RX;
    UART_HandleStruct.Init.OverSampling = UART_OVERSAMPLING_16;
    UART_HandleStruct.Init.HwFlowCtl = UART_HWCONTROL_NONE;
    if( HAL_UART_Init( &UART_HandleStruct ) != HAL_OK )
        while( 1 );
    __HAL_UART_ENABLE( &UART_HandleStruct );

    usart3Mutex = xSemaphoreCreateMutex();
    while( usart3Mutex == NULL );
}

void usart3_EnableRxInterrupt( void ) {
    // Setup the USART3 interrupt
    HAL_NVIC_SetPriority( USART3_IRQn, 5, 1 );
    HAL_NVIC_EnableIRQ( USART3_IRQn );
    __HAL_UART_ENABLE_IT( &UART_HandleStruct, UART_IT_RXNE );
}

void usart3_SetupCallback( usartRxCallbackFunction callback ) {
    usart3_ReceiveCallback = callback;
}

uint8_t usart3_WriteArray( uint8_t *arrayToWrite, uint16_t size ) {
    if( HAL_UART_Transmit( &UART_HandleStruct, arrayToWrite, size, 1000 ) != HAL_OK )
        return 0;
    return 1;
}

uint8_t usart3_WriteString( char *string ) {
    uint16_t size = 1000;    // maximum size
    char *p = string;
    while( *p && --size ) {
        if( HAL_UART_Transmit( &UART_HandleStruct, ( uint8_t* )p++, 1, 1000 ) != HAL_OK )
            break;
    }
    return ( size > 0 ) ? 1 : 0;
}

void USART3_IRQHandler( void ) {
    uint8_t rec;
    if( __HAL_UART_GET_FLAG( &UART_HandleStruct, UART_FLAG_RXNE ) ) {
        HAL_UART_Receive( &UART_HandleStruct, &rec, 1, 1000 );
        __HAL_UART_CLEAR_IT( &UART_HandleStruct, UART_FLAG_RXNE );
    }
    if( __HAL_UART_GET_FLAG( &UART_HandleStruct, UART_FLAG_ORE ) ) {
        __HAL_UART_CLEAR_IT( &UART_HandleStruct, UART_FLAG_ORE );
    }
    usart3_ReceiveCallback( rec );
}
