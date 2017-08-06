
#include <stdint.h>
#include "inc/stm32f10x.h"
#include "config.h"
#include "hdr/hdr_rcc.h"
#include "hdr/hdr_gpio.h"
#include "gpio/gpio.h"



static void sysclk_init(void);
static void system_init(void);


int main(void)
{
	SysTick_Config(4000000);
	system_init();
	sysclk_init();


	while (1)
	{

		if( GPIOC->IDR & GPIO_IDR_IDR13 )    // check if button is press
		{
			LED2_bb = 0;    // turn off diode
		}
		else
		{
			LED2_bb = 1;
		}

		if(GPIOA->IDR & GPIO_IDR_IDR8 )    // check if button is press
		{
			LED3_bb = 1;
		}
		else
		{
			LED3_bb = 0;
		}
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


__attribute__ (( interrupt )) void SysTick_Handler(void)
{
//	LED1_bb ^= 1;
	BB(GPIOA->ODR, P5) ^= 1;
}
