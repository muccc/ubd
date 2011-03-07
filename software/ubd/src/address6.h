#ifndef _ADDRESS6_H_
#define _ADDRESS6_H_
#include <glib.h>
#include <gio/gio.h>
#include "mgt.h"
#include "net.h"

void address6_init(GInetAddress *base, GInetAddress *multicastbase);
void address6_createAddress(struct node *n);
void address6_removeAddress(struct node *n);

#endif
