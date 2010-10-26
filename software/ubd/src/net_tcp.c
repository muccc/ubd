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
#include "bus.h"
#include "cmdparser.h"
#include "nodes.h"

static void tcp_reply(gpointer data);
static void tcp_queueNewCommand(gpointer data);
static void tcp_queueNewMgt(gpointer data);
void tcp_init(void)
{
}

static void tcp_queueNewCommand(gpointer data)
{
    g_assert(data != NULL);
    struct nodebuffer *nb = data;
    printf("tcp_cmd: new command for node %d\n", nb->n->busadr);
    bus_streamToID(nb->n->id, (guchar*)nb->cmd, nb->cmdlen,
                                tcp_reply, nb->out);
}

static void tcp_queueNewMgt(gpointer data)
{
    g_assert(data != NULL);
    struct nodebuffer *nb = data;
    printf("tcp_cmd: new mgt for node %d\n", nb->n->busadr);
    busmgt_streamData(nb->n, (guchar*)nb->cmd, nb->cmdlen,
                                tcp_reply, nb->out);
}

static void tcp_reply(gpointer data)
{
    g_assert(data != NULL);
    struct packetstream *ps = (struct packetstream *)data;
    //well these calls wil fail if the remote closed
    //the connection before we could answer
    //in this case the outputstream is invalid
    //and a error message is displayed
    if( ps->type == PACKET_DONE ){
        printf("tcp_reply: PACKET_DONE\n");
        g_output_stream_write(ps->data, "A", 1, NULL, NULL);
    }
    if( ps->type == PACKET_ABORT ){
        printf("tcp_reply: PACKET_ABORT\n");
        g_output_stream_write(ps->data, "N", 1, NULL, NULL);
    }
    if( ps->type == PACKET_PACKET ){
        printf("tcp_reply: PACKET_PACKET len=%d\n", ps->p.len);
        g_output_stream_write(ps->data, ps->p.data, ps->p.len, NULL, NULL);
    }
}

static void tcp_parse(struct nodebuffer *nb, guchar data)
{
    g_assert(data != nb);
    printf("state = %d data = %d\n",nb->state,data);
    switch(nb->state){
        case 0:
            if( data == 'C' ){
                nb->state=1;
                nb->cmdlen=0;
            }else if( data == 'B' ){
                nb->state=2;
                nb->cmdlen=0;
                nb->cmdbinlen=0;
            }
        break;
        case 1:
            if( data == '\n' || data == '\r' ){
                nb->callback(nb);
                nb->state = 0;
            }else if( data < 0x20 ){
                nb->state = 4;
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
        printf("tcp_listener_read: Received %d data bytes\n", len);
        if( nb->n == NULL ){
            printf("tcp_listener_read: node == NULL -> control data\n");
            for( i=0; i<len; i++ ){
                if( !cmdparser_parse(nb, nb->buf[i]) ){
                    //not sure if this is a clean way to close
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
        printf("tcp_listener_read: connection closed\n");
        g_object_unref(nb->connection);
        g_free(nb);
    }else{
        printf("tcp_listener_read: received an error\n");
    }
}

gboolean tcp_listener(GSocketService    *service,
                        GSocketConnection *connection,
                        GObject           *source_object,
                        gpointer           user_data){
    source_object = NULL;

    struct nodebuffer *nodebuf = g_new0(struct nodebuffer,1);
    g_assert(nodebuf != NULL);

    nodebuf->n = (struct node*)user_data;
    nodebuf->connection = connection; 
    nodebuf->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    nodebuf->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));
    nodebuf->cmdlen = 0;
    if( nodebuf->n == NULL ){
        if( nodebuf->n == NULL ){
            char *msg = "Welcome to the control interface\n>";
            g_output_stream_write(nodebuf->out, msg, strlen(msg),
                        NULL, NULL);
        }
    }else if( service == nodebuf->n->dataservice ){
        nodebuf->callback = tcp_queueNewCommand; 
    }else if( service == nodebuf->n->mgtservice ){
        nodebuf->callback = tcp_queueNewMgt;
    }
    g_input_stream_read_async(nodebuf->in, nodebuf->buf,
                        sizeof(nodebuf->buf), G_PRIORITY_DEFAULT, NULL,
                        (GAsyncReadyCallback)tcp_listener_read, nodebuf);
    g_object_ref(connection);

    return FALSE;
}


