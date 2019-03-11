#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>
#include "i2c_hw.h"


void i2c_init(void);
void i2c_transmit(uint16_t length, uint8_t *data);
uint8_t i2c_transmit_progmem(uint8_t address, uint16_t length, const uint8_t *data);
uint8_t i2c_wait(void);


#endif /* I2C_H */
