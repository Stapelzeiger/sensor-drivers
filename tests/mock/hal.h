#ifndef HAL_H
#define HAL_H

#ifdef __cplusplus
extern "C" {
#endif

#include <stdlib.h>

typedef int I2CDriver;
typedef int msg_t;
const int MSG_OK = 1;

msg_t i2cMasterReceive(I2CDriver *d, uint8_t addr, void *buffer, size_t len);

#ifdef __cplusplus
}
#endif

#endif /* HAL_H */
