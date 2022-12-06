#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "io.h"
#include "lcd_twi.h"
#include "steppermotor.h"
#include "ds3231.h"
#include "dht11.h"

#define LCD_ADDR 0x27
#define RTC_ADDR 0x68

int main () {
	uart_init(9600);
	dht11_t temp;
	for (;;) {
		temp = dht11_read(2);
		printf("%f, %f\n", temp.temp, temp.humidity);
		_delay_ms(2);
	}
}
