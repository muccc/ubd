#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gdbm.h>
#include <gio/gio.h>
#include "db.h"

GDBM_FILE db_nodes, db_groups;
datum key, data;

void db_init(gchar *nodes, gchar *groups)
{
    db_nodes = gdbm_open(nodes, 0, GDBM_WRCREAT, 0666, 0);
    db_groups = gdbm_open(groups, 0, GDBM_WRCREAT, 0666, 0);
}

void db_addNode(gchar *name, GInetAddress *a)
{
    struct DB_NODE  node;
    key.dptr = name;
    key.dsize = strlen(name) + 1;   //include NULL
    node.groups[0] = 0;
    memcpy(node.ip, g_inet_address_to_bytes(a), 16);
    data.dptr = (void*)&node;
    data.dsize = sizeof(node);
    g_assert(gdbm_store(db_nodes, key, data, GDBM_REPLACE) == 0);
    gdbm_sync(db_nodes);
}

gboolean db_isNodeKnown(gchar *name)
{
    key.dptr = name;
    key.dsize = strlen(name) + 1;   //include NULL

    if( gdbm_exists(db_nodes, key) )
        return TRUE;
    return FALSE;
}

gboolean db_isIPKnown(GInetAddress *a)
{
    key = gdbm_firstkey (db_nodes);
    while (key.dptr){
        data = gdbm_fetch(db_nodes, key);
        g_assert(data.dptr != NULL);
        if( memcmp( ((struct DB_NODE *)data.dptr)->ip,
            g_inet_address_to_bytes(a),16) == 0 )
            return TRUE;
        key = gdbm_nextkey(db_nodes, key);
    }
    return FALSE;
}

GInetAddress * db_getIP(gchar *name)
{
    key.dptr = name;
    key.dsize = strlen(name) + 1;   //include NULL

    data = gdbm_fetch(db_nodes, key);
    g_assert(data.dptr != NULL);
    GInetAddress *a = g_inet_address_new_from_bytes(
        ((struct DB_NODE *)data.dptr)->ip,
        G_SOCKET_FAMILY_IPV6);
    g_assert(a != NULL);
    free(data.dptr);
    return a;
}
