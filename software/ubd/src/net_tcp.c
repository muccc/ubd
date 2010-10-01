#include "net_tcp.h"
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
#include "cmdparser.h"
#include "nodes.h"

void tcp_listener_read(GInputStream *in, GAsyncResult *res,
                            struct nodebuffer *buf)
{
    GError * e = NULL;
    gssize len = g_input_stream_read_finish(in, res, &e);
    if( len > 0 ){
        printf("tcp_listener_read: Received %d data bytes\n", len);
        if( buf->n == NULL ){
            printf("tcp_listener_read: node == NULL -> control data\n");
            if( !cmdparser_cmdtostream(buf->buf, len, buf->out) ){
                //not sure if this is a clean way to close the tcp session
                g_object_unref(buf->connection);
                return;
            }
        }else{
            //gchar *msg = g_new(gchar,len);
            //memcpy(msg, buf->buf, len);
            //g_queue_push_tail(buf->n->tcpqueue, msg);
            //g_idle_add(tcp_in,buf->n);
            //bus_sendToID(buf->n->id, (guchar*)buf->buf, len, FALSE);
        }

        g_input_stream_read_async(in, buf->buf, MAX_BUF, G_PRIORITY_DEFAULT,
        NULL, (GAsyncReadyCallback) tcp_listener_read, buf); 
    }else if( len == 0){
        printf("tcp_listener_read: connection closed\n");
        g_object_unref(buf->connection);
    }else{
        printf("tcp_listener_read received an error\n");
    }
}

gboolean tcp_listener(GSocketService    *service,
                        GSocketConnection *connection,
                        GObject           *source_object,
                        gpointer           user_data){
    service = NULL;
    source_object = NULL;

    struct nodebuffer *nodebuf = g_new0(struct nodebuffer,1);
    g_assert(nodebuf != NULL);

    nodebuf->n = (struct node*)user_data;
    nodebuf->connection = connection; 
    nodebuf->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    nodebuf->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));

    if( user_data == NULL ){
        char *msg = "Welcome to the control interface\n>";
        g_output_stream_write(nodebuf->out, msg, strlen(msg), NULL, NULL);
    }
    g_input_stream_read_async(nodebuf->in, nodebuf->buf, sizeof(nodebuf->buf),
        G_PRIORITY_DEFAULT, NULL, (GAsyncReadyCallback)tcp_listener_read,
        nodebuf);
    g_object_ref(connection);
    return FALSE;
}

