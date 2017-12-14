#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>
#include <util/atomic.h>
#include "config.h"


typedef bool (*i2c_callback_t)(void *data);


void i2c_init(void);

void i2c_transmit_async(uint8_t address, i2c_callback_t callback, void *data);
void i2c_async_send_byte(uint8_t data);
void i2c_async_send_bytes(uint8_t *data, uint8_t n);
void i2c_async_send_start(void);
void i2c_async_end_transmission(void);
bool i2c_is_idle(void);
void i2c_wait(void);

void i2c_transmit_progmem(uint8_t address, const uint8_t *data, uint16_t length);


#endif /* I2C_H */
