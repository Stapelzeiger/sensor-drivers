#include "h3lis331dl.h"

// register map
#define H3LIS331DL_REG_WHO_AM_I         0x0F // 00110010 r
#define H3LIS331DL_REG_CTRL_REG1        0x20 // 00000111 rw
#define H3LIS331DL_REG_CTRL_REG2        0x21 // 00000000 rw
#define H3LIS331DL_REG_CTRL_REG3        0x22 // 00000000 rw
#define H3LIS331DL_REG_CTRL_REG4        0x23 // 00000000 rw
#define H3LIS331DL_REG_CTRL_REG5        0x24 // 00000000 rw
#define H3LIS331DL_REG_HP_FILTER_RESET  0x25 // -        r
#define H3LIS331DL_REG_REFERENCE        0x26 // 00000000 rw
#define H3LIS331DL_REG_STATUS_REG       0x27 // 00000000 r
#define H3LIS331DL_REG_OUT_X_L          0x28 // Output   r
#define H3LIS331DL_REG_OUT_X_H          0x29 // Output   r
#define H3LIS331DL_REG_OUT_Y_L          0x2A // Output   r
#define H3LIS331DL_REG_OUT_Y_H          0x2B // Output   r
#define H3LIS331DL_REG_OUT_Z_L          0x2C // Output   r
#define H3LIS331DL_REG_OUT_Z_H          0x2D // Output   r
#define H3LIS331DL_REG_INT1_CFG         0x30 // 00000000 rw
#define H3LIS331DL_REG_INT1_SRC         0x31 // 00000000 r
#define H3LIS331DL_REG_INT1_THS         0x32 // 00000000 rw
#define H3LIS331DL_REG_INT1_DURATION    0x33 // 00000000 rw
#define H3LIS331DL_REG_INT2_CFG         0x34 // 00000000 rw
#define H3LIS331DL_REG_INT2_SRC         0x35 // 00000000 r
#define H3LIS331DL_REG_INT2_THS         0x36 // 00000000 rw
#define H3LIS331DL_REG_INT2_DURATION    0x37 // 00000000 rw

// register bit definitions
#define H3LIS331DL_CTRL_REG1_PM2        (1<<7)
#define H3LIS331DL_CTRL_REG1_PM1        (1<<6)
#define H3LIS331DL_CTRL_REG1_PM0        (1<<5)
#define H3LIS331DL_CTRL_REG1_DR1        (1<<4)
#define H3LIS331DL_CTRL_REG1_DR0        (1<<3)
#define H3LIS331DL_CTRL_REG1_Zen        (1<<2)
#define H3LIS331DL_CTRL_REG1_Yen        (1<<1)
#define H3LIS331DL_CTRL_REG1_Xen        (1<<0)

#define H3LIS331DL_CTRL_REG2_BOOT       (1<<7)
#define H3LIS331DL_CTRL_REG2_HPM1       (1<<6)
#define H3LIS331DL_CTRL_REG2_HPM0       (1<<5)
#define H3LIS331DL_CTRL_REG2_FDS        (1<<4)
#define H3LIS331DL_CTRL_REG2_HPen2      (1<<3)
#define H3LIS331DL_CTRL_REG2_HPen1      (1<<2)
#define H3LIS331DL_CTRL_REG2_HPCF1      (1<<1)
#define H3LIS331DL_CTRL_REG2_HPCF0      (1<<0)

#define H3LIS331DL_CTRL_REG3_IHL        (1<<7)
#define H3LIS331DL_CTRL_REG3_PP_OD      (1<<6)
#define H3LIS331DL_CTRL_REG3_LIR2       (1<<5)
#define H3LIS331DL_CTRL_REG3_I2_CFG1    (1<<4)
#define H3LIS331DL_CTRL_REG3_I2_CFG0    (1<<3)
#define H3LIS331DL_CTRL_REG3_LIR1       (1<<2)
#define H3LIS331DL_CTRL_REG3_I1_CFG1    (1<<1)
#define H3LIS331DL_CTRL_REG3_I1_CFG0    (1<<0)

#define H3LIS331DL_CTRL_REG4_BDU        (1<<7)
#define H3LIS331DL_CTRL_REG4_BLE        (1<<6)
#define H3LIS331DL_CTRL_REG4_FS1        (1<<5)
#define H3LIS331DL_CTRL_REG4_FS0        (1<<4)
#define H3LIS331DL_CTRL_REG4_SIM        (1<<0)

#define H3LIS331DL_CTRL_REG5_TurnOn1    (1<<1)
#define H3LIS331DL_CTRL_REG5_TurnOn0    (1<<0)

#define H3LIS331DL_STATUS_REG_ZYXOR     (1<<7)
#define H3LIS331DL_STATUS_REG_ZOR       (1<<6)
#define H3LIS331DL_STATUS_REG_YOR       (1<<5)
#define H3LIS331DL_STATUS_REG_XOR       (1<<4)
#define H3LIS331DL_STATUS_REG_ZYXDA     (1<<3)
#define H3LIS331DL_STATUS_REG_ZDA       (1<<2)
#define H3LIS331DL_STATUS_REG_YDA       (1<<1)
#define H3LIS331DL_STATUS_REG_XDA       (1<<0)

#define H3LIS331DL_INT1_CFG_AOI         (1<<7)
#define H3LIS331DL_INT1_CFG_ZHIE        (1<<5)
#define H3LIS331DL_INT1_CFG_ZLIE        (1<<4)
#define H3LIS331DL_INT1_CFG_YHIE        (1<<3)
#define H3LIS331DL_INT1_CFG_YLIE        (1<<2)
#define H3LIS331DL_INT1_CFG_XHIE        (1<<1)
#define H3LIS331DL_INT1_CFG_XLIE        (1<<0)

#define H3LIS331DL_INT2_CFG_AOI         (1<<7)
#define H3LIS331DL_INT2_CFG_ZHIE        (1<<5)
#define H3LIS331DL_INT2_CFG_ZLIE        (1<<4)
#define H3LIS331DL_INT2_CFG_YHIE        (1<<3)
#define H3LIS331DL_INT2_CFG_YLIE        (1<<2)
#define H3LIS331DL_INT2_CFG_XHIE        (1<<1)
#define H3LIS331DL_INT2_CFG_XLIE        (1<<0)



#define STANDARD_GRAVITY 9.80665f


static int i2c_reg_write(h3lis331dl_t *dev, uint8_t reg, uint8_t data)
{
    uint8_t buf[2] = {reg, data};
    I2CDriver *driver = dev->i2c_driver;
    msg_t msg;

    msg = i2cMasterTransmit(driver, dev->i2c_addr, buf, 2, NULL, 0);

    if (msg != MSG_OK) {
        return -1;
    }

    return 0;
}

static int i2c_reg_read(h3lis331dl_t *dev, uint8_t reg, uint8_t *data)
{
    I2CDriver *driver = dev->i2c_driver;
    msg_t msg;

    msg = i2cMasterTransmit(driver, dev->i2c_addr, &reg, 1, data, 1);

    if (msg != MSG_OK) {
        return -1;
    }

    return 0;
}

static int i2c_reg_read_multi(h3lis331dl_t *dev, uint8_t reg, uint8_t *data, uint8_t len)
{
    I2CDriver *driver = dev->i2c_driver;
    msg_t msg;
    reg = reg | 0x80;

    msg = i2cMasterTransmit(driver, dev->i2c_addr, &reg, 1, data, len);

    if (msg != MSG_OK) {
        return -1;
    }

    return 0;
}



bool h3lis331dl_ping(h3lis331dl_t *dev)
{
    uint8_t reg;
    if (i2c_reg_read(dev, H3LIS331DL_REG_WHO_AM_I, &reg)) {
        return false;
    }
    if (reg != 0x32) {
        return false;
    }
    return true;
}


void h3lis331dl_init_using_i2c(h3lis331dl_t *dev, I2CDriver *i2c_driver, uint8_t addr)
{
    dev->i2c_driver = i2c_driver;
    dev->i2c_addr = addr;
}

void h3lis331dl_setup(h3lis331dl_t *dev, uint32_t config)
{
    // ctrl_reg1 : power mode normal, all axes on, set output data rate
    uint8_t odr = (config << 3) & 0x18;
    i2c_reg_write(dev, H3LIS331DL_REG_CTRL_REG1, odr |
                                                H3LIS331DL_CTRL_REG1_PM0 |
                                                H3LIS331DL_CTRL_REG1_Zen |
                                                H3LIS331DL_CTRL_REG1_Yen |
                                                H3LIS331DL_CTRL_REG1_Xen);
    // ctrl reg2 : default 0, no high pass filter

    // ctrl reg3 : interrupt 1: data ready, active high, push pull
    i2c_reg_write(dev, H3LIS331DL_REG_CTRL_REG3, H3LIS331DL_CTRL_REG3_I1_CFG1);

    // ctrl reg4 : set full scale, set BDU (block data update), little endian (BLE=0)
    uint8_t fs = (config >> 4) & 0x30;
    i2c_reg_write(dev, H3LIS331DL_REG_CTRL_REG4, fs | H3LIS331DL_CTRL_REG4_BDU);

    // ctrl reg5 : default 0

    if ((config & 0x0300) == H3LIS331DL_CONFIG_FS_100G) {
        dev->sensitivity = 49;
    } else if ((config & 0x0300) == H3LIS331DL_CONFIG_FS_200G) {
        dev->sensitivity = 98;
    } else { // 400G
        dev->sensitivity = 195;
    }
}


static int16_t read_word(const uint8_t *buf)
{
    return ((int16_t)((int8_t)buf[1]) << 8 | buf[0]);
}


int h3lis331dl_read_int(h3lis331dl_t *dev, int32_t *acc)
{
    uint8_t buf[1+3*2];
    if (i2c_reg_read_multi(dev, H3LIS331DL_REG_STATUS_REG, buf, sizeof(buf))) {
        return -1;
    }
    acc[0] =  (int32_t)dev->sensitivity * read_word(&buf[1]) / 16;
    acc[1] =  (int32_t)dev->sensitivity * read_word(&buf[1+2]) / 16;
    acc[2] =  (int32_t)dev->sensitivity * read_word(&buf[1+4]) / 16;
    return 0;
}


int h3lis331dl_read(h3lis331dl_t *dev, float *acc)
{
    int32_t acc_i[3];
    if (h3lis331dl_read_int(dev, acc_i)) {
        return -1;
    }
    acc[0] = acc_i[0] * (STANDARD_GRAVITY/1000);
    acc[1] = acc_i[1] * (STANDARD_GRAVITY/1000);
    acc[2] = acc_i[2] * (STANDARD_GRAVITY/1000);
    return 0;
}
