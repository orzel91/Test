/*
 * one_wire.c
 *
 *  Created on: 1 wrz 2018
 *      Author: jarek
 */



/*============================================================================+
| includes
+============================================================================*/

#include "one_wire.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/stm32f10x.h"
#include "../config.h"
#include "../hdr/hdr_rcc.h"
#include "../hdr/hdr_gpio.h"
#include "../gpio/gpio.h"
#include "../uart/uart.h"


/*============================================================================+
| Defines
+============================================================================*/
#define BUFF_SIZE  128
#define BUFF_MASK  (BUFF_SIZE -1)
#define HUM_MASK   16
#define TEMP_MASK  32
#define CHCK_MASK  40
#define BIT_AMOUNT 16
#define BYTE_MASK  0xFF

/*============================================================================+
| typedefs
+============================================================================*/

typedef struct {
	uint16_t rising;
	uint16_t falling;
} one_wire_edgeCnt;

typedef struct {
	uint16_t high;
	uint16_t low;
} one_wire_stateTimes;

//typedef enum {
//	ONE_WIRE_ZERO_BIT = 0,
//	ONE_WIRE_ONE_BIT,
//	ONE_WIRE_END_BIT,
//	ONE_WIRE_ERROR_BIT
//} one_wire_posibBits;

typedef enum {
	ONE_WIRE_HOST_START = 0,
	ONE_WIRE_START_RESPONSE,
	ONE_WIRE_PAYLOAD,
	ONE_WIRE_CHECKSUM,
	ONE_WIRE_DECODED,
	ONE_WIRE_ERROR
} one_wire_decodePhase;

typedef enum {
	ONE_WIRE_TRIG_TIME = 0,
	ONE_WIRE_START_TIME,
	ONE_WIRE_PAYLOAD_START_TIME,
	ONE_WIRE_PAYLOAD_ONE_TIME,
	ONE_WIRE_PAYLOAD_ZERO_TIME,
	ONE_WIRE_END_TIME
} one_wire_bitDuration;

//typedef struct {
//	uint16_t humadity;
//	uint16_t temperature;
//	uint8_t  checksum;
//} one_wire_frame;

typedef struct {
	volatile one_wire_states state;
	volatile one_wire_edgeCnt edge;
	volatile one_wire_stateTimes stateTime;
	volatile bool startMeas;
	volatile uint8_t buff[BUFF_SIZE];
	volatile uint8_t index;
	one_wire_measurments measurments;
} one_wire_module;


/*============================================================================+
| module variables
+============================================================================*/
one_wire_module module;

const one_wire_timeThr possibleTimings[6] = {
	{18, 40},
	{76, 82},
	{48, 68},
	{68, 74},
	{24, 29},
	{45, 48}
};


/*============================================================================+
| local functions' declarations
+============================================================================*/
static void one_wire_setCapture(void);
static void one_wire_setCompare(void);
static void one_wire_initClock(void);
static uint8_t one_wire_timeConvert(uint16_t cnt);
//static bool one_wire_checkTime(uint8_t time, const uint16_t* thr);
/*============================================================================+
| global functions
+============================================================================*/


void one_wire_init(void)
{
	memset(&module, 0, sizeof(module));
	one_wire_initClock();
}

void one_wire_sendStart(void)
{
	one_wire_setCompare();
}

one_wire_states one_wire_getState(void)
{
	return module.state;
}

/*============================================================================+
| local functions
+============================================================================*/

static void one_wire_setCapture(void)
{
	TIM3->CCMR1 = TIM_CCMR1_CC1S_0;  //  CC1 channel is configured as input, IC1 is mapped on TI1
	TIM3->CCMR1 |= TIM_CCMR1_CC2S_1; //  CC2 channel is configured as input, IC2 is mapped on TI1
	TIM3->CCMR1 |= TIM_CCMR1_IC1F_0; //  fSAMPLING=fCK_INT, N=2
	TIM3->CCMR1 |= TIM_CCMR1_IC2F_0; //  fSAMPLING=fCK_INT, N=2

	TIM3->CCER = TIM_CCER_CC1P;  // capture is done on a falling edge
	TIM3->CCER |= TIM_CCER_CC1E; // Capture enabled
	TIM3->CCER |= TIM_CCER_CC2E; // Capture enabled

	TIM3->DIER |= TIM_DIER_CC1IE; // Capture 1 interrupt enable
	TIM3->DIER |= TIM_DIER_CC2IE; // Capture 2 interrupt enable

	TIM3->SMCR = TIM_SMCR_TS_2 | TIM_SMCR_TS_0; // Trigger selection: TI1FP1
	TIM3->SMCR |= TIM_SMCR_SMS_2; // Reset Mode - Rising edge of the selected trigger input

	TIM3->CR1 = TIM_CR1_CEN; // Counter enable

	LED1_BB = 1;

	gpio_pin_cfg(AM2302_GPIO, AM2302_PIN, GPIO_CRx_MODE_CNF_IN_FLOATING_value);

	module.index = 0;
	module.startMeas = false;
}

static void one_wire_setCompare(void)
{
	gpio_pin_cfg(AM2302_GPIO, AM2302_PIN, GPIO_CRx_MODE_CNF_OUT_OD_10M_value);
	AM2302_OUT_BB = 0;

	// clear prevoius configuration
	TIM3->SMCR = 0;
	TIM3->CCER = 0;
	TIM3->CCMR1 = 0;
	TIM3->SR = 0;

	TIM3->CCR3 = UINT16_MAX-100;
	TIM3->CCR2 = UINT16_MAX;
	TIM3->CCR1 = UINT16_MAX;
	TIM3->DIER = TIM_DIER_CC3IE; // Compare 3 interrupt enable
	TIM3->CR1 = TIM_CR1_OPM;     // One pulse mode
	TIM3->EGR = TIM_EGR_UG;      // preload register are transferred into the shadow register

	TIM3->CR1 |= TIM_CR1_CEN;    // Counter enable
	module.state = ONE_WIRE_TRIGGER;
}

static void one_wire_initClock(void)
{
	// Timer3
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN; // Timer 3 clock enable
	TIM3->PSC = 2;
	TIM3->ARR = UINT16_MAX;
	NVIC_EnableIRQ(TIM3_IRQn);
}

uint8_t one_wire_timeConvert(uint16_t cnt)
{
	uint32_t time = 0;
	time = (TIM3->PSC + 1) * (cnt+1);
	time = (time * 1000000) / FREQUENCY; // time in us
	return (uint8_t)time;
}

void one_wire_test_fillBuff(uint8_t* src, uint8_t size)
{
	memset(&module.buff, 0, sizeof(module.buff));
	memcpy(&module.buff, src, size);
}

uint8_t one_wire_frameDecoder(void)
{
	one_wire_decodePhase phase = ONE_WIRE_HOST_START;
	uint16_t index = 0;
	one_wire_frame tempFrame;
	uint8_t idxBit = 0;
	one_wire_posibBits bit = 0;
	bool bitScore = false;
	bool chckScore = false;
	uint8_t result = 0;

	memset(&tempFrame, 0, sizeof(tempFrame));

	while ( phase < ONE_WIRE_DECODED )
	{
		switch (phase)
		{
			case ONE_WIRE_HOST_START:

				if ( one_wire_checkTime(module.buff[index], possibleTimings[ONE_WIRE_TRIG_TIME]) ) {
					phase = ONE_WIRE_START_RESPONSE;
					index++;
				} else {
					result = 1;
					phase = ONE_WIRE_ERROR;
				}

				break;

			case ONE_WIRE_START_RESPONSE:

				if ( one_wire_checkTime(module.buff[index], possibleTimings[ONE_WIRE_START_TIME]) ) {
					index++;
					if ( one_wire_checkTime(module.buff[index], possibleTimings[ONE_WIRE_START_TIME]) ) {
						phase = ONE_WIRE_PAYLOAD;
						index++;
					} else {
						result = 2;
						phase = ONE_WIRE_ERROR;
					}

				} else {
					result = 3;
					phase = ONE_WIRE_ERROR;
				}

				break;

			case ONE_WIRE_PAYLOAD:

				bit = one_wire_checkBit(module.buff, &index);

				if ( (bit == ONE_WIRE_ZERO_BIT) || (bit == ONE_WIRE_ONE_BIT) ) {

					bitScore = one_wire_shiftBit(&tempFrame, bit, idxBit);

					if (bitScore) {
						idxBit++;
					} else {
						result = 4;
						phase = ONE_WIRE_ERROR;
					}

				} else if (bit == ONE_WIRE_END_BIT) {
					phase = ONE_WIRE_CHECKSUM;
				} else {
					result = 5;
					phase = ONE_WIRE_ERROR;
				}

				break;

			case ONE_WIRE_CHECKSUM:

				chckScore = one_wire_checkSum(tempFrame);

				if (chckScore) {
					module.measurments.temperature = tempFrame.temp;
					module.measurments.humadity = tempFrame.hum;
					phase = ONE_WIRE_DECODED;
				} else {
					result = 6;
					phase = ONE_WIRE_ERROR;
				}
				break;

			default:
				result = 7;
				phase = ONE_WIRE_ERROR;
				break;
		}
	}

	return result;
}

bool one_wire_checkTime(uint8_t value, one_wire_timeThr thr)
{
	bool result = false;

	if ( (thr.low  < value)  && (value < thr.high) ) {
		result = true;
	}

	return result;
}

one_wire_posibBits one_wire_checkBit(volatile uint8_t* buff, uint16_t* idx)
{
	if (one_wire_checkTime(buff[*idx], possibleTimings[ONE_WIRE_PAYLOAD_START_TIME])) {
		*idx = *idx + 1;

		if (one_wire_checkTime(buff[*idx], possibleTimings[ONE_WIRE_PAYLOAD_ONE_TIME])) {
			*idx = *idx + 1;
			return ONE_WIRE_ONE_BIT;
		} else if (one_wire_checkTime(buff[*idx], possibleTimings[ONE_WIRE_PAYLOAD_ZERO_TIME])) {
			*idx = *idx + 1;
			return ONE_WIRE_ZERO_BIT;
		}
	} else if (one_wire_checkTime(buff[*idx], possibleTimings[ONE_WIRE_END_TIME])) {
		*idx = *idx + 1;
		return ONE_WIRE_END_BIT;
	}

	return ONE_WIRE_ERROR_BIT;
}

bool one_wire_shiftBit(one_wire_frame* tmp, uint8_t data, uint8_t idx)
{
	bool result = false;

	if (idx < HUM_MASK) {
		tmp->hum |= ( data << ( (HUM_MASK-1) - idx) );
		result = true;
	} else if ((idx < TEMP_MASK)) {
		tmp->temp |= ( data << ( (TEMP_MASK-1) - idx) );
		result = true;
	} else if ((idx < CHCK_MASK)) {
		tmp->chck |= ( data << ((CHCK_MASK-1) - idx) );
		result = true;
	}

	return result;
}


bool one_wire_checkSum(one_wire_frame data)
{
	bool result = false;
	uint8_t tmp = 0;

	tmp = one_wire_checkOne(data.hum);
	tmp += one_wire_checkOne(data.temp);

	if (tmp == data.chck) {
		result = true;
	}

	return result;
}


uint8_t one_wire_checkOne(uint16_t data)
{
	uint8_t val = 0;

	val = data & BYTE_MASK;
	val = val + (data>>8);

	return val;
}

uint16_t one_wire_getHum(void)
{
	return module.measurments.humadity;
}

uint16_t one_wire_getTemp(void)
{
	return module.measurments.temperature;
}


/*============================================================================+
| ISRs
+============================================================================*/
void TIM3_IRQHandler(void)
{
	if (TIM3->SR & TIM_SR_CC3IF) {
		TIM3->SR = ~TIM_SR_CC3IF; // Clear Capture/Compare 3 interrupt Flag

		if (module.state == ONE_WIRE_TRIGGER) {
			one_wire_setCapture();
			module.startMeas = false;
			module.state = ONE_WIRE_READING;
			TIM3->SR = TIM_SR_CC2IF;
		} else {
			module.state = ONE_WIRE_CAPTURED;
			TIM3->CR1 = ~TIM_CR1_CEN; // Counter disable
		}
	}

	if (TIM3->SR & TIM_SR_CC2IF) {
		TIM3->SR = ~TIM_SR_CC2IF; // Clear Capture/Compare 2 interrupt Flag

		module.edge.rising = TIM3->CCR2;
		LED1_BB = 1;

		if (module.startMeas) {
			module.stateTime.low = module.edge.rising - 0;

			module.buff[module.index] = one_wire_timeConvert(module.stateTime.low);
			module.index = (module.index+1) & BUFF_MASK;
		} else {
			module.startMeas = true;
		}
	}

	if (TIM3->SR & TIM_SR_CC1IF) {
		TIM3->SR = ~TIM_SR_CC1IF; // Clear Capture/Compare 1 interrupt Flag

		module.edge.falling = TIM3->CCR1;
		LED1_BB = 0;

		if (module.startMeas) {
			module.stateTime.high = module.edge.falling - module.edge.rising;

			module.buff[module.index] = one_wire_timeConvert(module.stateTime.high);
			module.index = (module.index+1) & BUFF_MASK;
		}
	}
}

/******************************************************************************
* END OF FILE
******************************************************************************/
