#ifndef FAULT_H
#define FAULT_H

#include <stdint.h>


typedef enum fault_code_t_ {
  FAULT_I2C = 0x01,
  FAULT_ASSERTION_FAILED = 0xaa
} fault_code_t;


void fault(fault_code_t code, uint8_t extended_status, const char *error_text);

#endif /* FAULT_H */
