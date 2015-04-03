#ifndef H3LIS331DL_H
#define H3LIS331DL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <hal.h>

typedef struct {
    I2CDriver *i2c_driver;
    uint8_t i2c_addr;
    uint8_t sensitivity; // mg/digit
} h3lis331dl_t;


#define H3LIS331DL_ADDR_SA0_HIGH    0x19
#define H3LIS331DL_ADDR_SA0_LOW     0x18

#define H3LIS331DL_CONFIG_ODR_50HZ      0
#define H3LIS331DL_CONFIG_ODR_100HZ     1
#define H3LIS331DL_CONFIG_ODR_400HZ     2
#define H3LIS331DL_CONFIG_ODR_1000HZ    3
#define H3LIS331DL_CONFIG_FS_100G       (0<<8)
#define H3LIS331DL_CONFIG_FS_200G       (1<<8)
#define H3LIS331DL_CONFIG_FS_400G       (3<<8)

bool h3lis331dl_ping(h3lis331dl_t *dev);

void h3lis331dl_init_using_i2c(h3lis331dl_t *dev, I2CDriver *i2c_driver, uint8_t addr);

void h3lis331dl_setup(h3lis331dl_t *dev, uint32_t config);

/*
 * reads acceleration in mg
 */
int h3lis331dl_read_int(h3lis331dl_t *dev, int32_t *acc);

/*
 * reads acceleration in m/s^2
 */
int h3lis331dl_read(h3lis331dl_t *dev, float *acc);


#ifdef __cplusplus
}
#endif

#endif /* H3LIS331DL_H */