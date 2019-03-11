#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "config.h"
#include "utils.h"
#include "assert.h"
#include "i2c.h"
#include "images.h"
#include "oled.h"
#include "display.h"
#include "progmem_image_sprite.h"
#include "needle_sprite.h"
#include "splash_sprite.h"
#include "benchmark.h"
#include "adc.h"
#include "calibration.h"


calibration_t CALIBRATION_L EEFIXED = {
  .needle_zero = 247,
  .needle_ref = 428,
  .peak_zero = 247,
  .peak_ref = 404
};

calibration_t CALIBRATION_R EEFIXED = {
  .needle_zero = 247,
  .needle_ref = 428,
  .peak_zero = 247,
  .peak_ref = 404
};


typedef struct vu_meter_t_ {
  display_t display;
  calibration_t calibration;
  calibration_t *calibration_eeprom;
  bool flipped;
  progmem_image_sprite_t background;
  needle_sprite_t needle;
  progmem_image_sprite_t peak_indicator;
  uint8_t peak_timer;

  #ifndef NOLOGO
    splash_sprite_t splash_sprite;
  #endif
} vu_meter_t;



static void
watchdog_init()
{
  MCUSR = 0;
#ifdef NDEBUG
  wdt_enable(WDTO_120MS);
#else
  wdt_disable();
#endif
}


#ifdef NDEBUG
  #define watchdog_reset() wdt_reset()
#else
  #define watchdog_reset() do { } while (0)
#endif


void
vu_meter_init(vu_meter_t *meter, int8_t address, calibration_t *calibration, bool flipped)
{
  meter->flipped = flipped;
  meter->calibration_eeprom = calibration;
  eeprom_read_block(&(meter->calibration), calibration, sizeof(calibration_t));

  display_init(&(meter->display), address);

  if (flipped) {
    progmem_image_sprite_init(&(meter->background), BACKGROUND_FLIPPED, 0, 0);
  }
  else {
    progmem_image_sprite_init(&(meter->background), BACKGROUND, 0, 0);
  }

  display_add_sprite(&(meter->display), &(meter->background.sprite));

  needle_sprite_init(&(meter->needle));
  needle_sprite_draw(&(meter->needle), 0);
  display_add_sprite(&(meter->display), &(meter->needle).sprite);

  progmem_image_sprite_init(&(meter->peak_indicator), PEAK_INDICATOR, 107, 7);
  display_add_sprite(&(meter->display), &(meter->peak_indicator).sprite);
  meter->peak_indicator.sprite.visible = false;

  #ifndef NOLOGO
    splash_sprite_init(&(meter->splash_sprite), SPLASH);
    display_add_sprite(&(meter->display), &(meter->splash_sprite.sprite));
  #endif
}


void
vu_meter_update(vu_meter_t *meter, uint16_t needle_level, uint16_t peak_level)
{
  #if INCLUDE_CALIBRATION
    calibration_run(&(meter->calibration), meter->calibration_eeprom, needle_level, peak_level);
  #endif

  uint8_t angle = calibration_adc_to_angle(&(meter->calibration), needle_level);
  bool peak = calibration_adc_to_peak(&(meter->calibration), peak_level);

  if (meter->flipped) {
    angle = 255 - angle;
  }

  if (peak) {
    meter->peak_timer = 10;
  }
  else if (meter->peak_timer != 0) {
    --(meter->peak_timer);
  }

  meter->peak_indicator.sprite.visible = (meter->peak_timer > 0);

  needle_sprite_draw(&(meter->needle), angle);

  #ifndef NOLOGO
    splash_sprite_advance(&(meter->splash_sprite));
  #endif

  display_update(&(meter->display));

  #ifndef NOLOGO
    if (splash_sprite_is_finished(&(meter->splash_sprite)) && meter->display.sprites_n == 4) {
      meter->display.sprites_n = 3;
    }
  #endif
}


vu_meter_t VU_METER_L;
vu_meter_t VU_METER_R;


int main(void)
{
  watchdog_init();
  i2c_init();
  adc_init();
  calibration_init();
  sei();

  watchdog_reset();

  vu_meter_init(&VU_METER_L, DISPLAY_A_ADDRESS, &CALIBRATION_L, true);
  vu_meter_init(&VU_METER_R, DISPLAY_B_ADDRESS, &CALIBRATION_R, false);

  adc_data_t adc_data;

  while (1) {
    watchdog_reset();

    adc_get(&adc_data);

    vu_meter_update(&VU_METER_L, adc_data.l_needle, adc_data.l_peak);
    vu_meter_update(&VU_METER_R, adc_data.r_needle, adc_data.r_peak);
  }

  while (1);
}
