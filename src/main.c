#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "io.h"
#include "lcd_twi.h"

#define LCD_ADDR 0x27

int main () {
	uart_init(9600);
	twi_init();
	FILE lcd = lcd_twi_init(0x27);
	for (;;) {
		lcd_twi_cursor(&lcd, 5, 0);
		fprintf(&lcd, "Hello");
		lcd_twi_cursor(&lcd, 6, 1);
		fprintf(&lcd, "World");
		_delay_ms(1000);
		lcd_twi_clear(&lcd);
	}
}
