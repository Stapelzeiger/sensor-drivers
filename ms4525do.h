#ifndef MS4525DO_H
#define MS4525DO_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>
#include "hal.h"

typedef struct {
    I2CDriver *i2c_driver;
    uint8_t addr;
    uint8_t output_scale_percent;
    float pressure_max;
    float pressure_min;
} ms4525do_t;

/**
 * @returns 0 on success
 */
int ms4525do_init(ms4525do_t *drv, I2CDriver *i2c_driver, char output_type, char interface_type, int pressure_range, char pressure_type);

float ms4525do_pressure_raw_to_Pa(ms4525do_t *drv, uint32_t adc);
float ms4525do_temperature_raw_to_Celsius(ms4525do_t *drv, uint32_t adc);

const int MS4525DO_READ_RES_OK           = 0;
const int MS4525DO_READ_RES_I2C_ERR      = -1;
const int MS4525DO_READ_RES_STALE_DATA   = 2;
const int MS4525DO_READ_RES_SENSOR_FAULT = 3;
/**
 * returns 0 on success, -1 on I2C failure, 2 for stale data, 3 for sensor fault
 */
int ms4525do_read(ms4525do_t *drv, float *pressure, float *temperature);
int ms4525do_read_raw(ms4525do_t *drv, uint16_t *pressure, uint16_t *temperature);



#ifdef __cplusplus
}
#endif

#endif /* MS4525DO_H */
