#include "ubmaster.h"
#include "ubconfig.h"
#include "settings.h"
#include "ubrs485master.h"
#include "serial_handler.h"
#include "ubaddress.h"
#include "ubrf.h"
#include "leds.h"
#include "ubstat.h"

#include <avr/io.h>
uint16_t            ubm_ticks = 0;

void ubmaster_init(void)
{
    serial_sendFrames("DInit Master");
#ifdef UB_ENABLERS485
    if( ubconfig.rs485master ){
        //serial_sendFrames("DInit RS485");
        rs485master_init();
    }
#endif
#ifdef UB_ENABLERF
    if( ubconfig.rf ){
        //serial_sendFrames("DInit RF");
        ubrf_init();
    }
#endif
}

//1ms
inline void ubmaster_tick(void)
{
    ubm_ticks++;
#ifdef UB_ENABLERS485
    if( ubconfig.rs485master )
        rs485master_tick();
#endif
#ifdef UB_ENABLERF
    if( ubconfig.rf )
        ubrf_tick();
#endif
}

inline void ubmaster_process(void)
{
#ifdef UB_ENABLERS485
    if( ubconfig.rs485master )
        rs485master_process();
#endif
#ifdef UB_ENABLERF
    if( ubconfig.rf )
        ubrf_process();
#endif
}

inline UBSTATUS ubmaster_sendPacket(struct ubpacket_t * packet)
{
    uint8_t dest = packet->header.dest;
    
    if( dest == UB_ADDRESS_MASTER ){
        ubmaster_forward(packet);
        return UB_OK;
    }

    if( ubadr_isBroadcast(dest) || ubadr_isMulticast(dest) ){
#ifdef UB_ENABLERS485
        if( ubconfig.rs485master )
            rs485master_sendPacket(packet);
#endif
#ifdef UB_ENABLERF
        if( ubconfig.rf )
            ubrf_sendPacket(packet);
#endif
        if( packet->header.src != UB_ADDRESS_MASTER ){
            ubmaster_forward(packet);
        }

    }else{
        //decide on which interface the dest is reachable
        struct ubstat_t * flags = ubstat_getFlags(dest);
#ifdef UB_ENABLERS485
        if( ubconfig.rs485master && flags->rs485 )
                return rs485master_sendPacket(packet);
#endif
#ifdef UB_ENABLERF
        if( flags->rf && ubconfig.rf )
            return ubrf_sendPacket(packet);
#endif
        serial_sendFrames("Dinterface not found");
        return UB_ERROR;
    }

    return UB_OK; 
}

inline uint8_t ubmaster_getPacket(struct ubpacket_t * packet)
{
    uint8_t len = 0;
    //leds_rx();
    //are we free to send?
    //make sure that a packet from the master can be sent on the interfaces
    if( ubpacket_free() ){
#ifdef UB_ENABLERS485
    if( !ubconfig.rs485master || rs485master_free() == UB_OK ){
#endif
#ifdef UB_ENABLERF
    if( !ubconfig.rf || ubrf_free() == UB_OK ){
#endif
        if( (len = serial_readline((uint8_t *)packet,
                                    sizeof(struct ubpacket_t))) ){
            //we got a packet from the host
            //ignore control messages for now
            if( len > 1 ){
                serial_sendFrames("Dgot serial packet");
                return UB_OK;
            }
            //FIXME: that was this for again?
            //seems like every command with len=1
            //just resets the master?
            serial_sendFrames("D1");
            ub_init(UB_MASTER, -1, -1);
        }
#ifdef UB_ENABLERF
    }
#endif
#ifdef UB_ENABLERS485
    }
#endif
    }

#ifdef UB_ENABLERS485
    if( ubconfig.rs485master && rs485master_getPacket(packet) == UB_OK ){
        ubstat_getFlags(packet->header.src)->rs485 = 1;
        ubstat_getFlags(packet->header.src)->rf = 0;
        return UB_OK;
    }
#endif
#ifdef UB_ENABLERF
    else if( ubconfig.rf && ubrf_getPacket(packet) == UB_OK){
        ubstat_getFlags(packet->header.src)->rf = 1;
        ubstat_getFlags(packet->header.src)->rs485 = 0;
        return UB_OK;
    }
#endif 
    return UB_ERROR;
}


void ubmaster_forward(struct ubpacket_t * packet)
{
    serial_putStart();
    serial_putcenc('P');
    serial_putenc((uint8_t *) packet, packet->header.len + sizeof(packet->header));
    serial_putStop();
}

void ubmaster_done(void)
{
    serial_sendFramec('S');
}

void ubmaster_abort(void)
{
    serial_sendFramec('A');
}

