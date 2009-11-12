#ifndef _BUS_H_
#define _BUS_H_

#include <glib.h>
#include <stdio.h>

#include "packet.h"
#include "ubpacket.h"
#include "debug.h"


gint bus_sendToID(gchar *id, guchar *buf, gint len, gboolean reply);

#endif
