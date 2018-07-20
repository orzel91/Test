#include <stdint.h>
#include "inc/stm32f10x.h"
#include <time.h>
#include "config.h"
#include "hdr/hdr_rcc.h"
#include "hdr/hdr_gpio.h"
#include "gpio/gpio.h"
#include "uart/uart.h"
#include "uart/AT_commands.h"


#define PWM1_FREQ 10000
#define ADC_BUFFER_SIZE 10
#define ENC_TICKS 0x27

//#define EXTERNAL_CLOCK
//#define ADC_AVARAGE

// Local variables
volatile uint16_t cnt = 0;
volatile uint16_t uartFlag = 0;
volatile uint16_t adcFlag = 0;

uint16_t buffer[ADC_BUFFER_SIZE];

// GLobal variables
volatile uint16_t Duty[]= {1599, 1598, 1597, 1596, 1595, 1594, 1593, 1592, 1591, 1590, 1580,
                           1570, 1560, 1550, 1540, 1530, 1520, 1510, 1500, 1475, 1450, 1425,
                           1400, 1350, 1300, 1250, 1200, 1150, 1100, 1050, 1000,  900,  800,
                           700,  600,  500,  400,  300,  200,  100,    0,  100,  200,  300,
                           400,  500,  600,  700,  800,  900, 1000, 1050, 1100, 1150, 1200,
                           1250, 1300, 1350, 1400, 1425, 1450, 1475, 1500, 1510, 1520, 1530,
                           1540, 1550, 1560, 1570, 1580, 1590, 1591, 1592, 1593, 1594, 1595,
                           1596, 1597, 1598, 1599
                          };

uint16_t size =  sizeof(Duty) / sizeof(uint16_t);

volatile uint16_t enc_cnt = 0;

// Local functions declarations
#ifdef EXTERNAL_CLOCK
static void externalClockMode1(void);
#endif
#ifdef ADC_AVARAGE
static uint16_t ADC_filter(uint16_t *array, uint16_t size);
#endif
static void sysclk_init(void);
static void system_init(void);
static void PWMInit(void);
static void DMA_init(void);
static void ADC_init(void);
static void Encoder_init(void);



int main(void)
{
	sysclk_init();
	SysTick_Config(9000000);    // set systick to 250ms
	system_init();
	DMA_init();
	ADC_init();
	PWMInit();
	Encoder_init();
	UART_init(BAUD_RATE);
	AT_commandInit();

	UART_putStr("System init...");

	while(1)
	{
		UART_RX_STR_EVENT();
	}
}



static void sysclk_init(void)
{
    RCC_CR_HSION_bb = 1;    // enable HSI clock
    while(!RCC_CR_HSIRDY_bb);    // wait until HIS clock will be ready
    RCC_CFGR_PLLSRC_bb = 0;    // HSI oscillator clock / 2 selected as PLL input clock
    RCC->CFGR |= RCC_CFGR_PLLMULL9;    // set PLL to multiplay x9
    RCC_CR_PLLON_bb = 1;    // enable PLL
    while (!RCC_CR_PLLRDY_bb);   // wait until PLL will be ready
    RCC->CFGR |= RCC_CFGR_SW_PLL;   // change SYSCLK to PLL
    RCC->CFGR |= RCC_CFGR_PPRE1_DIV1; // HCLK not divided
}



static void system_init(void)
{
    gpio_init();

    gpio_pin_cfg(LED1_GPIO, LED1_PIN, GPIO_CRx_MODE_CNF_OUT_PP_2M_value);
	gpio_pin_cfg(LED2_GPIO, LED2_PIN, GPIO_CRx_MODE_CNF_OUT_PP_2M_value);
    gpio_pin_cfg(LED3_GPIO, LED3_PIN, GPIO_CRx_MODE_CNF_OUT_PP_2M_value);

    gpio_pin_cfg(TEST1_GPIO, TEST1_PIN, GPIO_CRx_MODE_CNF_OUT_PP_2M_value);
    gpio_pin_cfg(TEST2_GPIO, TEST2_PIN, GPIO_CRx_MODE_CNF_OUT_PP_2M_value);

    gpio_pin_cfg(BUTTON1_GPIO, BUTTON1_PIN, GPIO_CRx_MODE_CNF_IN_FLOATING_value);

    LED1_BB = 0; // turn off LED2 by 0V (which is activated by 3V3)
    LED2_BB = 1; // turn off LED2 by 3V3 (which is activated by 0V)
    LED3_BB = 1; // turn off LED2 by 3V3 (which is activated by 0V)

    TEST1_BB = 0; // Set GPIO to Low
    TEST2_BB = 0; // Set GPIO to Low
}



static void PWMInit(void)
{
    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;    // turn on clock for Timer1
    RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;    // turn on clock for Timer3
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;    // turn on clock for Timer4

    gpio_pin_cfg(PWM1_GPIO, PWM1_PIN, GPIO_CRx_MODE_CNF_ALT_PP_2M_value);    // PA8 configured as alternate function
    gpio_pin_cfg(PWM2_GPIO, PWM2_PIN, GPIO_CRx_MODE_CNF_ALT_PP_2M_value);    // PA6 configured as alternate function


    // Timer1

    TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;    // set PWM mode 1
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;    // Preload register on TIMx_CCR1 enabled

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


    // Timer3

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


    DMA1_Channel1->CPAR = (uint32_t)(&ADC1->DR);
    DMA1_Channel1->CMAR = (uint32_t)(&buffer);
    DMA1_Channel1->CNDTR = ADC_BUFFER_SIZE;

    DMA1_Channel1->CCR = DMA_CCR1_PL_1;    // Channel priority level: High
    DMA1_Channel1->CCR |= DMA_CCR1_CIRC;    // Circular mode enabled
    DMA1_Channel1->CCR |= DMA_CCR1_MINC;    // Memory increment mode enabled
    DMA1_Channel1->CCR |= DMA_CCR1_PSIZE_0;    // Peripheral size - 16bit
    DMA1_Channel1->CCR |= DMA_CCR1_MSIZE_0;    // Memory size - 16bit
    DMA1_Channel1->CCR |= DMA_CCR1_EN;  // Channel enabled

    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}


static void ADC_init(void)
{
    uint32_t delay;

    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN | RCC_APB2ENR_ADC2EN;    // turn on clock for ADC1 and ADC2

    gpio_pin_cfg(ADC15_GPIO, ADC15_PIN, GPIO_CRx_MODE_CNF_IN_ANALOG_value);    // PC5 configured as alternate function
    gpio_pin_cfg(ADC14_GPIO, ADC14_PIN, GPIO_CRx_MODE_CNF_IN_ANALOG_value);    // PC4 configured as alternate function

    ADC1->CR2 = ADC_CR2_ADON;    // wakes up the ADC1 from Power Down mode
    ADC2->CR2 = ADC_CR2_ADON;    // wakes up the ADC2 from Power Down mode
    for(delay = 100000; delay; delay--);    // ADC power-up time - tstab
    ADC1->CR2 |= ADC_CR2_CAL;    // Calibration ADC1
    ADC2->CR2 |= ADC_CR2_CAL;    // Calibration ADC2
    while((ADC1->CR2 & ADC_CR2_CAL) || (ADC2->CR2 & ADC_CR2_CAL));    // wait to the end of calibration
    ADC1->CR2 |= ADC_CR2_CONT;    // Continuous Conversion
    ADC1->CR1 |= ADC_CR1_DUALMOD_1 | ADC_CR1_DUALMOD_2;    // Dualmode: Regular simultaneous mode only
    ADC1->CR2 |= ADC_CR2_EXTTRIG;    // Conversion on external event enabled
    ADC1->CR2 |= ADC_CR2_EXTSEL_0 | ADC_CR2_EXTSEL_2;    // TIM4_CC4 event set as trigger for start of conversion
    ADC1->CR2 |= ADC_CR2_DMA;    // Direct Memory access mode

    ADC1->SQR3 = ADC_SQR3_SQ1_0 | ADC_SQR3_SQ1_1 | ADC_SQR3_SQ1_2 | ADC_SQR3_SQ1_3;    // 1st conversion in regular sequence at 15 chanell
}

static void Encoder_init(void)
{
    // Timer 2

    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;    // turn on clock for Timer2

    gpio_pin_cfg(ENC_TI1_GPIO, ENC_TI1_PIN, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);    // PA0 configured as input floating
	gpio_pin_cfg(ENC_TI2_GPIO, ENC_TI2_PIN, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);    // PA1 configured as input floating
	gpio_pin_cfg(ENC_SW_GPIO, ENC_SW_PIN, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);    // PC12 configures as Pull Up/Down
	GPIOC->BSRR |= GPIO_BSRR_BS12;    // Pull Up


    TIM2->SMCR = TIM_SMCR_SMS_1; // counter is counting on TI1 edges only
    TIM2->CCMR1 = TIM_CCMR1_IC1F | TIM_CCMR1_IC2F; // Input capture filters
    TIM2->CCER = TIM_CCER_CC1P; // inverted: capture is done on a falling edge of IC1
    TIM2->ARR = ENC_TICKS; // quadrature encoder
    TIM2->DIER = TIM_DIER_UIE; // Update interrupt enabled
    TIM2->CR1 = TIM_CR1_CEN; // Counter enable, start counting!

    NVIC_ClearPendingIRQ(TIM2_IRQn);
    NVIC_EnableIRQ(TIM2_IRQn);

    // SW button on encoder
    AFIO->EXTICR[3] = AFIO_EXTICR4_EXTI12_PC; // source input for EXTIx external interrupt
    EXTI->IMR = EXTI_IMR_MR12;    // configure interrupt mask
    EXTI->FTSR =  EXTI_FTSR_TR12;    // setting trigerring by falling edge
    NVIC_EnableIRQ(EXTI15_10_IRQn);
}



#ifdef EXTERNAL_CLOCK
static void externalClockMode1(void)
{
    RCC->APB1ENR |= RCC_APB1ENR_TIM2EN;    // turn on clock for Timer2

    gpio_pin_cfg(TI2_GPIO, TI2_PIN, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value);    // set PA0 as input, in pull up/down mode
    GPIOA->BSRR = GPIO_BSRR_BS0;	// pull up PA0

    TIM2->CCER = TIM_CCER_CC2P;    // capture is done on a falling edge
    TIM2->SMCR = TIM_SMCR_SMS;    // Slave mode selection: External Clock Mode 1
    TIM2->SMCR |=  TIM_SMCR_TS_2;    // trigger selection: TI1 Edge Detector
    TIM2->ARR = 1;    // value of overload
    TIM2->DIER = TIM_DIER_UIE;    // Update interrupt enable
    TIM2->CR1 = TIM_CR1_CEN;    // Counter enable

    NVIC_EnableIRQ(TIM2_IRQn);    // TIM2 Update Interrupt enable in NVIC
}
#endif


#ifdef ADC_AVARAGE
static uint16_t ADC_filter(uint16_t *array, uint16_t size)
{
	uint16_t i;
	uint16_t avg = 0;

	for(i=0; i<size; i++)
	{
		avg = avg + *array;
		array++;
	}
	avg = avg/size;
	return avg;
}
#endif

__attribute__ (( interrupt )) void SysTick_Handler(void)
{
	LED1_BB ^= 1;
}

__attribute__((interrupt)) void TIM2_IRQHandler(void)
{
	if (TIM2->SR & TIM_SR_UIF)
	{
		TIM2->SR = ~TIM_SR_UIF;
		enc_cnt++;
		LED2_BB ^= 1;
	}
}

__attribute__ (( interrupt )) void EXTI15_10_IRQHandler(void)
{

	if (EXTI->PR & EXTI_PR_PR12)
	{
		EXTI->PR = EXTI_PR_PR12;
	}
}

