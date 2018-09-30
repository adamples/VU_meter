#include "i2c_hw.h"

extern inline void i2c_hw_init(void);
extern inline void i2c_hw_wait_for_int(void);
extern inline void i2c_hw_send_start_condition(void);
extern inline void i2c_hw_send_start_condition_int(void);
extern inline void i2c_hw_send_stop_condition(void);
extern inline void i2c_hw_send_byte(uint8_t octet);
extern inline void i2c_hw_send_byte_int(uint8_t octet);
extern inline void i2c_hw_go_idle(void);
extern inline void i2c_hw_disable_int(void);
