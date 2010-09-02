#include <stdint.h>
#include <string.h>

#include "ubslave.h"
#include "ubrs485slave.h"
#include "ubstat.h"
#include "ubaddress.h"
#include "ubrf.h"

//uint8_t buf[30];
uint8_t ubslave_configured = 0;
uint8_t ubslave_interface = UB_NOIF;

inline void ubslave_init(void)
{
    if( ubconfig.rs485slave )
        rs485slave_init();
#ifdef UB_ENABLERF
    if( ubconfig.rf )
        ubrf_init();
#endif
}

inline void ubslave_process(void)
{
    if( ubconfig.rs485slave )
        rs485slave_process();
#ifdef UB_ENABLERF
    if( ubconfig.rf )
        ubrf_process();
#endif
}

inline void ubslave_tick(void)
{
    //send discovers until the bus is configured
    if( ubconfig.configured == 0){
        //moved to busmgt
    }else{
        //TODO: get the information about the configured interface from
        //the busmgt
        //rs485slave_setConfigured(1);
        //ubslave_interface = UB_RS485;
        ubslave_interface = UB_RF;

    }
    //TODO: disable unused interfaces after configuring one
    if( ubconfig.rs485slave )
        rs485slave_tick();
#ifdef UB_ENABLERF
    if( ubconfig.rf )
        ubrf_tick();
#endif
}

inline UBSTATUS ubslave_sendPacket(struct ubpacket_t * packet)
{
    switch( ubslave_interface ){
        case UB_NOIF:
            //we have no interface selected yet
            if( ubconfig.rs485slave )
                rs485slave_sendPacket(packet);
#ifdef UB_ENABLERF
            if( ubconfig.rf )
                ubrf_sendPacket(packet);
#endif
            return UB_OK;
        break;
        case UB_RS485:
            return rs485slave_sendPacket(packet);
        break;
#ifdef UB_ENABLERF
        case UB_RF:
            return ubrf_sendPacket(packet);
        break;
#endif
    }

    //no interface found
    return UB_ERROR;
}

inline uint8_t ubslave_getPacket(struct ubpacket_t * packet)
{
    uint8_t ret = 0;

    switch( ubslave_interface ){
        case UB_NOIF:
            if( ubconfig.rs485slave )
                ret = rs485slave_getPacket(packet);
#ifdef UB_ENABLERF
            if( ret == UB_ERROR && ubconfig.rf )
                ret = ubrf_getPacket(packet);
#endif
            return ret;
        break;
        case UB_RS485:
            return rs485slave_getPacket(packet);
        break;
#ifdef UB_ENABLERF
        case UB_RF:
            return ubrf_getPacket(packet);
#endif
        break;
    }

    //no interface found
    //TODO: how is this return value defined? 
    return UB_ERROR;
}

