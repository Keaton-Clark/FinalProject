#include "io.h"
#include <avr/pgmspace.h>
#include <stdio.h>
#define GROUPA 1
#define GROUPB 2
#define GROUPC 3
#define GROUPD 4
#define GROUPE 5
#define GROUPF 6
#define GROUPG 7
#define GROUPH 8

#define get_digital_pin_to_group(pinNum) (uintptr_t)pgm_read_byte(&(digital_pin_to_group[pinNum]))
const uint8_t digital_pin_to_group[100] PROGMEM = {
	[22] = GROUPA,
	[23] = GROUPA,
	[24] = GROUPA,
	[25] = GROUPA,
	[26] = GROUPA,
	[27] = GROUPA,
	[28] = GROUPA,
	[29] = GROUPA,

	[53] = GROUPB,
	[52] = GROUPB,
	[51] = GROUPB,
	[50] = GROUPB,
	[10] = GROUPB,
	[11] = GROUPB,
	[12] = GROUPB,
	[13] = GROUPB,
	
	[37] = GROUPC,
	[36] = GROUPC,
	[35] = GROUPC,
	[34] = GROUPC,
	[33] = GROUPC,
	[32] = GROUPC,
	[31] = GROUPC,
	[30] = GROUPC,

	[0] = GROUPE,
	[1] = GROUPE,
	[5] = GROUPE,
	[2] = GROUPE,
	[3] = GROUPE,

	[4] = GROUPG,

	[6] = GROUPH,
	[7] = GROUPH,
	[8] = GROUPH,
	[9] = GROUPH,
};

#define get_group_to_ddr(group) (uintptr_t)pgm_read_byte(&(group_to_ddr[group]))
volatile PGM_P const group_to_ddr[] PROGMEM = {
	[GROUPA] = &DDRA,
	[GROUPB] = &DDRB,
	[GROUPC] = &DDRC,
	[GROUPE] = &DDRE,
	[GROUPG] = &DDRG,
	[GROUPH] = &DDRH,
};	
#define get_group_to_port(group) (uintptr_t)pgm_read_byte(&(group_to_port[group]))
volatile PGM_P const group_to_port[] PROGMEM = {
	[GROUPA] = &PORTA,
	[GROUPB] = &PORTB,
	[GROUPC] = &PORTC,
	[GROUPE] = &PORTE,
	[GROUPG] = &PORTG,
	[GROUPH] = &PORTH,
};
#define get_group_to_pin(group) (uintptr_t)pgm_read_byte(&(group_to_pin[group]))
volatile PGM_P const group_to_pin[] PROGMEM = {
	[GROUPA] = &PINA,
	[GROUPB] = &PINB,
	[GROUPC] = &PINC,
	[GROUPE] = &PINE,
	[GROUPG] = &PING,
	[GROUPH] = &PINH,
};
#define get_digital_pin_to_timer(pinNum) (uintptr_t)pgm_read_byte(&(digital_pin_to_timer[pinNum]))
volatile PGM_P const digital_pin_to_timer[] PROGMEM = {
};
#define get_digital_pin_to_bitmask(pinNum) pgm_read_byte(&(digital_pin_to_bitmask[pinNum]))
const uint8_t digital_pin_to_bitmask[100] PROGMEM = {
	[22] = _BV(PA0),
	[23] = _BV(PA1),
	[24] = _BV(PA2),
	[25] = _BV(PA3),
	[26] = _BV(PA4),
	[27] = _BV(PA5),
	[28] = _BV(PA6),
	[29] = _BV(PA7),

	[53] = _BV(PB0),
	[52] = _BV(PB1),
	[51] = _BV(PB2),
	[50] = _BV(PB3),
	[10] = _BV(PB4),
	[11] = _BV(PB5),
	[12] = _BV(PB6),
	[13] = _BV(PB7),

	[37] = _BV(PC0),
	[36] = _BV(PC1),
	[35] = _BV(PC2),
	[34] = _BV(PC3),
	[33] = _BV(PC4),
	[32] = _BV(PC5),
	[31] = _BV(PC6),
	[30] = _BV(PC7),

	[0] = _BV(PE0),
	[1] = _BV(PE1),
	[5] = _BV(PE3),
	[2] = _BV(PE4),
	[3] = _BV(PE5),

	[4] = _BV(PG5),

	[6] = _BV(PH3),
	[7] = _BV(PH4),
	[8] = _BV(PH5),
	[9] = _BV(PH6),
};
void pin_mode(pin_t pin, uint8_t mode) {
	if (mode)
		*(uint8_t*)pin._ddr |= pin._bitmask;
	else
		*(uint8_t*)pin._ddr &= ~pin._bitmask;
}
void toggle_pin(pin_t pin) {
	*(uint8_t*)pin._port ^= pin._bitmask;
}
void write_pin(pin_t pin, uint8_t value) {
	if (value) {		 	
		*(uint8_t*)pin._port |= pin._bitmask;
	} else {
		*(uint8_t*)pin._port &= ~pin._bitmask;
	}
}
uint8_t read_pin(pin_t pin) {
	if ((pin._bitmask & *(uint8_t*)pin._pin))
		return HIGH;
	return LOW;
}

pin_t new_pin(uint8_t pinNum) {
	pin_t pin = {
		._group = get_digital_pin_to_group(pinNum),
	       	._bitmask = get_digital_pin_to_bitmask(pinNum)
	};
	pin._ddr = (void*)get_group_to_ddr(pin._group);
	pin._port = (void*)get_group_to_port(pin._group);
	pin._pin = (void*)get_group_to_pin(pin._group);
	return pin;
}

int uart_put_char(char c, FILE *stream);
int uart_get_char(FILE *stream);
FILE uart_str = {
	.put = uart_put_char,
	.get = uart_get_char,
	.flags = 0b10|0b1, //Read | Write
	.udata = 0,
};
void uart_init(uint16_t baud_rate) {
	stdout = stdin = &uart_str; //set stdout to our FILE stream we made above ^
	UBRR0L = (F_CPU / (16UL * baud_rate)) - 1;
	UCSR0B = _BV(TXEN0) | _BV(RXEN0);
}
int uart_put_char(char c, FILE *stream) {
	if (c == '\n') {
		uart_put_char('\r', stream);
	}
	loop_until_bit_is_set(UCSR0A, UDRE0);
	UDR0 = c;
	return 0;
}
int uart_read_char() {
	loop_until_bit_is_set(UCSR0A, RXC0);
	if (UCSR0A & _BV(FE0)) return _FDEV_EOF;
	if (UCSR0A & _BV(DOR0)) return _FDEV_ERR;
	return UDR0;
}
int uart_available() {
	return UCSR0A & _BV(RXC0);
}
int uart_get_char(FILE *stream) {
	char c;
	char *cp0, *cp1;
	static char buf[RX_BUFSIZE], *rxp;
	if (!rxp) {
		for (cp0 = buf;;) {
			loop_until_bit_is_set(UCSR0A, RXC0);
			if (UCSR0A & _BV(FE0)) return _FDEV_EOF;
			if (UCSR0A & _BV(DOR0)) return _FDEV_ERR;
			c = UDR0;

			if (c == '\r') c = '\n';
			if (c == '\t') c = ' ';
			if (c == '\n') {
				*cp0 = c;
				uart_put_char(c, stream);
				rxp = buf;
				break;
			}

			if (c >= ' ' && c <= '~') {
				if (cp0 == buf + RX_BUFSIZE - 1) {
					uart_put_char('\a', stream);
				} else {
					*cp0++ = c;
					uart_put_char(c, stream);
				}
				continue;
			}

			switch (c) {
				case 'c' & 0x1f: //CTRL+C
					return -1;
				case '\b': //BACKSPC
				case 0x7f: //DEL
					if (cp0 > buf) {
						uart_put_char('\b', stream);
						uart_put_char(' ', stream);
						uart_put_char('\b', stream);
						cp0--;
					}
			}
		}
	}

	c = *rxp++;
	if (c == '\n') rxp = NULL;

	return c;
}
void adc_init() {
	ADMUX |= _BV(REFS0); //Set reference voltage to AVCC
	ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0) | _BV(ADEN);
	//ADMUX |= _BV(ADLAR); //Left justify ADC result
}

uint16_t adc_read(uint8_t channel) {
	ADMUX = (ADMUX & 0xf0) | channel; //Select channel to read
	ADCSRA |= _BV(ADSC); //Start conversion
	loop_until_bit_is_set(ADCSRA, ADIF); //SC bit will be clear when conversion is done
	ADCSRA |= _BV(ADIF);
	return ADC;
}
