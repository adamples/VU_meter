#ifndef CONFIG_H
#define CONFIG_H

/* Constants -- do not modify */

#define I2C_DRIVER_ASYNC (0)
#define I2C_DRIVER_SYNC (1)

#define OLED_DRIVER_SSD1306 (1)
#define OLED_DRIVER_SH1106 (2)

/* End of constants */


#define I2C_DRIVER (I2C_DRIVER_ASYNC)
#define I2C_CLOCK (400000L)
#define I2C_BUFFER_SIZE (64)
#define I2C_QUEUE_SIZE (4)

#define OLED_DRIVER (OLED_DRIVER_SH1106)

#define DISPLAY_A_ADDRESS (0x7A)
#define DISPLAY_B_ADDRESS (0x78)


#define ADC_CHANNEL_L_NEEDLE (0)
#define ADC_CHANNEL_L_PEAK (2)
#define ADC_CHANNEL_R_NEEDLE (1)
#define ADC_CHANNEL_R_PEAK (3)


/* ADC value at which to show 0VU */
#define ZERO_VU_LEVEL (181)
#define ZERO_VU_ANGLE (181)

/* Ballistics output / ballistics input (p-p) */
#define BALLISTICS_GAIN (3)

/* Headroom above reference level above which peak indicator will fire */
#define PEAK_LEVEL_GAIN (10)


#define NEEDLE_RESOLUTION (128)


#define CALIBRATION_ZERO_PORT (PORTD)
#define CALIBRATION_ZERO_PIN (PIND)
#define CALIBRATION_ZERO_P (PD0)

#define CALIBRATION_REF_PORT (PORTD)
#define CALIBRATION_REF_PIN (PIND)
#define CALIBRATION_REF_P (PD1)

#endif /* CONFIG_H */
