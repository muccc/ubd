#ifndef __UDP_H_
#define __UDP_H_
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "debug.h"
#include "bus.h"
#include "cmdparser.h"
#include "nodes.h"

gboolean udp_read(GSocket *socket, GIOCondition condition, gpointer user_data);
#endif
