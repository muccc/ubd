//#include "config.h"
#include "ubconfig.h"
#include "ubrs485uart.h"
#include "ubstat.h"
#include "ubmaster.h"

struct ub_config ubconfig;

void ub_init(uint8_t ubmode)
{
    if( ubmode == UB_MASTER ){
        ubmaster_init();
    }else if ( ubmode == UB_CLIENT ){
//        ubclient_init();
    }
    ubstat_init();
}

