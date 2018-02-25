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

#define CRYSTAL 8000000ul    // quartz crystal resonator which is connected to the chip
#define FREQUENCY 72000000ul    // desired target frequency of the core

// Leds
#define LED1_GPIO GPIOA    // GPIO port to which the LED is connected
#define LED1_PIN 5    // pin number of the LED

#define LED2_GPIO GPIOC    // GPIO port to which the LED is connected
#define LED2_PIN 9   // pin number of the LED

#define LED3_GPIO GPIOC    // GPIO port to which the LED is connected
#define LED3_PIN 8    // pin number of the LED

#define LED1 (1 << LED1_PIN)
#define LED1_ODR LED1_GPIO->ODR    // output register for the LED
#define LED1_BB bitband_t m_BITBAND_PERIPH(&LED1_ODR, LED1_PIN)    // bit-band "variable" to directly handle the pin

#define LED2 (1 << LED2_pin)
#define LED2_ODR LED2_GPIO->ODR    // output register for the LED
#define LED2_BB bitband_t m_BITBAND_PERIPH(&LED2_ODR, LED2_PIN)    // bit-band "variable" to directly handle the pin

#define LED3 (1 << LED3_PIN)
#define LED3_ODR  LED3_GPIO->ODR    // output register for the LED
#define LED3_BB bitband_t m_BITBAND_PERIPH(&LED3_ODR, LED3_PIN)    // bit-band "variable" to directly handle the pin

//Buttons
#define BUTTON1_GPIO GPIOC    // GPIO port to which the BUTTON is connected
#define BUTTON1_PIN 13    // pin number of the BUTTON


//PWM outputs
#define PWM1_GPIO GPIOA    // GPIO port of PWM1 output
#define PWM1_PIN 8    // pin number of PWM1 output

#define PWM2_GPIO GPIOA    // GPIO port of PWM2 output
#define PWM2_PIN 6    // pin number of PWM2 output

#define PWM3_GPIO GPIOB    // GPIO port of PWM3 output
#define PWM3_PIN 9    // pin number of PWM3 output

#define TI2_GPIO GPIOA    // GPIO port to which the external clock TI2 is connected
#define TI2_PIN 0    // pin number of the external clock TI2

//Adc
#define ADC15_GPIO GPIOC    // GPIO port for ADC input
#define ADC15_PIN 5    /// pin number for ADC input

#define ADC14_GPIO GPIOC    // GPIO port for ADC input
#define ADC14_PIN 4    // pin number for ADC input

#define ADC10_GPIO GPIOC    // GPIO port for ADC input
#define ADC10_PIN 0    // pin number for ADC input

//Encoder
#define ENC_TI1_GPIO GPIOA
#define ENC_TI1_PIN 0

#define ENC_TI2_GPIO GPIOA
#define ENC_TI2_PIN 1

#define ENC_SW_GPIO GPIOC
#define ENC_SW_PIN 12


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
