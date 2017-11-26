#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>


#define I2C_CB_FLAG_START (0x0100)
#define I2C_CB_FLAG_EOT (0x0200)


typedef void (*i2c_callback_t)(void *data);


void i2c_init(void);

void i2c_transmit_async(uint8_t address, i2c_callback_t callback, void *data);
void i2c_async_send_data(uint8_t data);
void i2c_async_send_start(void);
void i2c_async_end_transmission(void);
bool i2c_is_idle(void);
void i2c_set_idle(void);


#endif /* I2C_H */
