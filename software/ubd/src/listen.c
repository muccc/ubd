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
    tcp_writeCharacterEncoded(data,ps->p.data, ps->p.len, TRUE);
}

void listen_newMessage(struct packetstream *ps)
{
    ub_assert(ps != NULL);
    ub_assert(ps->n != NULL);
    guint i;
    guint classid = 0;
    //guint classid = ps->p.classid;
    for(i=0; i<sizeof(ps->n->classes); i++){
        if( ps->n->classes[i] == ps->p.class )
            classid = i;
    }

    g_slist_foreach(ps->n->tcpsockets[classid].listeners,
            (GFunc)listen_iterate,  ps);
}

