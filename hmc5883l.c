#include <string.h>
#include "hmc5883l.h"

#define HMC5883L_REG_CONFIG_A       0
#define HMC5883L_REG_CONFIG_B       1
#define HMC5883L_REG_MODE           2
#define HMC5883L_REG_DATA_X_MSB     3
#define HMC5883L_REG_DATA_X_LSB     4
#define HMC5883L_REG_DATA_Y_MSB     5
#define HMC5883L_REG_DATA_Y_LSB     6
#define HMC5883L_REG_DATA_Z_MSB     7
#define HMC5883L_REG_DATA_Z_LSB     8
#define HMC5883L_REG_STATUS         9
#define HMC5883L_REG_ID_A           10 // read as 'H'
#define HMC5883L_REG_ID_B           11 // read as '4'
#define HMC5883L_REG_ID_C           12 // read as '3'

#define HMC5883L_I2C_ADDR           0x1E

#define GAUSS_TO_TESLA              0.0001


static int hmc5883l_reg_write(hmc5883l_t *dev, uint8_t reg, uint8_t data);
static int hmc5883l_reg_read_multi(hmc5883l_t *dev, uint8_t reg, uint8_t *data, uint8_t len);

static int hmc5883l_reg_write(hmc5883l_t *dev, uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    uint8_t addr = HMC5883L_I2C_ADDR;
    I2CDriver *driver = dev->i2c_driver;
    msg_t msg;

    msg = i2cMasterTransmit(driver, addr, buf, 2, NULL, 0);

    if (msg != MSG_OK) {
        return 1;
    }

    return 0;
}

static int hmc5883l_reg_read_multi(hmc5883l_t *dev, uint8_t reg, uint8_t *data, uint8_t len)
{
    uint8_t addr = HMC5883L_I2C_ADDR;
    I2CDriver *driver = dev->i2c_driver;
    msg_t msg;

    msg = i2cMasterTransmit(driver, addr, &reg, 1, data, len);

    if (msg != MSG_OK) {
        return 1;
    }

    return 0;
}

bool hmc5883l_ping(hmc5883l_t *dev)
{
    uint8_t buf[4];
    if (hmc5883l_reg_read_multi(dev, HMC5883L_REG_ID_A, buf, 3)) {
        return false;
    }
    buf[3] = 0;
    return strcmp("H43", (const char *)buf) == 0;
}

/* Returns magnetic field vector {x,y,z} in Tesla. */
int hmc5883l_read(hmc5883l_t *dev, float *mag)
{
    static const uint16_t mag_gain[] = {1370, 1090, 820, 660, 440, 390, 330, 230};
    uint16_t gain;
    uint8_t buf[3*2];
    if (hmc5883l_reg_read_multi(dev, HMC5883L_REG_DATA_X_MSB, buf, sizeof(buf))) {
        return 1;
    }
    gain = mag_gain[(dev->config >> 5) & 0x07];
    // registers are [xh, xl, zh, zl, yh, yl]
    mag[0] = (float)((int16_t)(buf[0]<<8) | buf[1]) / gain * GAUSS_TO_TESLA;
    mag[1] = (float)((int16_t)(buf[4]<<8) | buf[5]) / gain * GAUSS_TO_TESLA;
    mag[2] = (float)((int16_t)(buf[2]<<8) | buf[3]) / gain * GAUSS_TO_TESLA;
    return 0;
}

void hmc5883l_init(hmc5883l_t *dev, I2CDriver *driver)
{
    dev->i2c_driver = driver;
    dev->config = 0;
}

int hmc5883l_setup(hmc5883l_t *dev, uint8_t config)
{
    uint8_t data;
    dev->config = config;

    // set output data rate and sample average, normal measurement
    data = (config & (0x07 << 2)) | ((config & 0x03) << 5);
    if (hmc5883l_reg_write(dev, HMC5883L_REG_CONFIG_A, data)) {
        return 1;
    }
    // set data gain form config
    data = config & (0x07 << 5);
    if (hmc5883l_reg_write(dev, HMC5883L_REG_CONFIG_B, data)) {
        return 1;
    }
    // set continuous measurement mode
    data = 0;
    if (hmc5883l_reg_write(dev, HMC5883L_REG_MODE, data)) {
        return 1;
    }
    return 0;
}

