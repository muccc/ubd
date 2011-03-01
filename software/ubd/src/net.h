#ifndef __NET6_H__
#define __NET6_H__
#include <glib.h>
#include "mgt.h"

#include "nodes.h"

gint net_init(gchar* interface, gchar* baseaddress);
void net_createSockets(struct node *n);
void net_removeSockets(struct node *n);

#endif
