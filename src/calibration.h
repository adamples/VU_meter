#ifndef CALIBRATION_H
#define CALIBRATION_H

#include <stdbool.h>
#include <stdint.h>


typedef struct calibration_t_ {
  uint16_t needle_zero;
  uint16_t needle_ref;
  uint16_t peak_zero;
  uint16_t peak_ref;
} calibration_t;


void calibration_init();
void calibration_run(calibration_t *calibration, calibration_t *eeprom, uint16_t needle, uint16_t peak);
uint8_t calibration_adc_to_angle(calibration_t *calibration, uint16_t needle);
bool calibration_adc_to_peak(calibration_t *calibration, uint16_t peak);


#endif /* CALIBRATION_H */
