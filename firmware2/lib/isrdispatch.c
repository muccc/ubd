#include <avr/interrupt.h>
#include "ubconfig.h"
#include "ubrs485master.h"
#include "ubrs485client.h"

ISR(RS485_ISR_RX, ISR_NOBLOCK)
{
#ifdef UB_MASTER
    if( ubconfig.rs485master ){
        rs485master_rx();
    }
#endif

#ifdef UB_CLIENT
    if( ubconfig.rs485client ){
        rs485client_rx();
    }
#endif
}

ISR(RS485_ISR_TX, ISR_NOBLOCK)
{
#ifdef UB_MASTER
    if( ubconfig.rs485master ){
        rs485master_tx();
    }
#endif

#ifdef UB_CLIENT
    if( ubconfig.rs485client ){
        rs485client_tx();
    }
#endif
}

ISR(RS485_ISR_TXEND, ISR_NOBLOCK)
{
#ifdef UB_MASTER
    if( ubconfig.rs485master ){
        rs485master_txend();
    }
#endif

#ifdef UB_CLIENT
    if( ubconfig.rs485client ){
        rs485client_txend();
    }
#endif
}

ISR(RS485_ISR_EDGE, ISR_NOBLOCK)
{
#ifdef UB_MASTER
    if( ubconfig.rs485master ){
        rs485master_edge();
    }
#endif

#ifdef UB_CLIENT
    if( ubconfig.rs485client ){
        rs485client_edge();
    }
#endif
}

ISR(TIMER2_COMPA_vect, ISR_NOBLOCK)
{
#ifdef UB_MASTER
    if( ubconfig.rs485master ){
        rs485master_timer();
    }
#endif

#ifdef UB_CLIENT
    if( ubconfig.rs485client ){
        rs485client_timer();
    }
#endif
   
}
