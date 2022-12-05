#ifndef LCD_TWI_H
#define LCD_TWI_H
#include "io.h"

#define LCD_TWI_CT 5 // defines the maximum amount of lcds to be driven with this library

FILE lcd_twi_init(uint8_t addr);
void lcd_twi_clear(FILE *stream);
void lcd_twi_cursor(FILE *stream, uint8_t col, uint8_t row);
#endif //LCD_TWI_H
