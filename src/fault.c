#include "fault.h"
#include <string.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "lcd.h"


void
lcd_fault(fault_code_t code, uint16_t extended_status, const char *error_text)
{
  cli();

  lcd_init();
  lcd_puts("F: ");

  switch (code) {
    case FAULT_I2C: lcd_puts("I2C"); break;
    case FAULT_ASSERTION_FAILED: lcd_puts("ASSERT"); break;
  }

  lcd_putc('/');
  lcd_put_int(extended_status);

  int text_len = strlen(error_text);

  TWCR = 0;
  DDRD = 0xff;
  PORTD = 0x00;

  if (error_text == NULL) {
    lcd_goto(0, 1);
    lcd_puts("No error text");

    while (1) {
      PORTD = ~PORTD;
      _delay_ms(500);
    }
  }

  while (1) {
    lcd_goto(0, 1);
    lcd_puts((char *) error_text);
    PORTD = ~PORTD;
    _delay_ms(2000);

    for (int i = 1; i <= text_len - 16; ++i) {
      lcd_goto(0, 1);
      lcd_puts((char *) error_text + i);
      lcd_putc(' ');
      PORTD = ~PORTD;
      _delay_ms(1000);
    }

    PORTD = ~PORTD;
    _delay_ms(1000);
  }
}
