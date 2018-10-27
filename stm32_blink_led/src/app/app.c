/*
 *  app.c
 *
 *  Created on: 28 paz 2018
 *      Author: jarek
 */

/*
+=============================================================================+
| includes
+=============================================================================+
*/
#include "app.h"
#include <stdint.h>
#include "../inc/stm32f10x.h"
#include "../hdr/hdr_gpio.h"
#include "../config.h"
#include "gpio.h"
#include "led.h"
#include "system.h"
#include "encoder.h"
#include "measure.h"
#include "uart.h"
#include "oled1306.h"
#include "am2302.h"
#include "spi.h"

/*
+=============================================================================+
| defines
+=============================================================================+
*/
#define DISLPAY_REFRESH_TIME 20

/*
+=============================================================================+
| module variables
+=============================================================================+
*/
static BasicTimer displayTimer;

/*
+=============================================================================+
| local functions' declarations
+=============================================================================+
*/
static void app_task(void);
static void app_timer(void);

/*
+=============================================================================+
| global functions
+=============================================================================+
*/
void app_init(void)
{
    gpio_init();
    led_init();
	Encoder_init();
	measure_init();
	uart_init();
	OLED_init();
	AM2302_init();
	uart_putStr("System init end\n");

    system_createTask(app_task, app_timer, 1, "app", 1);
	TIMER_START(displayTimer, DISLPAY_REFRESH_TIME);
}


/*
+=============================================================================+
| local functions
+=============================================================================+
*/
static void app_task(void)
{
	SPI_checkDmaStatus();

	if (displayTimer.flag) {
		OLED_SetCursor(11, 10);
		OLED_writeString("cnt:    ", Font_11x18, WHITE);
		OLED_SetCursor(55, 10);
		OLED_writeInt(TIM2->CNT, 10, Font_11x18, WHITE);

		OLED_SetCursor(11, 30);
		OLED_writeString("vol:    ", Font_11x18, WHITE);
		OLED_SetCursor(55, 30);
		OLED_writeInt(measure_getVoltage(), 10, Font_11x18, WHITE);

		uart_putStr("Voltage: ");
		uart_putInt(measure_getVoltage(), 10);
		uart_putStr("\n");

		OLED_refresh();
		TIMER_START(displayTimer, DISLPAY_REFRESH_TIME);
	}
}


/*
+=============================================================================+
| Timers
+=============================================================================+
*/
static void app_timer(void)
{
    if (displayTimer.cnt != 0) {
        if (--displayTimer.cnt == 0) {
        	displayTimer.flag = true;
        }
    }
}


/*
+=============================================================================+
| ISRs
+=============================================================================+
*/

/******************************************************************************
* END OF FILE
******************************************************************************/
