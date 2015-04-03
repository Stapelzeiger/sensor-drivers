
#include <stdint.h>
#include <stdbool.h>
#include "ms5611.h"

#define PROM_SENS       0
#define PROM_OFF        1
#define PROM_TCS        2
#define PROM_TCO        3
#define PROM_TREF       4
#define PROM_TEMPSENS   5

#define MS5611_CMD_ADC_PRESS(osr)  (0x40 | ((osr)<<1))
#define MS5611_CMD_ADC_TEMP(osr)   (0x50 | ((osr)<<1))
#define MS5611_CMD_RESET            0x1E
#define MS5611_CMD_ADC_READ         0x00
#define MS5611_CMD_PROM_READ_BASE   0XA0

#define SQUARE(x) ((x)*(x))

static const uint16_t ms5611_osr_dly_us[] = {600, 1170, 2280, 4540, 9040};

static int ms5611_command_i2c(ms5611_t *ms5611, uint8_t cmd, uint32_t *data, uint8_t len);
static int ms5611_command(ms5611_t *ms5611, uint8_t cmd, uint32_t *data, uint8_t len);
static uint16_t ms5611_crc4_update(uint16_t crc, uint16_t data);

int ms5611_i2c_init(ms5611_t *ms5611, I2CDriver *driver, int csb_pin_value)
{
    int err;
    uint8_t addr;

    ms5611->mode = ms5611_i2c;

    /* LSbit of addr is complementary of CSB pin */
    if (csb_pin_value == 1) {
        addr = 0x76;
    } else {
        addr = 0x77;
    }

    ms5611->dev.i2c.driver = driver;
    ms5611->dev.i2c.address = addr;

    err = ms5611_reset(ms5611);

    chThdSleepMilliseconds(5);

    if (err) {
        return err; /* error */
    }

    err = ms5611_prom_read(ms5611);

    return err;
}

int ms5611_reset(ms5611_t *ms5611)
{
    uint8_t reset_cmd = MS5611_CMD_RESET;
    return ms5611_command(ms5611, reset_cmd, NULL, 0);
}

static int ms5611_command_i2c(ms5611_t *ms5611, uint8_t cmd, uint32_t *data, uint8_t len)
{
    int i;
    uint32_t val = 0;
    uint8_t buf[4];

    uint8_t addr = ms5611->dev.i2c.address;
    I2CDriver *driver = ms5611->dev.i2c.driver;
    msg_t msg;


    if (len > 3) {
        return 1;
    }

    msg = i2cMasterTransmit(driver, addr, &cmd, 1, buf, len);

    if (msg != MSG_OK) {
        return 1;
    }

    for (i = 0; i < len; i++) {
        /* MSByte received first */
        val = (val << 8) | buf[i];
    }

    if (data != NULL) {
        *data = val;
    }

    return 0;
}

static int ms5611_command(ms5611_t *ms5611, uint8_t cmd, uint32_t *data, uint8_t len)
{
    if (ms5611->mode == ms5611_i2c) {
        return ms5611_command_i2c(ms5611, cmd, data, len);
    } else {
        // return ms5611_command_spi(ms5611, cmd, data, len);
        return 1;
    }
}

/* MS5611 4bit CRC calculation as described in AN520. */
static uint16_t ms5611_crc4_update(uint16_t crc, uint16_t data)
{
    uint8_t n_bit;

    crc ^= data>>8;

    for (n_bit = 8; n_bit > 0; n_bit--) {
        if (crc & (0x8000)) {
            crc = (crc << 1) ^ 0x3000;
        } else {
            crc = (crc << 1);
        }
    }

    crc ^= (data & 0x00ff);

    for (n_bit = 8; n_bit > 0; n_bit--) {
        if (crc & (0x8000)) {
            crc = (crc << 1) ^ 0x3000;
        } else {
            crc = (crc << 1);
        }
    }

    return crc;
}

int ms5611_prom_read(ms5611_t *ms5611)
{
    uint8_t i, addr;
    uint16_t crc;
    uint32_t crc_read, d;

    addr = MS5611_CMD_PROM_READ_BASE;

    /* read reserved 16 bit for CRC */
    if (ms5611_command(ms5611, addr, &d, 2)) {
        return 1;   /* read failed */
    }
    crc = ms5611_crc4_update(0, d);
    addr += 2;

    /* read PROM memory */
    for (i = 0; i < 6; i++) {
        if (ms5611_command(ms5611, addr, &d, 2)) {
            return 1;   /* read failed */
        }
        crc = ms5611_crc4_update(crc, d);
        ms5611->prom[i] = d;
        addr += 2;
    }

    /* read CRC word */
    if (ms5611_command(ms5611, addr, &crc_read, 2)) {
        return 1;   /* read failed */
    }
    /* mask out CRC byte for calcualtion */
    crc = ms5611_crc4_update(crc, crc_read & 0xff00);
    /* get 4-bit CRC */
    crc = (crc>>12) & 0x000f;

    /* check 4-bit CRC */
    if ((crc_read & 0x000f) != crc) {
        return 2;   /* CRC error */
    }

    return 0;
}

int16_t ms5611_adc_start(ms5611_t *ms5611, uint8_t adc, uint8_t osr)
{
    uint8_t cmd;

    if (adc == MS5611_ADC_PRESS) {
        cmd = MS5611_CMD_ADC_PRESS(osr);
    } else {
        cmd = MS5611_CMD_ADC_TEMP(osr);
    }

    if (ms5611_command(ms5611, cmd, NULL, 0)) {
        return -1;
    } else {
        return ms5611_osr_dly_us[osr];
    }
}

int ms5611_adc_read(ms5611_t *ms5611, uint32_t *adc_result)
{
    return ms5611_command(ms5611, MS5611_CMD_ADC_READ, adc_result, 3);
}

int32_t ms5611_calc_temp(ms5611_t *ms5611, uint32_t adc_temp)
{
    int32_t dt, temp;

    dt = (int32_t) adc_temp - ms5611->prom[PROM_TREF] * (1<<8);
    temp = (int32_t) 2000 + (int64_t) dt * ms5611->prom[PROM_TEMPSENS] / (1<<23);
    /* low temperature correcture, (temp < 20.00 C) */
    if (temp < 2000) {
        temp = temp - SQUARE((int64_t)dt) / (1<<31);
    }
    return temp;
}

uint32_t ms5611_calc_press(ms5611_t *ms5611, uint32_t adc_press, uint32_t adc_temp, int32_t *p_temp)
{
    int32_t dt, temp;
    int64_t off, sens;

    dt = (int32_t) adc_temp - ms5611->prom[PROM_TREF] * (1<<8);
    temp = (int32_t) 2000 + (int64_t) dt * ms5611->prom[PROM_TEMPSENS] / (1<<23);

    off = (int64_t) ms5611->prom[PROM_OFF] * (1<<16) + (int64_t) ms5611->prom[PROM_TCO] * dt / (1<<7);
    sens = (int64_t) ms5611->prom[PROM_SENS] * (1<<15) + (int64_t) ms5611->prom[PROM_TCS] * dt / (1<<8);

    /* low temperature correcture, (temp < 20.00 C) */
    if (temp < 2000) {
        uint32_t t2, off2, sens2;

        t2 = (uint32_t) (SQUARE((int64_t)dt) / (1<<31));
        off2 = (uint32_t) (5 * SQUARE(temp - 2000)) / 2;
        sens2 = off2 / 2;

        /* very low temperature correcture (temp < -15.00 C) */
        if (temp < -1500) {
            off2 = off2 + 7 * SQUARE(temp + 1500);
            sens2 = sens2 + ((11 * SQUARE(temp + 1500)) / 2);
        }

        /* correction */
        temp = temp - t2;
        off = off - off2;
        sens = sens - sens2;
    }

    if (p_temp != NULL) {
        *p_temp = temp;
    }

    /* calculate pressure */
    uint32_t p = (uint32_t) ((adc_press * sens / (1<<21)) - off) / (1<<15);
    return p;
}
