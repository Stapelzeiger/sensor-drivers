#include "../ms4525do.h"
#include <CppUTest/TestHarness.h>
#include "CppUTestExt/MockSupport.h"

TEST_GROUP(MS4525DO)
{
    I2CDriver i2c;
    ms4525do_t d;
    uint16_t pressure_raw, temperature_raw;

    void teardown() {
        mock().clear();
    }
};

TEST(MS4525DO, init_driver_pointer)
{
    CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', 'I', 2, 'D'));
    CHECK_EQUAL(&i2c, d.i2c_driver);
}

TEST(MS4525DO, init_type_A_and_B)
{
    CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', 'I', 2, 'D'));
    CHECK_EQUAL(80, d.output_scale_percent);
    CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'B', 'I', 2, 'D'));
    CHECK_EQUAL(90, d.output_scale_percent);
}

TEST(MS4525DO, init_invalid_type)
{
    CHECK_EQUAL(-1, ms4525do_init(&d, &i2c, 'C', 'I', 2, 'D'));
}

TEST(MS4525DO, init_addr_I_J_and_K)
{
    CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', 'I', 2, 'D'));
    CHECK_EQUAL(0x28, d.addr);
    CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', 'J', 2, 'D'));
    CHECK_EQUAL(0x36, d.addr);
    CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', 'K', 2, 'D'));
    CHECK_EQUAL(0x46, d.addr);
}

TEST(MS4525DO, init_addr_0_to_9)
{
    for (int i = 0; i <= 9; i++) {
        CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', '0'+i, 2, 'D'));
        CHECK_EQUAL(0x48 + i, d.addr);
    }
}

TEST(MS4525DO, init_invalid_addr)
{
    CHECK_EQUAL(-2, ms4525do_init(&d, &i2c, 'A', 'X', 2, 'D'));
}

const float PSI = 6894.75728f; // 1 psi in Pa

TEST(MS4525DO, init_pressure_range)
{
    uint8_t pressures[] = {2, 4, 5, 10, 20, 30};
    for (unsigned i = 0; i < sizeof(pressures); i++) {
        CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', 'I', pressures[i], 'D'));
        CHECK_EQUAL(pressures[i]*PSI, d.pressure_max);
    }
}

TEST(MS4525DO, init_invalid_pressure_range)
{
    CHECK_EQUAL(-3, ms4525do_init(&d, &i2c, 'A', 'I', 3, 'D'));
}

TEST(MS4525DO, init_pressure_type)
{
    CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', 'I', 2, 'D'));
    CHECK_EQUAL(-d.pressure_max, d.pressure_min);
    CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', 'I', 2, 'A'));
    CHECK_EQUAL(0, d.pressure_min);
    CHECK_EQUAL(0, ms4525do_init(&d, &i2c, 'A', 'I', 2, 'G'));
    CHECK_EQUAL(0, d.pressure_min);
}

TEST(MS4525DO, init_invalid_pressure_type)
{
    CHECK_EQUAL(-4, ms4525do_init(&d, &i2c, 'A', 'I', 2, 'F'));
}

extern "C"
int ms4525do_extract_pressure_temperature_from_buf(uint8_t buf[4], uint16_t *pressure, uint16_t *temperature);

TEST(MS4525DO, extract_status)
{
    uint8_t buf[4] = {3<<6, 0, 0, 0};
    int status = ms4525do_extract_pressure_temperature_from_buf(buf, &pressure_raw, &temperature_raw);
    CHECK_EQUAL(3, status);
}

TEST(MS4525DO, extract_pressure)
{
    uint8_t buf[4] = {0xff, 0xab, 0, 0};
    ms4525do_extract_pressure_temperature_from_buf(buf, &pressure_raw, &temperature_raw);
    CHECK_EQUAL(0x3fab, pressure_raw);
}

TEST(MS4525DO, extract_temperature)
{
    uint8_t buf[4] = {0, 0, 0xab, 0xff};
    ms4525do_extract_pressure_temperature_from_buf(buf, &pressure_raw, &temperature_raw);
    CHECK_EQUAL((0x00ab<<3)+0x07, temperature_raw);
}

TEST(MS4525DO, pressure_conversion_absolute_type_A)
{
    ms4525do_init(&d, &i2c, 'A', 'I', 2, 'A');
    DOUBLES_EQUAL(0, ms4525do_pressure_raw_to_Pa(&d, 1638), 1); // values from datasheet table
    DOUBLES_EQUAL(1*PSI, ms4525do_pressure_raw_to_Pa(&d, 8192), 1);
    DOUBLES_EQUAL(2*PSI, ms4525do_pressure_raw_to_Pa(&d, 14746), 3);
}

TEST(MS4525DO, pressure_conversion_differential_type_B)
{
    ms4525do_init(&d, &i2c, 'B', 'I', 2, 'D');
    DOUBLES_EQUAL(-2*PSI, ms4525do_pressure_raw_to_Pa(&d, 819), 1); // values from datasheet table
    DOUBLES_EQUAL(0, ms4525do_pressure_raw_to_Pa(&d, 8192), 1);
    DOUBLES_EQUAL(2*PSI, ms4525do_pressure_raw_to_Pa(&d, 15563), 3);
}

TEST(MS4525DO, temperature_conversion)
{
    ms4525do_init(&d, &i2c, 'B', 'I', 2, 'D');
    DOUBLES_EQUAL(-50, ms4525do_temperature_raw_to_Celsius(&d, 0), 0.1); // values from datasheet table
    DOUBLES_EQUAL(0, ms4525do_temperature_raw_to_Celsius(&d, 511), 0.1);
    DOUBLES_EQUAL(25, ms4525do_temperature_raw_to_Celsius(&d, 767), 0.1);
    DOUBLES_EQUAL(150, ms4525do_temperature_raw_to_Celsius(&d, 2047), 0.1);
}

TEST(MS4525DO, read_raw)
{
    uint8_t buf[] = {0, 100, 10, 0};
    ms4525do_init(&d, &i2c, 'A', 'I', 2, 'D');
    mock().expectOneCall("i2cMasterReceive")
        .withParameter("d", &i2c)
        .withParameter("addr", d.addr)
        .withOutputParameterReturning("buffer", buf, sizeof(buf))
        .withParameter("len", 4)
        .andReturnValue(MSG_OK);

    uint16_t pressure_raw, temperature_raw;
    CHECK_EQUAL(MS4525DO_READ_RES_OK, ms4525do_read_raw(&d, &pressure_raw, &temperature_raw));
    CHECK_EQUAL(100, pressure_raw);
    CHECK_EQUAL(10<<3, temperature_raw);

    mock().checkExpectations();
}

TEST(MS4525DO, read_raw_fail)
{
    uint8_t buf[] = {0, 0, 0, 0};
    ms4525do_init(&d, &i2c, 'A', 'I', 2, 'D');
    mock().expectOneCall("i2cMasterReceive")
        .withParameter("d", &i2c)
        .withParameter("addr", d.addr)
        .withOutputParameterReturning("buffer", buf, sizeof(buf))
        .withParameter("len", 4)
        .andReturnValue(-1);

    CHECK_EQUAL(MS4525DO_READ_RES_I2C_ERR, ms4525do_read_raw(&d, &pressure_raw, &temperature_raw));

    mock().checkExpectations();
}

TEST(MS4525DO, read)
{
    uint16_t press_raw = 8192, temp_raw = 767;
    uint8_t buf[] = {(uint8_t)(press_raw>>8), (uint8_t)press_raw,
                     (uint8_t)(temp_raw>>3), (uint8_t)(temp_raw<<5)};
    ms4525do_init(&d, &i2c, 'B', 'I', 2, 'D');
    mock().expectOneCall("i2cMasterReceive")
        .withParameter("d", &i2c)
        .withParameter("addr", d.addr)
        .withOutputParameterReturning("buffer", buf, sizeof(buf))
        .withParameter("len", 4)
        .andReturnValue(MSG_OK);

    float pressure, temperature;
    CHECK_EQUAL(MS4525DO_READ_RES_OK, ms4525do_read(&d, &pressure, &temperature));
    DOUBLES_EQUAL(0, pressure, 1);
    DOUBLES_EQUAL(25, temperature, 0.1);

    mock().checkExpectations();
}

TEST(MS4525DO, read_fail)
{
    uint8_t buf[] = {0, 0, 0, 0};
    ms4525do_init(&d, &i2c, 'A', 'I', 2, 'D');
    mock().expectOneCall("i2cMasterReceive")
        .withParameter("d", &i2c)
        .withParameter("addr", d.addr)
        .withOutputParameterReturning("buffer", buf, sizeof(buf))
        .withParameter("len", 4)
        .andReturnValue(-1);

    float pressure, temperature;
    CHECK_EQUAL(MS4525DO_READ_RES_I2C_ERR, ms4525do_read(&d, &pressure, &temperature));

    mock().checkExpectations();
}
