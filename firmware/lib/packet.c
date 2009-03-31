#include <stdint.h>
#include <stdlib.h>

#include <avr/io.h>

#include "packet.h"
#include "bus.h"
#include "frame.h"

struct frame *inframe = NULL;
struct frame outframe;
address_t   packet_address = 0x23;
seq_t       packet_ack_seq;
uint8_t packet_acked; 

void packet_init(void)
{
    bus_init();
    DDRC |= (1<<PC2);
}

inline uint8_t packet_isUnicast(struct ubpacket * in)
{
    return in->flags & UB_PACKET_UNICAST;
}

inline struct ubpacket * packet_getSendBuffer(void)
{
    return (struct ubpacket *) outframe.data;
}

void packet_send(void)
{
    struct ubpacket *out = (struct ubpacket *)outframe.data;
    outframe.len = out->len+UB_PACKET_HEADER;
    bus_send(&outframe,1);
}

void packet_ack(struct ubpacket * p)
{
    struct ubpacket *out = packet_getSendBuffer();
    out->flags = UB_PACKET_ACK;
    out->seq = p->seq;
    out->len = 0;
    out->dest = p->src;
    out->src = p->dest;
    packet_send();
}

uint8_t packet_isLocal(address_t adr)
{
    return adr == packet_address;
}

void packet_process(struct ubpacket * in)
{
    if( packet_isUnicast(in)) {
        packet_ack(in);
    }
}

void packet_in(struct ubpacket * in)
{
    if( packet_isLocal(in->dest) ){
        //Is this an ack packet for us?
        if( in->flags & UB_PACKET_ACK && in->seq == packet_ack_seq){
            packet_acked = 1;
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
    PORTC &= ~(1<<PC2);
}

