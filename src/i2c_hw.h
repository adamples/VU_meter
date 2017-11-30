#ifndef I2C_HW_H
#define I2C_HW_H

#include <stdint.h>
#include "config.h"


inline void
i2c_hw_init(void)
{
  TWSR = 0x00;
  TWCR = 0x00;
  TWBR = ((F_CPU / I2C_CLOCK) - 16) / 2;
}


inline void
i2c_hw_wait(void)
{
  while (!(TWCR & _BV(TWINT)));
}


inline void
i2c_hw_send_start_condition(void)
{
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE) | _BV(TWSTA);
}


inline void
i2c_hw_send_stop_condition(void)
{
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWSTO);
  while (TWCR & _BV(TWSTO));
}


inline void
i2c_hw_send_byte(uint8_t octet)
{
  TWDR = octet;
  TWCR = _BV(TWINT) | _BV(TWEN) | _BV(TWIE);
}


inline void
i2c_hw_go_idle(void)
{
  TWCR = 0;
}


inline void
i2c_hw_disable_interrupt(void)
{
  /* Clear InterruptEnable bit, but don't write INT bit, so it doesn't get cleared */
  TWCR &= ~_BV(TWIE) & ~_BV(TWINT);
}


#endif /* I2C_HW_H */
