#ifndef __PACKET_H_
#define __PACKET_H_
#include <stdint.h>

#include "frame.h"

typedef uint8_t    address_t;
typedef uint8_t     seq_t;

#define UB_PACKET_HEADER        5
#define UB_PACKET_DATA          (FRAME_MAX-UB_PACKET_HEADER)
struct ubpacket{
    address_t src;
    address_t dest;
    uint8_t flags;
    seq_t seq;
    uint8_t len;
    uint8_t data[UB_PACKET_DATA];
};

#define UB_PACKET_ACK           1

#define UB_ADDRESS_MULTICAST     (1<<(sizeof(address_t)*8-1)) //first bit is one
#define UB_ADDRESS_BROADCAST     ((1<<sizeof(address_t)*8)-1)  //all ones

#define PACKET_TIMEOUT          200     //we wait max 200ms for an ack
#endif
