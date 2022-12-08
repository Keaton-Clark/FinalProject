#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "io.h"
#include "lcd_twi.h"
#include "steppermotor.h"

#define LCD_ADDR 0x27

int main () {
	uart_init(9600);
	stepper_t stepper = stepper_init(28, 26, 24, 22);
	uint16_t analog_val = adc_read(0);
	printf("%u\n", analog_val);
	for (;;) {
		analog_val = adc_read(0);
		printf("%u\n", analog_val);
		stepper_rotate(stepper, -100);
		printf("%u\n", analog_val);
		stepper_rotate(stepper, 100);
		_delay_ms(1000);
	}
}