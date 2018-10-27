/*
+=============================================================================+
| includes
+=============================================================================+
*/
#include "measure.h"
#include <stdint.h>
#include "../config.h"
#include "../hdr/hdr_gpio.h"
#include "../gpio/gpio.h"
#include "system.h"


/*
+=============================================================================+
| defines
+=============================================================================+
*/
#define ADC_BUFFER_SIZE 10
#define MEASURE_REFRESH_TIME 100

/*
+=============================================================================+
| module variables
+=============================================================================+
*/
static uint16_t adcBuff[ADC_BUFFER_SIZE];
static BasicTimer timer;
static measurmentData measures;

/*
+=============================================================================+
| local functions' declarations
+=============================================================================+
*/
static uint16_t measure_filter(uint16_t *array, uint16_t size);
static void measure_initAdc(void);
static void measure_initDMA(void);
static void measure_initTrigger(void);
static void measure_task(void);
static void measure_timer(void);

/*
+=============================================================================+
| global functions
+=============================================================================+
*/
void measure_init(void)
{
	measure_initAdc();
	measure_initDMA();
	measure_initTrigger();
	system_createTask(measure_task, measure_timer, 1, "measure", 5);
	TIMER_START(timer, MEASURE_REFRESH_TIME);
}

uint16_t measure_getVoltage(void)
{
	return measures.potentiometer;
}
/*
+=============================================================================+
| local functions
+=============================================================================+
*/
static void measure_task(void)
{
	if (timer.flag) {
		measures.potentiometer = measure_filter(adcBuff, ADC_BUFFER_SIZE);
		TIMER_START(timer, MEASURE_REFRESH_TIME);
	}

}


static uint16_t measure_filter(uint16_t *array, uint16_t size)
{
	uint16_t i;
	uint32_t avg = 0;

	for(i=0; i<size; i++)
	{
		avg = avg + *array;
		array++;
	}
	avg = avg/size;
	return (uint16_t)avg;
}

static void measure_initAdc(void)
{
    uint32_t delay;

    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN;    // turn on clock for ADC1

    gpio_pin_cfg(ADC15_GPIO, ADC15_PIN, GPIO_CRx_MODE_CNF_IN_ANALOG_value);    // PC5 configured as alternate function

    ADC1->CR2 = ADC_CR2_ADON;    // wakes up the ADC1 from Power Down mode
    for (delay = 100000; delay; delay--);    // ADC power-up time - tstab
    ADC1->CR2 |= ADC_CR2_CAL;    // Calibration ADC1
    while (ADC1->CR2 & ADC_CR2_CAL);    // wait to the end of calibration
    ADC1->CR2 |= ADC_CR2_CONT;    // Continuous Conversion
    ADC1->CR2 |= ADC_CR2_EXTTRIG;    // Conversion on external event enabled
    ADC1->CR2 |= ADC_CR2_EXTSEL_0 | ADC_CR2_EXTSEL_2;    // TIM4_CC4 event set as trigger for start of conversion
    ADC1->CR2 |= ADC_CR2_DMA;    // Direct Memory access mode

    ADC1->SQR3 = ADC_SQR3_SQ1_0 | ADC_SQR3_SQ1_1 | ADC_SQR3_SQ1_2 | ADC_SQR3_SQ1_3;    // 1st conversion in regular sequence at 15 chanell
}

static void measure_initDMA(void)
{
	if (!(RCC->AHBENR & RCC_AHBENR_DMA1EN)) {
	    RCC->AHBENR |= RCC_AHBENR_DMA1EN;    // DMA1 clock enable
	}

    DMA1_Channel1->CPAR = (uint32_t)(&ADC1->DR);
    DMA1_Channel1->CMAR = (uint32_t)(&adcBuff);
    DMA1_Channel1->CNDTR = ADC_BUFFER_SIZE;

    DMA1_Channel1->CCR = DMA_CCR1_PL_1;    // Channel priority level: High
    DMA1_Channel1->CCR |= DMA_CCR1_CIRC;    // Circular mode enabled
    DMA1_Channel1->CCR |= DMA_CCR1_MINC;    // Memory increment mode enabled
    DMA1_Channel1->CCR |= DMA_CCR1_PSIZE_0;    // Peripheral size - 16bit
    DMA1_Channel1->CCR |= DMA_CCR1_MSIZE_0;    // Memory size - 16bit
    DMA1_Channel1->CCR |= DMA_CCR1_EN;  // Channel enabled

    NVIC_EnableIRQ(DMA1_Channel1_IRQn);
}

static void measure_initTrigger(void)
{
	// Timer4
    RCC->APB1ENR |= RCC_APB1ENR_TIM4EN;    // turn on clock for Timer4

	TIM4->CCMR2 = TIM_CCMR2_OC4M_2 | TIM_CCMR2_OC4M_1;    // set PWM mode 1
	TIM4->CCMR2 |= TIM_CCMR2_OC4PE;    // Output compare 4 preload enable

	TIM4->CCER |= TIM_CCER_CC4E;    // CC4 channel configured as output: On

	TIM4->PSC = 200-1;    // value of prescaler
	TIM4->ARR = 40-1;    // value of overload
	TIM4->CCR4 = (TIM4->ARR / 2);    // value to be compared to the counter CNT, and signaled on OC4 output

	TIM4->EGR = TIM_EGR_UG;    // Reinitialize the counter and generates an update of the registers
	TIM4->CR1 |= TIM_CR1_ARPE;    // Auto-reload preload enable

	TIM4->CR1 |= TIM_CR1_CEN;    // Counter enable, start counting!
}


/*
+=============================================================================+
| Timers
+=============================================================================+
*/
static void measure_timer(void)
{
    if (timer.cnt != 0) {
        if (--timer.cnt == 0) {
            timer.flag = true;
        }
    }
}

/*
+=============================================================================+
| ISRs
+=============================================================================+
*/



/******************************************************************************
* END OF FILE
******************************************************************************/
