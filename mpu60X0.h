#ifndef MPU60X0_H
#define MPU60X0_H

#include <hal.h>

#include <stdint.h>
typedef struct {
    uint32_t config;
    SPIDriver *spi;
    I2CDriver *i2c;
    uint8_t i2c_address;
} mpu60X0_t;


// mpu60X0_setup() config options
#define MPU60X0_ACC_FULL_RANGE_2G           (0<<0)
#define MPU60X0_ACC_FULL_RANGE_4G           (1<<0)
#define MPU60X0_ACC_FULL_RANGE_8G           (2<<0)
#define MPU60X0_ACC_FULL_RANGE_16G          (3<<0)
#define MPU60X0_GYRO_FULL_RANGE_250DPS      (0<<2)
#define MPU60X0_GYRO_FULL_RANGE_500DPS      (1<<2)
#define MPU60X0_GYRO_FULL_RANGE_1000DPS     (2<<2)
#define MPU60X0_GYRO_FULL_RANGE_2000DPS     (3<<2)
#define MPU60X0_SAMPLE_RATE_DIV(x)          ((0xff & x)<<8) // sample rate is gyro Fs divided by x+1, x in [0, 255]
#define MPU60X0_LOW_PASS_FILTER_0           (0<<16) // acc: BW=260Hz, delay=   0ms, Fs=1kHz gyro: BW=256Hz, delay=0.98ms, Fs=8kHz
#define MPU60X0_LOW_PASS_FILTER_1           (1<<16) // acc: BW=184Hz, delay= 2.0ms, Fs=1kHz gyro: BW=188Hz, delay= 1.9ms, Fs=1kHz
#define MPU60X0_LOW_PASS_FILTER_2           (2<<16) // acc: BW= 94Hz, delay= 3.0ms, Fs=1kHz gyro: BW= 98Hz, delay= 2.8ms, Fs=1kHz
#define MPU60X0_LOW_PASS_FILTER_3           (3<<16) // acc: BW= 44Hz, delay= 4.9ms, Fs=1kHz gyro: BW= 42Hz, delay= 4.8ms, Fs=1kHz
#define MPU60X0_LOW_PASS_FILTER_4           (4<<16) // acc: BW= 21Hz, delay= 8.5ms, Fs=1kHz gyro: BW= 20Hz, delay= 8.3ms, Fs=1kHz
#define MPU60X0_LOW_PASS_FILTER_5           (5<<16) // acc: BW= 10Hz, delay=13.8ms, Fs=1kHz gyro: BW= 10Hz, delay=13.4ms, Fs=1kHz
#define MPU60X0_LOW_PASS_FILTER_6           (6<<16) // acc: BW=  5Hz, delay=19.0ms, Fs=1kHz gyro: BW=  5Hz, delay=18.6ms, Fs=1kHz

void mpu60X0_init_using_spi(mpu60X0_t *dev, SPIDriver *spi_dev);
void mpu60X0_init_using_i2c(mpu60X0_t *dev, I2CDriver *i2c_dev, int ad0_pin_value);
void mpu60X0_setup(mpu60X0_t *dev, int config);
bool mpu60X0_ping(mpu60X0_t *dev);
bool mpu60X0_self_test(mpu60X0_t *dev);
void mpu60X0_read(mpu60X0_t *dev, float *gyro, float *acc, float *temp);

#endif // MPU60X0_H
