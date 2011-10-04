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
gint busmgt_getFreeBusAdr(); 
void busmgt_setName(uint8_t adr, char *name);
void busmgt_sendCmdData(uint8_t adr, uint8_t cmd, uint8_t *data, uint8_t len);
void busmgt_streamData(struct node *n, guchar *buf, gint len,
                UBSTREAM_CALLBACK callback, gpointer data);

#endif
