#ifndef _NODES_H_
#define _NODES_H_

#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>
#include "packet.h"
#include "avahi.h"

#define MAX_NODE    256
#define MAX_ID      100

struct socketdata{
    struct node *n;
    GSource *source;
    GSocket *socket;
    GSocketService *socketservice;
    GSocketAddress *socketaddress;
    guint classid;
    GSList          *listeners;
    AvahiEntryGroup *avahiservicegroup;
    gchar           *avahiservicename;
};

struct node{
    gint        type;
    gchar       id[MAX_ID];
    gchar       name[MAX_ID];
    gchar       domain[MAX_ID];
    gchar       version[MAX_ID];
    gchar       hostname[MAX_ID];
    gint        groups[32];
    guchar      classes[32];
    
    gboolean    active;
    gboolean    free;
    gint        state;

    gint        poll;
    gint        timeout;
    gint        tpoll;
    gint        ttimeout;

    //address of the node on the bus 
    gint        busadr;
    gboolean    busup;

    //unicast address of the node
    GInetAddress    *netadr;
    AvahiEntryGroup *avahiaddressgroup;
    gboolean        netup;

    //connection to the ubnetd used to create
    //the unicast address
    GSocket         *ubnetd;
    
    //one socketpair per class
    struct socketdata udpsockets[32];
    struct socketdata tcpsockets[32];
    struct socketdata mgtsocket;

    //callbacks for the packet threads
    UBSTREAM_CALLBACK   currentcallback;
    gpointer            currentdata;

    UBSTREAM_CALLBACK   nextcallback;
    gpointer            nextdata;


};

void nodes_init(void);
struct node *nodes_getFreeNode(void);
void nodes_addNode(struct node *newnode);

gint nodes_getNodeCount(void);
struct node *nodes_getNode(gint node);

struct node *nodes_getNodeById(gchar *id);
struct node *nodes_getNodeByBusAdr(gint adr);
struct node *nodes_getNodeByNetAdr(GInetAddress *addr);

void nodes_activateNode(struct node *node);
void nodes_deactivateNode(struct node *node);


void nodes_setNameFromID(struct node *n);
#endif

