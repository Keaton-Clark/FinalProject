// i have no idea how to do stuff in C so i'm just copying the formatting that keaton is doing O.o
#ifndef STEPPERMOTOR_H
#define STEPPERMOTOR_H

#include <stdio.h>
#include "io.h" // keaton had this so... so will I :) // jk found out this is like the essential library for everything??? keaton wtf u just rewrote the whole pinmode thingy LMAO

typedef struct stepper_data {
	pin_t pins[4];
	int16_t deg;
	uint8_t speed;
} stepper_t;

void stepper_rotate(stepper_t stepper, int16_t steps);
stepper_t stepper_init(uint8_t in1, uint8_t in2, uint8_t in3, uint8_t in4);

#endif //STEPPERMOTOR_H


// i have no idea if what i'm doing is correct.

// typedef struct stepper_motor {
//     pin_t *pins; // array of pins that the steppermotor has to use on the arduino board
//     // not sure what else we need here
// } motor_t;

// motor_t init_motor(pin_t *pins);
// void one_step_turn(motor_t motor);
// void write(motor_t motor, int a,int b,int c,int d); // helper function for one_step_turn
