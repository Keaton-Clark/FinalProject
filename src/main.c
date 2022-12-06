#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include "io.h"
#include "lcd_twi.h"
#include "steppermotor.h"
#include "ds3231.h"
#include "dht11.h"

#define LCD_ADDR 0x27
#define RTC_ADDR 0x68


uint8_t disabled();
#define DISABLED 0x00
uint8_t idle();
#define IDLE 0x01
uint8_t error();
#define ERROR 0x02
uint8_t running();
#define RUNNING 0x03

uint8_t (*handler[])() = {
	[DISABLED] = disabled,
	[IDLE] = idle,
	[ERROR] = error,
	[RUNNING] = running
};

uint8_t state = RUNNING;

int main () {
	sei();
	uart_init(9600);
	for (;;) {
		if (handler[state]) {
			state = handler[state]();
		} else {
			state = handler[ERROR]();
		}
	}
}

/*** *** *** *** *** *** *** *** *** *** ***
 * For each of these handler functions, simply return the macro for the next state you want to transition to
 * E.G. `return ERROR;` will promptly transition the state to error
 *** *** *** *** *** *** *** *** *** *** ***/


// Yellow LED on
// No monitoring of temp or water
// Start button monitored with ISR
uint8_t disabled() {
	return DISABLED;
}

// Exact time stamp (using real time clock) should record transition times
// Water level should be continuously monitored and state changed to error if level is too low
// GREEN LED should be ON
uint8_t idle() {
	return IDLE;
}

// Motor should be off and not start regardless of temperature
// A reset button should trigger a change to the IDLE stage if the water level is above the threshold
// Error message should be displayed on LCD
// RED LED should be turned on (all other LEDs turned off)
uint8_t error() {
	return ERROR;
}

// Fan motor should be on
// System should transition to IDLE as soon as temperature drops below threshold
// System should transition to ERROR state if water becomes too low
// BLUE LED should be turned on (all other LEDs turned off)
uint8_t running() {
	return RUNNING;
}
