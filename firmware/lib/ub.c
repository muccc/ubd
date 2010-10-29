#include <avr/interrupt.h>

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
#include "ubslavemgt.h"
#include "ubmastermgt.h"
#include "serial_handler.h"

struct ub_config ubconfig;
uint8_t ub_address = 0;
uint8_t ub_slaveinterfaces = 0;
uint8_t ub_masterinterfaces = 0;

void ub_init(uint8_t ubmode, int8_t slaveinterfaces, int8_t masterinterfaces)
{
    cli();
    if( slaveinterfaces != -1 ){
        ub_slaveinterfaces = slaveinterfaces;
    }
    if( masterinterfaces != -1 ){
        ub_masterinterfaces = masterinterfaces;
    }
    ubconfig.master = 0;
    ubconfig.slave = 0;   
    ubconfig.rs485slave = 0;
    ubconfig.rs485master = 0;
    ubconfig.rf = 0;

    udebug_init();
    ubadr_init();
    random_init(ubadr_getID(),ubadr_getIDLen());
#ifdef UB_ENABLEMASTER
    if( ubmode == UB_MASTER ){
#ifdef UB_ENABLERS485
        if( ub_masterinterfaces & UB_RS485 )
            ubconfig.rs485master = 1;
#endif
#ifdef UB_ENABLERF
        if( ub_masterinterfaces & UB_RF )
            ubconfig.rf = 1;
#endif
        ubconfig.master = 1;
        //the bridge has a fixed address
        ubadr_setAddress(UB_ADDRESS_BRIDGE);
        ubstat_init();
        ubmaster_init();
        ubpacket_init();
        ubmastermgt_init();
        ubconfig.configured = 1;
        //This tells the host that we are ready
        sei();
        serial_sendFramec('B');
    }
#endif
#ifdef UB_ENABLESLAVE
    if ( ubmode == UB_SLAVE ){
#ifdef UB_ENABLERS485
        if( ub_slaveinterfaces & UB_RS485 ){
            ubconfig.rs485slave = 1;
            //sei();
            //serial_sendFrames("Dslavers485 enabled");
        }
#endif
#ifdef UB_ENABLERF
        if( ub_slaveinterfaces & UB_RF )
            ubconfig.rf = 1;
#endif
        ubconfig.slave = 1;
        //we don't know better
        ubadr_setAddress(0);
        ubslave_init();
        ubpacket_init();
        ubslavemgt_init();
        //wait until we get an address assigned to an interface
        ubconfig.configured = 0;
        sei();
    }
#endif
}

void ub_process(void)
{
#ifdef UB_ENABLEMASTER
    if( ubconfig.master ){
        ubmaster_process();
    }else{
        //check if the host wants us to be the master
        uint8_t buf[16];
        uint8_t l = serial_readline(buf, sizeof(buf));
        if( l == 1 && buf[0] == 'B'){
            ub_init(UB_MASTER,-1,-1);
            return;
        }
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
        ubmastermgt_tick();
   }
#endif
#ifdef UB_ENABLESLAVE
    if( ubconfig.slave ){
        ubslave_tick();
        ubslavemgt_tick();
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

/*uint8_t ub_mgt(struct ubpacket_t * packet)
{
#ifdef UB_ENABLEMASTER
    if( ubconfig.master ){
        return ubmastermgt_process(packet);
    }
#endif
#ifdef UB_ENABLESLAVE
    if( ubconfig.slave ){
        return ubslavemgt_process(packet);
    }
#endif
    return 0;
}*/
