#include "ubmaster.h"
#include "ubconfig.h"
#include "settings.h"
#include "ubrs485master.h"
#include "serial_handler.h"
#include "ubaddress.h"

#include <avr/io.h>
uint16_t            ubm_ticks = 0;

void ubmaster_init(void)
{
    rs485master_init();
}

//1ms
inline void ubmaster_tick(void)
{
    ubm_ticks++;
    rs485master_tick();
}

inline void ubmaster_process(void)
{
    rs485master_process();
}

inline UBSTATUS ubmaster_sendPacket(struct ubpacket_t * packet)
{
    //TDOD: use the correct interface
    if( packet->header.dest == UB_ADDRESS_MASTER ){
        ubmaster_forward(packet);
    }

    if( ubadr_isBroadcast(packet->header.dest) ){
        rs485master_sendPacket(packet);
        if( packet->header.src != UB_ADDRESS_MASTER ){
            ubmaster_forward(packet);
        }
    }else{
        return rs485master_sendPacket(packet);
    }

    return UB_OK; 
}

inline uint8_t ubmaster_getPacket(struct ubpacket_t * packet)
{
    uint8_t len = 0;
    //are we free to send?
    if(  ubpacket_free() && (rs485master_free() == UB_OK) ){
        if( (len = serial_readline((uint8_t *)packet,
                                    sizeof(struct ubpacket_t))) ){
            //we got a packet from the host
            return len;
        }
    }
    if( (len = rs485master_getPacket(packet)) )
        return len;
    return 0;
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
