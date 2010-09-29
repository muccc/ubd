#include <stdint.h>
#include <string.h>

#include "ubslave.h"
#include "ubrs485slave.h"
#include "ubstat.h"
#include "ubaddress.h"
#include "ubrf.h"

//uint8_t buf[30];
//uint8_t ubslave_configured = 0;
uint8_t ubslave_interface = UB_NOIF;

inline void ubslave_init(void)
{
    //XXX: Give the rf priority
    if( ubconfig.rf ){
        ubconfig.rs485slave = 0;
    }

#ifdef UB_ENABLERS485
    if( ubconfig.rs485slave ){
        rs485slave_init();
        ubslave_interface = UB_RS485;
    }else{
        //Stop timers and interrupts
        rs485slave_stop();
    }
#endif
#ifdef UB_ENABLERF
    if( ubconfig.rf ){
        ubrf_init();
        ubslave_interface = UB_RF;
    }
#endif
}

inline void ubslave_process(void)
{
#ifdef UB_ENABLERS485
    if( ubconfig.rs485slave )
        rs485slave_process();
#endif
#ifdef UB_ENABLERF
    if( ubconfig.rf )
        ubrf_process();
#endif
}

inline void ubslave_tick(void)
{
    //send discovers until the bus is configured
    /*
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
    */
#ifdef UB_ENABLERS485
    if( ubconfig.rs485slave )
        rs485slave_tick();
#endif
#ifdef UB_ENABLERF
    if( ubconfig.rf )
        ubrf_tick();
#endif
}

inline UBSTATUS ubslave_sendPacket(struct ubpacket_t * packet)
{
    switch( ubslave_interface ){
/*        case UB_NOIF:
            //we have no interface selected yet
#ifdef UB_ENABLERS485
            if( ubconfig.rs485slave )
                rs485slave_sendPacket(packet);
#endif
#ifdef UB_ENABLERF
            if( ubconfig.rf )
                ubrf_sendPacket(packet);
#endif
            return UB_OK;
        break;*/
#ifdef UB_ENABLERS485
        case UB_RS485:
            return rs485slave_sendPacket(packet);
        break;
#endif
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
    //uint8_t ret = 0;

    switch( ubslave_interface ){
/*        case UB_NOIF:
#ifdef UB_ENABLERS485
            if( ubconfig.rs485slave )
                ret = rs485slave_getPacket(packet);
#endif
#ifdef UB_ENABLERF
            if( ret == UB_ERROR && ubconfig.rf )
                ret = ubrf_getPacket(packet);
#endif
            return ret;
        break;*/
#ifdef UB_ENABLERS485
        case UB_RS485:
            return rs485slave_getPacket(packet);
        break;
#endif
#ifdef UB_ENABLERF
        case UB_RF:
            return ubrf_getPacket(packet);
        break;
#endif
    }

    //no interface found
    //TODO: how is this return value defined? 
    return UB_ERROR;
}

