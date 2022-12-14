#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include "io.h"
#include "lcd_twi.h"
#include "steppermotor.h"
#include "ds3231.h"
#include "dht11.h"

#define LCD_TWI_ADDR 0x27
#define RTC_ADDR 0x68

#define DHT11_PIN 7

#define POT 12
#define WATER 14
#define WATER_THRESHOLD 0
#define TEMP_THRESHOLD 10.0

#define STEPPER_1 25
#define STEPPER_2 27
#define STEPPER_3 29
#define STEPPER_4 31

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

uint8_t curr_state = IDLE;
uint8_t prev_state = IDLE;

int main () {
	PORTE |= _BV(PE4);
	EICRB |= 0b00000011;
	EIMSK |= 0b00010000;
	sei();

	DDRH |= _BV(PH3) | _BV(PH5);
	DDRG |= _BV(PG5);
	DDRE |= _BV(PE5);

	time_t time;
	dht11_t temphum;
	FILE lcd;
	stepper_t stepper = stepper_init(STEPPER_1, STEPPER_2, STEPPER_3, STEPPER_4);
	uint16_t stepper_pos = 512;

	adc_init();
	uart_init(9600);
	twi_init();
	lcd = lcd_twi_init(LCD_TWI_ADDR);
	for (;;) {
		if (handler[curr_state]) {
			curr_state = handler[curr_state]();
		} else {
			curr_state = handler[ERROR]();
		}
		if (curr_state != prev_state) {
			printf("----------\nState switched to ");
			switch (curr_state) {
				case ERROR:
					printf("Error\n");
					break;
				case RUNNING:
					printf("Running\n");
					break;
				case IDLE:
					printf("Idle\n");
					break;
				case DISABLED:
					printf("Disabled\n");
					break;
			}
			time = get_time(RTC_ADDR);
			printf("%d/%d/%d %d:%.2d:%.2d\n----------\n", time.month, time.day, time.year, time.hour, time.min, time.sec);
		}
		if (curr_state != ERROR) {
			uint16_t n = adc_read(POT);
			printf("New stepper position - %d\n", n);
			stepper_rotate(stepper, n - stepper_pos);
			stepper_pos = n;
		}
		lcd_twi_clear(&lcd);
		if (curr_state != DISABLED) {
			temphum = dht11_read(DHT11_PIN);
			fprintf(&lcd, "%.1f %% - %.1f C\n", temphum.humidity, temphum.temp);
		}
		if (curr_state == ERROR) {
			fprintf(&lcd, "ERROR");
		}
		prev_state = curr_state;
		if (temphum.temp < TEMP_THRESHOLD && curr_state != ERROR) {
			curr_state = IDLE;
		}
		_delay_ms(1000);
	}
}

#define RED 0b001
#define BLUE 0b100
#define GREEN 0b010
#define YELLOW 0b011

void rgb_led(uint8_t color) {
	if (color & _BV(0)) {
		PORTG |= _BV(PG5);
	} else {
		PORTG &= ~_BV(PG5);
	}
	if (color & _BV(1)) {
		PORTH |= _BV(PH3);
	} else {
		PORTH &= ~_BV(PH3);
	}
	if (color & _BV(2)) {
		PORTH |= _BV(PH5);
	} else {
		PORTH &= ~_BV(PH5);
	}
}

// Yellow LED on
// No monitoring of temp or water
// Start button monitored with ISR
uint8_t disabled() {
	PORTE &= ~_BV(PE5);
	rgb_led(YELLOW);
	return DISABLED;
}

// Exact time stamp (using real time clock) should record transition times
// Water level should be continuously monitored and state changed to error if level is too low
// GREEN LED should be ON
uint8_t idle() {
	PORTE &= ~_BV(PE5);
	rgb_led(GREEN);
	if (adc_read(WATER) < WATER_THRESHOLD) {
		return ERROR;
	}
	return IDLE;
}

// Motor should be off and not start regardless of temperature
// A reset button should trigger a change to the IDLE stage if the water level is above the threshold
// Error message should be displayed on LCD
// RED LED should be turned on (all other LEDs turned off)
uint8_t error() {
	PORTE &= ~_BV(PE5);
	rgb_led(RED);
	return ERROR;
}

// Fan motor should be on
// System should transition to IDLE as soon as temperature drops below threshold
// System should transition to ERROR state if water becomes too low
// BLUE LED should be turned on (all other LEDs turned off)
uint8_t running() {
	PORTE |= _BV(PE5);
	rgb_led(BLUE);
	if (adc_read(WATER) < WATER_THRESHOLD) {
		return ERROR;
	}
	return RUNNING;
}

/*** *** *** *** *** *** *** *** *** *** ***
 * ISRs
 *** *** *** *** *** *** *** *** *** *** ***/

ISR(INT4_vect) {
	if (curr_state == DISABLED) {
		curr_state = RUNNING;
	} else if (curr_state == RUNNING || curr_state == IDLE) {
		curr_state = DISABLED;
	} else if (curr_state == ERROR) {
		curr_state = IDLE;
	}
}
