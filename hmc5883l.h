
#ifndef HMC5883L_H
#define HMC5883L_H

#include <stdint.h>
#include "hal.h"

#define HMC5883L_SAMPLE_AVG_1       (0)
#define HMC5883L_SAMPLE_AVG_2       (1)
#define HMC5883L_SAMPLE_AVG_4       (2)
#define HMC5883L_SAMPLE_AVG_8       (3)

#define HMC5883L_RATE_HZ_0_75       (0<<2)
#define HMC5883L_RATE_HZ_1_5        (1<<2)
#define HMC5883L_RATE_HZ_3          (2<<2)
#define HMC5883L_RATE_HZ_7_5        (3<<2)
#define HMC5883L_RATE_HZ_15         (4<<2)
#define HMC5883L_RATE_HZ_30         (5<<2)
#define HMC5883L_RATE_HZ_75         (6<<2)

#define HMC5883L_GAIN_1370          (0<<5)  // +/- 0.88 Gauss
#define HMC5883L_GAIN_1090          (1<<5)  // +/- 1.3 Gauss
#define HMC5883L_GAIN_820           (2<<5)  // +/- 1.9 Gauss
#define HMC5883L_GAIN_660           (3<<5)  // +/- 2.5 Gauss
#define HMC5883L_GAIN_440           (4<<5)  // +/- 4.0 Gauss
#define HMC5883L_GAIN_390           (5<<5)  // +/- 4.7 Gauss
#define HMC5883L_GAIN_330           (6<<5)  // +/- 5.6 Gauss
#define HMC5883L_GAIN_230           (7<<5)  // +/- 8.1 Gauss

typedef struct {
    I2CDriver *i2c_driver;
    uint8_t config;
} hmc5883l_t;

/* Returns true, if device ID is correct */
bool hmc5883l_ping(hmc5883l_t *dev);

/* Returns magnetic field vector {x,y,z} in Tesla. */
int hmc5883l_read(hmc5883l_t *dev, float *mag);

/* Initializes the HMC5883L device datastructure */
void hmc5883l_init(hmc5883l_t *dev, I2CDriver *driver);

/* HMC5883L setup for given configuration options */
int hmc5883l_setup(hmc5883l_t *dev, uint8_t config);

#endif // HMC5883L_H
