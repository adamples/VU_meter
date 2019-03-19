#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>


typedef struct calibration_data_t_ {
  uint16_t needle_zero;
  uint16_t needle_ref;
  uint16_t peak_zero;
  uint16_t peak_ref;
} calibration_data_t;

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
