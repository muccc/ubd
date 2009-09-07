//#include "config.h"
#include "ubconfig.h"
#include "ubrs485uart.h"
#include "ubstat.h"
#include "ubmaster.h"
#include "ub.h"

struct ub_config ubconfig;

void ub_init(uint8_t ubmode)
{
    if( ubmode == UB_MASTER ){
        ubconfig.rs485master = 1;
        ubconfig.master = 1;
        ubstat_init();
        ubmaster_init();
    }else if ( ubmode == UB_CLIENT ){
//        ubclient_init();
    }
}

void ub_process(void)
{
    if( ubconfig.master ){
        ubmaster_process();
    }else if( ubconfig.client ){
        //ubclient_tick();
    }
}

void ub_tick(void)
{
    if( ubconfig.master ){
        ubmaster_tick();
    }else if( ubconfig.client ){
        //ubclient_process);
    }
   
}
