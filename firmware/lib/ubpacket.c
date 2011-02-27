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

#include "udebug.h"

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
uint8_t packet_unsolicitedsent = 0;
uint8_t packet_unsolicitedlock = 255;


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
        UDEBUG("Dpacketincomming");
    return packet_incomming;
}

inline void ubpacket_processed(void)
{
    UDEBUG("Dprocessed");
    uint8_t noack = ubpacket_getIncomming()->header.flags & UB_PACKET_NOACK;
    packet_incomming = 0;
    if( !noack ){
        //we have to send a reply for this packet
        ubpacket_send();
    }
}

inline struct ubpacket_t * ubpacket_getSendBuffer(void)
{
    return &outpacket;
}


uint8_t ubpacket_acquireUnsolicited(uint8_t class)
{
    if( ubpacket_free() ){
        if( packet_unsolicitedlock == class ){
            return 1;
        }else{
            if( packet_unsolicitedlock == 255 ){
                packet_unsolicitedlock = class;
                packet_unsolicitedsent = 0;
                return 1;
            }else{
                return 0;
            }
        }
    }else{
        return 0;
    }
}

uint8_t ubpacket_isUnsolicitedDone(void)
{
    return packet_unsolicitedsent;
}

void ubpacket_releaseUnsolicited(uint8_t class)
{
    if( class == packet_unsolicitedlock ){
        packet_unsolicitedlock = 255;
    }
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
    if( outpacket.header.len == 0 &&
        outpacket.header.dest == UB_ADDRESS_MASTER &&
        outpacket.header.src == UB_ADDRESS_BRIDGE ){
        return;
    }

    UDEBUG("Dresettimeout");
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
        UDEBUG("Dresetretries");
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
        UDEBUG("Dsetseq");
        outpacket.header.flags |= UB_PACKET_SEQ;
    }else{
        UDEBUG("Dunsetseq");
    }

    //outpacket.header.src = ubadr_getAddress();

    if(ubadr_isUnicast(outpacket.header.dest) &&
        !(outpacket.header.flags & UB_PACKET_NOACK) &&
        !(outpacket.header.len == 0) ){
        UDEBUG("Dresetpacketacked");
        packet_acked = 0;
    }else{
        //don't wait for an ack
        packet_acked = 1;
        UDEBUG("Dsetpacketacked");
    }

    packet_out_full = 1;

    //PORTA ^= (1<<4);
    if( ub_sendPacket(&outpacket) == UB_OK ){
        packet_fired = 1;
        if( outpacket.header.flags & UB_PACKET_NOACK ||
            outpacket.header.len == 0 ){
            packet_out_full = 0;
            if( outpacket.header.flags & UB_PACKET_UNSOLICITED &&
                    !(outpacket.header.flags & UB_PACKET_MGT) ){
                packet_unsolicitedsent = 1;
            }
        }
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
        UDEBUG("DsPok");
}
#endif
    }else{
        //PORTA ^= (1<<4);
        packet_fired = 0;
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
        UDEBUG("DsPerror");
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
    UDEBUG("Dabort");
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
    if( outpacket.header.src == UB_ADDRESS_MASTER ){
        ubbridge_abort();
    }
}
#endif
    //PORTA |= 0x02;
    UDEBUG("Dsetpacketacked");
    packet_acked = 1;
    packet_out_full = 0;
}

static void ubpacket_sendack(void)
{        
    if( ub_sendPacket((struct ubpacket_t *)&ack)
                                    == UB_OK ){
        packet_ack_full = 0;
        UDEBUG("DAsPok");
    }else{
        UDEBUG("DAsPerror");
    }
}

void ubpacket_process(void)
{
    if( packet_ack_full ){
        ubpacket_sendack();
        return;
    }
    if( packet_out_full && !packet_fired ){
        //PORTA ^= (1<<5);
        if( ub_sendPacket(&outpacket) == UB_OK ){
            packet_fired = 1;
            if( outpacket.header.flags & UB_PACKET_NOACK )
                packet_out_full = 0;
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
            UDEBUG("DsPok");
}
#endif
        }else{
            //PORTA ^= (1<<7);
#ifdef UB_ENABLEBRIDGE
if( ubconfig.bridge ){
            UDEBUG("DsPerror");
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
    UDEBUG("Dbridge: processing");
if( ubconfig.bridge ){
    if( in->header.src == UB_ADDRESS_MASTER ){
        UDEBUG("Dbridge: src=master");
        if( ubadr_isLocal(in->header.dest) ){
            UDEBUG("Dbridge: local");
            //this packet was only for us and needs no special care
            UDEBUG("Dincomming");
            packet_incomming = 1;
            if( !(in->header.flags & UB_PACKET_NOACK) ){
                UDEBUG("Dacking");
                packet_prepareack(in);
                outpacket.header = ack;
            }
            //in->header.flags |= UB_PACKET_NOACK;
            ubbridge_done();
        }else if( ubadr_isBroadcast(in->header.dest) ){
            UDEBUG("Dbridge: bc");
            //broadcasts also go the other interfaces
            UDEBUG("Dincomming");
            packet_incomming = 1;
            //packets from the master will only be received when
            //the other interfaces are ready to accept a packet
            ub_sendPacket(in);
            ubbridge_done();
        }else if( ubadr_isLocalMulticast(in->header.dest) ){
            UDEBUG("Dbridge: lmc");
            UDEBUG("Dincomming");
            packet_incomming = 1;
            ub_sendPacket(in);
            ubbridge_done();
        }else if( ubadr_isMulticast(in->header.dest) ){
            UDEBUG("Dbridge: mc");
            ub_sendPacket(in);
            ubbridge_done();
        }else{
            //send and wait for the ack
            UDEBUG("Dbridge: sc");
            memcpy(&outpacket,in,in->header.len + sizeof(in->header));
            ubpacket_send();
            if( in->header.flags & UB_PACKET_NOACK){
                ubbridge_done();
            }
        }
        if( packet_incomming && in->header.flags & UB_PACKET_MGT ){
            //this is a management packet
            UDEBUG("Disformgt");
            packet_incomming = 0;
            outpacket.header.flags = 0;
            if( ubbridgemgt_process(&inpacket) ){
                UDEBUG("Dmgthasanswer");
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

                UDEBUG("Dcheckpacket");
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
                UDEBUG("Dackseqok");
                UDEBUG("Dsetpacketacked");
                packet_acked = 1;
                packet_out_full = 0;
                if( outpacket.header.flags & UB_PACKET_UNSOLICITED &&
                    !(outpacket.header.flags & UB_PACKET_MGT) ){
                    packet_unsolicitedsent = 1;
                }
            }else{
                UDEBUG("DackseqNok");
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
            UDEBUG("Dacking");
            packet_prepareack(in);
        }
        //don't forward a dupe to the app
        if( dupe ){
            UDEBUG("DseqNok");
            UDEBUG("Ddupe");
            //but still send the ack
            if( !(in->header.flags & UB_PACKET_NOACK) )
                ubpacket_sendack();
            return;
        }else{
            UDEBUG("Dseqok");
        }
        //if this packet is for the master forward it to the serial line
        //else send it to the application
        if( in->header.dest == UB_ADDRESS_MASTER ){
            forward = 1;
            //we still have to send the ack
            if( !(in->header.flags & UB_PACKET_NOACK) )
                ubpacket_sendack();
        }else{
            UDEBUG("Dincomming");
            packet_incomming = 1;
            outpacket.header = ack;
        }
    }else if( ubadr_isBroadcast(in->header.dest) ){
        //broadcasts go to both
        UDEBUG("Dincomming");
        packet_incomming = 1;
        forward = 1;
    }else if(  ubadr_isLocalMulticast(in->header.dest) ){
        //this multicast is for us
        UDEBUG("Dincomming");
        packet_incomming = 1;
    }
#ifdef UB_ENABLEBRIDGE
if ( ubconfig.bridge ){
    if( forward ){
        ubbridge_forward(in);
    }
    //management packets should have already been processed
    if( packet_incomming && in->header.flags & UB_PACKET_MGT ){
    UDEBUG("Dprocessed");
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
            UDEBUG("Dwas for mgt");
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
        UDEBUG("Drt");
        packet_retries++;
        if( packet_retries > UB_PACKET_RETRIES ){
            ubpacket_abort();
            return;
        }
        ubpacket_send();
        //PORTA ^= 0x04;
    }
}

