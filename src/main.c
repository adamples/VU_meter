#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include <avr/wdt.h>
#include "config.h"
#include "assert.h"
#include "i2c.h"
#include "images.h"
#include "oled.h"
#include "display.h"
#include "progmem_image_sprite.h"
#include "needle_sprite.h"
#include "benchmark.h"
#include "adc.h"
#include "calibration.h"


#ifndef NOLOGO
const uint8_t WATERMARK[34] PROGMEM = {
  0x20, /* = width */
  0x08, /* = height */
  0x60, 0x48, 0x78, 0x40, 0x78, 0x48, 0x7c, 0x40, 0x60, 0x48, 0x78, 0x40, 0x78, 0x08, 0x78, 0x08,
  0x78, 0x40, 0x00, 0xf8, 0x48, 0x78, 0x00, 0x7c, 0x40, 0x78, 0x48, 0x58, 0x40, 0x58, 0x48, 0x68,
};
#endif

calibration_t CALIBRATION_L EEMEM;
calibration_t CALIBRATION_R EEMEM;


typedef struct vu_meter_t_ {
  oled_t device;
  display_t display;
  calibration_t calibration;
  calibration_t *calibration_eeprom;
  needle_sprite_t needle;
  progmem_image_sprite_t peak_indicator;
  region_t update_regions[18];
  update_extents_t update_extents;
  uint8_t peak_timer;
} vu_meter_t;


progmem_image_sprite_t BACKGROUND_SPRITE;
#ifndef NOLOGO
progmem_image_sprite_t WATERMARK_SPRITE;
#endif
vu_meter_t VU_METER_L;
vu_meter_t VU_METER_R;


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
vu_meter_init(vu_meter_t *meter, int8_t address, calibration_t *calibration)
{
  meter->update_extents.regions = meter->update_regions;

  meter->calibration_eeprom = calibration;
  eeprom_read_block(&(meter->calibration), calibration, sizeof(calibration_t));

  oled_init(&(meter->device), address);
  display_init(&(meter->display), &(meter->device));

  display_add_sprite(&(meter->display), &(BACKGROUND_SPRITE.sprite));

  #ifndef NOLOGO
  display_add_sprite(&(meter->display), &(WATERMARK_SPRITE.sprite));
  #endif

  needle_sprite_init(&(meter->needle));
  needle_sprite_draw(&(meter->needle), 0);
  display_add_sprite(&(meter->display), &(meter->needle).sprite);

  progmem_image_sprite_init(&(meter->peak_indicator), PEAK_INDICATOR, 107, 7);
  display_add_sprite(&(meter->display), &(meter->peak_indicator).sprite);
  meter->peak_indicator.sprite.visible = false;

  display_update_async(&(meter->display));
}


void
vu_meter_update(vu_meter_t *meter, uint16_t needle_level, uint16_t peak_level)
{
  calibration_run(&(meter->calibration), meter->calibration_eeprom, needle_level, peak_level);

  uint8_t angle = calibration_adc_to_angle(&(meter->calibration), needle_level);
  bool peak = calibration_adc_to_peak(&(meter->calibration), peak_level);

  update_extents_reset(&(meter->update_extents));
  needle_sprite_add_to_extents(&(meter->needle), &(meter->update_extents));

  if (peak) {
    meter->peak_timer = 10;
  }
  else if (meter->peak_timer != 0) {
    --(meter->peak_timer);
  }

  meter->peak_indicator.sprite.visible = (meter->peak_timer > 0);

  needle_sprite_draw(&(meter->needle), angle);
  needle_sprite_add_to_extents(&(meter->needle), &(meter->update_extents));
  progmem_image_sprite_add_to_extents(&(meter->peak_indicator), &(meter->update_extents));
  update_extents_optimize(&(meter->update_extents));

  display_update_partial_async(&(meter->display), &(meter->update_extents));
}


int main(void)
{
  watchdog_init();
  i2c_init();
  adc_init();
  calibration_init();
  sei();

  progmem_image_sprite_init(&BACKGROUND_SPRITE, BACKGROUND, 0, 0);
  #ifndef NOLOGO
  progmem_image_sprite_init(&WATERMARK_SPRITE, WATERMARK, 0, 7);
  #endif

  watchdog_reset();

  vu_meter_init(&VU_METER_L, DISPLAY_A_ADDRESS, &CALIBRATION_L);
  i2c_wait();
  vu_meter_init(&VU_METER_R, DISPLAY_B_ADDRESS, &CALIBRATION_R);
  i2c_wait();

  adc_data_t adc_data;

  while (1) {
    watchdog_reset();

    adc_get(&adc_data);

    i2c_wait();
    vu_meter_update(&VU_METER_L, adc_data.l_needle, adc_data.l_peak);
    i2c_wait();
    vu_meter_update(&VU_METER_R, adc_data.r_needle, adc_data.r_peak);
  }
}
