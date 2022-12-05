#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include "io.h"
#include "lcd_twi.h"
#include "steppermotor.h"
#include "ds3231.h"

#define LCD_ADDR 0x27
#define RTC_ADDR 0x68

int main () {
	uart_init(9600);
	twi_init();
	for (;;) {
		time_t time = get_time(RTC_ADDR);
		char time_str[50];
		tsnprintf(time_str, 50, time);
		printf("%s\n", time_str);
		_delay_ms(1000);
	}
}
