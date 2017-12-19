#include "i2c.h"
#include "config.h"

#if I2C_DRIVER == I2C_DRIVER_SYNC
#include "i2c_sync.c.inc"
#elif I2C_DRIVER == I2C_DRIVER_ASYNC
#include "i2c_async.c.inc"
#endif
