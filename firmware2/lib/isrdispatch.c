#include <avr/interrupt.h>
#include "ubconfig.h"
#include "ubrs485master.h"
#include "ububclient.h"

ISR(UB_RX, ISR_NOBLOCK)
{
#ifdef UB_MASTER
    if( ubconfig.rs485master ){
        rs485master_rx();
    }
#endif

#ifdef UB_CLIENT
    if( ubconfig.ubclient ){
        ubclient_rx();
    }
#endif
}

ISR(UB_TX, ISR_NOBLOCK)
{
#ifdef UB_MASTER
    if( ubconfig.rs485master ){
        rs485master_tx();
    }
#endif

#ifdef UB_CLIENT
    if( ubconfig.ubclient ){
        ubclient_tx();
    }
#endif
}

ISR(UB_EDGE, ISR_NOBLOCK)
{
#ifdef UB_MASTER
    if( ubconfig.rs485master ){
        rs485master_edge();
    }
#endif

#ifdef UB_CLIENT
    if( ubconfig.ubclient ){
        ubclient_edge();
    }
#endif
}

ISR(TIMER2_COMPA_vect, ISR_NOBLOCK)
{
#ifdef UB_MASTER
    if( ubconfig.rs485master ){
        ubmaster_timer();
    }
#endif

#ifdef UB_CLIENT
    if( ubconfig.ubclient ){
        ubclient_timer();
    }
#endif
   
}
