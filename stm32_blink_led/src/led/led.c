/*
+=============================================================================+
| includes
+=============================================================================+
*/
#include "led.h"
#include "../gpio/gpio.h"
#include "../config.h"
#include "../hdr/hdr_gpio.h"


/*
+=============================================================================+
| defines
+=============================================================================+
*/


/*
+=============================================================================+
| module variables
+=============================================================================+
*/
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

/*
+=============================================================================+
| local functions' declarations
+=============================================================================+
*/
static void led_pwmInit(void);
static void led_initDMA(void);

/*
+=============================================================================+
| global functions
+=============================================================================+
*/
void led_init(void)
{
    gpio_pin_cfg(LED1_GPIO, LED1_PIN, GPIO_CRx_MODE_CNF_OUT_PP_2M_value);
	gpio_pin_cfg(LED2_GPIO, LED2_PIN, GPIO_CRx_MODE_CNF_OUT_PP_2M_value);
    gpio_pin_cfg(LED3_GPIO, LED3_PIN, GPIO_CRx_MODE_CNF_OUT_PP_2M_value);

    LED1_BB = 0; // turn off LED2 by 3V3 (which is activated by 0V)
    LED2_BB = 0; // turn off LED2 by 3V3 (which is activated by 0V)
    LED3_BB = 0; // turn off LED2 by 3V3 (which is activated by 0V)

    led_pwmInit();
    led_initDMA();
}


/*
+=============================================================================+
| local functions
+=============================================================================+
*/
static void led_pwmInit(void)
{
    gpio_pin_cfg(PWM1_GPIO, PWM1_PIN, GPIO_CRx_MODE_CNF_ALT_PP_2M_value);    // PA8 configured as alternate function

    RCC->APB2ENR |= RCC_APB2ENR_TIM1EN;    // turn on clock for Timer1

    // Timer1

    TIM1->CCMR1 = TIM_CCMR1_OC1M_2 | TIM_CCMR1_OC1M_1;    // set PWM mode 1
    TIM1->CCMR1 |= TIM_CCMR1_OC1PE;    // Preload register on TIMx_CCR1 enabled

    TIM1->CCER |= TIM_CCER_CC1E;    // CC1 channel configured as output: On
    TIM1->BDTR = TIM_BDTR_MOE;    // Main output enable

    // PWM freq = CPU_FREQ / ( (PSC+1)*(ARR+1) )
    TIM1->PSC = 100;    // value of prescaler
    TIM1->ARR = 1600;    // value of overload
    TIM1->CCR1 = TIM1->ARR;    // value to be compared to the counter CNT, and signaled on OC1 output
    TIM1->EGR = TIM_EGR_UG;    // Reinitialize the counter and generates an update of the registers
    TIM1->CR1 |= TIM_CR1_ARPE;    // Auto-reload preload enable
    TIM1->DIER = TIM_DIER_CC1DE;    // Capture/Compare 1 DMA request enable
    TIM1->CR1 |= TIM_CR1_CEN;    // Counter enable, start counting!
}

static void led_initDMA(void)
{
	if (!(RCC->AHBENR & RCC_AHBENR_DMA1EN)) {
	    RCC->AHBENR |= RCC_AHBENR_DMA1EN;    // DMA1 clock enable
	}

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
}

/*
+=============================================================================+
| ISRs
+=============================================================================+
*/



/******************************************************************************
* END OF FILE
******************************************************************************/
