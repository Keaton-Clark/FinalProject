#include "matrix.h"

matrix_t init_matrix(uint8_t rows, pin_t* row_pins, uint8_t cols, pin_t* col_pins, char* lookup_table) {
	for (int i = 0; i < rows; ++i) {
		pin_mode(row_pins[i], OUTPUT);
		write_pin(row_pins[i], HIGH); // Start polling
	}
	for (int i = 0; i < cols; ++i) {
		pin_mode(col_pins[i], INPUT);
		write_pin(col_pins[i], HIGH); // Activate internal Pull-ups
	}
	return (matrix_t) {
		.rows = rows,
		.row_pins = row_pins,
		.cols = cols,
		.col_pins = col_pins,
		.lookup_table = lookup_table
	};
}

char read_matrix(matrix_t matrix) {
	for (int i = 0; i < matrix.rows; ++i) {
		write_pin(matrix.row_pins[i], LOW); // Look at pin

		for (int j = 0; j < matrix.cols; ++j) {
			if (!read_pin(matrix.col_pins[j])) {
				write_pin(matrix.row_pins[i], HIGH); // Reset pin to high
				return matrix.lookup_table[(i * matrix.cols) + j];
			}
		}

		write_pin(matrix.row_pins[i], HIGH); // Reset pin to high
	}
	return 0;
}
