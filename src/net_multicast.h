#ifndef _NET_MULTICAST_H_
#define _NET_MULTICAST_H_
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <net/if.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

GSocket * multicast_createSocket(gchar *groupname, guint port,
                                 GSocketAddress **sa);
#endif
