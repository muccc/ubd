#include <stdint.h>
#include <string.h>

#include "ubslave.h"
#include "ubrs485slave.h"
#include "ubstat.h"
#include "ubaddress.h"

//uint8_t buf[30];
uint8_t ubslave_configured = 0;
uint8_t ubslave_interface = 0;

inline void ubslave_init(void)
{
   rs485slave_init();
}

inline void ubslave_process(void)
{
    rs485slave_process();
}

inline void ubslave_tick(void)
{
    static uint16_t discover = 0;
    struct ubpacket_t p;
    //send discovers until the bus is configured
    if( ubconfig.configured == 0){
        if( discover++ == 500 ){
            discover = 0;
            p.header.src = ubadr_getAddress();
            p.header.dest = UB_ADDRESS_BROADCAST;
            p.header.flags = 0;
            p.header.len = ubadr_getIDLen()+2;
            p.data[0] = 'M';
            p.data[1] = 'D';
            memcpy(p.data+2,ubadr_getID(),ubadr_getIDLen());
            ubslave_sendPacket(&p);
        }
    }else{
        //TODO: get the information about the configured interface from
        //the busmgt
        rs485slave_setConfigured(1);
        ubslave_interface = UB_RS485;

    }
    rs485slave_tick();
}

inline UBSTATUS ubslave_sendPacket(struct ubpacket_t * packet)
{
    switch( ubslave_interface ){
        case 0:
            rs485slave_sendPacket(packet);
            return UB_OK;
        break;
        case UB_RS485:
            return rs485slave_sendPacket(packet);
        break;
    }

    //no interface found
    return UB_ERROR;
}

inline uint8_t ubslave_getPacket(struct ubpacket_t * packet)
{
    uint8_t len = 0;

    switch( ubslave_interface ){
        case 0:
            if( (len = rs485slave_getPacket(packet)) )
                return len;
            return 0;
        break;
        case UB_RS485:
            return rs485slave_getPacket(packet);
        break;
    }

    //no interface found
    return UB_ERROR;
}
