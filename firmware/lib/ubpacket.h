#ifndef __PACKET_H_
#define __PACKET_H_
#include <stdint.h>
#include "ubconfig.h"

struct ubheader_t{
    ubaddress_t src;
    ubaddress_t dest;
    uint8_t flags;
    uint8_t len;
};

struct ubpacket_t{
    struct ubheader_t header;
    uint8_t data[UB_PACKETLEN - sizeof(struct ubheader_t)];
};

#define UB_PACKET_HEADER        (sizeof(struct ubheader_t)

#define UB_PACKET_ACK           (1<<0)
#define UB_PACKET_SEQ           (1<<1)
#define UB_PACKET_NOACK         (1<<3)
#define UB_PACKET_MGT           (1<<4)
#define UB_PACKET_UNSOLICITED   (1<<5)  //when the node starts talking
#define UB_PACKET_ACKSEQ        (1<<6)

#define UB_PACKET_IDLE          0
#define UB_PACKET_BUSY          1
#define UB_PACKET_NEW           4

void ubpacket_init(void);
void ubpacket_tick(void);
void ubpacket_process(void);
void ubpacket_processPacket(struct ubpacket_t * in);

uint8_t ubpacket_free(void);
inline struct ubpacket_t * ubpacket_getSendBuffer(void);
void ubpacket_send(void);

inline uint8_t ubpacket_gotPacket(void);
inline struct ubpacket_t * ubpacket_getIncomming(void);
inline void ubpacket_processed(void);
uint8_t ubpacket_isUnsolicitedDone(void);
void ubpacket_setUnsolicited(void);
#endif
