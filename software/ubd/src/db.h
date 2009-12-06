#ifndef _DB_H_
#define _DB_H_
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gdbm.h>
#include <gio/gio.h>

typedef gchar* DB_NODEKEY;
struct DB_NODE {
    
    uint16_t    groups[32];
    uint16_t    permission;
    uint8_t     ip[16];
};

typedef uint16_t DB_GROUP_KEY;
struct DB_GROUP {
    char        id[100];
};

void db_init(gchar *nodes, gchar *groups);
void db_addNode(gchar *name, GInetAddress *a);
gboolean db_isNodeKnown(gchar *name);
gboolean db_isIPKnown(GInetAddress *a);
GInetAddress * db_getIP(gchar *name);

#endif
