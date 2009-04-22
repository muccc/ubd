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
//#define UB_PACKET_UNICAST       2
//#define UB_PACKET_BROADCAST     4
//#define UB_PACKET_MULTICAST     8


#define UB_ADDRESS_MULTICAST     (1<<(sizeof(address_t)*8-1))     //first bit is one
#define UB_ADDRESS_BROADCAST     ((1<<sizeof(address_t)*8)-1)     //all ones

#define PACKET_TIMEOUT          200     //we wait max 200ms for an ack

void packet_init(uint8_t adr);
void packet_send(void);
//uint8_t packet_sendstate(void);

//uint8_t packet_recv(struct ubpacket*);
//uint8_t packet_recvstate(void);

void packet_tick(void);
inline struct ubpacket * packet_getSendBuffer(void);
void packet_send(void);
uint8_t packet_done(void);
inline uint8_t packet_gotPacket(void);
inline void packet_processed(void);
inline struct ubpacket * packet_getIncomming(void);

#define UB_PACKET_IDLE         0
#define UB_PACKET_BUSY          1
#define UB_PACKET_TIMEOUT       2


#define UB_PACKET_NEW           4
#endif
