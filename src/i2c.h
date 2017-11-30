#ifndef I2C_H
#define I2C_H

#include <stdbool.h>
#include <stdint.h>
#include <util/atomic.h>
#include "config.h"


typedef enum i2c_command_code_t_ {
  I2C_COMMAND_START = 0x01,
  I2C_COMMAND_SEND_DATA = 0x02,
  I2C_COMMAND_STOP = 0x03,
  I2C_COMMAND_PENDING = 0x04
} i2c_command_code_t;


typedef struct i2c_command_t_ {
  i2c_command_code_t code;
  uint8_t data;
} i2c_command_t;


typedef struct i2c_command_buffer_t_ {
  uint8_t length;
  i2c_command_t commands[I2C_BUFFER_SIZE];
} i2c_command_buffer_t;


typedef bool (*i2c_callback_t)(i2c_command_buffer_t *commands, void *data);


void i2c_init(void);

void i2c_transmit_async(uint8_t address, i2c_callback_t callback, void *data);
void i2c_async_send_data(uint8_t data);
void i2c_async_send_start(void);
void i2c_async_end_transmission(void);
bool i2c_is_idle(void);


inline void
i2c_command_buffer_append_start(i2c_command_buffer_t *buffer)
{
  buffer->commands[buffer->length].code = I2C_COMMAND_START;
  ++buffer->length;
}

inline void
i2c_command_buffer_append_send_data(i2c_command_buffer_t *buffer, uint8_t data)
{
  buffer->commands[buffer->length].code = I2C_COMMAND_SEND_DATA;
  buffer->commands[buffer->length].data = data;
  ++buffer->length;
}

inline void
i2c_command_buffer_append_stop(i2c_command_buffer_t *buffer)
{
  buffer->commands[buffer->length].code = I2C_COMMAND_STOP;
  ++buffer->length;
}

#endif /* I2C_H */
