#include <avr/io.h>
#include <stdio.h>

#ifndef IO_H
#define IO_H
#define HIGH 0xFF
#define LOW 0x0
#define OUTPUT 0x01
#define INPUT 0x0

#define RX_BUFSIZE 80


typedef struct pin_data {
	uint8_t _group;
	volatile void *_ddr; // Data Direction Register
	volatile void *_port; // Pin Output Register
	volatile void *_pin; // Pin Input Register
	volatile void *_timer; // Timer if pwm pin
	uint8_t _bitmask; // Bit position
} pin_t;

uint8_t read_pin(pin_t pin);
void pin_mode(pin_t pin, uint8_t mode);
void write_pin(pin_t pin, uint8_t value);
void toggle_pin(pin_t pin);
pin_t new_pin(uint8_t pinNum);
void uart_init(uint16_t baud_rate);
int uart_get_char(FILE *stream);
int uart_put_char(char c, FILE *stream);
int uart_available();
int uart_read_char();
void adc_init();
uint16_t adc_read(uint8_t channel);

#endif //IO_H
