#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>
#include <stdlib.h>
#include "io.h"
#include "lcd_twi.h"
#include "steppermotor.h"

#define LCD_ADDR 0x27

#define STEPPER_POT_ANALOG_CHANNEL 0
#define VENT_DIRECTION_THRESHOLD 50
#define STEPS_PER_ANALOG_UNIT 1

uint16_t last_pot_change_value;

int main () {
	uart_init(9600);
	stepper_t stepper = stepper_init(28, 26, 24, 22);
	last_pot_change_value = adc_read(STEPPER_POT_ANALOG_CHANNEL);
	for (;;) {
		
        uint16_t current_pot_value = adc_read(STEPPER_POT_ANALOG_CHANNEL);
		uint16_t abs_difference = abs(current_pot_value - last_pot_change_value);
        printf("current pot value: %u, abs difference: %u\n", current_pot_value, abs_difference);
		if (abs_difference > VENT_DIRECTION_THRESHOLD){
            printf("STEPPING - difference: %ld, steps: %d\n", difference, steps);
			int32_t difference = current_pot_value - last_pot_change_value;
            int16_t steps = difference * STEPS_PER_ANALOG_UNIT;
            stepper_rotate(stepper, steps);
            last_pot_change_value = current_pot_value;
        }
	}
}