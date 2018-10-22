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
#include "gpio/gpio.h"
#include <stdbool.h>
#include <stdint.h>
#include "hdr/hdr_bitband.h"
#include "gpio/gpio.h"

/*
+=============================================================================+
| global definitions
+=============================================================================+
*/

#define CRYSTAL 8000000ul    // quartz crystal resonator which is connected to the chip
#define FREQUENCY 36000000ul    // desired target frequency of the core
#define SYSTICK_FREQ 1000

// LEDs
#define LED1_GPIO GPIOC    // GPIO port to which the LED is connected
#define LED1_PIN 6    // pin number of the LED

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

// DEBUG GPIOs
#define TEST1_GPIO GPIOB    // GPIO port to which the test gpio is connected
#define TEST1_PIN 10    // pin number of the test gpio

#define TEST1 (1 << TEST1_PIN)
#define TEST1_ODR TEST1_GPIO->ODR    // output register for the GPIO
#define TEST1_BB bitband_t m_BITBAND_PERIPH(&TEST1_ODR, TEST1_PIN)    // bit-band "variable" to directly handle the pin

// BUTTONs
#define BUTTON1_GPIO GPIOC    // GPIO port to which the BUTTON is connected
#define BUTTON1_PIN 13    // pin number of the BUTTON


// PWM outputs
#define PWM1_GPIO GPIOA    // GPIO port of PWM1 output
#define PWM1_PIN 8    // pin number of PWM1 output

#define PWM2_GPIO GPIOA    // GPIO port of PWM2 output
#define PWM2_PIN 6    // pin number of PWM2 output

#define PWM3_GPIO GPIOB    // GPIO port of PWM3 output
#define PWM3_PIN 9    // pin number of PWM3 output

#define TI2_GPIO GPIOA    // GPIO port to which the external clock TI2 is connected
#define TI2_PIN 0    // pin number of the external clock TI2

// ADC
#define ADC15_GPIO GPIOC    // GPIO port for ADC input
#define ADC15_PIN 5    // pin number for ADC input

#define ADC14_GPIO GPIOC    // GPIO port for ADC input
#define ADC14_PIN 4    // pin number for ADC input

#define ADC10_GPIO GPIOC    // GPIO port for ADC input
#define ADC10_PIN 0    // pin number for ADC input

// ENCODER
#define ENC_TI1_GPIO GPIOA    // GPIO port for ENC_CLK
#define ENC_TI1_PIN 0    // pin number for ENC_CLK

#define ENC_TI2_GPIO GPIOA    // GPIO port for ENC_DT
#define ENC_TI2_PIN 1    // pin number for ENC_DT

#define ENC_SW_GPIO GPIOC    // GPIO port for ENC_SW
#define ENC_SW_PIN 12    // pin number for ENC_SW

// UART2
#define UART2_TX_GPIO GPIOA
#define UART2_TX_PIN 2

#define UART2_RX_GPIO GPIOA
#define UART2_RX_PIN 3

// SPI1
#define SPI1_SCK_GPIO GPIOA
#define SPI1_SCK_PIN 5

#define SPI1_MOSI_GPIO GPIOA
#define SPI1_MOSI_PIN 7

//#define SPI1_MISO_GPIO GPIOA
//#define SPI1_MISO_PIN 6

#define SPI1_NSS_GPIO GPIOA
#define SPI1_NSS_PIN 4

#define SPI1_NSS (1 << SPI1_NSS_PIN)
#define SPI1_NSS_ODR  SPI1_NSS_GPIO->ODR
#define SPI1_NSS_BB bitband_t m_BITBAND_PERIPH(&SPI1_NSS_ODR, SPI1_NSS_PIN)

// OLED1306
#define OLED_DC_GPIO GPIOA
#define OLED_DC_PIN 9

#define OLED_RES_GPIO GPIOC
#define OLED_RES_PIN 7

#define OLED_DC (1 << OLED_DC_PIN)
#define OLED_DC_ODR  OLED_DC_GPIO->ODR
#define OLED_DC_BB bitband_t m_BITBAND_PERIPH(&OLED_DC_ODR, OLED_DC_PIN)

#define OLED_RES (1 << OLED_RES_PIN)
#define OLED_RES_ODR  OLED_RES_GPIO->ODR
#define OLED_RES_BB bitband_t m_BITBAND_PERIPH(&OLED_RES_ODR, OLED_RES_PIN)


// AM2302 - temperature/humadity sensor
#define AM2302_OUT_GPIO GPIOD
#define AM2302_OUT_PIN 2

#define AM2302_OUT (1 << AM2302_OUT_PIN)
#define AM2302_OUT_ODR AM2302_OUT_GPIO->ODR
#define AM2302_OUT_BB bitband_t m_BITBAND_PERIPH(&AM2302_OUT_ODR, AM2302_OUT_PIN)

#define AM2302_IN_GPIO GPIOC
#define AM2302_IN_PIN 10


//MCO - The microcontroller clock output
//#define MCO_GPIO GPIOA
//#define MCO_PIN 8

#define TIMER_START(timer, value) 	{timer.cnt = value; timer.flag = false;}
#define TIMER_STOP(timer)			{timer.cnt = 0; timer.flag = false;}


typedef struct {
	uint16_t cnt;
	bool flag;
} BasicTimer;


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
