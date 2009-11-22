#ifndef __UBSTAT_H_
#define __UBSTAT_H_

#include "ubconfig.h"

struct ubstat_t {
    uint8_t known:1;
    uint8_t rs485:1;
    uint8_t rf:1;
    uint8_t interval:4;
    uint8_t counter:4;
    uint8_t inseq:1;
    uint8_t outseq:1;
};

void ubstat_init(void);
inline struct ubstat_t * ubstat_getFlags(uint8_t adr);
void ubstat_addNode(uint8_t adr, struct ubstat_t flags);

#endif
