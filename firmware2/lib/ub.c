//#include "config.h"
#include "ubconfig.h"
#include "ubrs485uart.h"
#include "ubstat.h"
#include "ubmaster.h"
#include "ubslave.h"
#include "ub.h"
#include "udebug.h"
#include "random.h"

struct ub_config ubconfig;
uint8_t ub_address = 0;

void ub_init(uint8_t ubmode)
{
    udebug_init();
    random_init((uint8_t*)"11",2);
#ifdef UB_ENABLEMASTER
    if( ubmode == UB_MASTER ){
        ubconfig.rs485master = 1;
        ubconfig.master = 1;
        ubstat_init();
        ubmaster_init();
    }
#endif
#ifdef UB_ENABLESLAVE
    if ( ubmode == UB_SLAVE ){
        ubconfig.rs485slave = 1;
        ubconfig.slave = 1;
        ubstat_init();
        ubslave_init();
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
    }   
#endif
}

inline uint8_t ub_getAddress(void)
{
    return ub_address;
}

void ub_setAddress(uint8_t address)
{
    ub_address = address;
}
