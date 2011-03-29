#include "net_tcp.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <syslog.h>

#include "debug.h"
#include "busmgt.h"
#include "mgt.h"
#include "bus.h"
#include "cmdparser.h"
#include "nodes.h"
#include "listen.h"

static void tcp_reply(gpointer data);
static void tcp_queueNewCommand(gpointer data);
static void tcp_queueNewMgt(gpointer data);
void tcp_init(void)
{
}

static void tcp_queueNewCommand(gpointer data)
{
    ub_assert(data != NULL);
    struct nodebuffer *nb = data;
    syslog(LOG_DEBUG,"tcp_cmd: new command for node %d\n", nb->n->busadr);
    bus_streamToID(nb->n->id, (guchar*)nb->cmd, nb->cmdlen, nb->classid,
                                tcp_reply, nb->out);
}

static void tcp_queueNewMgt(gpointer data)
{
    ub_assert(data != NULL);
    struct nodebuffer *nb = data;
    syslog(LOG_DEBUG,"tcp_cmd: new mgt for node %d\n", nb->n->busadr);
    busmgt_streamData(nb->n, (guchar*)nb->cmd, nb->cmdlen,
                                tcp_reply, nb->out);
}

static void tcp_writeBinaryEncoded(GOutputStream *out,
                                    guchar *data, gint len)
{
    g_output_stream_write(out, "B", 1, NULL, NULL);
    guchar tmp = len;
    g_output_stream_write(out, &tmp, 1, NULL, NULL);
    g_output_stream_write(out, data, len, NULL, NULL);
}

void tcp_writeCharacterEncoded(GOutputStream *out,
                                        guchar *data, gint len)
{
    //find newlines in the data
    //they must not be transmitted in the encoded data
    //TODO: find a defined way to deal with this
    //maybe switching to binary encoding in this case is usefull
    gint i;
    for( i=0; i<len; i++ ){
        if( data[i] == '\r' || data[i] == '\n' ){
            tcp_writeBinaryEncoded(out, data, len);
            return;
        }
    }
    //Write the header
    g_output_stream_write(out, "C", 1, NULL, NULL);
    g_output_stream_write(out, data, i, NULL, NULL);
    g_output_stream_write(out, "\n", 1, NULL, NULL);
}

static void tcp_reply(gpointer data)
{
    ub_assert(data != NULL);
    struct packetstream *ps = (struct packetstream *)data;
    //well these calls wil fail if the remote closed
    //the connection before we could answer
    //in this case the outputstream is invalid
    //and a error message is displayed
    if( ps->type == PACKET_DONE ){
        syslog(LOG_DEBUG,"tcp_reply: PACKET_DONE\n");
        g_output_stream_write(ps->data, "A", 1, NULL, NULL);
    }
    if( ps->type == PACKET_ABORT ){
        syslog(LOG_DEBUG,"tcp_reply: PACKET_ABORT\n");
        g_output_stream_write(ps->data, "N", 1, NULL, NULL);
    }
    if( ps->type == PACKET_PACKET ){
        syslog(LOG_DEBUG,"tcp_reply: PACKET_PACKET len=%d\n", ps->p.len);
        //g_output_stream_write(ps->data, ps->p.data, ps->p.len, NULL, NULL);
        tcp_writeCharacterEncoded(ps->data, ps->p.data, ps->p.len);
    }
}

static void tcp_parse(struct nodebuffer *nb, guchar data)
{
    ub_assert(nb != NULL);
    syslog(LOG_DEBUG,"state = %d data = %d\n",nb->state,data);
    switch(nb->state){
        case 0:
            if( data == 'C' ){
                nb->state=1;
                nb->cmdlen=0;
            }else if( data == 'B' ){
                nb->state=2;
                nb->cmdlen=0;
                nb->cmdbinlen=0;
            }else if( data == 'L' ){
                listen_register(nb->n, nb->classid, nb->out);
            }
        break;
        case 1:
            if( data == '\n' || data == '\r' ){
                nb->callback(nb);
                nb->state = 0;
            }else{
                nb->cmd[nb->cmdlen++] = data;
                if( nb->cmdlen == sizeof(nb->cmd) ){
                    nb->state = 4;
                }
            }
        break;
        case 2:
            if( data <= sizeof(nb->cmd) ){
                nb->cmdbinlen = data;
                nb->state = 3;
            }else{
                nb->state = 0;
            }
        break;
        case 3:
            nb->cmd[nb->cmdlen++] = data;
            if( --nb->cmdbinlen == 0 ){
                ub_assert(nb->callback != NULL);
                nb->callback(nb);
                nb->state = 0;
            }
        break;
        case 4:
            if( data == '\n' || data == '\r' )
                nb->state = 0;
        break;
    }
}

void tcp_listener_read(GInputStream *in, GAsyncResult *res,
                            struct nodebuffer *nb)
{
    GError * e = NULL;
    gssize len = g_input_stream_read_finish(in, res, &e);
    int i;
    if( len > 0 ){
        syslog(LOG_DEBUG,"tcp_listener_read: Received %d data bytes\n", len);
        if( nb->n == NULL ){
            syslog(LOG_DEBUG,"tcp_listener_read: node == NULL -> control data\n");
            for( i=0; i<len; i++ ){
                if( !cmdparser_parse(nb, nb->buf[i]) ){
                    //TODO: not sure if this is a clean way to close
                    //the tcp session
                    g_object_unref(nb->connection);
                    g_free(nb);
                    return;
                }
            }   
        }else{
            for( i=0; i<len; i++ ){
                tcp_parse(nb, nb->buf[i]);
            }
        }
        //keep the stream open
        g_input_stream_read_async(in, nb->buf, MAX_BUF,
            G_PRIORITY_DEFAULT, NULL,
            (GAsyncReadyCallback) tcp_listener_read, nb); 
    }else if( len == 0){
        syslog(LOG_DEBUG,"tcp_listener_read: connection closed\n");
        listen_unregister(nb->n, nb->classid, nb->out);
        g_object_unref(nb->connection);
        g_free(nb);
    }else{
        syslog(LOG_WARNING,"tcp_listener_read: received an error\n");
    }
}

gboolean tcp_listener(GSocketService    *service,
                        GSocketConnection *connection,
                        GObject           *source_object,
                        gpointer           user_data){
    source_object = NULL;
    struct socketdata *sd = user_data;
    struct nodebuffer *nodebuf = g_new0(struct nodebuffer,1);
    ub_assert(nodebuf != NULL);
    syslog(LOG_DEBUG,"new listener\n");
    if( user_data ){
        syslog(LOG_DEBUG,"socketdata is set\n");
        nodebuf->n = sd->n;
        nodebuf->classid = sd->classid;
        syslog(LOG_DEBUG,"service is for classid %u\n", nodebuf->classid);
    }else{
        syslog(LOG_DEBUG,"socketdata is null\n");
        nodebuf->n = NULL;
    }
    nodebuf->connection = connection; 
    nodebuf->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    nodebuf->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    nodebuf->cmdlen = 0;

    if( nodebuf->n == NULL ){
        char *msg = "Welcome to the control interface\n>";
        g_output_stream_write(nodebuf->out, msg, strlen(msg),
                        NULL, NULL);
    }else if( service ==
            nodebuf->n->tcpsockets[nodebuf->classid].socketservice ){
        nodebuf->callback = tcp_queueNewCommand; 
    }else if( service == nodebuf->n->mgtsocket.socketservice ){
        nodebuf->callback = tcp_queueNewMgt;
    }else{
        syslog(LOG_ERR,"tcp_listener: should not happen\n");
        ub_assert(FALSE);
    }
    g_input_stream_read_async(nodebuf->in, nodebuf->buf,
            sizeof(nodebuf->buf), G_PRIORITY_DEFAULT, NULL,
            (GAsyncReadyCallback)tcp_listener_read, nodebuf);
    g_object_ref(connection);

    return FALSE;
}


