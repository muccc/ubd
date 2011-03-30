#ifndef _NET_TCP_
#define _NET_TCP_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "debug.h"
#include "busmgt.h"
#include "mgt.h"
#include "address6.h"
#include "interface.h"
#include "bus.h"
//#include "cmdparser.h"
#include "nodes.h"

typedef void (*UBNODEBUF_CALLBACK)(gpointer);
struct nodebuffer {
    struct node * n;
    char buf[MAX_BUF];
    char cmd[MAX_BUF];
    gint cmdlen;
    gint state;
    gint cmdbinlen;
    GSocketConnection *connection;
    GOutputStream *out;
    GInputStream *in;
    UBNODEBUF_CALLBACK callback;
    guint classid;
};

void tcp_listener_read(GInputStream *in, GAsyncResult *res,
                            struct nodebuffer *buf);
gboolean tcp_listener(GSocketService    *service,
                        GSocketConnection *connection,
                        GObject           *source_object,
                        gpointer           user_data);

void tcp_init(void);
void tcp_writeCharacterEncoded(GOutputStream *out,
                               guchar *data, gint len, gboolean unsolicited);

#endif
