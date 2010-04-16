#ifndef _MGT_H_
#define _MGT_H_
#include <stdint.h>
#include <glib.h>
#include <gio/gio.h>

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

#define MAX_ID    100
struct node{
    gint        type;
    gchar       id[MAX_ID];
    gchar       name[MAX_ID];
    gchar       domain[MAX_ID];

    gint        busadr;
    gboolean    busup;
    GInetAddress *netadr;
    gboolean    netup;

    gint        state;

    gint        poll;
    gint        timeout;
    gint        tpoll;
    gint        ttimeout;
    
    GSocket*    udp;
    GSocket*    mgtudp;

    uint8_t     groups[32];
};

#define MAX_NODE    256

void mgt_init(void);

struct node *mgt_createBridge(gchar *id);
struct node *mgt_createNode(gint type, gchar *id);
struct node* mgt_getNodeById(gchar* id);
struct node* mgt_getNodeByBusAdr(gint adr);
struct node* mgt_getNodeByNetAdr(GInetAddress *addr);

gint mgt_removeNodeById(gchar * id);

#endif
