#include <avr/io.h>

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#include "ub.h"
#include "ubpacket.h"
#include "ubstat.h"
#include "ubaddress.h"
#include "ubbridge.h"

#include "ubslavemgt.h"
#include "ubbridgemgt.h"

#include "serial_handler.h"

typedef uint16_t    time_t;

struct ubpacket_t outpacket;
uint8_t packet_out_full = 0;
uint8_t packet_fired = 0;
uint8_t packet_acked = 1;
uint8_t packet_retries;
time_t  packet_timeout = 0;

struct ubheader_t ack;
uint8_t packet_ack_full = 0;

struct ubpacket_t inpacket;
uint8_t packet_incomming;

uint8_t packet_sniff = 0;

uint8_t packet_outseq = 0;

void ubpacket_init(void)
{
    packet_out_full = 0;
    packet_fired = 0;
    packet_acked = 1;
    packet_ack_full = 0;
    packet_sniff = 0;
    packet_outseq = 0;
}

inline struct ubpacket_t * ubpacket_getIncomming(void)
{
    return &inpacket;
}

inline uint8_t ubpacket_gotPacket(void)
{
    if( packet_incomming )
        serial_sendFrames("Dpacketincomming");
    return packet_incomming;
}

inline void ubpacket_processed(void)
{
    serial_sendFrames("Dprocessed");
    packet_incomming = 0;
}

inline struct ubpacket_t * ubpacket_getSendBuffer(void)
{
    return &outpacket;
}

void ubpacket_send(void)
{
    //don't do anything until we are configured
    //the busmgt is allowed to send
    if( !(outpacket.header.flags & UB_PACKET_MGT) &&
                        ubconfig.configured == 0){
        return;
    }
    //if( outpacket.header.len == 0 &&
    //    !(outpacket.header.flags & UB_PACKET_ACK) ){
    //    return;
    //}
    serial_sendFrames("Dresettimeout");
    packet_timeout = UB_PACKET_TIMEOUT;

    //Don't request an ack from the host computer
    if( outpacket.header.dest == UB_ADDRESS_MASTER && ubconfig.bridge )
        outpacket.header.flags |= UB_PACKET_NOACK;
    
    //Dont't request acks on multicast and braodcast packets
    if( !ubadr_isUnicast(outpacket.header.dest) )
        outpacket.header.flags |= UB_PACKET_NOACK;

    //packet_acked won't be set if this is a retransmit
    if(ubadr_isUnicast(outpacket.header.dest) &&
            packet_acked &&
            !(outpacket.header.flags & UB_PACKET_NOACK)){
        serial_sendFrames("Dresetretries");
        packet_retries = 0;
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
        struct ubstat_t *flags=ubstat_getFlags(outpacket.header.dest);
        packet_outseq = flags->outseq;
}
#endif
    }
    //the application can set some own flags
    outpacket.header.flags &= (UB_PACKET_ACK | UB_PACKET_ACKSEQ |
                                UB_PACKET_NOACK | UB_PACKET_MGT | 
                                UB_PACKET_UNSOLICITED );

    //mark this packet as a retransmit
    //this is only used to debug the system.
    if( !packet_acked && !(outpacket.header.flags & UB_PACKET_ACK) )
        outpacket.header.flags |= 0x80;

    //set the sequence number
    if( packet_outseq ){
        serial_sendFrames("Dsetseq");
        outpacket.header.flags |= UB_PACKET_SEQ;
    }else{
        serial_sendFrames("Dunsetseq");
    }

    //outpacket.header.src = ubadr_getAddress();

    if(ubadr_isUnicast(outpacket.header.dest) &&
        !(outpacket.header.flags & UB_PACKET_NOACK) &&
        !(outpacket.header.len == 0) ){
        serial_sendFrames("Dresetpacketacked");
        packet_acked = 0;
    }else{
        //don't wait for an ack
        packet_acked = 1;
        serial_sendFrames("Dsetpacketacked");
    }

    packet_out_full = 1;

    //PORTA ^= (1<<4);
    if( ub_sendPacket(&outpacket) == UB_OK ){
        packet_fired = 1;
        if( outpacket.header.flags & UB_PACKET_NOACK ||
            outpacket.header.len == 0 )
            packet_out_full = 0;
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
        serial_sendFrames("DsPok");
}
#endif
    }else{
        //PORTA ^= (1<<4);
        packet_fired = 0;
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
        serial_sendFrames("DsPerror");
}
#endif
    }
}

uint8_t ubpacket_free(void)
{
    return !packet_out_full;
}

static void packet_prepareack(struct ubpacket_t * p)
{
    ack.dest = p->header.src;
    ack.src = p->header.dest;
    ack.flags = UB_PACKET_ACK;

    //seq+1
    if( !(p->header.flags & UB_PACKET_SEQ) ){
        ack.flags |= UB_PACKET_ACKSEQ;
    }
    ack.len = 0;
    //packet_ack_full = 1;
}

static void ubpacket_abort(void)
{
    serial_sendFrames("Dabort");
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
    if( outpacket.header.src == UB_ADDRESS_MASTER ){
        ubbridge_abort();
    }
}
#endif
    PORTA |= 0x02;
    serial_sendFrames("Dsetpacketacked");
    packet_acked = 1;
    packet_out_full = 0;
}

static void ubpacket_sendack(void)
{        
    if( ub_sendPacket((struct ubpacket_t *)&ack)
                                    == UB_OK ){
        packet_ack_full = 0;
        serial_sendFrames("DAsPok");
    }else{
        serial_sendFrames("DAsPerror");
    }
}

void ubpacket_process(void)
{
    if( packet_ack_full ){
        ubpacket_sendack();
        return;
    }
    if( packet_out_full && !packet_fired ){
        PORTA ^= (1<<5);
        if( ub_sendPacket(&outpacket) == UB_OK ){
            packet_fired = 1;
            if( outpacket.header.flags & UB_PACKET_NOACK )
                packet_out_full = 0;
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
            serial_sendFrames("DsPok");
}
#endif
        }else{
            //PORTA ^= (1<<7);
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
            serial_sendFrames("DsPerror");
}
#endif
        }
        //return;
    }

    //don't overwrite data
    if( !ubpacket_gotPacket() ){
        int8_t ret = ub_getPacket(&inpacket);
        if( ret == UB_OK )
            ubpacket_processPacket(&inpacket);
    }
}

void ubpacket_processPacket(struct ubpacket_t * in)
{
    uint8_t seq = (in->header.flags & UB_PACKET_SEQ)?1:0;
    uint8_t ackseq = (in->header.flags & UB_PACKET_ACKSEQ)?1:0;
    //do we have to forward that packet to the serial interface?
    uint8_t forward = 0;
    //did we see this packet bevore? gets reset when the se is correct
    //or the noack flag is set
    uint8_t dupe    = 1;
#ifdef UB_ENABLEBRIDGE
    serial_sendFrames("Dbridge: processing");
if( ubconfig.bridge ){
    if( in->header.src == UB_ADDRESS_MASTER ){
        serial_sendFrames("Dbridge: src=bridge");
        if( ubadr_isLocal(in->header.dest) ){
            serial_sendFrames("Dbridge: local");
            //this packet was only for us and needs no special care
            serial_sendFrames("Dincomming");
            packet_incomming = 1;
            in->header.flags |= UB_PACKET_NOACK;
            ubbridge_done();
        }else if( ubadr_isBroadcast(in->header.dest) ){
            serial_sendFrames("Dbridge: bc");
            //broadcasts also go the other interfaces
            serial_sendFrames("Dincomming");
            packet_incomming = 1;
            //packets from the master will only be received when
            //the other interfaces are ready to accept a packet
            ub_sendPacket(in);
            ubbridge_done();
        }else if( ubadr_isLocalMulticast(in->header.dest) ){
            serial_sendFrames("Dbridge: lmc");
            serial_sendFrames("Dincomming");
            packet_incomming = 1;
            ub_sendPacket(in);
            ubbridge_done();
        }else if( ubadr_isMulticast(in->header.dest) ){
            serial_sendFrames("Dbridge: mc");
            ub_sendPacket(in);
            ubbridge_done();
        }else{
            //send and wait for the ack
            serial_sendFrames("Dbridge: sc");
            memcpy(&outpacket,in,in->header.len + sizeof(in->header));
            ubpacket_send();
            if( in->header.flags & UB_PACKET_NOACK){
                ubbridge_done();
            }
        }
        if( packet_incomming && in->header.flags & UB_PACKET_MGT ){
            //this is a management packet
            serial_sendFrames("Disformgt");
            packet_incomming = 0;
            outpacket.header.flags = 0;
            if( ubbridgemgt_process(&inpacket) ){
                serial_sendFrames("Dmgthasanswer");
                //Well this should always be true.
                //But the master could send a multicast
                //management packet and we might have our
                //buffer already full. This comes with
                //little memory on the chip and
                //no packet queues...
                if( ubpacket_free() )
                    ubpacket_send();
            }
        }
        return;
    }
}
#endif

                serial_sendFrames("Dcheckpacket");
    //ignore empty packets
    if( in->header.len == 0 && !(in->header.flags & UB_PACKET_ACK) )
        return;
    //is this packet addressed to us?
    if( ubadr_isLocal(in->header.dest) ||
        (ubconfig.bridge && (in->header.dest == UB_ADDRESS_MASTER)) ){

        if( in->header.flags & UB_PACKET_ACK ){
            uint8_t ackok = 0;
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
            //the bridge has to track seq for all slaves
            struct ubstat_t * flags = ubstat_getFlags(in->header.src);
            if( ackseq != flags->outseq ){
                flags->outseq = ackseq;
                ackok = 1;
            }

            if( ackok && packet_out_full && 
                    outpacket.header.src == UB_ADDRESS_MASTER ){
                ubbridge_done();
            }
}
#endif
#ifdef UB_ENABLESLAVE
if( ubconfig.slave ){
            //a slave only gets packets from the bridge
            if( ackseq != packet_outseq ){
                packet_outseq = ackseq;
                ackok = 1;
            }
}
#endif
            if( ackok ){
                serial_sendFrames("Dackseqok");
                serial_sendFrames("Dsetpacketacked");
                packet_acked = 1;
                packet_out_full = 0;
            }else{
                serial_sendFrames("DackseqNok");
            }

            //Ignore empty packets
            if( in->header.len == 0 )
                return;
            //acks can contain data
            in->header.flags ^= UB_PACKET_ACK;
        }

#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
        if( !(in->header.flags & UB_PACKET_NOACK) ){
            //the bridge has to track seq for all slaves
            struct ubstat_t * flags = ubstat_getFlags(in->header.src);
            if( seq == flags->inseq ){
                flags->inseq = seq?0:1;
                dupe = 0;
            }
        }else{
            dupe = 0;
        }
}
#endif
#ifdef UB_ENABLESLAVE
if( ubconfig.slave ){
        if( !(in->header.flags & UB_PACKET_NOACK) ){
            //if we still have something to send let the bridge retry
            if( !ubpacket_free() ){
                //the packet might have been lost
                //force a retransmit
                packet_fired = 0;
                //the bridge will retry
                return;
            }
            //a slave only gets packets from the bridge
            static uint8_t inseq = 0;
            if( seq == inseq ){
                inseq = seq?0:1;
                dupe = 0;
            }
        }else{
            dupe = 0;
        }
}
#endif
        if( !(in->header.flags & UB_PACKET_NOACK) ){
            serial_sendFrames("Dacking");
            packet_prepareack(in);
        }
        //don't forward a dupe to the app
        if( dupe ){
            serial_sendFrames("DseqNok");
            serial_sendFrames("Ddupe");
            //but still send the ack
            if( !(in->header.flags & UB_PACKET_NOACK) )
                ubpacket_sendack();
            return;
        }else{
            serial_sendFrames("Dseqok");
        }
        //if this packet is for the master forward it to the serial line
        //else send it to the application
        if( in->header.dest == UB_ADDRESS_MASTER ){
            forward = 1;
            //we still have to send the ack
            if( !(in->header.flags & UB_PACKET_NOACK) )
                ubpacket_sendack();
        }else{
            serial_sendFrames("Dincomming");
            packet_incomming = 1;
            outpacket.header = ack;
        }
    }else if( ubadr_isBroadcast(in->header.dest) ){
        //broadcasts go to both
        serial_sendFrames("Dincomming");
        packet_incomming = 1;
        forward = 1;
    }else if(  ubadr_isLocalMulticast(in->header.dest) ){
        //this multicast is for us
        serial_sendFrames("Dincomming");
        packet_incomming = 1;
    }
#ifdef UB_ENABLEBRIDGE
if ( ubconfig.bridge ){
    if( forward ){
        ubbridge_forward(in);
    }
    //management packets should have already been processed
    if( packet_incomming && in->header.flags & UB_PACKET_MGT ){
    serial_sendFrames("Dprocessed");
        packet_incomming = 0;
    }
}
#endif
#ifdef UB_ENABLESLAVE
if ( ubconfig.slave ){
    if( packet_incomming ){
        uint8_t rc = ubslavemgt_process(&inpacket);
        if( rc ){
            //this was a management packet
            serial_sendFrames("Dwas for mgt");
            packet_incomming = 0;
            if( rc == 2 ){
                //packet was valid
                if( ubadr_isLocal(in->header.dest) )
                    ubpacket_send();
            }
        }
    }
}
#endif
}

void ubpacket_tick(void)
{
    if(!packet_acked && packet_timeout && --packet_timeout==0){
        serial_sendFrames("Drt");
        packet_retries++;
        if( packet_retries > UB_PACKET_RETRIES ){
            ubpacket_abort();
            return;
        }
        ubpacket_send();
        //PORTA ^= 0x04;
    }
}

