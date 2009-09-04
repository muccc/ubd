#include "config.h"
#include "ubconfig.h"
#include "ubuart.h"
#include "ubstat.h"

void ub_init(uint8_t ubmode)
{
    UART_INIT( UART_BAUD_SELECT(UB_BITRATE,F_CPU));
    if( ubmode == UB_MASTER ){
        ubmaster_init();
    }else if ( ubmode == UB_CLIENT ){
        ubclient_init();
    }
    ubstat_init();
}

