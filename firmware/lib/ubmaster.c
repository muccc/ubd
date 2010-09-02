#include "ubmaster.h"
#include "ubconfig.h"
#include "settings.h"
#include "ubrs485master.h"
#include "serial_handler.h"
#include "ubaddress.h"
#include "ubrf.h"
#include "leds.h"

#include <avr/io.h>
uint16_t            ubm_ticks = 0;

void ubmaster_init(void)
{
    rs485master_init();

    if( ubconfig.rf )
        ubrf_init();
    serial_sendFrames("DInit Master");
}

//1ms
inline void ubmaster_tick(void)
{
    ubm_ticks++;
    rs485master_tick();
#ifdef UB_ENABLERF
    if( ubconfig.rf )
    ubrf_tick();
#endif
}

inline void ubmaster_process(void)
{
    rs485master_process();
#ifdef UB_ENABLERF
    if( ubconfig.rf )
    ubrf_process();
#endif
}

inline UBSTATUS ubmaster_sendPacket(struct ubpacket_t * packet)
{
    //TDOD: use the correct interface
    if( packet->header.dest == UB_ADDRESS_MASTER ){
        ubmaster_forward(packet);
        return UB_OK;
    }

    if( ubadr_isBroadcast(packet->header.dest) ){
        rs485master_sendPacket(packet);
        if( packet->header.src != UB_ADDRESS_MASTER ){
            ubmaster_forward(packet);
        }
#ifdef UB_ENABLERF
        ubrf_sendPacket(packet);
#endif
    }else{
        //TODO: decide on which interface the dest is reachable
        return rs485master_sendPacket(packet);
    }

    return UB_OK; 
}

inline uint8_t ubmaster_getPacket(struct ubpacket_t * packet)
{
    uint8_t len = 0;
    //leds_rx();
    //are we free to send?
    if(  ubpacket_free() && (rs485master_free() == UB_OK) ){
#ifdef UB_ENABLERF
    if( ubrf_free() == UB_OK ){
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
            serial_sendFrames("D1");
            ub_init(UB_MASTER, -1);
        }
#ifdef UB_ENABLERF
    }
#endif
    }
    PORTC ^= (1<<PC0);
    //if( (len = rs485master_getPacket(packet)) )
    //    return len;
    //return 0;
    uint8_t ret;
    ret = rs485master_getPacket(packet);
#ifdef UB_ENABLERF
    //if( ret == UB_ERROR )
    if( ubconfig.rf ){
        ret = ubrf_getPacket(packet);
        PORTC ^= (1<<PC1);
    }
#endif
    return ret;
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
