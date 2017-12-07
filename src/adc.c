#include "adc.h"

#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/atomic.h>


static volatile uint16_t ADC_OUTPUT = 0;


void
adc_init(void)
{
  /* Reference is AREF (external) */
  ADMUX = 0; // _BV(REFS0);

  /* Input is ADC2 */
  ADMUX |= _BV(MUX1);

  /* Set clock to CLK/128 = 156.25kHz */
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

  /* Auto-trigger from ADIF flag: free-running mode */
  ADCSRB |= 0;
  ADCSRA |= _BV(ADATE);

  /* Enable ADC interrupt */
  ADCSRA |= _BV(ADIE);

  /* Enable ADC and start continuous conversion */
  ADCSRA |= _BV(ADEN) | _BV(ADSC);
}


uint16_t
adc_get(void)
{
  uint16_t result = 0;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    result = ADC_OUTPUT;
  }

  return result;
}


ISR(ADC_vect)
{
  ADC_OUTPUT = ADC;
}
