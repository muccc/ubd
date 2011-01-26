#ifndef _LISTEN_H_
#define _LISTEN_H_
#include <glib.h>
#include <gio/gio.h>
#include "nodes.h"
#include "packet.h"
void listen_register(struct node *n, GOutputStream *out);
void listen_unregister(struct node *n, GOutputStream *out);
void listen_newMessage(struct packetstream *ps);
#endif
