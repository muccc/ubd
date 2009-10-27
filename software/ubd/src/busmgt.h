#ifndef _BUSMGT_H_
#define _BUSMGT_H_

#include <glib.h>
#include <stdio.h>

#include "packet.h"
#include "ubpacket.h"
#include "debug.h"

#define BUSMGT_ID   'M'

void busmgt_inpacket(struct ubpacket* p);
void busmgt_init(void);


#endif
