/*
 * OLED1306.h
 *
 *  Created on: 7 sie 2018
 *      Author: jarek
 */

#ifndef OLED1306_OLED1306_H_
#define OLED1306_OLED1306_H_


#include <stdint.h>
#include "fonts.h"

/*
+=============================================================================+
| global definitions
+=============================================================================*
*/
#define OLED1306_LCDWIDTH 128
#define OLED1306_LCDHEIGHT 64

// Commands
#define OLED1306_SETCONTRAST                  0x81
#define OLED1306_DISPLAYALLON_RESUME          0xA4
#define OLED1306_DISPLAYALLON                 0xA5
#define OLED1306_NORMALDISPLAY                0xA6
#define OLED1306_INVERTDISPLAY                0xA7
#define OLED1306_DISPLAYOFF                   0xAE
#define OLED1306_DISPLAYON                    0xAF
#define OLED1306_SETDISPLAYOFFSET             0xD3
#define OLED1306_SETCOMPINS                   0xDA
#define OLED1306_SETVCOMDETECT                0xDB
#define OLED1306_SETDISPLAYCLOCKDIV           0xD5
#define OLED1306_SETPRECHARGE                 0xD9
#define OLED1306_SETMULTIPLEX                 0xA8
#define OLED1306_SETLOWCOLUMN                 0x00
#define OLED1306_SETHIGHCOLUMN                0x10
#define OLED1306_SETSTARTLINE                 0x40
#define OLED1306_MEMORYMODE                   0x20
#define OLED1306_COLUMNADDR                   0x21
#define OLED1306_PAGEADDR                     0x22
#define OLED1306_COMSCANINC                   0xC0
#define OLED1306_COMSCANDEC                   0xC8
#define OLED1306_SEGREMAP                     0xA0
#define OLED1306_CHARGEPUMP                   0x8D
#define OLED1306_EXTERNALVCC                  0x01
#define OLED1306_SWITCHCAPVCC                 0x02

// Scrolling defines
#define OLED1306_ACTIVATE_SCROLL              0x2F
#define OLED1306_DEACTIVATE_SCROLL            0x2E
#define OLED1306_SET_VERTICAL_SCROLL_AREA     0xA3
#define OLED1306_RIGHT_HORIZONTAL_SCROLL      0x26
#define OLED1306_LEFT_HORIZONTAL_SCROLL       0x27
#define OLED1306_VERT_AND_RIGHT_HORIZ_SCROLL  0x29
#define OLED1306_VERT_AND_LEFT_HORIZ_SCROLL	  0x2A

/*
+=============================================================================+
| Typedefs
+=============================================================================*
*/

// Enumeration for screen colors
typedef enum {
    BLACK = 0x00, // Black color, no pixel
    WHITE = 0x01  // Pixel is set. Color depends on OLED
} OLED1306_COLOR;

typedef struct {
    int16_t CurrentX;
    int16_t CurrentY;
    uint8_t Inverted;
    uint8_t Initialized;
} OLED1306_t;


void OLED_init(void);
void OLED_refresh(void);
void OLED_turnOn(void);
void OLED_turnOff(void);
void OLED_clearScreen(void);
void OLED_DrawPixel(uint8_t x, uint8_t y, OLED1306_COLOR color);
char OLED_WriteChar(char ch, FontDef Font, OLED1306_COLOR color);
char OLED_writeString(char* str, FontDef Font, OLED1306_COLOR color);
void OLED_writeInt(int value, int radix, FontDef font, OLED1306_COLOR color);
void OLED_SetCursor(uint8_t x, uint8_t y);
void OLED_fill(OLED1306_COLOR color);


#endif /* OLED1306_OLED1306_H_ */
