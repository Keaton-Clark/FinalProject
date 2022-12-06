#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "io.h"

int main () {
	uart_init(9600);
	adc_init();
	pin_t led = new_pin(13);
	pin_mode(led, OUTPUT);
	uint16_t i;
	for (;;) {
		i = adc_read(0);
		printf("%d\n", i);
		if (i > 512) {
			write_pin(led, HIGH);
		} else {
			write_pin(led, LOW);
		}
	}
}
