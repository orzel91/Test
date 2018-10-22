/*
 * spi.c
 *
 *  Created on: 7 sie 2018
 *      Author: jarek
 */


#include <oled1306.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../inc/stm32f10x.h"
#include "../config.h"
#include "../hdr/hdr_rcc.h"
#include "../hdr/hdr_gpio.h"
#include "../gpio/gpio.h"
#include "../uart/uart.h"
#include "spi.h"


/*******************  Defines  *******************/



/*******************  Declarations  *******************/

typedef enum {
	COMMAND = 1,
	DATA = 2
} SPI_MessageTypes;

typedef enum {
	IDLE = 0,
	SENDING = 1
} SPI_DmaState;

typedef struct {
	uint8_t* adress;
	uint16_t cnt;
	SPI_MessageTypes type;
} SPI_DataFormat;

typedef struct {
	volatile uint16_t head;
	volatile uint16_t tail;
	SPI_DataFormat buffer[SPI_MESSAGE_QUEQUE_SIZE];
	volatile SPI_DmaState dmaState;
} SPI_DataPointer;

typedef struct {
	volatile uint16_t head;
	volatile uint16_t tail;
	uint8_t buffer[SPI_COMMAND_SIZE];
} SPI_CmdPointer;


/*******************  Global variables  *******************/
static SPI_CmdPointer command;
static SPI_DataPointer queue;


/*******************  Local variables  *******************/



/*******************  Private function declarations  *******************/



/*******************  Public function definitions  *******************/

// SPI init function
void SPI_init(void)
{
	RCC->APB2ENR |= RCC_APB2ENR_SPI1EN; // SPI_1 clock enable

    // Set GPIOs for SPI_1
	gpio_pin_cfg(SPI1_SCK_GPIO, SPI1_SCK_PIN, GPIO_CRx_MODE_CNF_ALT_PP_10M_value); // Set SCK Pin
	gpio_pin_cfg(SPI1_MOSI_GPIO, SPI1_MOSI_PIN, GPIO_CRx_MODE_CNF_ALT_PP_10M_value); // Set MOSI Pin
//	gpio_pin_cfg(SPI1_MISO_GPIO, SPI1_MISO_PIN, GPIO_CRx_MODE_CNF_IN_PULL_U_D_value); // Set MISO Pin
	gpio_pin_cfg(SPI1_NSS_GPIO, SPI1_NSS_PIN, GPIO_CRx_MODE_CNF_OUT_PP_10M_value); // Set NSS Pin

	SPI1_NSS_BB = 1; // set CS pin to state: deselect

	// Set configuration registers
	SPI1->CR1 = SPI_CR1_SSM; // Software slave management
	SPI1->CR1 |= SPI_CR1_SSI; // Internal slave select

	SPI1->CR1 |= SPI_CR1_BR_0| SPI_CR1_BR_2; // Baud_Rate = Fpclk/64
	SPI1->CR1 |= SPI_CR1_CPOL; // Clock Polarity = 1, SCK has got high state in idle
	SPI1->CR1 |= SPI_CR1_CPHA; // Clock Phase = 1, rising edge
	SPI1->CR1 |= SPI_CR1_MSTR; // Master configuration

	SPI1->CR2 |= SPI_CR2_TXDMAEN; // Tx Buffer DMA Enable

	// Configure DMA chanell_3 - Transmit
	DMA1_Channel3->CCR = DMA_CCR3_PL_1;    // Channel priority level: High
    DMA1_Channel3->CCR |= DMA_CCR3_MINC;    // Memory increment mode enabled
    DMA1_Channel3->CCR |= DMA_CCR3_TCIE;    // Transfer complete interrupt enable
    DMA1_Channel3->CCR |= DMA_CCR3_DIR;    // Data transfer direction: Read from memory

	NVIC_EnableIRQ(DMA1_Channel3_IRQn);

	SPI1->CR1 |= SPI_CR1_SPE; // SPI Enable

}

// Send data
void SPI_sendData(uint8_t* src, uint16_t cnt)
{
	uint16_t tmpHead = 0;

	tmpHead = (queue.head + 1) & SPI_MESSAGE_QUEQUE_MASK;

	if (tmpHead == queue.tail) {
		queue.head = queue.tail; // loss whole buffer of messages
	} else {
		queue.head = tmpHead;

		queue.buffer[queue.head].adress = src;
		queue.buffer[queue.head].cnt = cnt;
		queue.buffer[queue.head].type = DATA;
	}
	SPI_checkDmaStatus();
}

// Send command
void SPI_sendCmd(uint8_t cmd)
{
	uint16_t tmpHead = 0;

	// Put command to circular buffer
	tmpHead = (command.head + 1) & SPI_COMMAND_MASK;

	if (tmpHead == command.tail) {
		command.head = command.tail; // loss whole buffer of commands
	} else {
		command.head = tmpHead;
		command.buffer[command.head] = cmd;
	}

	// Add command to massage queue
	tmpHead = (queue.head + 1) & SPI_MESSAGE_QUEQUE_MASK;

	if (tmpHead == queue.tail) {
		queue.head = queue.tail; // loss whole buffer of messages
	} else {
		queue.head = tmpHead;

		queue.buffer[queue.head].adress = &command.buffer[command.head];
		queue.buffer[queue.head].cnt = 1;
		queue.buffer[queue.head].type = COMMAND;
	}
	SPI_checkDmaStatus();
}

void SPI_checkDmaStatus(void)
{
	uint16_t tmpTail = 0;

	// check if DMA is not sending
	if (!queue.dmaState ) {

		// check if there is some messsage in buffer
		if (queue.tail != queue.head) {
			tmpTail = (queue.tail + 1) & SPI_MESSAGE_QUEQUE_MASK;

			// check if the next message is the same type, and if the message was send - D/C pin set
			if ( queue.buffer[queue.tail].type != queue.buffer[tmpTail].type
					&& (SPI1->SR & SPI_SR_BSY) ) {
				return;
			}

			queue.tail = tmpTail;

			DMA1_Channel3->CPAR = (uint32_t)(&SPI1->DR); // destination
			DMA1_Channel3->CMAR = (uint32_t)(queue.buffer[tmpTail].adress); // src = adress of message
			DMA1_Channel3->CNDTR = queue.buffer[tmpTail].cnt; // count

			queue.dmaState = SENDING;
			SPI1_NSS_BB = 0;

			if (queue.buffer[tmpTail].type == COMMAND) {
				OLED_DC_BB = 0;
			}

			if (queue.buffer[tmpTail].type == DATA) {
				OLED_DC_BB = 1;
			}

			DMA1_Channel3->CCR |= DMA_CCR3_EN; // Channel enabled

		} else {
			if (!(SPI1->SR & SPI_SR_BSY)) {
				SPI1_NSS_BB = 1;
			}
		}
	}
}


/*******************  Private function definitions  *******************/



/*******************  ISR  *******************/

void DMA1_Channel3_IRQHandler(void)
{
	// Transfer Complete ISR
	if (DMA1->ISR & DMA_ISR_TCIF3) {
		DMA1->IFCR |= DMA_IFCR_CTCIF3; // clear transfer complete flag

		DMA1_Channel3->CCR &= ~DMA_CCR3_EN;

		if (queue.buffer[queue.tail].type == COMMAND) {
			command.tail = (command.tail +1) & SPI_COMMAND_MASK;
		}
		queue.dmaState = IDLE;

		SPI_checkDmaStatus();
	}
}

