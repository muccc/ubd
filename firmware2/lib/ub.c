#include "ubconfig.h"
#include "ubrs485uart.h"
#include "ubstat.h"
#include "ubmaster.h"
#include "ubslave.h"
#include "ub.h"
#include "udebug.h"
#include "random.h"
#include "ubaddress.h"
#include "ubpacket.h"

struct ub_config ubconfig;
uint8_t ub_address = 0;

void ub_init(uint8_t ubmode)
{
    udebug_init();
    ubadr_init();
    random_init(ubadr_getID(),ubadr_getIDLen());
#ifdef UB_ENABLEMASTER
    if( ubmode == UB_MASTER ){
        ubconfig.rs485master = 1;
        ubconfig.master = 1;
        //the bridge has a fixed address
        ubadr_setAddress(UB_ADDRESS_BRIDGE);
        ubstat_init();
        ubmaster_init();
        ubconfig.configured = 1;
    }
#endif
#ifdef UB_ENABLESLAVE
    if ( ubmode == UB_SLAVE ){
        ubconfig.rs485slave = 1;
        ubconfig.slave = 1;
        //we don't know better
        ubadr_setAddress(0);
//        ubstat_init();
        ubslave_init();
        ubpacket_init();
        ubconfig.configured = 0;
    }
#endif
}

void ub_process(void)
{
#ifdef UB_ENABLEMASTER
    if( ubconfig.master ){
        ubmaster_process();
    }
#endif
#ifdef UB_ENABLESLAVE
    if( ubconfig.slave ){
        ubslave_process();
    }
#endif
    ubpacket_process();
}

void ub_tick(void)
{
#ifdef UB_ENABLEMASTER
    if( ubconfig.master ){
        ubmaster_tick();
   }
#endif
#ifdef UB_ENABLESLAVE
    if( ubconfig.slave ){
        ubslave_tick();

        busmgt_tick();
    }   
#endif
    ubpacket_tick();
}

UBSTATUS ub_sendPacket(struct ubpacket_t * packet)
{
#ifdef UB_ENABLEMASTER
    if( ubconfig.master ){
        return ubmaster_sendPacket(packet);
    }
#endif
#ifdef UB_ENABLESLAVE
    if( ubconfig.slave ){
        return ubslave_sendPacket(packet);
    }   
#endif
    return UB_ERROR;
}


uint8_t ub_getPacket(struct ubpacket_t * packet)
{
#ifdef UB_ENABLEMASTER
    if( ubconfig.master ){
        return ubmaster_getPacket(packet);
    }
#endif
#ifdef UB_ENABLESLAVE
    if( ubconfig.slave ){
        return ubslave_getPacket(packet);
    }   
#endif
    return 0;
}
