#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

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

    
    key = gdbm_firstkey (db_nodes);
    while (key.dptr){
        data = gdbm_fetch(db_nodes, key);
        g_assert(data.dptr != NULL);
        struct DB_NODE * dbnode = (struct DB_NODE *) data.dptr;
        GInetAddress *addr = g_inet_address_new_from_bytes(
            ((struct DB_NODE *)data.dptr)->ip,
            G_SOCKET_FAMILY_IPV6);
        gchar *tmp = g_inet_address_to_string(addr);
        printf("node: id=%s ip=%s name=%s\n",key.dptr,tmp,dbnode->name);
        g_free(tmp);
        key = gdbm_nextkey(db_nodes, key);
    }
    
}

/*void db_addNode(gchar *id, GInetAddress *a)
{
    struct DB_NODE  node;
    key.dptr = id;
    key.dsize = strlen(id) + 1;   //include NULL
    node.groups[0] = 0;
    memcpy(node.ip, g_inet_address_to_bytes(a), 16);
    data.dptr = (void*)&node;
    data.dsize = sizeof(node);
    g_assert(gdbm_store(db_nodes, key, data, GDBM_REPLACE) == 0);
    gdbm_sync(db_nodes);
}*/

void db_addNode(struct node *n)
{
    struct DB_NODE  dbnode;
    key.dptr = n->id;
    key.dsize = strlen(n->id) + 1;   //include NULL
    memcpy(dbnode.groups, n->groups, 32);
    memcpy(dbnode.ip, g_inet_address_to_bytes(n->netadr), 16);
    memcpy(dbnode.name, n->name, MAX_ID);
    data.dptr = (void*)&dbnode;
    data.dsize = sizeof(dbnode);
    g_assert(gdbm_store(db_nodes, key, data, GDBM_REPLACE) == 0);
    gdbm_sync(db_nodes);
}

void db_loadNode(struct node *n)
{
    struct DB_NODE  *dbnode;
    key.dptr = n->id;
    key.dsize = strlen(n->id) + 1;   //include NULL
    data = gdbm_fetch(db_nodes, key);
    g_assert(data.dptr != NULL);
    dbnode = (struct DB_NODE *)data.dptr;

    memcpy(n->groups, dbnode->groups, 32);
    n->netadr = g_inet_address_new_from_bytes(dbnode->ip, G_SOCKET_FAMILY_IPV6);
    memcpy(n->name, dbnode->name, MAX_ID);
    free(data.dptr);
}

gboolean db_isNodeKnown(gchar *id)
{
    key.dptr = id;
    key.dsize = strlen(id) + 1;   //include NULL

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

GInetAddress * db_getIP(gchar *id)
{
    key.dptr = id;
    key.dsize = strlen(id) + 1;   //include NULL

    data = gdbm_fetch(db_nodes, key);
    g_assert(data.dptr != NULL);
    GInetAddress *a = g_inet_address_new_from_bytes(
        ((struct DB_NODE *)data.dptr)->ip,
        G_SOCKET_FAMILY_IPV6);
    g_assert(a != NULL);
    free(data.dptr);
    return a;
}
