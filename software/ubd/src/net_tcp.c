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

struct tcpcmd{
    GOutputStream   *source;
    struct node     *n;
    gchar           cmd[MAX_BUF];
    gint            len;
};
static void tcp_newcommand(struct node*n, gchar *buf, gint len,
                            GOutputStream *source);
static gboolean tcp_cmd(gpointer data);

GQueue  *gq_tcpcommands;
static void tcp_newcommand(struct node*n, gchar *buf, gint len,
                            GOutputStream *source)
{
    struct tcpcmd *cmd = g_new(struct tcpcmd,1);
    memcpy(cmd->cmd, buf, len);
    cmd->len = len;
    cmd->n = n;
    cmd->source = source;
    g_queue_push_tail(gq_tcpcommands, cmd);
    g_idle_add(tcp_cmd,NULL);
}

static gboolean tcp_cmd(gpointer data)
{
    data = NULL;
    struct tcpcmd *cmd = (struct tcpcmd *)g_queue_pop_head(gq_tcpcommands);
    g_assert(cmd != NULL);
    printf("tcp_cmd: new command for node %d\n", cmd->n->busadr);
    g_free(cmd);
    return FALSE;
}

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
            tcp_newcommand(buf->n, buf->buf, len, buf->out);
        }
        //We keep the stream open
        g_input_stream_read_async(in, buf->buf, MAX_BUF,
            G_PRIORITY_DEFAULT, NULL,
            (GAsyncReadyCallback) tcp_listener_read, buf); 
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

void tcp_init(void)
{
    gq_tcpcommands = g_queue_new();
}

