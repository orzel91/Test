/*
 * at_commands.c
 *
 *  Created on: 23 maj 2018
 *      Author: jarek
 */

#include "at_commands.h"
#include <string.h>
#include <strings.h>
#include "uart.h"
#include "../config.h"


#define AT_CNT 1

// Declarations
typedef struct {
	char* name;
	int16_t (*at_service)(uint16_t inout, char* params);
}ATcommand;

// Declarations of functions
static int16_t at_ledService(uint16_t inout, char* params);

// Local variables
static ATcommand commands[AT_CNT] = {
		{"AT+LED", at_ledService}
};


void parse_uart_data(char* pBuf)
{
	int8_t (*_at_srv)(uint8_t inout, char * data);
	char * cmd_wsk;
	char * rest;
	uint8_t i=0, len;

	// check if in string there is "=" or "?"
	if ( strpbrk(pBuf, "=?"))	{
		// check if in string there is "?"
		if ( strpbrk(pBuf, "?") ) {
			// divide string on tokens, "?" is a separator of token
			cmd_wsk = strtok_r(pBuf, "?", &rest);
			len = strlen(cmd_wsk);
			for(i=0;i<AT_CNT;i++) {
				// check string if it is the same as in AT command table - if yes, it will return zero
				if ( len && ( 0 == strncasecmp(cmd_wsk, commands[i].name, len) ) ) {
					if (commands[i].name) {
						_at_srv = (void *)( commands[i].at_service );
						if( _at_srv) {
							if( _at_srv(0, rest) < 0 ) uart_putStr("ERROR\r\n");
						}
					}
					uart_putStr("\r\n");
					break;
				}
			}

		} else {
			// divide string on tokens, "=" is a separator of token
			cmd_wsk = strtok_r(pBuf, "=", &rest);
			len = strlen(cmd_wsk);

			for(i=0;i<AT_CNT;i++)
			{
				// check string if it is the same as in AT command table - if yes, it will return zero
				if ( len && 0 == strncasecmp(cmd_wsk, commands[i].name, len) )
				{
					if (commands[i].name)
					{
						_at_srv = (void *)( commands[i].at_service );

						if( _at_srv && ! _at_srv( 1, rest ) ) {
							uart_putStr("OK\r\n");
						} else {
							uart_putStr("ERROR\r\n");
						}
					}
					break;
				}
			}
		}

	} else {

		if( 0 == pBuf[0] ) uart_putStr("\r\n");
		else {
			for(i=0;i<AT_CNT;i++) {
				// check string if it is the same as in AT command table - if yes, it will return zero
				if ( 0 == strncasecmp(pBuf, commands[i].name, strlen(pBuf)) ) {
					if( commands[i].name ) {
						_at_srv = (void *)( commands[i].at_service );
						if( _at_srv) {
							_at_srv(2,0);
						}
					}
					break;
				}
			}
		}
	}

	if( AT_CNT == i ) uart_putStr("ERROR\r\n");
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

		uart_putStr("LED: ");

		if( LED3_ODR & LED3 ) {
			uart_putInt(1,10);
		} else {
			uart_putInt(0,10);
		}

		uart_putStr("\r\n");

	} else if( 2 == inout ) {
		uart_putStr("AT+LED = (0-1)\r\n");
	}
	return 0;
}

