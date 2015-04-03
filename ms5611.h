
#ifndef MS5611_H
#define MS5611_H

#include <stdint.h>
#include "hal.h"

#define MS5611_OSR_256      0x00
#define MS5611_OSR_512      0x01
#define MS5611_OSR_1024     0x02
#define MS5611_OSR_2048     0x03
#define MS5611_OSR_4096     0x04

#define MS5611_ADC_PRESS    0
#define MS5611_ADC_TEMP     1

#define MS5611_I2C_ADDR1    0x76
#define MS5611_I2C_ADDR2    0x77

typedef struct {
    union {
        /* todo: SPI */
        struct {
            I2CDriver *driver;
            uint8_t address;
        } i2c;
    } dev;
    enum {ms5611_i2c = 0, ms5611_spi = 1} mode;
    uint16_t prom[6];
} ms5611_t;

/** Initializes MS5611 device for I2C.
 *  The address depends on the value of the CSB pin.
 *  Returns 0 if initialization was successful. */
int ms5611_i2c_init(ms5611_t *ms5611, I2CDriver *driver, int csb_pin_value);

/** Initializes MS5611 device for SPI */
// void ms5611_spi_init(ms5611_t *ms5611, spi_dev_t *intf);

/** Resets the MS5611 device.
 * Returns 1 on error accessing the sensor. */
int ms5611_reset(ms5611_t *ms5611);

/** Read the PROM calibration values.
 *  Returns 1 on error accessing the sensor. */
int ms5611_prom_read(ms5611_t *ms5611);

/** Starts an ADC conversion with oversampling rate osr.
 * Returns required conversion time in microseconds.
 * Command failed if returned value is negative. */
int16_t ms5611_adc_start(ms5611_t *ms5611, uint8_t adc, uint8_t osr);

/** Reads the ADC result.
 * Variable at adc_result holds 0 if ADC conversion is not finished.
 * Returns 1 on error accessing the sensor. */
int ms5611_adc_read(ms5611_t *ms5611, uint32_t *adc_result);

/** Calculates pressure from pressure and temperature adc values.
 * Returns pressure in Pascal.
 * Optional: Save temperature in 1/100 deg Celsius to variable at p_temp.*/
uint32_t ms5611_calc_press(ms5611_t *ms5611, uint32_t adc_press, uint32_t adc_temp, int32_t *p_temp);

/** Calculates temperature in 1/100 degrees Celsius. */
int32_t ms5611_calc_temp(ms5611_t *ms5611, uint32_t adc_temp);

#endif /* MS5611_H */
