
#include <stdint.h>
#include "inc/stm32f10x.h"
#include "config.h"
#include "hdr/hdr_rcc.h"
#include "hdr/hdr_gpio.h"
#include "gpio/gpio.h"



static void sysclk_init(void);
static void system_init(void);
static void buttonInterruptInit(void);
static void timerInit(void);


int main(void)
{
	SysTick_Config(4000000);    // set systick to 500us
	system_init();
	sysclk_init();
	buttonInterruptInit();
	timerInit();


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
	gpio_pin_cfg(BUTTON2_GPIO, BUTTON2_pin, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);
	gpio_pin_cfg(BUTTON3_GPIO, BUTTON3_pin, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);

	GPIOA->BSRR = GPIO_BSRR_BS8;	// pull up Button2
	GPIOA->BSRR = GPIO_BSRR_BS9;	// pull up Button3

}


static void buttonInterruptInit(void)
{
	AFIO->EXTICR[3] = AFIO_EXTICR3_EXTI8_PA;    // alternative function on pin PA8 and PA9
	EXTI->IMR =  EXTI_EMR_MR8;    // configure interrupt mask
	EXTI->FTSR =  EXTI_FTSR_TR8;    // setting trigerring by falling edge
	NVIC->ISER[0] = NVIC_ISER_SETENA_8;    // Enable interrupt in NVIC
	NVIC_EnableIRQ(EXTI9_5_IRQn);

}


static void timerInit(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;    // turn on clock for Timer1
	NVIC_EnableIRQ(TIM1_UP_IRQn);    // TIM1 Update Interrupt enable in NVIC
	TIM1->DIER = TIM_DIER_UIE;    // Update interrupt enable
	TIM1->PSC = 1000;    // value of prescaler
	TIM1->ARR = 8000;    // value of overload
	TIM1->CR1 |= TIM_CR1_CEN;    // Counter enable, start counting!

}


__attribute__ (( interrupt )) void SysTick_Handler(void)
{
//	LED1_bb ^= 1;
	BB(GPIOA->ODR, P5) ^= 1;
}


__attribute__ (( interrupt )) void EXTI9_5_IRQHandler(void)
{

	if (EXTI->PR & EXTI_PR_PR8)
	{
		EXTI->PR = EXTI_PR_PR8;

		//	LED3_bb ^= 1;
		BB(GPIOA->ODR, P6) ^= 1;
	}
}

__attribute__ (( interrupt )) void TIM1_UP_IRQHandler(void)
{
	if(TIM1->SR & TIM_SR_UIF)    // check interrupt flag in status register
	{
		TIM1->SR &= ~TIM_SR_UIF;    // turn off interrupt flag in status register
		BB(GPIOA->ODR, P7) ^= 1;    // toggle LED
	}
}


