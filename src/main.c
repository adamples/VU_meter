#include <stdlib.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <util/delay.h>
#include "config.h"
#include "assert.h"
#include "i2c.h"
#include "lcd.h"
#include "images.h"
#include "ssd1306.h"
#include "display.h"
#include "progmem_image_sprite.h"
#include "needle_sprite.h"
#include "benchmark.h"
#include "adc.h"


typedef struct vu_meter_t_ {
  ssd1306_t device;
  display_t display;
  needle_sprite_t needle;
  region_t update_regions[18];
  update_extents_t update_extents;
} vu_meter_t;


progmem_image_sprite_t BACKGROUND_SPRITE;
progmem_image_sprite_t PEAK_INDICATOR_SPRITE;
vu_meter_t VU_METER_L;
vu_meter_t VU_METER_R;


void
vu_meter_init(vu_meter_t *meter, int8_t address)
{
  meter->update_extents.regions = meter->update_regions;

  ssd1306_init(&(meter->device), address);
  display_init(&(meter->display), &(meter->device));

  display_add_sprite(&(meter->display), &(BACKGROUND_SPRITE.sprite));
  display_add_sprite(&(meter->display), &(PEAK_INDICATOR_SPRITE.sprite));

  needle_sprite_init(&(meter->needle));
  needle_sprite_draw(&(meter->needle), 0);
  display_add_sprite(&(meter->display), &(meter->needle).sprite);

  display_update_async(&(meter->display));
}


void
vu_meter_update(vu_meter_t *meter, uint8_t angle)
{
  update_extents_reset(&(meter->update_extents));
  needle_sprite_add_to_extents(&(meter->needle), &(meter->update_extents));

  needle_sprite_draw(&(meter->needle), angle);
  needle_sprite_add_to_extents(&(meter->needle), &(meter->update_extents));
  progmem_image_sprite_add_to_extents(&PEAK_INDICATOR_SPRITE, &(meter->update_extents));
  update_extents_optimize(&(meter->update_extents));

  display_update_partial_async(&(meter->display), &(meter->update_extents));
}


static uint8_t
percent_to_angle(int percent)
{
  return percent * 181 / 100;
}


int main(void)
{
  lcd_init();
  i2c_init();
  sei();

  progmem_image_sprite_init(&BACKGROUND_SPRITE, BACKGROUND, 0, 0);
  progmem_image_sprite_init(&PEAK_INDICATOR_SPRITE, PEAK_INDICATOR, 107, 7);

  vu_meter_init(&VU_METER_L, DISPLAY_A_ADDRESS);
  vu_meter_init(&VU_METER_R, DISPLAY_B_ADDRESS);
  i2c_wait();

  benchmark_start();

  while (1) {
    uint16_t adc_l = adc_get(2);
    int16_t angle_l = (int32_t) (498 - adc_l) * 128 / 164;
    if (angle_l < 0) angle_l = -angle_l;
    if (angle_l > 255) angle_l = 255;

    i2c_wait();
    time_t frame_start = get_current_time();
    vu_meter_update(&VU_METER_L, angle_l);
    i2c_wait();
    time_t frame_time = get_current_time() - frame_start;
    int16_t fps = (int32_t) 1000000 / frame_time;

    PEAK_INDICATOR_SPRITE.sprite.visible = (angle_l > 192);

    vu_meter_update(&VU_METER_R, percent_to_angle(fps / 2));
  }
}
