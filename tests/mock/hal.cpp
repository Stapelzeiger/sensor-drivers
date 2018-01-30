#include "hal.h"
#include "CppUTest/TestHarness.h"
#include "CppUTestExt/MockSupport.h"


msg_t i2cMasterReceive(I2CDriver *d, uint8_t addr, void *buffer, size_t len)
{
    mock().actualCall("i2cMasterReceive")
        .withParameter("d", d)
        .withParameter("addr", addr)
        .withOutputParameter("buffer", buffer)
        .withParameter("len", len);
    return mock().intReturnValue();;
}
