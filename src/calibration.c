#include "calibration.h"
#include <avr/eeprom.h>
#include <avr/io.h>
#include "config.h"


#define ZERO_CALIBRATION_ACTIVE() ((CALIBRATION_ZERO_PIN & _BV(CALIBRATION_ZERO_P)) == 0)
#define REF_CALIBRATION_ACTIVE() ((CALIBRATION_REF_PIN & _BV(CALIBRATION_REF_P)) == 0)

static bool EEPROM_SAVE_PENDING = false;


void
calibration_init()
{
  /* Enable pull-ups */
  CALIBRATION_ZERO_PORT |= _BV(CALIBRATION_ZERO_P);
  CALIBRATION_REF_PORT |= _BV(CALIBRATION_REF_P);
}


static bool
calibration_is_ref_too_low(calibration_t *calibration)
{
  return (calibration->needle_ref < calibration->needle_zero + 400);
}


static bool
calibration_is_ref_too_high(calibration_t *calibration)
{
  return (calibration->needle_ref > calibration->needle_zero + 500);
}


void
calibration_run(calibration_t *calibration, calibration_t *eeprom, uint16_t needle, uint16_t peak)
{
  if (ZERO_CALIBRATION_ACTIVE()) {
    /* Run zero calibration */
    calibration->needle_zero = (calibration->needle_zero + needle) / 2;
    calibration->peak_zero = (calibration->peak_zero + peak) / 2;
    EEPROM_SAVE_PENDING = true;
  }
  else if (REF_CALIBRATION_ACTIVE()) {
    /* Run reference point calibration */
    calibration->needle_ref = (calibration->needle_ref + needle) / 2;
    calibration->peak_ref = (calibration->peak_ref + peak) / 2;
    EEPROM_SAVE_PENDING = true;
  }
  else if (EEPROM_SAVE_PENDING) {
    eeprom_update_block(calibration, eeprom, sizeof(calibration_t));
    EEPROM_SAVE_PENDING = false;
  }
}


uint8_t
calibration_adc_to_angle(calibration_t *calibration, uint16_t needle)
{
  if (REF_CALIBRATION_ACTIVE()) {
    if (calibration_is_ref_too_low(calibration)) {
      return 32;
    }
    else if (calibration_is_ref_too_high(calibration)) {
      return 224;
    }

    return 128;
  }

  int16_t normalized = ((int32_t) needle - calibration->needle_zero) *
    ZERO_VU_ANGLE / (calibration->needle_ref - calibration->needle_zero);

  if (normalized < 0) return 0;
  if (normalized > 255) return 255;

  return normalized;
}


bool
calibration_adc_to_peak(calibration_t *calibration, uint16_t peak)
{
  int16_t normalized = peak - calibration->peak_zero;
  int16_t limit = (calibration->peak_ref - calibration->peak_zero) * PEAK_LEVEL;
  return (normalized >= limit);
}
