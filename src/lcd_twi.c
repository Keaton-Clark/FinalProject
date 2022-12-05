#include "lcd_twi.h"
#include <stdlib.h>

#define LCD_CLEARDISPLAY 0x01
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCTL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5X10 0x04
#define LCD_5X8 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define LCD_EN 0b00000100  // Enable bit
#define Rw 0b00000010  // Read/Write bit
#define Rs 0b00000001  // Register select bit

typedef struct lcd_twi_data {
	uint8_t row;
	uint8_t col;
	uint8_t addr;
	uint8_t displaycontrol;
	uint8_t backlightval;
} lcd_twi_t;

lcd_twi_t lcd_twi_array[LCD_TWI_CT]; // allocate the lcd data to avoid using malloc

void lcd_twi_w8(lcd_twi_t *lcd, uint8_t data) {
	data |= lcd->backlightval;
	twi_write(&data, 1, 0x27);
}

void lcd_twi_pulse_en(lcd_twi_t *lcd, uint8_t data) {
	lcd_twi_w8(lcd, data | LCD_EN);
	lcd_twi_w8(lcd, data & ~LCD_EN);
}

void lcd_twi_w4(lcd_twi_t *lcd, uint8_t data) {
	lcd_twi_w8(lcd, data);
	lcd_twi_pulse_en(lcd, data);
}

void lcd_twi_send(lcd_twi_t *lcd, uint8_t data, uint8_t mode) {
	lcd_twi_w4(lcd, (data & 0xf0) | mode ); //send top 4 bits with mode
	lcd_twi_w4(lcd, ((data<<4) & 0xf0) | mode ); //send bottom 4 bits with mode
}

void lcd_twi_display(lcd_twi_t *lcd) {
	lcd_twi_send(lcd, lcd->displaycontrol, 0);
}

void lcd_twi_clear(FILE *stream) {
	lcd_twi_send(stream->udata, LCD_CLEARDISPLAY, 0);
	_delay_ms(2);
}

void lcd_twi_home(lcd_twi_t *lcd) {
	lcd->col = 0;
	lcd->row = 0;
	lcd_twi_send(lcd, LCD_RETURNHOME, 0);
	_delay_ms(2);
}

void lcd_twi_cursor(FILE *stream, uint8_t col, uint8_t row) {
	uint8_t row_offsets[] = {0x00, 0x40, 0x14, 0x54};
	if (row > 2) {
		row = 1;
	}
	lcd_twi_send(stream->udata, LCD_SETDDRAMADDR | (col + row_offsets[row]), 0);
}

int lcd_twi_put_char(char c, FILE *stream) {
	if (c == '\n') {
		lcd_twi_cursor(stream, 0, 1);
		return 0;
	}
	lcd_twi_send(stream->udata, c, Rs);
	return 0;
}

FILE lcd_twi_init(uint8_t addr) {
	static int lcd_twi_num = 0;
	FILE lcd_stream;
	if (lcd_twi_num == LCD_TWI_CT) {
		return lcd_stream;
	}
	lcd_stream.udata = &lcd_twi_array[lcd_twi_num];
	((lcd_twi_t*)lcd_stream.udata)->addr = addr;
	((lcd_twi_t*)lcd_stream.udata)->displaycontrol = LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF | LCD_DISPLAYCTL;
	((lcd_twi_t*)lcd_stream.udata)->backlightval = LCD_BACKLIGHT;
	_delay_ms(50); // We need at least 40ms before power rising to 2.7v before sending commands
	
	// Put LCD into 4 bit mode, read datasheet fig.24

	lcd_twi_w4(lcd_stream.udata, 0x03<<4);
	_delay_ms(5); // Wait min 4.1 ms

	lcd_twi_w4(lcd_stream.udata ,0x03<<4);
	_delay_ms(5);

	lcd_twi_w4(lcd_stream.udata, 0x03<<4);
	_delay_ms(1);

	lcd_twi_w4(lcd_stream.udata, 0x02<<4); //set 4 bit interface
	lcd_twi_send(lcd_stream.udata, LCD_FUNCSET | LCD_4BITMODE | LCD_2LINE | LCD_5X8, 0);
	_delay_ms(1);

	lcd_twi_display(lcd_stream.udata);

	lcd_twi_send(lcd_stream.udata, LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT | LCD_ENTRYMODESET, 0);
	lcd_twi_clear(lcd_stream.udata);
	lcd_stream.put = lcd_twi_put_char;
	lcd_stream.flags = 0b10|0b1; //Read | Write
	return lcd_stream;
}
