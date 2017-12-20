#include "i2c.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/twi.h>
#include "config.h"
#include "fault.h"
#include "assert.h"
#include "ring_buffer.h"
#include "i2c_hw.h"


static uint8_t I2C_CURRENT_ADDRESS;


void i2c_init(void)
{
  i2c_hw_init();
}


bool
i2c_is_idle(void)
{
  return true;
}


void
i2c_wait(void)
{
  /* Nothing to do here */
}


void
i2c_transmit_async(uint8_t address, i2c_callback_t callback, void *data)
{
  I2C_CURRENT_ADDRESS = address;
  while (callback(data));
}


void
i2c_check_status()
{
  uint8_t i2c_status = TWSR & TW_STATUS_MASK;

  if (i2c_status > TW_MT_DATA_ACK || i2c_status == TW_MT_SLA_NACK) {
    fault(FAULT_I2C, i2c_status, "Error status");
  }
}


void
i2c_async_send_byte(uint8_t data)
{
  i2c_hw_send_byte(data);
  i2c_hw_wait_for_int();
  i2c_check_status();
}


void
i2c_async_send_bytes(uint8_t *data, uint8_t n)
{
  assert(n > 0);

  for (uint8_t i = 0; i < n; ++i) {
    i2c_async_send_byte(data[i]);
  }
}


void
i2c_async_send_start(void)
{
  i2c_hw_send_start_condition();
  i2c_hw_wait_for_int();
  i2c_check_status();
  i2c_async_send_byte(I2C_CURRENT_ADDRESS);
}


void
i2c_async_end_transmission(void)
{
  i2c_hw_send_stop_condition();
}


void
i2c_transmit_progmem(uint8_t address, const uint8_t *data, uint16_t length)
{
  I2C_CURRENT_ADDRESS = address;
  i2c_async_send_start();

  for (uint16_t i = 0; i < length; ++i) {
    uint8_t byte = pgm_read_byte(data + i);
    i2c_async_send_byte(byte);
  }

  i2c_async_end_transmission();
}
