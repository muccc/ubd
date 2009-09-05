#ifndef __UB_CONFIG_H_
#define __UB_CONFIG_H_
#include <stdint.h>

#define RS485_BITRATE       115200
#define RS485_ISR_EDGE      PCINT3_vect

#define UB_ESCAPE     '\\'
#define UB_START        '0'
#define UB_STOP         '1'
#define UB_DISCOVER     '2'
#define UB_QUERY        '3'

#define UB_NODEMAX      128

#define UB_MASTER       0
#define UB_CLIENT       1

#define UB_ENABLEMASTER 1
//#define UB_ENABLECLIENT 1

#define UB_HOSTADR      1
#define UB_MASTERADR    2

#define UB_QUERYMAX     30

#define UB_ERROR        1
#define UB_OK           0

struct ub_config {
    uint8_t rs485master;
    uint8_t rs485client;
};

extern struct ub_config ubconfig;
#endif

