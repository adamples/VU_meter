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


static uint8_t I2C_STATUS = TW_OK;

#define check_status() do { if (I2C_STATUS != TW_OK) goto finish; } while (0);


static void
i2c_start(void)
{
  i2c_hw_send_start_condition();
  i2c_hw_wait_for_int();
  I2C_STATUS = i2c_hw_get_status();
}


static void
i2c_finish(void)
{
  i2c_hw_send_stop_condition();
  i2c_hw_go_idle();
}


static void
i2c_send_byte(uint8_t data)
{
  i2c_hw_send_byte(data);
  i2c_hw_wait_for_int();
  I2C_STATUS = i2c_hw_get_status();
}


void i2c_init(void)
{
  i2c_hw_init();
}


uint8_t
i2c_wait(void)
{
  return I2C_STATUS;
}


void
i2c_transmit(uint16_t length, uint8_t *data)
{
  i2c_start();
  check_status()

  for (uint16_t i = 0; i < length; ++i) {
    i2c_send_byte(data[i]);
    check_status()
  }

  finish: i2c_finish();
}


uint8_t
i2c_transmit_progmem(uint8_t address, uint16_t length, const uint8_t *data)
{
  i2c_start();
  check_status();

  i2c_send_byte(address);
  check_status()

  for (uint16_t i = 0; i < length; ++i) {
    uint8_t byte = pgm_read_byte(data + i);
    i2c_send_byte(byte);
    check_status()
  }

  finish: i2c_finish();
  return I2C_STATUS;
}
