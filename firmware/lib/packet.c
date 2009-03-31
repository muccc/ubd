#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>

#include "packet.h"
#include "bus.h"
#include "frame.h"

struct frame *inframe = NULL;
struct frame outframe;
address_t   packet_address = 0x23;
seq_t       packet_ack_seq;
uint8_t packet_acked = 1;
struct ubpacket inpacket;
uint8_t packet_incomming;
uint8_t packet_timeout;

void packet_init(void)
{
    bus_init();
    DDRC |= (1<<PC2);
}

inline uint8_t packet_isUnicast(struct ubpacket * in)
{
    return in->flags & UB_PACKET_UNICAST;
}

inline uint8_t packet_isBroadcast(struct ubpacket * in)
{
    return in->flags & UB_PACKET_BROADCAST;
}

inline struct ubpacket * packet_getSendBuffer(void)
{
    return (struct ubpacket *) outframe.data;
}

void packet_transmit(void)
{
    struct ubpacket *out = (struct ubpacket *)outframe.data;
    outframe.len = out->len+UB_PACKET_HEADER;
    bus_send(&outframe,1);
}

void packet_send(void)
{
    struct ubpacket *out = (struct ubpacket *)outframe.data;
    packet_timeout = PACKET_TIMEOUT;
    out->seq = packet_ack_seq;
    out->src = packet_address;
    packet_acked = 0;
    packet_transmit();
}

uint8_t packet_done(void)
{
    return packet_acked;
}

void packet_ack(struct ubpacket * p)
{
    struct ubpacket *out = packet_getSendBuffer();
    out->flags = UB_PACKET_ACK;
    out->seq = p->seq;
    out->len = 0;
    out->dest = p->src;
    out->src = p->dest;
    packet_transmit();
}

uint8_t packet_isLocal(address_t adr)
{
    return adr == packet_address;
}

inline uint8_t packet_gotPacket(void)
{
    return packet_incomming;
}

inline void packet_processed(void)
{
    packet_incomming = 0;
}

void packet_process(struct ubpacket * in)
{
    if(packet_incomming)            //There is still an unprocessed packet
                                    //in the buffer
        return;                     //Just wait for the other end to retry

    if( packet_isUnicast(in) ){
        packet_ack(in);
        if(inpacket.seq != in->seq){               //avoid receiving duplicates
                                    //TODO: add some real seq counting here
            memcpy(&inpacket,in,sizeof(in));
            packet_incomming = 1;                   //mark packet as new
        }
    }else if( packet_isBroadcast(in) ){
        memcpy(&inpacket,in,sizeof(in));
        packet_incomming = 1;                   //mark packet as new
    }
}

void packet_in(struct ubpacket * in)
{
    if( packet_isLocal(in->dest) ){
        //Is this an ack packet for us?
        if( in->flags & UB_PACKET_ACK && in->seq == packet_ack_seq){
            packet_ack_seq++;
            packet_acked = 1;
            in->flags &= ~UB_PACKET_ACK;
        }
        //Data packed by?
        if( in->len > 0 ){
            packet_process(in);
        }
    }
}

void packet_tick(void)
{
    PORTC |= (1<<PC2);
    struct ubpacket * in;
    bus_tick();
    if(bus_receive() && inframe == NULL){
        //Keep track of the frame
        inframe = bus_frame;                 //no cli needed
        //cast the data part into a packet
        in = (struct ubpacket *) bus_frame->data;
        if(in->len > (inframe->len-UB_PACKET_HEADER)){
            //discard
        }else{
            packet_in(in);
        }
        inframe->isnew = 0;
        inframe = NULL;
    }
    if(!packet_acked && packet_timeout && --packet_timeout==0){
        packet_send();
    }
    PORTC &= ~(1<<PC2);
}

