#ifndef MATRIX_H
#define MATRIX_H

#include <avr/io.h>
#include <stdio.h>
#include "io.h"

typedef struct matrix_data {
	uint8_t rows;
	pin_t* row_pins;
	uint8_t cols;
	pin_t* col_pins;
	char* lookup_table;
} matrix_t;
matrix_t init_matrix(uint8_t rows, pin_t* row_pins, uint8_t cols, pin_t* col_pins, char* lookup_table);
char read_matrix(matrix_t matrix);

#endif //MATRIX_H
