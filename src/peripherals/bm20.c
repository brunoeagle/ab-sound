#include "stdbool.h"
#include "stm32f7xx_hal.h"
#include "bm20.h"

#define RST_PORT            GPIOB
#define RST_PIN             GPIO_PIN_12 // OD
#define MFB_PORT            GPIOH
#define MFB_PIN             GPIO_PIN_7  // PP
#define P0_0_PORT           GPIOH
#define P0_0_PIN            GPIO_PIN_6  // OD
#define PLAY_PAUSE_PORT     GPIOH
#define PLAY_PAUSE_PIN      GPIO_PIN_8  // OD
#define FWD_PORT            GPIOH
#define FWD_PIN             GPIO_PIN_12 // OD
#define REV_PORT            GPIOH
#define REV_PIN             GPIO_PIN_9  // OD
#define VOLUME_PLUS_PORT    GPIOH
#define VOLUME_PLUS_PIN     GPIO_PIN_11 // OD
#define VOLUME_MINUS_PORT   GPIOH
#define VOLUME_MINUS_PIN    GPIO_PIN_10 // OD
#define LED1_PORT           GPIOB
#define LED1_PIN            GPIO_PIN_15 // IN
#define LED2_PORT           GPIOB
#define LED2_PIN            GPIO_PIN_14 // IN

#define RST_LOW             HAL_GPIO_WritePin( RST_PORT, RST_PIN, GPIO_PIN_RESET )
#define RST_HIGH            HAL_GPIO_WritePin( RST_PORT, RST_PIN, GPIO_PIN_SET )
#define MFB_LOW             HAL_GPIO_WritePin( MFB_PORT, MFB_PIN, GPIO_PIN_RESET )
#define MFB_HIGH            HAL_GPIO_WritePin( MFB_PORT, MFB_PIN, GPIO_PIN_SET )
#define P0_0_LOW            HAL_GPIO_WritePin( P0_0_PORT, P0_0_PIN, GPIO_PIN_RESET )
#define P0_0_HIGH           HAL_GPIO_WritePin( P0_0_PORT, P0_0_PIN, GPIO_PIN_SET )

#define PLAY_PAUSE_LOW      HAL_GPIO_WritePin( PLAY_PAUSE_PORT, PLAY_PAUSE_PIN, GPIO_PIN_RESET )
#define PLAY_PAUSE_HIGH     HAL_GPIO_WritePin( PLAY_PAUSE_PORT, PLAY_PAUSE_PIN, GPIO_PIN_SET )
#define FWD_LOW             HAL_GPIO_WritePin( FWD_PORT, FWD_PIN, GPIO_PIN_RESET )
#define FWD_HIGH            HAL_GPIO_WritePin( FWD_PORT, FWD_PIN, GPIO_PIN_SET )
#define REV_LOW             HAL_GPIO_WritePin( REV_PORT, REV_PIN, GPIO_PIN_RESET )
#define REV_HIGH            HAL_GPIO_WritePin( REV_PORT, REV_PIN, GPIO_PIN_SET )

#define VOLUME_PLUS_LOW     HAL_GPIO_WritePin( VOLUME_PLUS_PORT, VOLUME_PLUS_PIN, GPIO_PIN_RESET )
#define VOLUME_PLUS_HIGH    HAL_GPIO_WritePin( VOLUME_PLUS_PORT, VOLUME_PLUS_PIN, GPIO_PIN_SET )
#define VOLUME_MINUS_LOW    HAL_GPIO_WritePin( VOLUME_MINUS_PORT, VOLUME_MINUS_PIN, GPIO_PIN_RESET )
#define VOLUME_MINUS_HIGH   HAL_GPIO_WritePin( VOLUME_MINUS_PORT, VOLUME_MINUS_PIN, GPIO_PIN_SET )

void bm20_Setup( void ) {
    GPIO_InitTypeDef GPIO_InitStruct;

    // configure the I/O
    __HAL_RCC_GPIOB_CLK_ENABLE();
    __HAL_RCC_GPIOH_CLK_ENABLE();

    // output push-pull pins init
    MFB_LOW;

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_PP;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_MEDIUM;
    GPIO_InitStruct.Pin = MFB_PIN;
    HAL_GPIO_Init( MFB_PORT, &GPIO_InitStruct );

    // output open-drain pins init
    RST_LOW;
    P0_0_HIGH;
    PLAY_PAUSE_HIGH;
    FWD_HIGH;
    REV_HIGH;
    VOLUME_PLUS_HIGH;
    VOLUME_MINUS_HIGH;

    GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
    GPIO_InitStruct.Pin = RST_PIN;
    HAL_GPIO_Init( RST_PORT, &GPIO_InitStruct );
    GPIO_InitStruct.Pin = P0_0_PIN;
    HAL_GPIO_Init( P0_0_PORT, &GPIO_InitStruct );
    GPIO_InitStruct.Pin = PLAY_PAUSE_PIN;
    HAL_GPIO_Init( PLAY_PAUSE_PORT, &GPIO_InitStruct ); 
    GPIO_InitStruct.Pin = FWD_PIN;
    HAL_GPIO_Init( FWD_PORT, &GPIO_InitStruct ); 
    GPIO_InitStruct.Pin = REV_PIN;
    HAL_GPIO_Init( REV_PORT, &GPIO_InitStruct ); 
    GPIO_InitStruct.Pin = VOLUME_PLUS_PIN;
    HAL_GPIO_Init( VOLUME_PLUS_PORT, &GPIO_InitStruct ); 
    GPIO_InitStruct.Pin = VOLUME_MINUS_PIN;
    HAL_GPIO_Init( VOLUME_MINUS_PORT, &GPIO_InitStruct ); 

    // input pins init
    GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
    GPIO_InitStruct.Pull = GPIO_NOPULL;
    GPIO_InitStruct.Pin = LED1_PIN | LED2_PIN;
    HAL_GPIO_Init( LED1_PORT, &GPIO_InitStruct );
}

void bm20_SetReset( bool on ) {
    if( on )
        RST_LOW;
    else
        RST_HIGH;
}

void bm20_SetMFB( bool on ) {
    if( on )
        MFB_HIGH;
    else
        MFB_LOW;
}

void bm20_SetP00( bool on ) {
    if( on )
        P0_0_LOW;
    else
        P0_0_HIGH;
}