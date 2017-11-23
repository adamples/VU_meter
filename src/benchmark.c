#include "benchmark.h"
#include <stdbool.h>
#include <avr/interrupt.h>
#include <util/atomic.h>
#include "lcd.h"


#define SECONDS_PER_TICK (0.0002)

static volatile uint16_t TCNT1_ADD = 0;
static volatile bool initialized = false;


void benchmark_init(void)
{
  /* Timer/Counter1: Reset to initial state */
  TCCR1A = 0;
  TCCR1B = 0;
  TCCR1C = 0;
  TIMSK1 = 0;

  /* Timer/Counter2: Reset to initial state */
  TCCR2A = 0;
  TCCR2B = 0;
  TIMSK2 = 0;

  /* Timer/Counter2: Mode 7 (Fast PWM) */
  TCCR2A |= _BV(WGM21) | _BV(WGM20);
  TCCR2B |= _BV(WGM22);
  /* Timer/Counter2: TOP = OCR2A = 124 */
  OCR2A = 124;
  /* Timer/Counter2: Duty Cycle 50% */
  OCR2B = 62;
  /* Timer/Counter2: Clear OC2B on Compare Match, set OC2B at BOTTOM */
  TCCR2A |= _BV(COM2B1);
  DDRD |= _BV(PD3);
  /* Timer/Counter2: Clock Source = CLK/32 */
  TCCR2B |= _BV(CS21) | _BV(CS20);

  /* Timer/Counter1: Mode 0 (Normal) */
  TCCR1A |= 0;
  /* Timer/Counter1: Clock Source = T1 Rising Edge */
  TCCR1B |= _BV(CS12) | _BV(CS11) | _BV(CS10);
  /* Enable timer overflow interrupt */
  //~ TIMSK1 |= _BV(TOIE1);
}


time_t get_current_time(void)
{
  time_t result = 0;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    result = (time_t) TCNT1 * SECONDS_PER_TICK;
  }

  return result;
}


time_t benchmark_start(void)
{
  time_t result = 0;

  ATOMIC_BLOCK(ATOMIC_RESTORESTATE) {
    if (!initialized)
      benchmark_init();
    result = get_current_time();
  }

  return result;
}


void
benchmark_end(char *name, time_t start_time)
{
  time_t time = get_current_time() - start_time;

  lcd_clear();
  lcd_puts(name);
  lcd_puts(": ");
  lcd_goto(0, 1);
  lcd_put_long(time * 1000000.0);
  lcd_puts("us");
}


ISR(TIMER1_OVF_vect)
{
  ++TCNT1_ADD;
}
