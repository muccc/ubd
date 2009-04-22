#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include <avr/io.h>

#include "packet.h"
#include "bus.h"
#include "frame.h"
#include "serial_handler.h"

struct frame outframe;
struct ubpacket inpacket;

address_t   packet_address = 0x23;
seq_t       packet_ack_seq;

uint8_t packet_acked = 1;
uint8_t packet_incomming;
uint8_t packet_timeout;
uint8_t packet_sniff = 0;
uint8_t packet_fired = 0;

void packet_init(uint8_t adr)
{
    bus_init();
    DDRC |= (1<<PC2);
    PORTD |= (1<<PD7);
    packet_address = adr;
    if(adr == 0){
        if( PIND & (1<<PD7) ){
            packet_address = 'A';
        }else{
            packet_address = 'B';
        }
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
    packet_acked = 0;
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

void packet_ack(struct ubpacket * p)
{
    struct frame f;
    struct ubpacket *out = (struct ubpacket *) f.data;
    out->flags = UB_PACKET_ACK;
    out->seq = p->seq;
    out->len = 0;
    out->dest = p->src;
    out->src = p->dest;
    packet_transmit(&f);
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

#define MAX_HOSTS   2
struct {
    address_t src;
    seq_t     seq;
}seqs[MAX_HOSTS];

//checks if seq was seen before. A round-robin table is used to
//keep track of the sequence numbers.
//returns 1 if the packet is new
uint8_t packet_checkseq(struct ubpacket * p, uint8_t update)
{
    static uint8_t seqrr = 0;
    uint8_t i;
    uint8_t ret;
    address_t src = p->src;
    seq_t seq = p->seq;

    for(i=0;i<MAX_HOSTS;i++){
        if(seqs[i].src == src){                 //host known
            ret = seqs[i].seq!=seq?1:0;       //check for old seq
            if(update)
                seqs[i].seq = seq;
                                    //TODO: add some real seq counting here
            return ret;
        }
    }                                           //host unknown
    if(!update)
        return 1;
    seqs[seqrr].src = src;                      //add host to table
    seqs[seqrr++].seq = seq;
    if(seqrr == MAX_HOSTS)                      //round robin style
        seqrr = 0;
    return 1;
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
//        DEBUG("broadcast");
        memcpy(&inpacket,in,sizeof(in));
        packet_incomming = 1;                   //mark packet as new
    }
}

void packet_in(struct ubpacket * in)
{
    if( packet_isLocal(in->dest) || packet_sniff){
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
    //struct frame *inframe; //= NULL;
    //PORTC |= (1<<PC2);
    struct ubpacket * in;
    bus_tick();
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

