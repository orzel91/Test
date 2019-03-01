/*
 * one_wire.h
 *
 *  Created on: 1 wrz 2018
 *      Author: jarek
 */

#ifndef AM2302_ONE_WIRE_H_
#define AM2302_ONE_WIRE_H_

#include <stdint.h>
#include <stdbool.h>

/*============================================================================+
| global definitions
+=============================================================================*/
typedef enum {
	ONE_WIRE_IDLE = 0,
	ONE_WIRE_TRIGGER,
	ONE_WIRE_READING,
	ONE_WIRE_CAPTURED
} one_wire_states;

typedef enum {
	ONE_WIRE_ZERO_BIT = 0,
	ONE_WIRE_ONE_BIT,
	ONE_WIRE_END_BIT,
	ONE_WIRE_ERROR_BIT
} one_wire_posibBits;

typedef struct {
	uint16_t low;
	uint16_t high;
} one_wire_timeThr;

typedef struct {
	uint16_t hum;
	uint16_t temp;
	uint8_t  chck;
} one_wire_frame;

typedef struct {
	uint16_t humadity;
	uint16_t temperature;
} one_wire_measurments;

/*============================================================================+
| strange variables
+=============================================================================*/

/*============================================================================+
| global variables
+=============================================================================*/

/*============================================================================+
| global functions' declarations
+=============================================================================*/

void one_wire_init(void);
void one_wire_sendStart (void);
one_wire_states one_wire_getState(void);
bool one_wire_checkTime(uint8_t time, one_wire_timeThr thr);
one_wire_posibBits one_wire_checkBit(volatile uint8_t* buff, uint16_t* idx);
bool one_wire_shiftBit(one_wire_frame* tmp, uint8_t data, uint8_t idx);
bool one_wire_checkSum(one_wire_frame data);
uint8_t one_wire_checkOne(uint16_t data);
void one_wire_test_fillBuff(uint8_t* src, uint8_t size);
uint8_t one_wire_frameDecoder(void);
uint16_t one_wire_getHum(void);
uint16_t one_wire_getTemp(void);

/******************************************************************************
* END OF FILE
******************************************************************************/
#endif /* AM2302_ONE_WIRE_H_ */
