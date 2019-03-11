#include "adc.h"

#include <stdlib.h>
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "config.h"
#include "utils.h"


void
adc_init(void)
{
  ADMUX = 0;
  ADCSRA = 0;
  ADCSRB = 0;

  /* Disable digital input buffers */
  DIDR0 |= _BV(ADC_CHANNEL_L_NEEDLE) | _BV(ADC_CHANNEL_L_PEAK)
    | _BV(ADC_CHANNEL_R_NEEDLE) | _BV(ADC_CHANNEL_R_PEAK);

  /* Set clock to CLK / 128:
   *   for CLK = 20Mhz: 156.25kHz
   *   for CLK = 16Mhz: 125.00kHz */
  ADCSRA |= _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0);

  /* Enable ADC */
  ADCSRA |= _BV(ADEN);
}


static uint16_t
adc_sample(uint8_t channel)
{
  ADMUX = channel;
  ADCSRA |= _BV(ADSC);

  while (!(ADCSRA & _BV(ADIF)));

  uint16_t result = ADC;

  ADCSRA |= _BV(ADIF);
  TIFR1 |= _BV(ICF1);

  return 1023 - result;
}


void
adc_get(adc_data_t *result)
{
  result->l_needle = adc_sample(ADC_CHANNEL_L_NEEDLE);
  result->l_peak = adc_sample(ADC_CHANNEL_L_PEAK);
  result->r_needle = adc_sample(ADC_CHANNEL_R_NEEDLE);
  result->r_peak = adc_sample(ADC_CHANNEL_R_PEAK);
}
