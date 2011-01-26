#include <glib.h>
#include <gio/gio.h>
#include "listen.h"
#include "nodes.h"
#include "packet.h"
#include "net_tcp.h"

void listen_register(struct node *n, GOutputStream *out)
{
    n->listeners = g_slist_prepend(n->listeners, out);
}

void listen_unregister(struct node *n, GOutputStream *out)
{
    n->listeners = g_slist_remove_all(n->listeners, out);
}

//static void listen_iterate(gpointer data, gpointer user_data)
static void listen_iterate(gpointer data, struct packetstream *ps)
{
    tcp_writeCharacterEncoded(data,ps->p.data, ps->p.len);
}

void listen_newMessage(struct packetstream *ps)
{
    g_slist_foreach(ps->n->listeners, (GFunc)listen_iterate,  ps);
}

