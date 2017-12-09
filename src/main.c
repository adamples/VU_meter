#include "i2c.h"
#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "config.h"
#include "assert.h"
#include "lcd.h"
#include "images.h"
#include "ssd1306.h"
#include "display.h"
#include "progmem_image_sprite.h"
#include "needle_sprite.h"
#include "benchmark.h"
#include "adc.h"
#include "fixed_point.h"

//~ typedef float fix_t;
//~ #define FIX_FROM_FLOAT(a) (a)
//~ #define FIX_MUL(a, b) ((a) * (b))
//~ #define FIX_DIV(a, b) ((a) / (b))


#define K1 FIX_FROM_FLOAT(0.010000)
#define K2 FIX_FROM_FLOAT(0.008000)
#define K3 FIX_FROM_FLOAT(0.250000)
#define I FIX_FROM_FLOAT(6.000000)
#define DT FIX_FROM_FLOAT(0.1)


int
main(void)
{
  lcd_init();
  sei();

  assert(K1 != 0);
  assert(K2 != 0);
  assert(K3 != 0);
  assert(I != 0);
  assert(DT != 0);

  volatile fix_t voltage = FIX_FROM_FLOAT(64);
  fix_t angle = FIX_FROM_FLOAT(0);
  fix_t force = FIX_FROM_FLOAT(0);
  fix_t velocity = FIX_FROM_FLOAT(0);

  BENCHMARK(Ballistics, {
    for (uint16_t i = 0; i < 40000; ++i) {
      force = FIX_MUL(voltage, K1) - FIX_MUL(angle, K2) - FIX_MUL(velocity, K3);
      velocity += FIX_MUL(force, FIX_DIV(DT, I));
      angle += FIX_MUL(velocity, DT);
      //~ angle = FIX_MUL(angle, 0.995) + FIX_MUL(voltage, 0.005);
    }
  });

  lcd_put_long(angle);
  force = velocity;
  velocity = force;
}
