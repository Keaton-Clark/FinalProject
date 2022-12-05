#ifndef DS3231_H
#define DS3231_H
#include "io.h"

typedef	struct time_struct {
		uint8_t sec;
		uint8_t min;
		uint8_t hour;
		uint8_t weekday;
		uint8_t day;
		uint8_t month;
		uint8_t year;
} time_t;

time_t get_time(uint8_t addr);
void tsnprintf(char* str, size_t size, time_t time);

#endif //DS3231_H
