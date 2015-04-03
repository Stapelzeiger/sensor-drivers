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
        spiSelect(dev->spi);
        reg |= 0x80;
        spiSend(dev->spi, 1, &reg);
        spiReceive(dev->spi, 1, &ret);
        spiUnselect(dev->spi);
    }
    return ret;
}

static void mpu60X0_reg_write(mpu60X0_t *dev, uint8_t reg, uint8_t val)
{
    if (dev->spi) {
        spiSelect(dev->spi);
        uint8_t buf[] = {reg, val};
        spiSend(dev->spi, 2, buf);
        spiUnselect(dev->spi);
    }
}

static void mpu60X0_reg_read_multi(mpu60X0_t *dev, uint8_t reg, uint8_t *buf, int8_t len)
{
    if (dev->spi) {
        spiSelect(dev->spi);
        reg |= 0x80;
        spiSend(dev->spi, 1, &reg);
        spiReceive(dev->spi, len, buf);
        spiUnselect(dev->spi);
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

void mpu60X0_init_using_spi(mpu60X0_t *dev, SPIDriver *spi_dev)
{
    dev->spi = spi_dev;
    dev->config = 0;
}

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
