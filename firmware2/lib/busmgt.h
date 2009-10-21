#ifndef __BUSMGT_H_
#define __BUSMGT_H_
#include <stdint.h>
#include "ubpacket.h"

#define MGT_DISCOVER 'D'
#define MGT_IDENTIFY 'I'
#define MGT_ALIVE    'A'

void busmgt_init(void);
void busmgt_tick(void);
uint8_t busmgt_process(struct ubpacket_t * p);

#endif
