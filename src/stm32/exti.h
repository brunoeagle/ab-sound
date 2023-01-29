#ifndef __EXTI_H
#define __EXTI_H

typedef void ( extiCallbackFunction )( void );

void exti_InitInput( uint8_t pinNumber, GPIO_TypeDef *port, uint32_t mode, uint32_t pull );
void exti_AddCallback( uint8_t pinNumber, extiCallbackFunction *callback );
void exti_DisableInterrupts( void );
void exti_EnableInterrupts( void );

#endif
