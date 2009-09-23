#include "ubconfig.h"
#include "ubstat.h"

uint8_t ubstat[UB_NODEMAX];
uint8_t ubstate_id[] = "newslave";
uint8_t ubstate_idlen = sizeof(ubstate_id);

void ubstat_init(void)
{
    uint8_t i;
    for(i=0; i<UB_NODEMAX; i++){
        ubstat[i] = 0;              //reset all nodes
    }
}

inline uint8_t * ubstat_getID(void)
{
    return ubstate_id;
}

inline uint8_t ubstat_getIDLen(void)
{
    return ubstate_idlen;
}

inline uint8_t ubstat_getFlags(uint8_t adr)
{
    return ubstat[adr];
}

void ubstat_addNode(uint8_t adr, uint8_t flags)
{
    ubstat[adr] = flags;
}
