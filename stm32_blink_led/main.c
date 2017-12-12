
#include <stdint.h>
#include "inc/stm32f10x.h"
#include <time.h>
#include "config.h"
#include "hdr/hdr_rcc.h"
#include "hdr/hdr_gpio.h"
#include "gpio/gpio.h"


#define CPU_FREQ 8000000
#define PWM1_FREQ 10000

// Local variables
volatile uint16_t cnt = 0;
uint16_t score = 4096;

// GLobal variables
volatile uint16_t Duty[]= {1599, 1598, 1597, 1596, 1595, 1594, 1593, 1592, 1591, 1590, 1580,
						   1570, 1560, 1550, 1540, 1530, 1520, 1510, 1500, 1475, 1450, 1425,
						   1400, 1350, 1300, 1250, 1200, 1150, 1100, 1050, 1000,  900,  800,
						    700,  600,  500,  400,  300,  200,  100,    0,  100,  200,  300,
							400,  500,  600,  700,  800,  900, 1000, 1050, 1100, 1150, 1200,
						   1250, 1300, 1350, 1400, 1425, 1450, 1475, 1500, 1510, 1520, 1530,
						   1540, 1550, 1560, 1570, 1580, 1590, 1591, 1592, 1593, 1594, 1595,
						   1596, 1597, 1598, 1599 };

uint16_t size =  sizeof(Duty) / sizeof(uint16_t);


// Local functions declarations
static void sysclk_init(void);
static void system_init(void);
static void PWMInit(void);
static void externalClockMode1(void);
static void DMA_init(void);
static void ADC_init(void);
static void ADC_start_conversion(void);
static uint16_t ADC_get_result(void);


int main(void)
{

	SysTick_Config(4000000);    // set systick to 500us
	system_init();
	sysclk_init();
	PWMInit();
	DMA_init();
//	externalClockMode1();
	ADC_init();
	ADC_start_conversion();
	while(!(ADC1->SR & ADC_SR_EOC));
	score = ADC_get_result();


	while (1)
	{
		ADC_start_conversion();
		while(!(ADC1->SR & ADC_SR_EOC));
		score = ADC_get_result();
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
//	gpio_pin_cfg(LED2_GPIO, LED2_pin, GPIO_CRx_MODE_CNF_OUT_PP_10M_value);
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
	RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;    // turn on clock for Timer3

	gpio_pin_cfg(PWM1_GPIO, PWM1_pin, GPIO_CRx_MODE_CNF_ALT_PP_2M_value);    // PA8 configured as alternate function
	gpio_pin_cfg(PWM2_GPIO, PWM2_pin, GPIO_CRx_MODE_CNF_ALT_PP_2M_value);    // PA8 configured as alternate function

	TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;    // set PWM mode 1
	TIM1->CCMR1 |= TIM_CCMR1_OC1PE;    // Preload register on TIMx_CCR1 enabled

//	TIM1->CR1 |= TIM_CR1_DIR;    //
//	TIM1->CCER = TIM_CCER_CC1P;    // CC1 channel configured as output: active low

	TIM1->CCER |= TIM_CCER_CC1E;    // CC1 channel configured as output: On
	TIM1->BDTR = TIM_BDTR_MOE;    // Main output enable

	// PWM freq = (PSC*ARR)/CPU_FREQ
	TIM1->PSC = 100;    // value of prescaler
	TIM1->ARR = 1600;    // value of overload
	TIM1->CCR1 = TIM1->ARR;    // value to be compared to the counter CNT, and signaled on OC1 output

	TIM1->EGR = TIM_EGR_UG;    // Reinitialize the counter and generates an update of the registers
	TIM1->CR1 |= TIM_CR1_ARPE;    // Auto-reload preload enable

	TIM1->DIER = TIM_DIER_CC1DE;    // Capture/Compare 1 DMA request enable

	TIM1->CR1 |= TIM_CR1_CEN;    // Counter enable, start counting!



	TIM3->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;    // set PWM mode 1
	TIM3->CCMR1 |= TIM_CCMR1_OC1PE;    // Preload register on TIMx_CCR1 enabled

	TIM3->CCER |= TIM_CCER_CC1E;    // CC1 channel configured as output: On
	TIM3->BDTR = TIM_BDTR_MOE;    // Main output enable

	// PWM freq = (PSC*ARR)/CPU_FREQ
	TIM3->PSC = 39;    // value of prescaler
	TIM3->ARR = 4096;    // value of overload
	TIM3->CCR1 = TIM3->ARR;    // value to be compared to the counter CNT, and signaled on OC1 output

	TIM3->EGR = TIM_EGR_UG;    // Reinitialize the counter and generates an update of the registers
	TIM3->CR1 |= TIM_CR1_ARPE;    // Auto-reload preload enable

	TIM3->DIER = TIM_DIER_CC1DE;    // Capture/Compare 1 DMA request enable

	TIM3->CR1 |= TIM_CR1_CEN;    // Counter enable, start counting!

}


static void DMA_init(void)
{
	RCC->AHBENR |= RCC_AHBENR_DMA1EN;    // DMA1 clock enable

	DMA1_Channel2->CPAR = (uint32_t)(&TIM1->CCR1);
	DMA1_Channel2->CMAR = (uint32_t)(&Duty);
	DMA1_Channel2->CNDTR = size;

	DMA1_Channel2->CCR = DMA_CCR2_PL_1;    // Channel priority level: High
	DMA1_Channel2->CCR |= DMA_CCR2_DIR;    // Data transfer direction: Read from memory
	DMA1_Channel2->CCR |= DMA_CCR2_CIRC;    // Circular mode enabled
	DMA1_Channel2->CCR |= DMA_CCR2_MINC;    // Memory increment mode enabled
	DMA1_Channel2->CCR |= DMA_CCR2_PSIZE_0;    // Peripheral size - 16bit
	DMA1_Channel2->CCR |= DMA_CCR2_MSIZE_0;    // Memory size - 16bit
	DMA1_Channel2->CCR |= DMA_CCR2_TCIE;    // Transfer complete interrupt enable
	DMA1_Channel2->CCR |= DMA_CCR2_EN;  // Channel enabled


	DMA1_Channel6->CPAR = (uint32_t)(&TIM3->CCR1);
	DMA1_Channel6->CMAR = (uint32_t)(&score);
	DMA1_Channel6->CNDTR = 1;

	DMA1_Channel6->CCR = DMA_CCR6_PL_1;    // Channel priority level: High
	DMA1_Channel6->CCR |= DMA_CCR6_DIR;    // Data transfer direction: Read from memory
	DMA1_Channel6->CCR |= DMA_CCR6_CIRC;    // Circular mode enabled
	DMA1_Channel6->CCR |= DMA_CCR6_MINC;    // Memory increment mode enabled
	DMA1_Channel6->CCR |= DMA_CCR6_PSIZE_0;    // Peripheral size - 16bit
	DMA1_Channel6->CCR |= DMA_CCR6_MSIZE_0;    // Memory size - 16bit
	DMA1_Channel6->CCR |= DMA_CCR6_TCIE;    // Transfer complete interrupt enable
	DMA1_Channel6->CCR |= DMA_CCR6_EN;  // Channel enabled

}


static void ADC_init(void)
{
	uint32_t delay;

	RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;    // turn on clock for ADC17

	gpio_pin_cfg(ADC15_GPIO, ADC15_pin, GPIO_CRx_MODE_CNF_IN_ANALOG_value);    // PC5 configured as alternate function

	ADC1->CR2 = ADC_CR2_ADON;    // wakes up the ADC from Power Down mode
	for(delay = 100000; delay; delay--);    // ADC power-up time - tstab
	ADC1->CR2 |= ADC_CR2_CAL;    // Calibration
	while(ADC1->CR2 & ADC_CR2_CAL);    // wait to the end of calibration
	ADC1->CR2 |= ADC_CR2_CONT;    // Continuous Conversion
	ADC1->CR2 |= ADC_CR2_EXTTRIG;    // Conversion on external event enabled
	ADC1->CR2 |= ADC_CR2_EXTSEL_2 | ADC_CR2_EXTSEL_1 | ADC_CR2_EXTSEL_0;    // SWSTART set as trigger for start of conversion
	ADC1->SQR3 = 15;    // 1st conversion in regular sequence at 15 chanell
	ADC1->SMPR1 = ADC_SMPR1_SMP15_1 | ADC_SMPR1_SMP15_0;    // Channel15 sample time: 28.5 cycles

//	ADC1->CR2 = ADC_CR2_ADON;    // Conversion starts

}


static void ADC_start_conversion(void)
{
	ADC1->CR2 |= ADC_CR2_SWSTART;    // triger Start of Conversion
}


static uint16_t ADC_get_result(void)
{
	return ADC1->DR;
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
