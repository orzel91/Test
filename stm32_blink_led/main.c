
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
	volatile uint32_t count, count_max = 1000000;

	system_init();
	sysclk_init();

	gpio_pin_cfg(LED_GPIO, LED_pin, GPIO_CRx_MODE_CNF_OUT_PP_10M_value);

	while (1)
	{
		for (count = 0; count < count_max; count++);	// delay
		LED_bb = 1;
		for (count = 0; count < count_max; count++);	// delay
		LED_bb = 0;
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
}
