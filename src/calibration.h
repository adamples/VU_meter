#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>

#define VALID_CALIBRATION (0xDEADU)

typedef struct calibration_data_t_ {
  uint16_t needle_zero;
  uint16_t needle_ref;
  uint16_t peak_zero;
  uint16_t peak_ref;
  uint16_t is_valid;
} calibration_data_t;

#define CALIBRATION_INITIALIZER { \
  .needle_zero = 247, \
  .needle_ref = 680, \
  .peak_zero = 247, \
  .peak_ref = 345, \
  .is_valid = VALID_CALIBRATION \
}

typedef struct calibration_t_ {
  calibration_data_t runtime;
  calibration_data_t *eeprom;
  bool eeprom_write_pending;
} calibration_t;


void calibration_hw_init();

void calibration_init(calibration_t *calibration, calibration_data_t *eeprom);
void calibration_run(calibration_t *calibration, uint16_t needle, uint16_t peak);
uint8_t calibration_adc_to_angle(calibration_t *calibration, uint16_t needle);
bool calibration_adc_to_peak(calibration_t *calibration, uint16_t peak);


#endif /* CALIBRATION_H */
