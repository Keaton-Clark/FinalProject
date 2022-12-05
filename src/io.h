#include <avr/io.h>
#include <util/delay.h>
#include <stdio.h>

#ifndef IO_H
#define IO_H
#define HIGH 0xFF
#define LOW 0x0
#define OUTPUT 0x01
#define INPUT 0x0


/*** *** *** *** *** *** *** *** *** *** *** *** *** ***
 *						GPIO
 *** *** *** *** *** *** *** *** *** *** *** *** *** ***/

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

/*** *** *** *** *** *** *** *** *** *** *** *** *** ***
 *						UART
 *** *** *** *** *** *** *** *** *** *** *** *** *** ***/

#define RX_BUFSIZE 80
void uart_init(uint16_t baud_rate);
int uart_get_char(FILE *stream);
int uart_put_char(char c, FILE *stream);
int uart_available();
int uart_read_char();


/*** *** *** *** *** *** *** *** *** *** *** *** *** ***
 *						ADC
 *** *** *** *** *** *** *** *** *** *** *** *** *** ***/

void adc_init();
uint16_t adc_read(uint8_t channel);

/*** *** *** *** *** *** *** *** *** *** *** *** *** ***
 *						TWI
 *** *** *** *** *** *** *** *** *** *** *** *** *** ***/

// twi bitrate
#define SCL_CLK 40000
// twi_start rw values
#define TWI_R 1
#define TWI_W 0
// twi_start return codes
#define TWI_START_FAIL 0x00
#define TWI_W_ADDR_ACK 0x18
#define TWI_W_ADDR_NACK 0x20
// twi_write return codes
#define TWI_W_DATA_ACK 0x28

#define TWI_R_ADDR_ACK 0x40

void twi_init();
uint8_t twi_start(uint8_t addr, uint8_t rw);
uint8_t twi_put(uint8_t data);
uint8_t twi_get();
void twi_stop();
void twi_write(uint8_t *data, size_t size, uint8_t addr);
void twi_read(uint8_t *data, size_t size, uint8_t addr);

#endif //IO_H
