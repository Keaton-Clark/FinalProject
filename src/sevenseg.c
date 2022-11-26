#include "sevenseg.h"

uint8_t seven_seg_lookup[10][7] = {
	[0] = {0, 1, 1, 1, 1, 1, 1},
	[1] = {0, 1, 0, 0, 0, 1, 0},
	[2] = {1, 0, 1, 1, 0, 1, 1},
	[3] = {1, 1, 1, 0, 0, 1, 1},
	[4] = {1, 1, 0, 0, 1, 1, 0},
	[5] = {1, 1, 1, 0, 1, 0, 1},
	[6] = {1, 1, 1, 1, 1, 0, 0},
	[7] = {0, 1, 0, 0, 0, 1, 1},
	[8] = {1, 1, 1, 1, 1, 1, 1},
	[9] = {1, 1, 0, 0, 1, 1, 1},
};

four_dig_seven_seg_t init_four_dig_seven_seg(pin_t *dig, pin_t *seg) {
	for (int i = 0; i < 4; ++i) {
		pin_mode(dig[i], OUTPUT);
		write_pin(dig[i], HIGH);
	}
	write_pin(dig[0], LOW);
	for (int i = 0; i < 7; ++i) {
		pin_mode(seg[i], OUTPUT);
		write_pin(seg[i], LOW);
	}
	return (four_dig_seven_seg_t) {
		.dig = dig,
		.seg = seg,
	};
}
void four_dig_seven_seg_write(four_dig_seven_seg_t *display, char content[4]) {
	for (int i = 0; i < 4; ++i) {
		display->dis[i] = content[i];
	}
}
void four_dig_seven_seg_update(four_dig_seven_seg_t display) {
	for (int i = 0; i < 4; ++i) {
		write_pin(display.dig[i], LOW);
		if (display.dis[i] >= 48) {
			for (int j = 0; j < 7; ++j) {
				write_pin(display.seg[j], seven_seg_lookup[display.dis[i] - 48][j]);
			}
		} else {
			for (int j = 0; j < 7; ++j) {
				write_pin(display.seg[j], LOW);
			}
		}
		_delay_ms(1);
		write_pin(display.dig[i], HIGH);
	}
}
