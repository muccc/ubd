#ifndef _INTERFACE_H_
#define _INTERFACE_H_
#include <glib.h>
#include <gio/gio.h>
#include "nodes.h"

void interface_init();
gint interface_createAddress(struct node *n); 
gint interface_removeAddress(struct node *n); 
GInetAddress *interface_getConfiguredAddress(struct node *n);
#endif
