
#include <stdint.h>
#include "inc/stm32f10x.h"
#include "config.h"
#include "hdr/hdr_rcc.h"
#include "hdr/hdr_gpio.h"
#include "gpio/gpio.h"


// Local variables
volatile uint16_t cnt = 0;


// Local functions declarations
static void sysclk_init(void);
static void system_init(void);
static void PWMInit(void);
static void externalClockMode1(void);


int main(void)
{
	SysTick_Config(4000000);    // set systick to 500us
	system_init();
	sysclk_init();
	PWMInit();
	externalClockMode1();

	while (1)
	{


	}
}



static void sysclk_init(void)
{
	RCC_CR_HSION_bb = 1;    // enable HSI clock
	while(!RCC_CR_HSIRDY_bb);    // wait until HIS clock will be ready
	RCC_CFGR_PLLSRC_bb = 0;    // HSI oscillator clock / 2 selected as PLL input clock
	RCC->CFGR = RCC_CFGR_PLLMUL16_value;    // set PLL to multiplay x16
	RCC_CR_PLLON_bb = 1;    // enable PLL
	while (!RCC_CR_PLLRDY_bb);   // wait until PLL will be ready
	RCC->CFGR |= RCC_CFGR_SW_PLL;   // change SYSCLK to PLL
}



static void system_init(void)
{
	gpio_init();

	gpio_pin_cfg(LED1_GPIO, LED1_pin, GPIO_CRx_MODE_CNF_OUT_PP_10M_value);
	gpio_pin_cfg(LED2_GPIO, LED2_pin, GPIO_CRx_MODE_CNF_OUT_PP_10M_value);
	gpio_pin_cfg(LED3_GPIO, LED3_pin, GPIO_CRx_MODE_CNF_OUT_PP_10M_value);

	GPIOA->BSRR = GPIO_BSRR_BS6;	// turn off LED2 by 3V3 (which is activated by 0V)
	GPIOA->BSRR = GPIO_BSRR_BS7;	// turn off LED3 by 3V3 (which is activated by 0V)

	gpio_pin_cfg(BUTTON1_GPIO, BUTTON1_pin, GPIO_CRx_MODE_CNF_IN_FLOATING_value);
	gpio_pin_cfg(BUTTON3_GPIO, BUTTON3_pin, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);

	GPIOA->BSRR = GPIO_BSRR_BS9;	// pull up Button3

}



static void PWMInit(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;    // turn on clock for Timer1

	gpio_pin_cfg(PWM1_GPIO, PWM1_pin, GPIO_CRx_MODE_CNF_ALT_PP_2M_value);    // PA8 configured as alternate function

	TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;    // set PWM mode 1
	TIM1->CCMR1 |= TIM_CCMR1_OC1PE;    // Preload register on TIMx_CCR1 enabled

//	TIM1->CR1 |= TIM_CR1_DIR;    //
//	TIM1->CCER = TIM_CCER_CC1P;    // CC1 channel configured as output: active low

	TIM1->CCER |= TIM_CCER_CC1E;    // CC1 channel configured as output: On
	TIM1->BDTR = TIM_BDTR_MOE;    // Main output enable

	TIM1->PSC = 1000;    // value of prescaler
	TIM1->ARR = 8000;    // value of overload
	TIM1->CCR1 = 4000;    // value to be compared to the counter CNT, and signaled on OC1 output

	TIM1->EGR = TIM_EGR_UG;    // Reinitialize the counter and generates an update of the registers
	TIM1->CR1 |= TIM_CR1_ARPE;    // Auto-reload preload enable
	TIM1->CR1 |= TIM_CR1_CEN;    // Counter enable, start counting!

}

static void externalClockMode1(void)
{
	RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;    // turn on clock for Timer2

	gpio_pin_cfg(TI2_GPIO, TI2_pin, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);    // set PA0 as input, in pull up/down mode
	GPIOA->BSRR = GPIO_BSRR_BS0;	// pull up PA0

	TIM2->CCER = TIM_CCER_CC2P;    // capture is done on a falling edge
	TIM2->SMCR = TIM_SMCR_SMS;    // Slave mode selection: External Clock Mode 1
	TIM2->SMCR |=  TIM_SMCR_TS_2;    // trigger selection: TI1 Edge Detector

	TIM2->ARR = 1;    // value of overload
	TIM2->DIER = TIM_DIER_UIE;    // Update interrupt enable
	TIM2->CR1 = TIM_CR1_CEN;    // Counter enable

	NVIC_EnableIRQ(TIM2_IRQn);    // TIM2 Update Interrupt enable in NVIC

}


__attribute__ (( interrupt )) void SysTick_Handler(void)
{
//	LED1_bb ^= 1;
	BB(GPIOA->ODR, P5) ^= 1;
}


__attribute__ (( interrupt )) void TIM2_IRQHandler(void)
{
	if(TIM2->SR & TIM_SR_UIF)    // check interrupt flag in status register
	{
		TIM2->SR &= ~TIM_SR_UIF;    // turn off interrupt flag in status register

		//	LED3_bb ^= 1;
		BB(GPIOA->ODR, P6) ^= 1;
		cnt = 0;
	}
}
