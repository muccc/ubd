#ifndef __UBSTAT_H_
#define __UBSTAT_H_

#include "ubconfig.h"

#define UB_KNOWN            0x01
#define UB_RS485            0x02
#define UB_RF               0x04
#define UB_UB_QUERYMAX      0x08

void ubstat_init(void);
inline uint8_t ubstat_getFlags(uint8_t adr);
uint8_t ubstat_addNode(uint8_t adr, uint8_t flags);
#endif
