#ifndef __RF12PACKET_H_
#define __RF12PACKET_H_
#include "packet.h"
struct rf12packet_t {
    uint8_t         type;
    uint8_t         seq;
    struct packet_t   packet;
};

struct rf12ack_t {
    uint8_t         type;
    uint8_t         seq;
    uint8_t         src;
    uint8_t         dest;
};

#define     RF12PACKET_HEADERLEN        (sizeof(struct rf12packet_t)-sizeof(struct packet_t))

#define STATE_SENDPACKET    0
#define STATE_WAITFREE      1
#define STATE_WAITRND       2
#define STATE_SEND          3
#define STATE_WAITACK       4
#define STATE_IDLE          5


#define PACKET_ACK          0
#define PACKET_DATA         1
#define PACKET_DATAACK      2

#define RF12PACKET_NEWDATA  1
#define RF12PACKET_TIMEOUT  2
#define RF12PACKET_PACKETDONE 4

void rf12packet_tick(void);
uint8_t rf12packet_packetOut(struct packet_t * p);
uint8_t rf12packet_nextHeader(struct packet_t * p);
uint8_t rf12packet_packetIn(struct packet_t * p);
uint8_t rf12packet_ready(void);

void rf12packet_setadr(unsigned char adr);
void rf12packet_process(struct rf12packet_t  * packet, unsigned char len);
void rf12packet_init(unsigned char adr);
unsigned char rf12packet_getstatus(void);
unsigned char rf12packet_incrseq(void);

//extern unsigned char rf12packet_data[50];
//extern unsigned char rf12packet_datalen;
unsigned char rf12packet_status;
extern unsigned char rf12packet_sniff;
unsigned char rf12packet_isidle(void);
extern uint8_t state;
#endif
