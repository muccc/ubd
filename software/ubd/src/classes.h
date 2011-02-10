#ifndef _CLASSES_H_
#define _CLASSES_H_
#include <glib.h>

gchar* classes_getClassName(guint class);
gchar* classes_getTcpServiceName(guint class);
gchar* classes_getUdpServiceName(guint class);
guint classes_getServicePort(guint class);

#endif
