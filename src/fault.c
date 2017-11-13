#include "fault.h"
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>


void
fault(fault_code_t code, uint8_t extended_status, const char *error_text)
{
  cli();

  TWCR = 0;
  DDRD = 0xff;
  DDRB = 0xff;

  while (1) {
    PORTB = 0x00;
    PORTD = 0xff;
    _delay_ms(1000);

    PORTB = 0xff;
    PORTD = code;
    _delay_ms(1000);

    PORTB = 0x00;
    PORTD = extended_status;
    _delay_ms(1000);
  }
}
