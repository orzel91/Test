/*
 * uart.h
 *
 *  Created on: 12 mar 2018
 *      Author: jarek
 */

#ifndef UART_UART_H_
#define UART_UART_H_

#include <stdint.h>

/*
+=============================================================================+
| Defines
+=============================================================================*
*/
#define BAUD_RATE 921600

#define UART_RX_BUF_SIZE 64
#define UART_RX_CIRC_BUF_SIZE 16
#define UART_RX_BUF_MASK (UART_RX_BUF_SIZE - 1)

#define UART_TX_BUF_SIZE 512
#define UART_TX_BUF_MASK (UART_TX_BUF_SIZE - 1)


/*
+=============================================================================+
| Public functions
+=============================================================================*
*/
void uart_init(void);
int uart_getChr(void);
void uart_putStr(char *s);
void uart_putInt(int value, int radix);
char *uart_getStr(char *buf);




#endif /* UART_UART_H_ */
