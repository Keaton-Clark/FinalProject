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
	for (;;) {
		stepper_rotate(stepper, -100);
		stepper_rotate(stepper, 100);
		_delay_ms(1000000);
	}
}