/** \file config.h
 * \brief Basic configuration of the project
 * \author Freddie Chopin, http://www.freddiechopin.info/
 * \date 2012-01-07
 */

/******************************************************************************
* project: stm32_blink_led
* chip: STM32F103RB
* compiler: arm-none-eabi-gcc (Sourcery CodeBench Lite 2011.09-69) 4.6.1
******************************************************************************/

#ifndef CONFIG_H_
#define CONFIG_H_

#include "hdr/hdr_bitband.h"

/*
+=============================================================================+
| global definitions
+=============================================================================+
*/

#define CRYSTAL								8000000ul	///< quartz crystal resonator which is connected to the chip
#define FREQUENCY							72000000ul	///< desired target frequency of the core

#define LED1_GPIO							GPIOA		///< GPIO port to which the LED is connected
#define LED1_pin							5			///< pin number of the LED

#define LED1								(1 << LED1_pin)
#define LED1_ODR							LED1_GPIO->ODR	///< output register for the LED
#define LED1_bb								bitband_t m_BITBAND_PERIPH(&LED1_ODR, LED1_pin)	///< bit-band "variable" to directly handle the pin

#define LED2_GPIO							GPIOA		///< GPIO port to which the LED is connected
#define LED2_pin							6			///< pin number of the LED

#define LED2								(1 << LED2_pin)
#define LED2_ODR							LED2_GPIO->ODR	///< output register for the LED
#define LED2_bb								bitband_t m_BITBAND_PERIPH(&LED2_ODR, LED2_pin)	///< bit-band "variable" to directly handle the pin

#define LED3_GPIO							GPIOA		///< GPIO port to which the LED is connected
#define LED3_pin							7			///< pin number of the LED

#define LED3								(1 << LED3_pin)
#define LED3_ODR							LED3_GPIO->ODR	///< output register for the LED
#define LED3_bb								bitband_t m_BITBAND_PERIPH(&LED3_ODR, LED3_pin)	///< bit-band "variable" to directly handle the pin


#define BUTTON1_GPIO						GPIOC		///< GPIO port to which the BUTTON is connected
#define BUTTON1_pin							13			///< pin number of the BUTTON

#define BUTTON2_GPIO						GPIOA		///< GPIO port to which the BUTTON is connected
#define BUTTON2_pin							8			///< pin number of the BUTTON

#define BUTTON3_GPIO						GPIOA		///< GPIO port to which the BUTTON is connected
#define BUTTON3_pin							9			///< pin number of the BUTTON


/*
+=============================================================================+
| strange variables
+=============================================================================+
*/

/*
+=============================================================================+
| global variables
+=============================================================================+
*/

/*
+=============================================================================+
| global functions' declarations
+=============================================================================+
*/

/******************************************************************************
* END OF FILE
******************************************************************************/
#endif /* CONFIG_H_ */
