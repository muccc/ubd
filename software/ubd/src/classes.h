#ifndef _CLASSES_H_
#define _CLASSES_H_
#include <glib.h>
struct classdata{
    guint class;
    gchar name[100];
    gchar tcpservicename[100];
    gchar udpservicename[100];
    guint serviceport;
};

gboolean classes_exists(guint class);
gchar* classes_getClassName(guint class);
gchar* classes_getTcpServiceName(guint class);
gchar* classes_getUdpServiceName(guint class);
guint classes_getServicePort(guint class);
guint classes_getServicePortByClassName(gchar *class);
struct classdata *classes_getClass(guint class);
struct classdata *classes_getClassByName(gchar *class);
#endif
