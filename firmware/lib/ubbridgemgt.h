#ifndef __UBBRIDGEMGT_H_
#define __UBBRIDGEMGT_H_
#include <stdint.h>
#include "ubpacket.h"

void ubbridgemgt_init(void);
void ubbridgemgt_tick(void);
uint8_t ubbridgemgt_process(struct ubpacket_t * p);

#endif
