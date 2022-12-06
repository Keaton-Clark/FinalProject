#ifndef DHT11_H
#define DHT11_H
#include "io.h"

typedef struct dht11_data {
	float humidity;
	float temp;
} dht11_t;

dht11_t dht11_read(uint8_t pin);

#endif //DHT11_H
