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
#define BAUD_RATE 460800

#define UART_RX_BUF_SIZE 64
#define UART_RX_CIRC_BUF_SIZE 16
#define UART_RX_BUF_MASK (UART_RX_BUF_SIZE - 1)

#define UART_TX_BUF_SIZE 64
#define UART_TX_BUF_MASK (UART_TX_BUF_SIZE - 1)


/*
+=============================================================================+
| Public functions
+=============================================================================*
*/
void UART_init(uint32_t baudrate);
int UART_getChr(void);
void UART_putStr(char *s);
void UART_putInt(int value, int radix);
char *UART_getStr(char *buf);
void UART_RX_STR_EVENT(void);
void register_uart_str_rx_event_callback(void (*callback)(char *pBuf));





#endif /* UART_UART_H_ */
