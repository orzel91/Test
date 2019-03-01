/*
 * am2302.c
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
#include "../system/system.h"
#include "../uart/uart.h"


/*============================================================================+
| defines
+============================================================================*/
#define AM2302_REFRESH_TIME     2000
#define AM2302_CONVRSION_TIME   20
#define AM2302_CHECK_STATE_TIME 1
#define AM2302_MEASURMENT_AVG   4
#define AM2302_MEASURMENT_MASK  (AM2302_MEASURMENT_AVG -1)

/*============================================================================+
| typedefs
+============================================================================*/
typedef struct {
	uint16_t buffTemp[AM2302_MEASURMENT_AVG];
	uint16_t buffHum[AM2302_MEASURMENT_AVG];
	uint8_t index;
	uint16_t temperature;
	uint16_t humadity;
} AM2320_module;


/*============================================================================+
| module variables
+============================================================================*/
static BasicTimer measureTimer;
static BasicTimer stateTimer;
static AM2320_module am2302Mod;


/*============================================================================+
| local functions' declarations
+============================================================================*/
static void AM2302_task(void);
static void AM2302_timer(void);
static void AM2320_checkResult(void);
static uint16_t AM2302_calcAvg(uint16_t* src, uint16_t size);

/*============================================================================+
| global functions
+============================================================================*/
void AM2302_init(void)
{
	memset(&am2302Mod, 0, sizeof(am2302Mod));

	one_wire_init();

	system_createTask(AM2302_task, AM2302_timer, 1, "AM2302", 5);

	TIMER_START(measureTimer, AM2302_REFRESH_TIME);
}

uint16_t AM2302_getTemp(void)
{
	return am2302Mod.temperature;
}

uint16_t AM2302_getHumad(void)
{
	return am2302Mod.humadity;
}


/*============================================================================+
| local functions
+============================================================================*/
static void AM2302_task(void)
{
	if (measureTimer.flag) {
		LED2_BB = 1;
		one_wire_sendStart();
		LED2_BB = 0;
		TIMER_STOP(measureTimer);
		TIMER_START(stateTimer, AM2302_CONVRSION_TIME);
	}

	if (stateTimer.flag) {
		if (one_wire_getState() == ONE_WIRE_CAPTURED) {

			LED3_BB = 1;
			AM2320_checkResult();
			LED3_BB = 0;

			TIMER_STOP(stateTimer);
			TIMER_START(measureTimer, AM2302_REFRESH_TIME);
		} else {
			TIMER_START(stateTimer, AM2302_CHECK_STATE_TIME);
		}
	}
}

static void AM2320_checkResult(void)
{
	uint16_t measResult = 0xFF;

	measResult = one_wire_frameDecoder();

	if (!measResult) {

		am2302Mod.index = ((am2302Mod.index + 1) & AM2302_MEASURMENT_MASK);

		am2302Mod.buffTemp[am2302Mod.index] = one_wire_getTemp();
		am2302Mod.buffHum[am2302Mod.index] = one_wire_getHum();

		am2302Mod.temperature = AM2302_calcAvg(am2302Mod.buffTemp, AM2302_MEASURMENT_AVG);
		am2302Mod.humadity = AM2302_calcAvg(am2302Mod.buffHum, AM2302_MEASURMENT_AVG);


		uart_putStr("Temp= ");
		uart_putInt((am2302Mod.temperature / 10), 10);
		uart_putStr(",");
		uart_putInt((am2302Mod.temperature % 10), 10);

		uart_putStr(" Hum= ");
		uart_putInt((am2302Mod.humadity / 10), 10);
		uart_putStr(",");
		uart_putInt((am2302Mod.humadity % 10), 10);
		uart_putStr("\n");


	} else {
		uart_putStr("AM2302 error:");
		uart_putInt(measResult, 10);
		uart_putStr("\n");
	}
}

static uint16_t AM2302_calcAvg(uint16_t* src, uint16_t size)
{
	uint32_t average = 0;
	uint16_t i = 0;

	for (i = 0; i < size; i++) {
		average = average + src[i];
	}

	average = average / size;

	return (uint16_t)average;
}


/*============================================================================+
| Timers
+============================================================================*/
static void AM2302_timer(void)
{
    if (measureTimer.cnt != 0) {
        if (--measureTimer.cnt == 0) {
            measureTimer.flag = true;
        }
    }

    if (stateTimer.cnt != 0) {
        if (--stateTimer.cnt == 0) {
            stateTimer.flag = true;
        }
    }
}



/*============================================================================+
| ISRs
+============================================================================*/




/******************************************************************************
* END OF FILE
******************************************************************************/
