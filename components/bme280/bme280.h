#ifndef BME280_H
#define BME280_H

#include <stdint.h>

void bme280_init(void);

float bme280_read_temperature(void);

float bme280_read_pressure(void);

#endif