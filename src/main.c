#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "io.h"

#define LCD_ADDR 0x27

int main () {
	uart_init(9600);
	twi_init();
	for (;;) {
		if(twi_start(LCD_ADDR, TWI_W) == TWI_W_ADDR_ACK) {
			printf("0x%x", twi_write('a'));
			twi_stop();
		}
		_delay_ms(1000);
	}
}
