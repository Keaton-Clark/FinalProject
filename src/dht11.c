#include "dht11.h"
#include <avr/interrupt.h>

#define DHT11_TIMEOUT UINT32_MAX
#define MAXCYCLES (1000 * (F_CPU / 1000000L))

uint32_t wait_pulse(uint8_t level, volatile uint8_t *pin, uint8_t bitmask) {
	uint32_t count = 0;
	uint8_t port_state = level ? bitmask : 0;
	while ((*pin & bitmask) == port_state) {
		if (count++ >= MAXCYCLES) {
			return DHT11_TIMEOUT;
		}
	}
	return count;
}

dht11_t dht11_read(uint8_t pin_num) {
	cli(); //disable interrupts
	pin_t pin = new_pin(pin_num);
	uint32_t cycles[80];
	uint8_t data[5];
	static dht11_t out;

	// start signal
	pin_mode(pin, INPUT);
	write_pin(pin, HIGH);
	_delay_ms(1);
	pin_mode(pin, OUTPUT);
	write_pin(pin, LOW);
	_delay_ms(20); // wait atleast 18ms
	
	// end start signal
	pin_mode(pin, INPUT);
	write_pin(pin, HIGH);

	_delay_us(55); // wait a moment for the sensor
	
	if (wait_pulse(LOW, pin._pin, pin._bitmask) == DHT11_TIMEOUT)  {
		return out;
	}
	if (wait_pulse(HIGH, pin._pin, pin._bitmask) == DHT11_TIMEOUT)  {
		return out;
	}

	for (int i = 0; i < 80; i+=2) {
		cycles[i] = wait_pulse(LOW, pin._pin, pin._bitmask);
		cycles[i+1] = wait_pulse(HIGH, pin._pin, pin._bitmask);
	}

	for (int i = 0; i < 80; i+=2) {
		if (cycles[i] == DHT11_TIMEOUT || cycles[i+1] == DHT11_TIMEOUT) {
			return out;
		}
		data[i/16] <<= 1;	
		if (cycles[i] < cycles[i+1]) {
			data[i/16] |= 1;
		}
	}
	out.humidity = data[0] + (data[1] * .1);
	out.temp = data[2];
	if (data[3] & 0x80) {
		out.temp = -1 - out.temp;
	}
	out.temp += (data[3] & 0x0f) * .1;
	sei(); //enable interrupts
	return out;
}
