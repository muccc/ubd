#ifndef __UBMASTERMGT_H_
#define __UBMASTERMGT_H_
#include <stdint.h>
#include "ubpacket.h"

void ubmastermgt_init(void);
void ubmastermgt_tick(void);
uint8_t ubmastermgt_process(struct ubpacket_t * p);

#endif
