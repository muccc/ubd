#include <string.h>
#include <avr/io.h>

#include "ub.h"
#include "ubpacket.h"
#include "ubmastermgt.h"
#include "ubaddress.h"

#include "settings.h"

void ubmastermgt_init(void)
{
}

uint8_t ubmastermgt_process(struct ubpacket_t * p)
{
    uint8_t * d = p->data;
    PORTB ^= (1<<PB0);
    if(!(p->header.flags & UB_PACKET_MGT))
        return 0;

    return 1;
}

void ubmastermgt_tick(void)
{
    struct ubpacket_t * p;
    static uint16_t time = 0;
    static uint16_t blubb = 0;
    if(!time--){
        time = 1000;
    }

    if(time == 0){
        p = ubpacket_getSendBuffer();
    }
}
