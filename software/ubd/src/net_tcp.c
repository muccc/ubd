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
            gchar* result = NULL;
            len = cmdparser_cmd(buf->buf, len, &result);
            if( len > 0 ){        //got something to reply
                g_output_stream_write(buf->out, result, len, NULL, NULL);
                g_free(result);
            }if( len < 0 ){       //close this session
                g_output_stream_write(buf->out, result, strlen(result), NULL, NULL);
                g_free(result);
                //not sure if this is a clean way to close the tcp session
                g_object_unref(buf->connection);
                return;
            }
        }else{
           bus_sendToID(buf->n->id, (guchar*)buf->buf, len, FALSE);
           //disable this callback and set n->lastconnection
           //then wait for ACK/NACK/RESULT, send it and reenable this callback?
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

    struct nodebuffer *buf = g_new0(struct nodebuffer,1);
    g_assert(buf != NULL);

    buf->n = (struct node*)user_data;
    buf->connection = connection; 
    buf->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    buf->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));

    if( user_data == NULL){
        char *msg = "Welcome to the data interface\n>";
        g_output_stream_write(buf->out, msg,strlen(msg),NULL,NULL);
    }
    g_input_stream_read_async(buf->in, buf->buf, MAX_BUF,
        G_PRIORITY_DEFAULT, NULL, (GAsyncReadyCallback)tcp_listener_read,
        buf);
    g_object_ref(connection);
    return FALSE;
}

