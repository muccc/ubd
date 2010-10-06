#ifndef _NODES_H_
#define _NODES_H_

#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>
#include <string.h>
#include <packet.h>

#define MAX_NODE    256
#define MAX_ID      100

struct node{
    gint        type;
    gchar       id[MAX_ID];
    gchar       name[MAX_ID];
    gchar       domain[MAX_ID];
    gchar       version[MAX_ID];
    gint        groups[32];
    
    gboolean    active;
    gboolean    free;
    gint        state;

    gint        poll;
    gint        timeout;
    gint        tpoll;
    gint        ttimeout;

    gint        busadr;
    gboolean    busup;

    GInetAddress    *netadr;
    GSocket         *ubnetd;
    gboolean        netup;
    GSocket         *udp;

    UBSTREAM_CALLBACK   currentcallback;
    gpointer            currentdata;

    UBSTREAM_CALLBACK   nextcallback;
    gpointer            nextdata;

    GSocketService * dataservice;
    GSocketService * mgtservice;

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

