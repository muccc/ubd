#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>

#include "packet.h"
#include "bus.h"
#include "frame.h"
#include "serial_handler.h"
#include "busmgt.h"

struct frame outframe;
struct ubpacket inpacket;

address_t   packet_address;
address_t   packet_master;

seq_t       packet_ack_seq;
typedef uint16_t    time_t;

uint8_t packet_acked = 1;
uint8_t packet_incomming;
uint8_t packet_timeout;
uint8_t packet_sniff = 0;
uint8_t packet_fired = 0;

void packet_init(uint8_t adr, uint8_t sniff)
{
    bus_init();
    DDRC |= (1<<PC2);
    PORTD |= (1<<PD7);
    packet_address = adr;
    packet_sniff = sniff;
}

void packet_setAdr(address_t adr)
{
    packet_address = adr;
}

void packet_setMaster(address_t master)
{
    packet_master = master;
}

address_t packet_getMaster(void)
{
    return packet_master;
}

void packet_setMode(uint8_t mode)
{
    switch(mode){
        case 0:
            packet_sniff = 0;
        break;
        case 1:
            packet_sniff = 1;
        break;
    }
}

inline struct ubpacket * packet_getIncomming(void)
{
    return &inpacket;
}

inline uint8_t packet_isBroadcast(struct ubpacket * in)
{
    return in->dest == UB_ADDRESS_BROADCAST;
}

inline uint8_t packet_isMulticast(struct ubpacket * in)
{
    return (in->dest & UB_ADDRESS_MULTICAST) && (!packet_isBroadcast(in));
}

inline uint8_t packet_isUnicast(struct ubpacket * in)
{
    return !packet_isBroadcast(in) && !packet_isMulticast(in);
}

uint8_t packet_isLocal(struct ubpacket * in)
{
    return packet_isBroadcast(in) || in->dest == packet_address; //|| packet_isUnicast(in);          //TODO: check own mulicast
}

inline struct ubpacket * packet_getSendBuffer(void)
{
    return (struct ubpacket *) outframe.data;
}

void packet_transmit(struct frame * f)
{
//    DEBUG("s");
    struct ubpacket *out = (struct ubpacket *)(f->data);
    f->len = out->len+UB_PACKET_HEADER;
    bus_send(f,1);
//    DEBUG("S");
}

void packet_send(void)
{
    struct ubpacket *out = (struct ubpacket *)outframe.data;
    packet_timeout = PACKET_TIMEOUT;
    out->seq = packet_ack_seq;
    out->src = packet_address;
    if(packet_isUnicast(out))
        packet_acked = 0;
    else
        packet_acked = 1;
    packet_transmit(&outframe);
    packet_fired = 1;
}

uint8_t packet_done(void)
{
    if(packet_fired && packet_acked){
        packet_fired = 0;
        return 1;
    }
    return 0;
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
    if(packet_sniff){
        memcpy(&inpacket,in,sizeof(inpacket));
        packet_incomming = 1;                   //mark packet as new
    }else if(!packet_acked){        //guarantee the userland that the buffer
                              //is empty when a new packet arrives
        if(packet_isUnicast(in))
            if( !packet_checkseq(in,0) )    //this ack was lost
                packet_ack(in);             //send it again
        //this packet is ignored.
        //the sender will try again.
    }else if( packet_isUnicast(in) ){
        //DEBUG("unicast");
        packet_ack(in);
//        if(inpacket.seq != in->seq || !packet_valid){    //avoid receiving duplicates
        if( packet_checkseq(in,1)){
            PORTB |= (1<<PB0);
            memcpy(&inpacket,in,sizeof(inpacket));
            packet_incomming = 1;                   //mark packet as new
            PORTB &= ~(1<<PB0);
        }
    }else if( packet_isBroadcast(in) ){
        //DEBUG("broadcast");
        memcpy(&inpacket,in,sizeof(inpacket));
        packet_incomming = 1;                   //mark packet as new
    }

    if(packet_incomming){
#ifndef CFG_BRIDGE
        if(busmgt_process(&inpacket)){
            packet_incomming = 0;
        }
#endif
    }
}

void packet_in(struct ubpacket * in)
{
    if( packet_isLocal(in) || packet_sniff){
        //Is this an ack packet for us?
        //DEBUG("local");
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
    //struct frame *inframe; //= NULL;
    //PORTC |= (1<<PC2);
    struct ubpacket * in;
    bus_tick();
    packet_seqtick();
    if(bus_receive() ){ //&& inframe == NULL){
        //Keep track of the frame
        //inframe = bus_frame;                 //no cli needed
        //cast the data part into a packet
        //DEBUG("I");
        in = (struct ubpacket *) bus_frame->data;
        if( (in->len + UB_PACKET_HEADER) > bus_frame->len ){
            //discard
        }else{
            packet_in(in);
        }
        //inframe->isnew = 0;
        //inframe = NULL;
        bus_frame->isnew = 0;
    }
    if(!packet_acked && packet_timeout && --packet_timeout==0){
        packet_send();
    }
    //PORTC &= ~(1<<PC2);
}

