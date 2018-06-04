#ifndef CONFIG_H
#define CONFIG_H

/* Constants -- do not modify */

#define I2C_DRIVER_ASYNC (0)
#define I2C_DRIVER_SYNC (1)

#define OLED_DRIVER_SSD1306 (1)
#define OLED_DRIVER_SH1106 (2)

/* End of constants */


#define I2C_DRIVER I2C_DRIVER_ASYNC
#define I2C_CLOCK (400000L)
#define I2C_BUFFER_SIZE (64)
#define I2C_QUEUE_SIZE (4)

#define OLED_DRIVER (OLED_DRIVER_SH1106)

#define DISPLAY_A_ADDRESS (0x78)
#define DISPLAY_B_ADDRESS (0x7A)


#define NEEDLE_RESOLUTION (128)

#endif /* CONFIG_H */
