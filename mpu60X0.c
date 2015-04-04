#include <stdint.h>
#include <math.h>

#include "mpu60X0.h"
#include "mpu60X0_registers.h"

#define STANDARD_GRAVITY 9.80665f
#define DEG2RAD(deg) (deg/180*M_PI)

static uint8_t mpu60X0_reg_read(mpu60X0_t *dev, uint8_t reg)
{
    uint8_t ret = 0;
    if (dev->spi) {
#if HAL_USE_SPI
        spiSelect(dev->spi);
        reg |= 0x80;
        spiSend(dev->spi, 1, &reg);
        spiReceive(dev->spi, 1, &ret);
        spiUnselect(dev->spi);
#endif
    } else if (dev->i2c) {
#if HAL_USE_I2C
        uint8_t addr = dev->i2c_address;
        i2cMasterTransmit(dev->i2c, addr, &reg, 1, &ret, 1);
#endif
    }
    return ret;
}

static void mpu60X0_reg_write(mpu60X0_t *dev, uint8_t reg, uint8_t val)
{
    uint8_t buf[] = {reg, val};
    if (dev->spi) {
#if HAL_USE_SPI
        spiSelect(dev->spi);
        spiSend(dev->spi, 2, buf);
        spiUnselect(dev->spi);
#endif
    } else if (dev->i2c) {
#if HAL_USE_I2C
        uint8_t addr = dev->i2c_address;
        i2cMasterTransmit(dev->i2c, addr, buf, 2, NULL, 0);
#endif
    }
}

static void mpu60X0_reg_read_multi(mpu60X0_t *dev, uint8_t reg, uint8_t *buf, int8_t len)
{
    if (dev->spi) {
#if HAL_USE_SPI
        spiSelect(dev->spi);
        reg |= 0x80;
        spiSend(dev->spi, 1, &reg);
        spiReceive(dev->spi, len, buf);
        spiUnselect(dev->spi);
#endif
    } else if (dev->i2c) {
#if HAL_USE_I2C
        uint8_t addr = dev->i2c_address;
        i2cMasterTransmit(dev->i2c, addr, &reg, 1, buf, len);
#endif
    }
}

#if 0
static void mpu60X0_reg_write_multi(mpu60X0_t *dev, uint8_t reg, const uint8_t *buf, int8_t len)
{
    if (dev->spi) {
        spiSelect(dev->spi);
        spiSend(dev->spi, 1, &reg);
        spiSend(dev->spi, len, buf);
        spiUnselect(dev->spi);
    }
}
#endif

#if HAL_USE_SPI
void mpu60X0_init_using_spi(mpu60X0_t *dev, SPIDriver *spi_dev)
{
    dev->spi = spi_dev;
    dev->i2c = NULL;
    dev->config = 0;
}
#endif

#if HAL_USE_I2C
void mpu60X0_init_using_i2c(mpu60X0_t *dev, I2CDriver *i2c_dev, int ad0_pin_value)
{
    dev->i2c = i2c_dev;
    dev->spi = NULL;
    dev->config = 0;
    if (ad0_pin_value == 0) {
        dev->i2c_address = 0x68;
    } else {
        dev->i2c_address = 0x69;
    }
}
#endif

void mpu60X0_setup(mpu60X0_t *dev, int config)
{
    dev->config = config;
    // reset device
    mpu60X0_reg_write(dev, MPU60X0_RA_PWR_MGMT_1, 0x80);
    chThdSleepMilliseconds(1);
    while (mpu60X0_reg_read(dev, MPU60X0_RA_PWR_MGMT_1) & 0x80)
        chThdSleepMilliseconds(1);
    chThdSleepMilliseconds(1);
    // select gyro x as clock source and disable sleep
    mpu60X0_reg_write(dev, MPU60X0_RA_PWR_MGMT_1, MPU60X0_CLOCK_PLL_XGYRO);
    chThdSleepMilliseconds(1);
    if (dev->spi) { // disable I2C interface
        mpu60X0_reg_write(dev, MPU60X0_RA_USER_CTRL, MPU60X0_USERCTRL_I2C_IF_DIS_BIT);
        chThdSleepMilliseconds(1);
    }
    // gyro full scale
    mpu60X0_reg_write(dev, MPU60X0_RA_GYRO_CONFIG, (config<<1) & 0x18);
    chThdSleepMilliseconds(1);
    // accelerometer full scale
    mpu60X0_reg_write(dev, MPU60X0_RA_ACCEL_CONFIG, (config<<3) & 0x18);
    chThdSleepMilliseconds(1);
    // sample rate divisor
    mpu60X0_reg_write(dev, MPU60X0_RA_SMPLRT_DIV, (config >> 8) & 0xff);
    chThdSleepMilliseconds(1);
    // enable interrupts: data ready
    mpu60X0_reg_write(dev, MPU60X0_RA_INT_ENABLE, MPU60X0_INTERRUPT_DATA_RDY);
    chThdSleepMilliseconds(1);
    // low pass filter config, FSYNC disabled
    mpu60X0_reg_write(dev, MPU60X0_RA_CONFIG, (config>>16) & 0x07);
    chThdSleepMilliseconds(1);
}

bool mpu60X0_ping(mpu60X0_t *dev)
{
    int id = mpu60X0_reg_read(dev, MPU60X0_RA_WHO_AM_I);
    chThdSleepMilliseconds(1);
    return (id == 0x68);
}

bool mpu60X0_self_test(mpu60X0_t *dev)
{
    (void)dev;
    return true; // TODO
}

static int32_t read_word(const uint8_t *buf) // signed int16
{
    return ((int16_t)((int8_t)buf[0]) << 8 | buf[1]);
}

void mpu60X0_read(mpu60X0_t *dev, float *gyro, float *acc, float *temp)
{
    static const float gyro_res[] = { DEG2RAD(1/131.f),
                                      DEG2RAD(1/65.5f),
                                      DEG2RAD(1/32.8f),
                                      DEG2RAD(1/16.4f) }; // rad/s/LSB
    static const float acc_res[] = { STANDARD_GRAVITY/16384.f,
                                     STANDARD_GRAVITY/8192.f,
                                     STANDARD_GRAVITY/4096.f,
                                     STANDARD_GRAVITY/2048.f }; // m/s^2 / LSB
    uint8_t buf[1 + 6 + 2 + 6]; // interrupt status, accel, temp, gyro
    mpu60X0_reg_read_multi(dev, MPU60X0_RA_INT_STATUS, buf, sizeof(buf));
    if (acc) {
        acc[0] = (float)read_word(&buf[1]) * acc_res[dev->config & 0x3];
        acc[1] = (float)read_word(&buf[3]) * acc_res[dev->config & 0x3];
        acc[2] = (float)read_word(&buf[5]) * acc_res[dev->config & 0x3];
    }
    if (temp) {
        *temp = (float)read_word(&buf[7]) / 340.0f + 36.53f;
    }
    if (gyro) {
        gyro[0] = (float)read_word(&buf[9]) * gyro_res[(dev->config >> 2) & 0x3];
        gyro[1] = (float)read_word(&buf[11]) * gyro_res[(dev->config >> 2) & 0x3];
        gyro[2] = (float)read_word(&buf[13]) * gyro_res[(dev->config >> 2) & 0x3];
    }
}
