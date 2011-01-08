#ifndef __UBMASTERMGT_H_
#define __UBMASTERMGT_H_
#include <stdint.h>
#include "ubpacket.h"

void ubbridgemgt_init(void);
void ubbridgemgt_tick(void);
uint8_t ubbridgemgt_process(struct ubpacket_t * p);

#endif
