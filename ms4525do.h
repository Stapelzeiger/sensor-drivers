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

/** Initialize MS4525DO driver
 * @parameter drv the driver
 * @parameter i2c_driver i2c driver to communicate over
 * @parameter output_type from device number, valid: A, B
 * @parameter interface_type from device number, valid: I,J,K,0-9
 * @parameter pressure_range from device number, valid: 2, 4, 5, 10, 20, 30
 * @parameter pressure_type from device number, valid: A, G, D
 * @returns 0 on success
 */
int ms4525do_init(ms4525do_t *drv, I2CDriver *i2c_driver, char output_type, char interface_type, int pressure_range, char pressure_type);

/** Convert raw reading to pressure in Pa */
float ms4525do_pressure_raw_to_Pa(ms4525do_t *drv, uint32_t adc);

/** Convert raw reading to temperature in degrees Celsius */
float ms4525do_temperature_raw_to_Celsius(ms4525do_t *drv, uint32_t adc);

#define MS4525DO_READ_RES_OK            0
#define MS4525DO_READ_RES_I2C_ERR      -1
#define MS4525DO_READ_RES_STALE_DATA    2
#define MS4525DO_READ_RES_SENSOR_FAULT  3

/** Read pressure in Pa and temperature in °C
 * @parameter [in] drv the driver
 * @parameter [out] pressure The pressure is written to this address
 * @parameter [out] temperature The temperature is written to this address
 * @returns MS4525DO_READ_RES_OK if the read was successful
 */
int ms4525do_read(ms4525do_t *drv, float *pressure, float *temperature);

/** Read pressure and temperature in raw ADC units
 * @parameter [in] drv the driver
 * @parameter [out] pressure The pressure is written to this address
 * @parameter [out] temperature The temperature is written to this address
 * @returns MS4525DO_READ_RES_OK if the read was successful
 */
int ms4525do_read_raw(ms4525do_t *drv, uint16_t *pressure, uint16_t *temperature);



#ifdef __cplusplus
}
#endif

#endif /* MS4525DO_H */
