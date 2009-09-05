#ifndef __UB_H_
#define __UB_H_
#include <stdint.h>

#define UB_ERROR        1
#define UB_OK           0

#define UB_MASTER       0
#define UB_CLIENT       1

struct ub_config {
    uint8_t rs485master;
    uint8_t rs485client;
};

extern struct ub_config ubconfig;
void ub_init(uint8_t ubmode);

#endif
