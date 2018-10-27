/*
 * uart.c
 *
 *  Created on: 12 mar 2018
 *      Author: jarek
 */

/*
+=============================================================================+
| includes
+=============================================================================+
*/
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/stm32f10x.h"
#include "../config.h"
#include "../hdr/hdr_rcc.h"
#include "../hdr/hdr_gpio.h"
#include "../gpio/gpio.h"
#include "uart.h"
#include "system.h"
#include "at_commands.h"

/*
+=============================================================================+
| defines
+=============================================================================+
*/


/*
+=============================================================================+
| typedefs
+=============================================================================+
*/
typedef enum {
	UART_TX_IDLE = 0,
	UART_TX_SENDING = 1
}Uart_txStates;

typedef struct {
	uint16_t head;
	uint16_t tail;
	uint16_t counter;
	Uart_txStates state;
	char buffer[UART_TX_BUF_SIZE];
}Uart_txModule;

typedef struct {
	uint16_t head;
	uint16_t tail;
	uint16_t counter;
	uint16_t ascii_line;
	char buffer[UART_RX_BUF_SIZE];
	char bufferCircular[UART_RX_CIRC_BUF_SIZE];
}Uart_rxModule;


/*
+=============================================================================+
| module variables
+=============================================================================+
*/
static volatile Uart_txModule Tx;
static volatile Uart_rxModule Rx;


/*
+=============================================================================+
| local functions' declarations
+=============================================================================+
*/
static void uart_task(void);
static void uart_timer(void);
static void uart_rxMemCpy(uint16_t cnt, uint16_t circIndex);
static void uart_putChr(char data);
static void uart_initHardware(uint32_t baudrate);
static void uart_reciveStringEvent(void);

/*
+=============================================================================+
| global functions
+=============================================================================+
*/

// UART init function
void uart_init(void)
{
	uart_initHardware(BAUD_RATE);

    system_createTask(uart_task, uart_timer, 1, "uart", 5);
}

// function to get string from circle buffer to new buffer
char* uart_getStr(char* buf)
{
	int8_t c;
	char* wsk = buf;
	if( Rx.ascii_line )
	{
		// take signs until CR occurance or empty circle buffer
		while( (c = uart_getChr()) )
		{
			// CR = 13, LF = 10
			if( 13 == c || c < 0 ) {
				break;
			}
			*buf++ = c;
		}
		*buf=0;
		Rx.ascii_line--;
	}
	return wsk;
}

// function to collect one sign from circle buffer
int uart_getChr(void)
{
    // check if index are equal
    if ( Rx.head == Rx.tail ) return -1;
    Rx.tail = (Rx.tail + 1) & UART_RX_BUF_MASK;
    return Rx.buffer[Rx.tail];
}


// function to sending string
void uart_putStr(char *s)
{
	register char c;
	uint16_t tempTail = 0;

	while ((c = *s++)) uart_putChr(c);

	if (Tx.state == UART_TX_IDLE) {

		DMA1_Channel7->CCR &= ~DMA_CCR7_EN;    // Channel disabled
		DMA1->IFCR |= DMA_IFCR_CGIF7;    // Clears the GIF, TEIF, HTIF and TCIF flags in the DMA_ISR register
		DMA1_Channel7->CPAR = (uint32_t)(&USART2->DR);

		tempTail = (Tx.tail+1) & UART_TX_BUF_MASK;

		if (Tx.tail < UART_TX_BUF_MASK) {
			// check if head has overfloved
			if (Tx.head > Tx.tail) {
				DMA1_Channel7->CMAR = (uint32_t)(&Tx.buffer[tempTail]);
				DMA1_Channel7->CNDTR = Tx.head - Tx.tail;
				Tx.counter = DMA1_Channel7->CNDTR;
			} else {
				DMA1_Channel7->CMAR = (uint32_t)(&Tx.buffer[tempTail]);
				DMA1_Channel7->CNDTR = UART_TX_BUF_MASK - Tx.tail;
				Tx.counter = DMA1_Channel7->CNDTR;
			}
		} else {
			DMA1_Channel7->CMAR = (uint32_t)(&Tx.buffer[tempTail]);
			Tx.counter = (Tx.head - tempTail) + 1;
			DMA1_Channel7->CNDTR = Tx.counter;
		}

		Tx.state = UART_TX_SENDING;
		DMA1_Channel7->CCR |= DMA_CCR7_EN;    // Channel enabled
	}
}

// function to sending number
void uart_putInt(int value, int radix)
{
	char string[17];
	itoa(value, string, radix);
	uart_putStr(string);
}


/*
+=============================================================================+
| local functions
+=============================================================================+
*/

static void uart_task(void)
{
	uart_reciveStringEvent();
}


// function to sending one char
static void uart_putChr(char data)
{
	uint16_t tmp_head;

	tmp_head  = (Tx.head + 1) & UART_TX_BUF_MASK;

	// wait until buffer will have free place
	while ( tmp_head == Tx.tail ){}

	Tx.buffer[tmp_head] = data;
	Tx.head = tmp_head;
}

// function to get data from Rx
static void uart_rxMemCpy(uint16_t cnt, uint16_t circIndex)
{
    register uint16_t tmp_head = 0;
    register uint16_t i = circIndex;

	while (cnt)
	{
	    tmp_head = (Rx.head + 1) & UART_RX_BUF_MASK;

	    if ( tmp_head == Rx.tail ) {
	    	Rx.head = Rx.tail; // waste whole buffer of data, too many data
	    } else {
	    	Rx.head = tmp_head;

	    	switch (Rx.bufferCircular[i])
	    	{
				case 0:
					/* no break */

				case 10: // LF
					Rx.head--;
					break;

				case 13: // CR
					Rx.ascii_line++;
					Rx.buffer[tmp_head] = Rx.bufferCircular[i++];
					break;

				default:
					Rx.buffer[tmp_head] = Rx.bufferCircular[i++];
					break;
			}
		}
	    cnt--;
	}
}


static void uart_initHardware(uint32_t baudrate)
{
	// Turn on USART2 and DMA clocks
	RCC->APB1ENR |= RCC_APB1ENR_USART2EN; // USART 2 clock enable
    RCC->AHBENR |= RCC_AHBENR_DMA1EN;    // DMA1 clock enable

    // Set GPIOs fot USART
	gpio_pin_cfg(UART2_TX_GPIO, UART2_TX_PIN, GPIO_CRx_MODE_CNF_ALT_PP_2M_value); // Set Tx Pin
	gpio_pin_cfg(UART2_RX_GPIO, UART2_RX_PIN, GPIO_CRx_MODE_CNF_IN_FLOATING_value); // Set Rx Pin

	// Configure DMA chanell_6 - Rx
    DMA1_Channel6->CPAR = (uint32_t)(&USART2->DR);
    DMA1_Channel6->CMAR = (uint32_t)(&Rx.bufferCircular);
    DMA1_Channel6->CNDTR = UART_RX_CIRC_BUF_SIZE;

    DMA1_Channel6->CCR = DMA_CCR6_PL_1;    // Channel priority level: High
    DMA1_Channel6->CCR |= DMA_CCR6_MINC;    // Memory increment mode enabled
    DMA1_Channel6->CCR |= DMA_CCR6_TCIE;    // Transfer complete interrupt enable
    DMA1_Channel6->CCR |= DMA_CCR6_HTIE;    // Half Transfer interrupt enable
    DMA1_Channel6->CCR |= DMA_CCR6_CIRC;    // Turn on Circular mode
    DMA1_Channel6->CCR |= DMA_CCR6_EN;    // Channel enabled

	// Configure DMA chanell_7 - Tx
    DMA1_Channel7->CPAR = (uint32_t)(&USART2->DR);
    DMA1_Channel7->CMAR = (uint32_t)(&Tx.buffer);

    DMA1_Channel7->CCR = DMA_CCR7_PL_1;    // Channel priority level: High
    DMA1_Channel7->CCR |= DMA_CCR7_DIR;    // Data transfer direction: Read from memory
    DMA1_Channel7->CCR |= DMA_CCR7_MINC;    // Memory increment mode enabled
    DMA1_Channel7->CCR |= DMA_CCR7_TCIE;    // Transfer complete interrupt enable


    // Configure USART
	USART2->BRR = FREQUENCY / baudrate;
	USART2->CR1 = USART_CR1_RE; // Receiver Enable
	USART2->CR1 |= USART_CR1_TE; // Transmitter Enable
	USART2->CR1 |= USART_CR1_IDLEIE; // IDLE Interrupt Enable
	USART2->CR3 |= USART_CR3_DMAR; // DMA Enable Receiver
	USART2->CR3 |= USART_CR3_DMAT; // DMA Enable Transmitter
	USART2->CR1 |= USART_CR1_UE; // USART Enable

	// Enable interrupts in NVIC
	NVIC_EnableIRQ(USART2_IRQn);
	NVIC_EnableIRQ(DMA1_Channel6_IRQn);
	NVIC_EnableIRQ(DMA1_Channel7_IRQn);
}

// Event for receive string from circle buffer
static void uart_reciveStringEvent(void)
{
	char tempBuff[64];

	if( Rx.ascii_line ) {
		memset(tempBuff, 0 ,sizeof(tempBuff));
		uart_getStr(tempBuff);
		parse_uart_data(tempBuff);
	}
}


/*
+=============================================================================+
| timers
+=============================================================================+
*/
static void uart_timer(void)
{

}


/*
+=============================================================================+
| ISRs
+=============================================================================+
*/
void USART2_IRQHandler(void)
{
	// IDLE line ISR
	if (USART2->SR & USART_SR_IDLE)
	{
        volatile uint16_t tmp;

        // Clear IDLE flag by reading SR and DR
        tmp = USART2->SR;
        tmp = USART2->DR;

        // check if IDLE was detected after Half Transfer Complete
        if (DMA1_Channel6->CNDTR < ((UART_RX_CIRC_BUF_SIZE/2)+1)) {
            tmp = (UART_RX_CIRC_BUF_SIZE/2) - DMA1_Channel6->CNDTR;
    		uart_rxMemCpy(tmp, (UART_RX_CIRC_BUF_SIZE/2)); // copy packets from RxCircBuf to RxBuf
        } else {
            tmp = UART_RX_CIRC_BUF_SIZE - DMA1_Channel6->CNDTR;
    		uart_rxMemCpy(tmp, 0); // copy packets from RxCircBuf to RxBuf
		}

	    DMA1_Channel6->CCR &= ~DMA_CCR6_EN; // Channel disabled
		DMA1->IFCR |= DMA_IFCR_CGIF6; // Clears the GIF, TEIF, HTIF and TCIF flags in the DMA_ISR register
	    DMA1_Channel6->CPAR = (uint32_t)(&USART2->DR);
	    DMA1_Channel6->CMAR = (uint32_t)(&Rx.bufferCircular);
	    DMA1_Channel6->CNDTR = UART_RX_CIRC_BUF_SIZE;
	    DMA1_Channel6->CCR |= DMA_CCR6_EN; // Channel enabled
	}
}

void DMA1_Channel6_IRQHandler(void)
{
	// Half Transfer ISR
	if (DMA1->ISR & DMA_ISR_HTIF6) {
		DMA1->IFCR |= DMA_ISR_HTIF6; // clear half transfer flag
		uart_rxMemCpy((UART_RX_CIRC_BUF_SIZE/2), 0); // copy packets from RxCircBuf to RxBuf
	}

	// Transfer Complete ISR
	if (DMA1->ISR & DMA_ISR_TCIF6) {
		DMA1->IFCR |= DMA_IFCR_CTCIF6; // clear transfer complete flag
		uart_rxMemCpy((UART_RX_CIRC_BUF_SIZE/2), (UART_RX_CIRC_BUF_SIZE/2)); // copy packets from RxCircBuf to RxBuf
	}
}

void DMA1_Channel7_IRQHandler(void)
{
	// Transfer Complete ISR
	if (DMA1->ISR & DMA_ISR_TCIF7)
	{
		DMA1->IFCR |= DMA_IFCR_CTCIF7; // clear transfer complete flag

		Tx.tail = (Tx.tail + Tx.counter) & UART_TX_BUF_MASK;
		Tx.counter = 0;

		if(Tx.tail == Tx.head) {
			Tx.state = UART_TX_IDLE;
		} else {
			uint16_t tempTail = 0;
			DMA1_Channel7->CCR &= ~DMA_CCR7_EN; // Channel disabled
			DMA1->IFCR |= DMA_IFCR_CTCIF7; // Channel 7 Transfer Complete clear
			DMA1_Channel7->CPAR = (uint32_t)(&USART2->DR);
			tempTail = (Tx.tail + 1) & UART_TX_BUF_MASK;

			if (Tx.tail < UART_TX_BUF_MASK) {
				// check if head has overfloved
				if (Tx.head > Tx.tail) {
					DMA1_Channel7->CMAR = (uint32_t)(&Tx.buffer[tempTail]);
					DMA1_Channel7->CNDTR = Tx.head - Tx.tail;
					Tx.counter = DMA1_Channel7->CNDTR;
				} else {
					DMA1_Channel7->CMAR = (uint32_t)(&Tx.buffer[tempTail]);
					DMA1_Channel7->CNDTR = UART_TX_BUF_MASK - Tx.tail;
					Tx.counter = DMA1_Channel7->CNDTR;
				}
			} else {
				DMA1_Channel7->CMAR = (uint32_t)(&Tx.buffer[tempTail]);
				Tx.counter = (Tx.head - tempTail) + 1;
				DMA1_Channel7->CNDTR = Tx.counter;
			}

			Tx.state = UART_TX_SENDING;
			DMA1_Channel7->CCR |= DMA_CCR7_EN; // Channel enabled
		}
	}
}


/******************************************************************************
* END OF FILE
******************************************************************************/

