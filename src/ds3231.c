#include "ds3231.h"

#define DS3231_REG_TIME 0x00

uint8_t htod(uint8_t hex) {
	return ( (hex/0x10) * 10 ) + (hex % 0x10);
}

time_t get_time(uint8_t addr) {
	uint8_t ds3231_reg_time = DS3231_REG_TIME;
	time_t out;
	twi_write(&ds3231_reg_time, 1, addr); 
	twi_read((uint8_t*)&out, 7, addr);
	for (int i = 0; i < 7; ++i) {
		((uint8_t*)&out)[i] = htod(((uint8_t*)&out)[i]);
	}
	return out;
}

void tsnprintf(char* str, size_t size, time_t time) {
	snprintf(str, size, "%d/%d/%d %d:%.2d:%.2d", time.month, time.day, time.year, time.hour, time.min, time.sec);
}
