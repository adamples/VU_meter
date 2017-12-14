#include "adc.h"

#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/atomic.h>


uint16_t
adc_get(uint8_t channel)
{
  ADMUX = 0;
  ADCSRA = 0;
  ADCSRB = 0;

  /* Input is selected based on channel */
  DDRC &= ~_BV(channel);
  ADMUX |= channel;

  /* Set clock to CLK/128 = 156.25kHz */
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

  /* Enable ADC and start continuous conversion */
  ADCSRA |= _BV(ADEN) | _BV(ADSC);

  while (ADCSRA & _BV(ADSC));

  uint16_t result = ADC;

  ADMUX = 0;
  ADCSRA = 0;
  ADCSRB = 0;

  return result;
}
