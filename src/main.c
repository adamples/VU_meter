#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <avr/eeprom.h>
#include "config.h"
#include "utils.h"
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


calibration_data_t CALIBRATION_LEFT EEFIXED = CALIBRATION_INITIALIZER;
calibration_data_t CALIBRATION_RIGHT EEFIXED = CALIBRATION_INITIALIZER;


typedef struct vu_meter_t_ {
  display_t display;
  calibration_t calibration;
  bool flipped;
  progmem_image_sprite_t background;
  needle_sprite_t needle;
  progmem_image_sprite_t peak_indicator;
#if ENABLE_WATERMARK
  progmem_image_sprite_t watermark;
#endif
  uint8_t peak_timer;
} vu_meter_t;


void
vu_meter_init(vu_meter_t *meter,
              int8_t address,
              calibration_data_t *calibration_eeprom,
              bool flipped,
              const uint8_t *background_image,
              const uint8_t *peak_indicator_image
#if ENABLE_WATERMARK
              , const uint8_t *watermark_image
#endif
              )
{
  meter->flipped = flipped;

  calibration_init(&(meter->calibration), calibration_eeprom);

  display_init(&(meter->display), address);
  progmem_image_sprite_init(&(meter->background), background_image, 0, 0);

  display_add_sprite(&(meter->display), &(meter->background.sprite));

  needle_sprite_init(&(meter->needle));
  needle_sprite_draw(&(meter->needle), 0);
  display_add_sprite(&(meter->display), &(meter->needle.sprite));

  progmem_image_sprite_init(
    &(meter->peak_indicator),
    peak_indicator_image,
    PEAK_POSITION_X,
    PEAK_POSITION_Y
  );

  display_add_sprite(&(meter->display), &(meter->peak_indicator.sprite));
  meter->peak_indicator.sprite.visible = false;

#if ENABLE_WATERMARK
  progmem_image_sprite_init(
    &(meter->watermark),
    watermark_image,
    WATERMARK_POSITION_X,
    WATERMARK_POSITION_Y
  );

  display_add_sprite(&(meter->display), &(meter->watermark.sprite));
#endif
}


void
vu_meter_update(vu_meter_t *meter, uint16_t needle_level, uint16_t peak_level)
{
  #if INCLUDE_CALIBRATION
    calibration_run(
      &(meter->calibration),
      needle_level,
      peak_level
    );
  #endif

  uint8_t angle = calibration_adc_to_angle(&(meter->calibration), needle_level);
  bool peak;

#if PEAK_IS_ZERO_VU
  peak = (angle > NEEDLE_ZERO_VU_ANGLE);
#else
  peak = calibration_adc_to_peak(&(meter->calibration), peak_level);
#endif

  if (meter->flipped) {
    angle = 255 - angle;
  }

  if (peak) {
    meter->peak_timer = 10;
  }
  else if (meter->peak_timer > 0) {
    --(meter->peak_timer);
  }

  bool new_visible = (meter->peak_timer > 0);
  meter->peak_indicator.sprite.changed = (meter->peak_indicator.sprite.visible != new_visible);
  meter->peak_indicator.sprite.visible = new_visible;

  needle_sprite_draw(&(meter->needle), angle);
  display_update(&(meter->display));
}


void vu_meter_splash(vu_meter_t *meter, const uint8_t *image)
{
  const uint8_t *background_tmp = meter->background.data;
  uint8_t sprites_n = meter->display.sprites_n;

  progmem_image_sprite_init(&(meter->background), image, 0, 0);
  meter->display.sprites_n = 1;

  display_force_full_update(&(meter->display));
  display_update(&(meter->display));

  progmem_image_sprite_init(&(meter->background), background_tmp, 0, 0);
  meter->display.sprites_n = sprites_n;
}


vu_meter_t VU_METER_L;
vu_meter_t VU_METER_R;


int main(void)
{
  watchdog_init();
  i2c_init();
  adc_init();
  calibration_hw_init();
#if OLED_INCLUDE_RESET
  oled_reset();
#endif
  sei();

  watchdog_reset();

  vu_meter_init(
    &VU_METER_L,
    DISPLAY_LEFT_ADDRESS,
    &CALIBRATION_LEFT,
    DISPLAY_LEFT_FLIPPED,
    DISPLAY_LEFT_BACKGROUND,
    DISPLAY_LEFT_PEAK_INDICATOR
#if ENABLE_WATERMARK
    , DISPLAY_LEFT_WATERMARK
#endif
  );

  vu_meter_init(
    &VU_METER_R,
    DISPLAY_RIGHT_ADDRESS,
    &CALIBRATION_RIGHT,
    DISPLAY_RIGHT_FLIPPED,
    DISPLAY_RIGHT_BACKGROUND,
    DISPLAY_RIGHT_PEAK_INDICATOR
#if ENABLE_WATERMARK
    , DISPLAY_RIGHT_WATERMARK
#endif
  );

  #if ENABLE_SPLASH_SCREEN
    vu_meter_splash(&VU_METER_L, DISPLAY_LEFT_SPLASH);
    vu_meter_splash(&VU_METER_R, DISPLAY_RIGHT_SPLASH);

    oled_set_display_on(&(VU_METER_L.display.device), true);
    oled_set_display_on(&(VU_METER_R.display.device), true);

    delay_ms(SPLASH_SCREEN_TIME_MS);

    oled_set_display_on(&(VU_METER_L.display.device), false);
    oled_set_display_on(&(VU_METER_R.display.device), false);
  #endif

  bool is_on = false;
  adc_data_t adc_data;

  for (int i = 0; true; ++i) {
    watchdog_reset();

    adc_get(&adc_data);

    if (i % 10 == 0) {
      adc_reset_peak(true, true);
      adc_reset_peak(false, false);
    }

    vu_meter_update(&VU_METER_L, adc_data.l_needle, adc_data.l_peak);
    vu_meter_update(&VU_METER_R, adc_data.r_needle, adc_data.r_peak);

    if (!is_on && i > BLANK_TIME_FRAMES) {
      oled_set_display_on(&(VU_METER_L.display.device), true);
      oled_set_display_on(&(VU_METER_R.display.device), true);
      is_on = true;
    }
  }

  while (1);
}
