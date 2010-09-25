#ifndef __DEBUG_H_
#define __DEBUG_H_
#include <stdint.h>

#include "packet.h"
void debug_hexdump(uint8_t * data, uint16_t len);
void debug_packet(gchar *reporter, struct ubpacket* p);
#endif
