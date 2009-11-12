#ifndef _INTERFACE_H_
#define _INTERFACE_H_
#include <glib.h>
#include <gio/gio.h>

void interface_init(gchar *interface);
void interface_pushAddress(GInetAddress *addr);
void interface_removeAddress(GInetAddress *addr);
GInetAddress* interface_getConfiguredAddress(void);
#endif
