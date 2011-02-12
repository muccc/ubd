#ifndef __UB_CONFIG_H_
#define __UB_CONFIG_H_
#include <stdint.h>

#define RS485_BITRATE       115200
#define RS485_ISR_EDGE      PCINT3_vect

#define RS485_DE_PIN        PC4
#define RS485_DE_PORT       PORTC
#define RS485_DE_DDR        DDRC

#define RS485_nRE_PIN       PC5
#define RS485_nRE_PORT      PORTC
#define RS485_nRE_DDR       DDRC

#define UB_PACKETLEN        50

#define UB_ESCAPE     '\\'
#define UB_NONE         '0'
#define UB_START        '1'
#define UB_STOP         '2'
#define UB_DISCOVER     '3'
#define UB_QUERY        '4'
#define UB_BOOTLOADER   '5'

#define UB_NODEMAX      128


#define UB_ENABLEMASTER 1
#define UB_ENABLESLAVE 1

#define UB_HOSTADR      1
#define UB_MASTERADR    2

#define UB_QUERYMAX     30
#define UB_MAXMULTICAST 8

#define UB_PACKET_TIMEOUT   2500
#define UB_PACKET_RETRIES   5

typedef uint8_t ubaddress_t;

#define UB_ADDRESS_MASTER       1
#define UB_ADDRESS_BRIDGE       2

#define UB_ADDRESS_BROADCAST     ((1<<sizeof(address_t)*8)-1)     //all ones
#define UB_ADDRESS_MULTICAST     (1<<(sizeof(address_t)*8-1))     //first bit is one

#define UB_INTERVAL         500
#define UB_CLASS           1
#endif

