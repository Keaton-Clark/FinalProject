#include "steppermotor.h"

uint8_t stepper_cw_map[4] = {
	0b1010,
	0b0110,
	0b0101,
	0b1001,
};
uint8_t stepper_ccw_map[4] = {
	0b1001,
	0b0101,
	0b0110,
	0b1010,
};

stepper_t stepper_init(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4) {
	stepper_t out = {
		.pins[0] = new_pin(in1),
		.pins[1] = new_pin(in3),
		.pins[2] = new_pin(in2),
		.pins[3] = new_pin(in4),
		.deg = 0,
		.speed = 3,
	};
	for (int i = 0; i<4; ++i) {
		pin_mode(out.pins[i], OUTPUT);
	}
	return out;
}

void stepper_stop(stepper_t stepper) {
	for (uint8_t i = 0; i<4; ++i) {
		write_pin(stepper.pins[i], LOW);
	}
}

void stepper_rotate(stepper_t stepper, int16_t steps) {
		if (steps > 0) {
			for (int16_t i = 0; i<steps; ++i) {
				for (uint8_t j = 0; j<4; ++j) {
					write_pin(stepper.pins[0], stepper_cw_map[j] & 0b0001);
					write_pin(stepper.pins[1], stepper_cw_map[j] & 0b0010);
					write_pin(stepper.pins[2], stepper_cw_map[j] & 0b0100);
					write_pin(stepper.pins[3], stepper_cw_map[j] & 0b1000);
					_delay_ms(3);
				}
			}
		}
		else if (steps < 0) {
			for (int16_t i = 0; i>steps; --i) {
				for (uint8_t j = 0; j<4; ++j) {
					write_pin(stepper.pins[0], stepper_ccw_map[j] & 0b0001);
					write_pin(stepper.pins[1], stepper_ccw_map[j] & 0b0010);
					write_pin(stepper.pins[2], stepper_ccw_map[j] & 0b0100);
					write_pin(stepper.pins[3], stepper_ccw_map[j] & 0b1000);
					_delay_ms(3);
				}
			}
		}
		stepper_stop(stepper);
}
