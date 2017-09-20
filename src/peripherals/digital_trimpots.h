#ifndef __DIGITAL_TRIMPOTS_H
#define __DIGITAL_TRIMPOTS_H

#define LEFT_TRIMPOT		0
#define	RIGHT_TRIMPOT		1
#define	SUBWOOFER_TRIMPOT	2

#define	LOW_ENCODER			0
#define	HIGH_ENCODER		1
#define	MASTER_ENCODER		2

#define	INCREMENT_COMMAND	0x04
#define	DECREMENT_COMMAND	0X08

void digitalTrimpots_Setup( void );
uint8_t digitalTrimpots_Command( uint8_t channel, uint8_t command );
uint8_t digitalTrimpots_WriteWiper( uint8_t channel, uint8_t value );
uint8_t digitalTrimpots_ReadWiper( uint8_t channel, uint8_t *valueRead );
uint8_t digitalTrimpots_ReadTCON( uint8_t channel, uint8_t *valueRead );
uint8_t digitalTrimpots_WriteTCON( uint8_t channel, uint8_t value );
#endif
