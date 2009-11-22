#include <string.h>
#include <avr/io.h>

#include "ub.h"
#include "ubpacket.h"
#include "ubmastermgt.h"
#include "ubaddress.h"

#include "settings.h"
#include "ubstat.h"
#include "ubrs485master.h"

void ubmastermgt_init(void)
{
}

uint8_t ubmastermgt_process(struct ubpacket_t * p)
{
    uint8_t * d = p->data;
    struct ubstat_t * flags;
    if(!(p->header.flags & UB_PACKET_MGT))
        return 0;
    switch(d[0]){
        case 'q':
            flags = ubstat_getFlags(d[1]);
            rs485master_setQueryInterval(d[1], (d[2] << 8) + d[3]);
            flags->known = 1;
            //TODO: check if its on the rf
            flags->rs485 = 1;
        break;
    }
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
