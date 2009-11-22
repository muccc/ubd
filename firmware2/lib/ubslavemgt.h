#ifndef __UBSLAVEMGT_H_
#define __UBSLAVEMGT_H_
#include <stdint.h>
#include "ubpacket.h"

#define MGT_DISCOVER 'D'
#define MGT_IDENTIFY 'I'
#define MGT_ALIVE    'A'
#define MGT_MASTER   'M'

void ubslavemgt_init(void);
void ubslavemgt_tick(void);
uint8_t ubslavemgt_process(struct ubpacket_t * p);

#endif
