/*
 * OLED1306.c
 *
 *  Created on: 7 sie 2018
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
#include "oled1306.h"
#include "spi.h"


/*******************  Defines  *******************/



/*******************  Declarations  *******************/



/*******************  Global variables  *******************/



/*******************  Local variables  *******************/
static uint8_t oledBuff[1024];
static OLED1306_t oled;

/*******************  Private function declarations  *******************/



/*******************  Public function definitions  *******************/
void OLED_init(void)
{
	SPI_init();

	gpio_pin_cfg(OLED_RES_GPIO, OLED_RES_PIN, GPIO_CRx_MODE_CNF_OUT_PP_10M_value); // Set Res Pin
	gpio_pin_cfg(OLED_DC_GPIO, OLED_DC_PIN, GPIO_CRx_MODE_CNF_OUT_PP_10M_value); // Set DC Pin

	OLED_RES_BB = 1;
	OLED_DC_BB = 1;

	//  1. set mux ratio
	SPI_sendCmd(OLED1306_SETMULTIPLEX);
	SPI_sendCmd(0x3F);

	//  2. set display offset
	SPI_sendCmd(OLED1306_SETDISPLAYOFFSET );
	SPI_sendCmd(0x0);

	//  3. set display start line
	SPI_sendCmd(OLED1306_SETSTARTLINE | 0x0);
	SPI_sendCmd(OLED1306_MEMORYMODE);
	SPI_sendCmd(0x00);

	//  4. set Segment re-map A0h/A1h
	SPI_sendCmd(OLED1306_SEGREMAP | 0x1);

	//  5. Set COM Output Scan Direction C0h/C8h
	SPI_sendCmd(OLED1306_COMSCANDEC);

	//  6. Set COM Pins hardware configuration DAh, 12
	SPI_sendCmd(OLED1306_SETCOMPINS);
	SPI_sendCmd(0x12);

	//  7. Set Contrast Control 81h, 7Fh
	SPI_sendCmd(OLED1306_SETCONTRAST );
	SPI_sendCmd(0xCf);

	//  8. Disable Entire Display On A4h
	SPI_sendCmd(OLED1306_DISPLAYALLON_RESUME);

	//  9. Set Normal Display A6h
	SPI_sendCmd(OLED1306_NORMALDISPLAY);

	//  10. Set Osc Frequency  D5h, 80h
	SPI_sendCmd(OLED1306_SETDISPLAYCLOCKDIV);
	SPI_sendCmd(0x80);

	//  11. Enable charge pump regulator 8Dh, 14h
	SPI_sendCmd(OLED1306_CHARGEPUMP );
	SPI_sendCmd(0x14);

	//  12. Display On AFh
	SPI_sendCmd(OLED1306_DISPLAYON);

	OLED_clearScreen();

	OLED_SetCursor(0, 0);

	OLED_refresh();

}

void OLED_sendData(void)
{

}
/*
void OLED_sendCmd(uint8_t cmd)
{

}*/

// Fill the whole screen with the given color
void OLED_fill(OLED1306_COLOR color)
{
    /* Set memory */
    uint32_t i;

    for(i = 0; i < sizeof(oledBuff); i++) {
    	oledBuff[i] = (color == BLACK) ? 0x00 : 0xFF;
    }
}

void OLED_refresh(void)
{
	uint8_t m;

	for (m = 0; m < 8; m++) {
	    SPI_sendCmd(OLED1306_SETLOWCOLUMN  | 0x0);  // low col = 0
	    SPI_sendCmd(OLED1306_SETHIGHCOLUMN | 0x0);  // hi col = 0
	    SPI_sendCmd(OLED1306_SETSTARTLINE | 0x0); // line #0

		/* Write multi data */
		SPI_sendData(&oledBuff[OLED1306_LCDWIDTH*m], OLED1306_LCDWIDTH);
	}
}

void OLED_turnOn(void)
{
	SPI_sendCmd(OLED1306_DISPLAYON);
}

void OLED_turnOff(void)
{
	SPI_sendCmd(OLED1306_DISPLAYOFF);
}

void OLED_clearScreen(void)
{
    memset(oledBuff, 0x00, 1024);
}

void OLED_DrawPixel(uint8_t x, uint8_t y, OLED1306_COLOR color)
{
	if (x >= OLED1306_LCDWIDTH || y >= OLED1306_LCDHEIGHT) {
		// Don't write outside the buffer
		return;
	}

	// Check if pixel should be inverted
	if (oled.Inverted) {
		color = (OLED1306_COLOR)!color;
	}

	// Draw in the right color
	if (color == WHITE) {
		oledBuff[x + (y / 8) * OLED1306_LCDWIDTH] |= 1 << (y % 8);
	} else {
		oledBuff[x + (y / 8) * OLED1306_LCDWIDTH] &= ~(1 << (y % 8));
	}

}

char OLED_WriteChar(char ch, FontDef Font, OLED1306_COLOR color)
{
	uint32_t i, b, j;

	// Check remaining space on current line
	if (OLED1306_LCDWIDTH <= (oled.CurrentX + Font.FontWidth) ||
			OLED1306_LCDHEIGHT <= (oled.CurrentY + Font.FontHeight))
	{
		// Not enough space on current line
		return 0;
	}

	// Use the font to write
	for(i = 0; i < Font.FontHeight; i++) {
		b = Font.data[(ch - 32) * Font.FontHeight + i];
		for(j = 0; j < Font.FontWidth; j++) {
			if((b << j) & 0x8000)  {
				OLED_DrawPixel(oled.CurrentX + j, (oled.CurrentY + i), (OLED1306_COLOR) color);
			} else {
				OLED_DrawPixel(oled.CurrentX + j, (oled.CurrentY + i), (OLED1306_COLOR)!color);
			}
		}
	}

	// The current space is now taken
	oled.CurrentX += Font.FontWidth;

	// Return written char for validation
	return ch;
}

// Write full string to screenbuffer
char OLED_writeString(char* str, FontDef Font, OLED1306_COLOR color)
{
    // Write until null-byte
    while (*str) {
        if (OLED_WriteChar(*str, Font, color) != *str) {
            // Char could not be written
            return *str;
        }

        // Next char
        str++;
    }

    // Everything ok
    return *str;
}

// function to sending number
void OLED_writeInt(int value, int radix, FontDef font, OLED1306_COLOR color)
{
	char string[17];
	itoa(value, string, radix);
	OLED_writeString(string, font, color);
}

// Position the cursor
void OLED_SetCursor(uint8_t x, uint8_t y)
{
    oled.CurrentX = x;
    oled.CurrentY = y;
}

/*******************  Private function definitions  *******************/
