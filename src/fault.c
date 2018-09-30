#include "fault.h"
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>

#ifndef NDEBUG
#include <util/delay.h>

static fault_code_t FAULT_CODE EEMEM;
static uint16_t FAULT_EXTENDED_STATUS EEMEM;
static char FAULT_ERROR_TEXT[32] EEMEM;

void
debug_fault(fault_code_t code, uint16_t extended_status, const char *error_text)
{
  cli();

  eeprom_update_byte(&FAULT_CODE, code);
  eeprom_update_word(&FAULT_EXTENDED_STATUS, extended_status);

  if (error_text != NULL) {
    eeprom_update_block(error_text, FAULT_ERROR_TEXT, strlen(error_text));
  }

  eeprom_busy_wait();

  DDRB |= _BV(PB5);

  while (1) {
    PORTB |= _BV(PB5);
    _delay_ms(500);
    PORTB &= ~_BV(PB5);
    _delay_ms(500);
  }
}

#endif


void
release_fault(fault_code_t code, uint16_t extended_status, const char *error_text)
{
  cli();
  wdt_enable(WDTO_15MS);
  while (1);
}
