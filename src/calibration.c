#include "calibration.h"
#include <avr/eeprom.h>
#include <avr/io.h>
#include "config.h"


#define ZERO_CALIBRATION_ACTIVE() ((CALIBRATION_ZERO_PIN & _BV(CALIBRATION_ZERO_P)) == 0)
#define REF_CALIBRATION_ACTIVE() ((CALIBRATION_REF_PIN & _BV(CALIBRATION_REF_P)) == 0)

#if OLED_INCLUDE_RESET
#define FACTORY_RESET_ACTIVE() (false)
#else
#define FACTORY_RESET_ACTIVE() ((CALIBRATION_RESET_PIN & _BV(CALIBRATION_RESET_P)) == 0)
#endif

void
calibration_hw_init()
{
  /* Enable pull-ups */
  CALIBRATION_ZERO_PORT |= _BV(CALIBRATION_ZERO_P);
  CALIBRATION_REF_PORT |= _BV(CALIBRATION_REF_P);
#if !OLED_INCLUDE_RESET
  CALIBRATION_RESET_PORT |= _BV(CALIBRATION_RESET_P);
#endif
}


void
calibration_init(calibration_t *calibration, calibration_data_t *eeprom)
{
  const calibration_data_t FACTORY_CALIBRATION = CALIBRATION_INITIALIZER;

  calibration->eeprom = eeprom;
  calibration->runtime = FACTORY_CALIBRATION;
  calibration->eeprom_write_pending = false;

#if INCLUDE_CALIBRATION
  if (FACTORY_RESET_ACTIVE())
  {
    calibration->eeprom_write_pending = true;
  }
  else
  {
    eeprom_read_block(
      &(calibration->runtime),
      calibration->eeprom,
      sizeof(calibration_data_t)
    );

    // Check if calibration data read from eeprom is valid
    if (calibration->runtime.is_valid != VALID_CALIBRATION)
    {
      calibration->runtime = FACTORY_CALIBRATION;
      calibration->eeprom_write_pending = true;
    }
  }
#endif
}


#if INCLUDE_CALIBRATION

static bool
calibration_is_ref_too_low(calibration_t *calibration)
{
  return (calibration->runtime.needle_ref < calibration->runtime.needle_zero + 400);
}


static bool
calibration_is_ref_too_high(calibration_t *calibration)
{
  return (calibration->runtime.needle_ref > calibration->runtime.needle_zero + 500);
}


void
calibration_run(calibration_t *calibration, uint16_t needle, uint16_t peak)
{
  if (ZERO_CALIBRATION_ACTIVE())
  {
    /* Run zero calibration */
    calibration->runtime.needle_zero = (calibration->runtime.needle_zero + needle) / 2;
    calibration->runtime.peak_zero = (calibration->runtime.peak_zero + peak) / 2;
    calibration->eeprom_write_pending = true;
  }
  else if (REF_CALIBRATION_ACTIVE())
  {
    /* Run reference point calibration */
    calibration->runtime.needle_ref = (calibration->runtime.needle_ref + needle) / 2;
    calibration->runtime.peak_ref = (calibration->runtime.peak_ref + peak) / 2;
    calibration->eeprom_write_pending = true;
  }
  else if (calibration->eeprom_write_pending)
  {
    eeprom_update_block(
      &(calibration->runtime),
      calibration->eeprom,
      sizeof(calibration_data_t)
    );

    calibration->eeprom_write_pending = false;
  }
}

#endif


uint8_t
calibration_adc_to_angle(calibration_t *calibration, uint16_t needle)
{
#if INCLUDE_CALIBRATION
  if (REF_CALIBRATION_ACTIVE()) {
    if (calibration_is_ref_too_low(calibration)) {
      return 32;
    }
    else if (calibration_is_ref_too_high(calibration)) {
      return 224;
    }

    return 128;
  }
#endif

  int16_t normalized = ((int32_t) needle - calibration->runtime.needle_zero) *
    (NEEDLE_ZERO_VU_ANGLE - NEEDLE_MIN_ANGLE) / (calibration->runtime.needle_ref - calibration->runtime.needle_zero) + NEEDLE_MIN_ANGLE;

  if (normalized < NEEDLE_MIN_ANGLE) return NEEDLE_MIN_ANGLE;
  if (normalized > NEEDLE_MAX_ANGLE) return NEEDLE_MAX_ANGLE;

  return normalized;
}


bool
calibration_adc_to_peak(calibration_t *calibration, uint16_t peak)
{
  int16_t normalized = peak - calibration->runtime.peak_zero;
  int16_t limit = (calibration->runtime.peak_ref - calibration->runtime.peak_zero) * PEAK_LEVEL;
  return (normalized >= limit);
}
