#ifndef _MGT_H_
#define _MGT_H_
#include <stdint.h>
#include <glib.h>
#include <gio/gio.h>

#include "nodes.h"
enum nodestate{
    NODE_TIMEOUT,
    NODE_DISCOVER,
    NODE_IDENTIFY,
    NODE_NORMAL
};

enum type{
    TYPE_NONE,
    TYPE_BRIDGE,
    TYPE_MULTICAST,
    TYPE_NODE
};
#define MAX_BUF     100
struct nodebuffer {
    struct node * n;
    char buf[MAX_BUF];
    char cmd[MAX_BUF];
    GSocketConnection *connection;
    GOutputStream *out;
    GInputStream *in;
};


void mgt_init(void);

struct node *mgt_createBridge(gchar *id);
struct node *mgt_createNode(gint type, gchar *id);
gint mgt_removeNodeById(gchar * id);

#endif
