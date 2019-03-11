#include "i2c.h"
#include <stdlib.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/atomic.h>
#include <util/twi.h>
#include "config.h"
#include "utils.h"
#include "fault.h"
#include "assert.h"
#include "ring_buffer.h"
#include "i2c_hw.h"


typedef struct i2c_ctrl_t_ {
  uint16_t data_length;
  uint16_t data_cursor;
  uint8_t *data;
  bool active;
  uint8_t status;
} i2c_ctrl_t;


static i2c_ctrl_t I2C_CTRL;


static void
i2c_finish()
{
  i2c_hw_send_stop_condition();
  i2c_hw_go_idle();
  I2C_CTRL.active = false;
}


ISR(TWI_vect) {
  I2C_CTRL.status = i2c_hw_get_status();

  if (I2C_CTRL.status != TW_OK) {
    i2c_finish();
    return;
  }
  else if (I2C_CTRL.data_cursor == I2C_CTRL.data_length) {
    i2c_finish();
    return;
  }

  i2c_hw_send_byte_int(I2C_CTRL.data[I2C_CTRL.data_cursor]);
  ++I2C_CTRL.data_cursor;
}


/* User API ----------------------------------------------------------------- */

void i2c_init(void)
{
  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    I2C_CTRL.active = false;
  }

  i2c_hw_init();
}


uint8_t
i2c_wait(void)
{
  while (1) {
    MEMORY_BARRIER();

    if (!I2C_CTRL.active) {
      return I2C_CTRL.status;
    }
  }
}


void
i2c_transmit(uint16_t length, uint8_t *data)
{
  i2c_wait();

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    I2C_CTRL.data_length = length;
    I2C_CTRL.data = data;
    I2C_CTRL.data_cursor = 0;
    I2C_CTRL.status = TW_OK;
    I2C_CTRL.active = true;
  }

  i2c_hw_send_start_condition_int();
}


uint8_t
i2c_transmit_progmem(uint8_t address, uint16_t length, const uint8_t *data)
{
  uint8_t status = 0;

  i2c_wait();
  i2c_hw_send_start_condition();
  i2c_hw_wait_for_int();
  status = i2c_hw_get_status();

  if (status != TW_OK) {
    i2c_finish();
    return status;
  }

  i2c_hw_send_byte(address);
  i2c_hw_wait_for_int();
  status = i2c_hw_get_status();

  for (uint16_t i = 0; (i < length) && (status == TW_OK); ++i) {
    uint8_t byte = pgm_read_byte(data + i);
    i2c_hw_send_byte(byte);
    i2c_hw_wait_for_int();
    status = i2c_hw_get_status();
  }

  i2c_finish();
  return status;
}

/* end of User API ---------------------------------------------------------- */
