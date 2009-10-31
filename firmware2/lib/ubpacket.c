#include <avr/io.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ubpacket.h"
#include "serial_handler.h"
#include "busmgt.h"
#include "ubstat.h"
#include "ubaddress.h"
#include "ub.h"
#include "ubmaster.h"

typedef uint16_t    time_t;

struct ubpacket_t outpacket;
struct ubpacket_t inpacket;
//struct ubpacket_t uspacket;     //packet for unsolicitet transmits

uint8_t packet_out_full = 0;
uint8_t packet_fired = 0;
uint8_t packet_acked = 1;
uint8_t packet_ack_full = 0;
struct ubheader_t ack;

uint8_t packet_retries;
uint8_t packet_incomming;
time_t  packet_timeout;
uint8_t packet_sniff = 0;

uint8_t packet_outseq = 0;

void ubpacket_init(void)
{
}

inline struct ubpacket_t * ubpacket_getIncomming(void)
{
    return &inpacket;
}

inline uint8_t ubpacket_gotPacket(void)
{
    return packet_incomming;
}

inline void ubpacket_processed(void)
{
    packet_incomming = 0;
}

inline struct ubpacket_t * ubpacket_getSendBuffer(void)
{
    return &outpacket;
}

void ubpacket_send(void)
{
    //don't do anything until we are configured
    if( ubconfig.configured == 0){
        return;
    }
    packet_timeout = UB_PACKET_TIMEOUT;

    //packet_acked won't be set if this is a retransmit
    if(ubadr_isUnicast(outpacket.header.dest) &&
        packet_acked &&
        !(outpacket.header.flags & UB_PACKET_NOACK)){
        packet_retries = 0;
#ifdef UB_ENABLEMASTER
if( ubconfig.master ){
        struct ubstat_t * flags = ubstat_getFlags(outpacket.header.dest);
        //toggle the seq bit
        packet_outseq = flags->outseq?0:1;
        flags->outseq = packet_outseq;
}
#endif
#ifdef UB_ENABLESLAVE
if ( ubconfig.slave ){
        packet_outseq = packet_outseq?0:1;
}
#endif
    }
    //the application can set some own flags
    outpacket.header.flags &= (UB_PACKET_DONE | UB_PACKET_NOACK);

    if( !packet_acked)
        outpacket.header.flags |= 0x80;

    if( packet_outseq )
        outpacket.header.flags |= UB_PACKET_SEQ;

    //outpacket.header.src = ubadr_getAddress();

    if(ubadr_isUnicast(outpacket.header.dest) &&
        !(outpacket.header.flags & UB_PACKET_NOACK)){
        packet_acked = 0;
    }else{
        //don't wait for an ack
        packet_acked = 1;
    }

    packet_out_full = 1;

    PORTA ^= (1<<4);
    if( ub_sendPacket(&outpacket) == UB_OK ){
        packet_fired = 1;
#ifdef UB_ENABLEMASTER
        serial_sendFrames("DsPok");
#endif
    }else{
        PORTA ^= (1<<4);
        packet_fired = 0;
#ifdef UB_ENABLEMASTER
        serial_sendFrames("DsPerror");
#endif
    }
}

uint8_t ubpacket_free(void)
{
    return !packet_out_full;
}

/*uint8_t ubpacket_done(void)
{
    if(packet_out_full && packet_acked){
        packet_out_full = 0;
        return 1;
    }
    return 0;
}*/

static void packet_ack(struct ubpacket_t * p)
{
    ack.src = p->header.dest;
    ack.dest = p->header.src;
    ack.flags = p->header.flags & (UB_PACKET_SEQ | 0x40);
    ack.flags |= UB_PACKET_ACK;
    ack.len = 0;
    if( ub_sendPacket((struct ubpacket_t *)&ack) == UB_ERROR ){
#ifdef UB_ENABLEMASTER
        serial_sendFrames("DAsPerror");
#endif
        packet_ack_full = 1;
    }else{
#ifdef UB_ENABLEMASTER
        serial_sendFrames("DAsPOK");
#endif
    }
}

static void ubpacket_abort(void)
{
#ifdef UB_ENABLEMASTER
if( ubconfig.master ){
    if( outpacket.header.src == UB_ADDRESS_MASTER ){
        ubmaster_abort();
    }
    serial_sendFrames("Dabort");
}
#endif
    PORTA |= 0x02;
    packet_acked = 1;
    packet_out_full = 0;
}

void ubpacket_process(void)
{
    if( packet_ack_full ){
        if( ub_sendPacket((struct ubpacket_t *)&ack) == UB_OK ){
            packet_ack_full = 0;
#ifdef UB_ENABLEMASTER
            serial_sendFrames("DAsPok");
#endif
        }else{
#ifdef UB_ENABLEMASTER
            serial_sendFrames("DAsPerror");
#endif
        }
        return;
    }
    if( packet_out_full && !packet_fired ){
        PORTA ^= (1<<5);
        if( ub_sendPacket(&outpacket) == UB_OK ){
            packet_fired = 1;
#ifdef UB_ENABLEMASTER
            serial_sendFrames("DsPok");
#endif
        }else{

        PORTA ^= (1<<7);
#ifdef UB_ENABLEMASTER
            serial_sendFrames("DsPerror");
#endif
        }
        //return;
    }

    //don't overwrite data
    if( !ubpacket_gotPacket() ){
        int8_t len = ub_getPacket(&inpacket);
        if( len > 0 )
            ubpacket_processPacket(&inpacket);
    }
}

void ubpacket_processPacket(struct ubpacket_t * in)
{
    uint8_t seq = (in->header.flags & UB_PACKET_SEQ)?1:0;
    //do we have to forward that packet to the serial interface?
    uint8_t forward = 0;
#ifdef UB_ENABLEMASTER
if( ubconfig.master ){
    if( in->header.src == UB_ADDRESS_MASTER ){
        if( ubadr_isLocal(in->header.dest) ){
            serial_sendFrames("Dbridge: local");
            //this packet was soly for us and needs no special care
            packet_incomming = 1;
            ubmaster_done();
        }else if( ubadr_isBroadcast(in->header.dest) ){
            serial_sendFrames("Dbridge: bc");
            //broadcasts also go the other interfaces
            packet_incomming = 1;
            //packets from teh master will only be received when
            //the other iterfaces are ready to accept a packet
            ub_sendPacket(in);
            ubmaster_done();
        }else if( ubadr_isLocalMulticast(in->header.dest) ){
            serial_sendFrames("Dbridge: lmc");
            packet_incomming = 1;
            ub_sendPacket(in);
            ubmaster_done();
        }else if( ubadr_isMulticast(in->header.dest) ){
            serial_sendFrames("Dbridge: mc");
            ub_sendPacket(in);
            ubmaster_done();
        }else{
            //send and wait for the ack
            serial_sendFrames("Dbridge: sc");
            memcpy(&outpacket,in,in->header.len + sizeof(in->header));
            ubpacket_send();
        }
        return;
    }
}
#endif
    if( in->header.flags & UB_PACKET_ACK ){
        //todo check also src and last dest
        if( ubadr_isLocal(in->header.dest) ||
            //if we are the master we have to check for packets to the master
            (ubconfig.master && (in->header.dest == UB_ADDRESS_MASTER)) ){
            if( seq == packet_outseq ){
#ifdef UB_ENABLEMASTER
if( ubconfig.master ){
                if( outpacket.header.src == UB_ADDRESS_MASTER ){
                    ubmaster_done();
                }
}
#endif
                packet_acked = 1;
                packet_out_full = 0;
            }
        }
        //acks contain no data
        return;
    }

    if( ubadr_isLocal(in->header.dest) ||
        (ubconfig.master && (in->header.dest == UB_ADDRESS_MASTER)) ){
#ifdef UB_ENABLEMASTER
if( ubconfig.master &&
        !(in->header.flags & UB_PACKET_NOACK)){
        //the master has to track seq for all slaves
        struct ubstat_t * flags = ubstat_getFlags(in->header.src);
        if( seq == flags->inseq ){
            //this packet was already seen
            serial_sendFrames("Ddupe");
            in->header.flags |= 0x40;
            //return;
        }else{
            flags->inseq = seq;
                       
        }
}
#endif
#ifdef UB_ENABLESLAVE
if ( ubconfig.slave &&
        !(in->header.flags & UB_PACKET_NOACK)){
        //if we still have something to send let the master retry
        if( !ubpacket_free() ){
            //the packet might have been lost
            packet_fired = 0;
            return;
        }
        //a slave only gets packets from the master
        static uint8_t inseq = 0xFF;
        if( seq == inseq ){
            in->header.flags |= 0x40;
            //return;
        }else{
            inseq = seq;
        }
}
#endif
        packet_ack(in);
        //don't forward the dupe to the app
        if( in->header.flags & 0x40 )
            return;

        //if this packet is for the master forward it to the serial line
        //else send it to the application
        if( in->header.dest == UB_ADDRESS_MASTER ){
            forward = 1;
        }else{
            packet_incomming = 1;
        }
    }else if( ubadr_isBroadcast(in->header.dest) ){
        //broadcasts go to both
        packet_incomming = 1;
        forward = 1;
    }else if(  ubadr_isLocalMulticast(in->header.dest)){
        //this multicast is for us
        packet_incomming = 1;
    }
#ifdef UB_ENABLEMASTER
if ( ubconfig.master ){
    if( forward ){
        ubmaster_forward(in);
    }
}
#endif

    if(packet_incomming){
#ifdef UB_ENABLESLAVE
if ( ubconfig.slave ){
        //don't bother the app with management packets
        if(busmgt_process(&inpacket)){
            packet_incomming = 0;
        }
}
#endif
    }
}

void ubpacket_tick(void)
{
    if(!packet_acked && packet_timeout && --packet_timeout==0){
        packet_retries++;
        if( packet_retries > UB_PACKET_RETRIES ){
            ubpacket_abort();
            return;
        }
        ubpacket_send();
        PORTA ^= 0x04;
    }
}

