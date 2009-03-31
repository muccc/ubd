#ifndef __PACKET_H_
#define __PACKET_H_
#include <stdint.h>

#include "frame.h"
#include "bus.h"

typedef uint8_t    address_t;
typedef uint8_t     seq_t;

struct ubpacket{
    address_t src;
    address_t dest;
    uint8_t flags;
    seq_t seq;
    uint8_t len;
    uint8_t data[FRAME_MAX];
};

#define UB_PACKET_HEADER        (sizeof(struct ubpacket) - FRAME_MAX)

#define UB_PACKET_ACK           1
#define UB_PACKET_UNICAST       2
#define UB_PACKET_BROADCAST     4
#define UB_PACKET_MULTICAST     8


void packet_init(void);
void packet_send(void);
uint8_t packet_sendstate(void);

uint8_t packet_recv(struct ubpacket*);
uint8_t packet_recvstate(void);

void packet_tick(void);

#define UB_PACKET_IDLE         0
#define UB_PACKET_BUSY          1
#define UB_PACKET_TIMEOUT       2


#define UB_PACKET_NEW           4
#endif
