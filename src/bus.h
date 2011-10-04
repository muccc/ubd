#ifndef _BUS_H_
#define _BUS_H_

#include <glib.h>
#include <stdio.h>

#include "packet.h"
#include "ubpacket.h"
#include "debug.h"


gint bus_sendToID(gchar *id, guchar *buf, gint len, guint classid,
                    gboolean reply);
void bus_sendToClass(guint class, guchar *buf, gint len );
gint bus_sendToAddress(gint dest, guchar *buf, gint len, guint class,
                  gboolean reply);
gint bus_streamToID(gchar *id, guchar *buf, gint len, guint classid,
                UBSTREAM_CALLBACK callback, gpointer data);

#endif
