#include "utils.h"
#include <avr/io.h>


void watchdog_init(void)
{
  MCUSR = 0;
#ifdef NDEBUG
  wdt_enable(WDTO_120MS);
#else
  wdt_disable();
#endif
}


static void delay_1ms(void)
{
  TCCR1A = 0x00;
  TCNT1 = 0x00;
  TCCR1B = _BV(CS10);
  while (TCNT1 < F_CPU / 1000);
  TCCR1B = 0x00;
}


void delay_ms(uint16_t t)
{
  for (int16_t i = 0; i < t; ++i) {
    watchdog_reset();
    delay_1ms();
  }
}
