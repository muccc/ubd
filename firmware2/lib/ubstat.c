#include "ubconfig.h"
#include "ubstat.h"

struct ubstat_t ubstat[UB_NODEMAX];


void ubstat_init(void)
{
    uint8_t i;
    for(i=0; i<UB_NODEMAX; i++){
        ubstat[i].known = 0;              //reset all nodes
    }
}

inline struct ubstat_t * ubstat_getFlags(uint8_t adr)
{
    return &ubstat[adr];
}

void ubstat_addNode(uint8_t adr, struct ubstat_t flags)
{
    ubstat[adr] = flags;
}
