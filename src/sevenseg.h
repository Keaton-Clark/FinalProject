#ifndef SEVEN_SEG_H
#define SEVEN_SEG_H

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>
#include "io.h"

typedef struct four_dig_seven_seg {
	pin_t *dig;
	pin_t *seg;
	uint8_t dis[4];
} four_dig_seven_seg_t;

four_dig_seven_seg_t init_four_dig_seven_seg(pin_t *dig, pin_t *seg);
void four_dig_seven_seg_write(four_dig_seven_seg_t *display, char content[4]);
void four_dig_seven_seg_update(four_dig_seven_seg_t display);

#endif //SEVEN_SEG_H
