/*
 * uart.c
 *
 *  Created on: 12 mar 2018
 *      Author: jarek
 */

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "inc/stm32f10x.h"
#include "../config.h"
#include "hdr/hdr_rcc.h"
#include "hdr/hdr_gpio.h"
#include "gpio/gpio.h"
#include "uart.h"


typedef enum {
	UART_TX_IDLE = 0,
	UART_TX_SENDING = 1
}UART_TxStates;

typedef struct {
	uint16_t head;
	uint16_t tail;
	uint16_t counter;
	UART_TxStates state;
	char buffer[UART_TX_BUF_SIZE];
}UART_TxModule;

typedef struct {
	uint16_t head;
	uint16_t tail;
	uint16_t counter;
	uint16_t ascii_line;
	char buffer[UART_RX_BUF_SIZE];
	char bufferCircular[UART_RX_CIRC_BUF_SIZE];
}UART_RxModule;


// Global variables
static volatile UART_TxModule Tx;
static volatile UART_RxModule Rx;

// Private function declarations
static void UART_rxMemCpy(uint16_t cnt, uint16_t circIndex);
static void UART_putChr(char data);

// ptr to callback function for event UART_RX_STR_EVENT()
static void (*uart_rx_str_event_callback)(char * pBuf);

// UART init function
void UART_init(uint32_t baudrate)
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


	Rx.head = UART_TX_BUF_SIZE - 1;
	Rx.tail = UART_TX_BUF_SIZE - 1;
}

// function to get string from circle buffer to new buffer
char* UART_getStr(char* buf)
{
	int8_t c;
	char* wsk = buf;
	if( Rx.ascii_line )
	{
		// take signs until CR occurance or empty circle buffer
		while( (c = UART_getChr()) )
		{
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

// register function of callback function for event UART_RX_STR_EVENT()
void register_uart_str_rx_event_callback(void (*callback)(char * pBuf))
{
	uart_rx_str_event_callback = callback;
}

// Event for receive string from circle buffer
void UART_RX_STR_EVENT(void)
{
	char tempBuff[100];

	if( Rx.ascii_line ) {

		memset(tempBuff, 0 ,sizeof(tempBuff));
		UART_getStr(tempBuff);
		UART_putStr(tempBuff);

		memset(tempBuff, 0 ,sizeof(tempBuff));

/*		if( uart_rx_str_event_callback ) {
			UART_getStr( tBuffer );
			(*uart_rx_str_event_callback)( tBuffer );
		} else {
			Rx.head = Rx.tail;
		}*/
	}
}

// function to sending one char
static void UART_putChr(char data)
{
	uint16_t tmp_head;

	tmp_head  = (Tx.head + 1) & UART_TX_BUF_MASK;

	// wait until buffer will have free place
	while ( tmp_head == Tx.tail ){}

	Tx.buffer[tmp_head] = data;
	Tx.head = tmp_head;
}

// function to sending string
void UART_putStr(char *s)
{
	register char c;
	uint16_t tempTail = 0;

	while ((c = *s++)) UART_putChr(c);

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
void UART_putInt(int value, int radix)
{
	char string[17];
	itoa(value, string, radix);
	UART_putStr(string);
}

// function to collect one sign from circle buffer
int UART_getChr(void)
{
    // check if index are equal
    if ( Rx.head == Rx.tail ) return -1;
    Rx.tail = (Rx.tail + 1) & UART_RX_BUF_MASK;
    return Rx.buffer[Rx.tail];
}

// function to get data from Rx
static void UART_rxMemCpy(uint16_t cnt, uint16_t circIndex)
{
    register uint16_t tmp_head = 0;
    register uint16_t i = circIndex = 0;

	while (cnt)
	{
	    tmp_head = ( Rx.head + 1) & UART_RX_BUF_MASK;

	    if ( tmp_head == Rx.tail ) {
	    	Rx.head = Rx.tail; // waste whole buffer of data, too many data
	    } else {
	    	Rx.head = tmp_head;

	    	switch (Rx.bufferCircular[i])
	    	{
				case 0: /* no break */
				case 10: break;
				case 13: Rx.ascii_line++;
				/* no break */
				default:
					Rx.buffer[tmp_head] = Rx.bufferCircular[i++];
					break;
			}
		}
	    cnt--;
	}
}

__attribute__ ((interrupt)) void USART2_IRQHandler(void)
{
	if (USART2->SR & USART_SR_IDLE)
	{
        volatile uint16_t tmp;
        tmp = USART2->SR; // Clear IDLE flag by reading SR and DR
        tmp = USART2->DR;
        (void)tmp;
		UART_rxMemCpy( (UART_RX_CIRC_BUF_SIZE - DMA1_Channel6->CNDTR), 0 ); // copy packets from RxCircBuf to RxBuf
	}
}

__attribute__((interrupt)) void DMA1_Channel6_IRQHandler(void)
{
	if ( DMA1->ISR & DMA_ISR_TCIF6 ) {
		DMA1->IFCR |= DMA_IFCR_CTCIF6; // clear transfer complete flag
		UART_rxMemCpy( (UART_RX_CIRC_BUF_SIZE - DMA1_Channel6->CNDTR), 0 ); // copy packets from RxCircBuf to RxBuf

		DMA1->IFCR |= DMA_IFCR_CGIF6; // Clears the GIF, TEIF, HTIF and TCIF flags in the DMA_ISR register
	    DMA1_Channel6->CPAR = (uint32_t)(&USART2->DR);
	    DMA1_Channel6->CMAR = (uint32_t)(&Rx.bufferCircular);
	    DMA1_Channel6->CNDTR = UART_RX_CIRC_BUF_SIZE;
	    DMA1_Channel6->CCR |= DMA_CCR6_EN; // Channel enabled
	}
}

__attribute__((interrupt)) void DMA1_Channel7_IRQHandler(void)
{
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
			DMA1->IFCR |= DMA_IFCR_CGIF7; // Clears the GIF, TEIF, HTIF and TCIF flags in the DMA_ISR register
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

