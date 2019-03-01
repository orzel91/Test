/*
 * encoder.c
 *
 *  Created on: 25 paz 2018
 *      Author: jarek
 */


/*
+=============================================================================+
| includes
+=============================================================================+
*/
#include "encoder.h"
#include "../inc/stm32f10x.h"
#include "../hdr/hdr_gpio.h"
#include "../config.h"
#include "../gpio/gpio.h"


/*
+=============================================================================+
| defines
+=============================================================================+
*/
#define ENC_TICKS 0x100


/*
+=============================================================================+
| module variables
+=============================================================================+
*/
volatile uint16_t enc_cnt = 0;

/*
+=============================================================================+
| local functions' declarations
+=============================================================================+
*/
static void encoder_buttonInit(void);
static void encoder_encoderHwInit(void);

/*
+=============================================================================+
| global functions
+=============================================================================+
*/
void encoder_init(void)
{
	encoder_encoderHwInit();
	encoder_buttonInit();
}

/*
+=============================================================================+
| local functions
+=============================================================================+
*/
static void encoder_buttonInit(void)
{
    // SW button on encoder
    AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI12_PC; // source input for EXTIx external interrupt
    EXTI->IMR = EXTI_IMR_MR12;    // configure interrupt mask
    EXTI->FTSR =  EXTI_FTSR_TR12;    // setting trigerring by falling edge
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}

static void encoder_encoderHwInit(void)
{
    // Timer 2

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;    // turn on clock for Timer2


    gpio_pin_cfg(ENC_TI1_GPIO, ENC_TI1_PIN, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);    // PA0 configured as input floating
	gpio_pin_cfg(ENC_TI2_GPIO, ENC_TI2_PIN, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);    // PA1 configured as input floating
	gpio_pin_cfg(ENC_SW_GPIO, ENC_SW_PIN, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);    // PC12 configured as input floating


    TIM2->SMCR = TIM_SMCR_SMS_1; // counter is counting on TI1 edges only
    TIM2->CCMR1 = TIM_CCMR1_IC1F | TIM_CCMR1_IC2F; // Input capture filters
    TIM2->CCER = TIM_CCER_CC1P; // inverted: capture is done on a falling edge of IC1
    TIM2->ARR = ENC_TICKS; // quadrature encoder
    TIM2->DIER = TIM_DIER_UIE; // Update interrupt enabled
    TIM2->CR1 = TIM_CR1_CEN; // Counter enable, start counting!

    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);
}

/*
+=============================================================================+
| ISRs
+=============================================================================+
*/
void TIM2_IRQHandler(void)
{
	if (TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR = ~TIM_SR_UIF;
		enc_cnt++;
	}
}

void EXTI15_10_IRQHandler(void)
{
	if (EXTI->PR & EXTI_PR_PR12)
	{
		EXTI->PR = EXTI_PR_PR12;
		TIM2->CNT = 0;
	}
}

