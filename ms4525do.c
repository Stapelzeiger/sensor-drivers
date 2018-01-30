#include "ms4525do.h"

const float psi_to_Pa = 6894.7572932f;

int ms4525do_init(ms4525do_t *drv, I2CDriver *i2c_driver, char output_type, char interface_type, int pressure_range, char pressure_type)
{
    switch (output_type) {
    case 'A':
        drv->output_scale_percent = 80;
        break;
    case 'B':
        drv->output_scale_percent = 90;
        break;
    default:
        return -1;
    }
    switch (interface_type) {
    case 'I':
        drv->addr = 0x28;
        break;
    case 'J':
        drv->addr = 0x36;
        break;
    case 'K':
        drv->addr = 0x46;
        break;
    default:
        if (interface_type >= '0' && interface_type <= '9') {
            drv->addr = 0x48 + (interface_type - '0');
        } else {
            return -2;
        }
    }
    switch (pressure_range) {
    case 2:
    case 4:
    case 5:
    case 10:
    case 20:
    case 30:
        drv->pressure_max = pressure_range * psi_to_Pa;
        break;
    default:
        return -3;
    }
    switch (pressure_type) {
    case 'A': // Absolute
    case 'G': // Gauge
        drv->pressure_min = 0;
        break;
    case 'D': // Differential
        drv->pressure_min = -drv->pressure_max;
        break;
    default:
        return -4;
    }
    drv->i2c_driver = i2c_driver;
    return 0;
}


float ms4525do_pressure_raw_to_Pa(ms4525do_t *drv, uint32_t adc)
{
    const float adc_max = 16383;
    float residual = (100 - drv->output_scale_percent)*0.01/2;
    float gain = (drv->pressure_max - drv->pressure_min) / (drv->output_scale_percent * adc_max) * 100;
    return (adc - residual*adc_max) * gain + drv->pressure_min;
}

float ms4525do_temperature_raw_to_Celsius(ms4525do_t *drv, uint32_t adc)
{
    (void)drv;
    float t_min = -50;
    float t_max = 150;
    float adc_max = 2047;
    return adc / adc_max * (t_max - t_min) + t_min;
}

int ms4525do_extract_pressure_temperature_from_buf(uint8_t buf[4], uint16_t *pressure, uint16_t *temperature)
{
    int status = buf[0] >> 6;
    *pressure = (((uint16_t)buf[0] & 0x3f) << 8) + buf[1];
    *temperature = (buf[2] << 3) + (buf[3] >> 5);
    return status;
}

int ms4525do_read_raw(ms4525do_t *drv, uint16_t *pressure, uint16_t *temperature)
{
    uint8_t buf[4];
    msg_t msg;

    msg = i2cMasterReceive(drv->i2c_driver, drv->addr, buf, 4);
    if (msg != MSG_OK) {
        return -1;
    }
    int status = ms4525do_extract_pressure_temperature_from_buf(buf, pressure, temperature);
    return status;
}

int ms4525do_read(ms4525do_t *drv, float *pressure, float *temperature)
{
    uint16_t temperature_raw, pressure_raw;
    int status = ms4525do_read_raw(drv, &pressure_raw, &temperature_raw);
    *pressure = ms4525do_pressure_raw_to_Pa(drv, pressure_raw);
    *temperature = ms4525do_temperature_raw_to_Celsius(drv, temperature_raw);
    return status;
}
