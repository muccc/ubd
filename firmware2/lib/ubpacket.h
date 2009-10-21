#ifndef __PACKET_H_
#define __PACKET_H_
#include <stdint.h>
#include "ubconfig.h"

typedef uint8_t    address_t;
typedef uint8_t     seq_t;

struct ubheader_t{
    address_t src;
    address_t dest;
    uint8_t flags;
    uint8_t len;
};

struct ubpacket_t{
    struct ubheader_t header;
    uint8_t data[UB_PACKETLEN - sizeof(struct ubheader_t)];
};

#define UB_PACKET_HEADER        (sizeof(struct ubheader_t)

#define UB_PACKET_ACK           1
#define UB_PACKET_SEQ           2


#define PACKET_TIMEOUT          200     //we wait max 200ms for an ack

void ubpacket_init(void);

void packet_send(void);
//uint8_t packet_sendstate(void);

//uint8_t packet_recv(struct ubpacket*);
//uint8_t packet_recvstate(void);

void ubpacket_tick(void);
inline struct ubpacket_t * ubpacket_getSendBuffer(void);
void ubpacket_send(void);
uint8_t packet_done(void);
uint8_t ubpacket_free(void);

inline uint8_t ubpacket_gotPacket(void);
inline struct ubpacket_t * ubpacket_getIncomming(void);
inline void ubpacket_processed(void);

void ubpacket_process(void);
void ubpacket_processPacket(struct ubpacket_t * in);
#define UB_PACKET_IDLE         0
#define UB_PACKET_BUSY          1
#define UB_PACKET_NEW           4
#endif
