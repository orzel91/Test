/*
 * AT_commands.c
 *
 *  Created on: 23 maj 2018
 *      Author: jarek
 */

#include "AT_commands.h"
#include <string.h>
#include <strings.h>
#include "uart.h"
#include "config.h"


#define AT_CNT 1

// Declarations
typedef struct {
	char* name;
	int16_t (*at_service)(uint16_t inout, char* params);
}ATcommand;

// Declarations of functions
static int16_t at_ledService(uint16_t inout, char* params);
static void parse_uart_data( char * pBuf );

// Local variables
static ATcommand commands[AT_CNT] = {
		{"AT+LED", at_ledService}
};


void AT_commandInit(void) {
	register_uart_str_rx_event_callback(parse_uart_data);
}

static void parse_uart_data( char * pBuf )
{
	int8_t (*_at_srv)(uint8_t inout, char * data);
	char * cmd_wsk;
	char * rest;
	uint8_t i=0, len;

	if ( strpbrk(pBuf, "=?"))	{
		if ( strpbrk(pBuf, "?"))	{
			cmd_wsk = strtok_r(pBuf, "?", &rest);
			len = strlen(cmd_wsk);
			for(i=0;i<AT_CNT;i++) {
				if ( len && 0 == strncasecmp(cmd_wsk, commands[i].name, len) ) {
					if (commands[i].name) {
						_at_srv = (void *)( commands[i].at_service );
						if( _at_srv) {
							if( _at_srv(0, rest) < 0 ) UART_putStr("ERROR\r\n");
						}
					}
					UART_putStr("\r\n");
					break;
				}
			}

		} else {

			cmd_wsk = strtok_r(pBuf, "=", &rest);
			len = strlen(cmd_wsk);
			for(i=0;i<AT_CNT;i++) {
				if ( len && 0 == strncasecmp(cmd_wsk, commands[i].name, len) ) {
					if (commands[i].name) {
						_at_srv = (void *)( &commands[i].at_service );
						if( _at_srv && ! _at_srv( 1, rest ) ) UART_putStr("OK\r\n");
						else UART_putStr("ERROR\r\n");
					}
					break;
				}
			}
		}

	} else {

		if( 0 == pBuf[0] ) UART_putStr("\r\n");
		else {
			for(i=0;i<AT_CNT;i++) {
				if ( 0 == strncasecmp(pBuf, commands[i].name, strlen(pBuf)) ) {
					if( commands[i].name ) {
						_at_srv = (void *)(&commands[i].name);
						if( _at_srv) _at_srv(2,0);
					}
					break;
				}
			}
		}
	}

	if( AT_CNT == i ) UART_putStr("ERROR\r\n");
}


static int16_t at_ledService(uint16_t inout, char* params)
{
	if( inout < 2 ) {
		if( 1 == inout ) {
			if( params[0] < '0' || params[0] > '1' ) {
				return -1;
			}

			if( '1' == params[0] ) {
				LED3_BB = 1;
			} else {
				LED3_BB = 0;
			}
		}

		UART_putStr("+LED: ");

		if( LED3_ODR & LED3 ) {
			UART_putInt(1,10);
		} else {
			UART_putInt(0,10);
		}

		UART_putStr("\r\n");

	} else if( 2 == inout ) {
		UART_putStr("AT+LED = (0-1)\r\n");
	}
	return 0;
}

