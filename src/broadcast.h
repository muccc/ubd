#ifndef _BROADCAST_H_
#define _BROADCAST_H_
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "debug.h"
#include "address6.h"
#include "interface.h"
#include "config.h"
#include "nodes.h"

void broadcast_init(void);
gboolean broadcast_addService(gint service);
gboolean broadcast_read(GSocket *socket, GIOCondition condition,
                                        gpointer user_data);
#endif
