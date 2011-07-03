
#include <avr/io.h>
#include <stdint.h>
#include <stdlib.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdio.h>
#include <string.h>
#include <avr/wdt.h>

#include "ub.h"
#include "ubaddress.h"
#include "ubpacket.h"

/** main function
 */
int main(void) {
    ub_init(UB_SLAVE, UB_RF, UB_RF|UB_RS485);
    /* enable interrupts globally */
    sei();

    while (1) {
        wdt_reset();
        ub_process();
        if( ubpacket_gotPacket() ){
            struct ubpacket_t * out = ubpacket_getSendBuffer();
            out->header.class = 23;
            if( ubpacket_getIncomming()->header.flags & UB_PACKET_NOACK ){
                ubpacket_processed();   //has to be after the if
            }else{
                ubpacket_processed();
                ubpacket_send();
            }
        }
        ub_tick();
    }
}
