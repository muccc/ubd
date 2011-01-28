#include <glib.h>
#include <gio/gio.h>
#include "listen.h"
#include "nodes.h"
#include "packet.h"
#include "net_tcp.h"

void listen_register(struct node *n, guint classid, GOutputStream *out)
{
    n->tcpsockets[classid].listeners = 
        g_slist_prepend(n->tcpsockets[classid].listeners, out);
}

void listen_unregister(struct node *n, guint classid, GOutputStream *out)
{
    n->tcpsockets[classid].listeners =
        g_slist_remove_all(n->tcpsockets[classid].listeners, out);
}

static void listen_iterate(gpointer data, struct packetstream *ps)
{
    tcp_writeCharacterEncoded(data,ps->p.data, ps->p.len);
}

void listen_newMessage(struct packetstream *ps)
{
    g_assert(ps != NULL);
    g_assert(ps->n != NULL);
    guint classid = ps->p.classid;
    g_slist_foreach(ps->n->tcpsockets[classid].listeners,
            (GFunc)listen_iterate,  ps);
}

