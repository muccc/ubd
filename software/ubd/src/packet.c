#include <stdint.h>
#include <glib.h>
#include <string.h>
#include <stdio.h>

#include "packet.h"
#include "ubpacket.h"
#include "message.h"
#include "serial.h"
#include "busmgt.h"

static void packet_inpacket(struct ubpacket* p);
//GHashTable* packet_callbacks;

GAsyncQueue * packet_status;
GAsyncQueue * packet_out;

gpointer packet_readerThread(gpointer data)
{
    data = NULL;
    struct message in;
    struct ubpacket p;

    printf("started packet_readerThread()\n");
    while( 1 ){
        serial_readMessage(&in);
        if( in.len > 0){
            switch( in.data[0] ){
                case PACKET_PACKET:
                    //printf("new packet\n");
                    if( sizeof(p) >= in.len ){
                        memcpy(&p,in.data+1,in.len-1);
                        //g_async_queue_push(packet_queues.packet_in,p);
                        packet_inpacket(&p);
                    }else{
                        //TODO: log this error
                    }
                break;
                case PACKET_DONE:
                    printf("new message: ");
                    printf("%c: packet done\n", in.data[0]);
                    g_async_queue_push(packet_status,(gpointer)PACKET_DONE);
                break;
                case PACKET_ABORT:
                    printf("new message: ");
                    printf("packet aborted\n");
                    g_async_queue_push(packet_status,(gpointer)PACKET_ABORT);
                break;
                case 'D':
                    //printf("debug\n");
                break;
            }
        }
    }
}

gpointer packet_writerThread(gpointer data)
{
    data = NULL;
    while( 1 ){
        struct message * msg = (struct message *)g_async_queue_pop(packet_out);
        g_assert(msg != NULL);
        
        struct ubpacket * p = (struct ubpacket *)msg->data;
        debug_packet("packet_writerThread",p);

        serial_writeMessage(msg);

        GTimeVal timeout;
        g_get_current_time(&timeout);
        g_time_val_add(&timeout,10 * 1000 * 1000);

        gpointer status = g_async_queue_timed_pop(packet_status,&timeout);
        g_assert(status != NULL);

        if( status == (gpointer)PACKET_ABORT ){
            //TODO: add log
            printf("PACKET WAS ABORTED\n");
        }else if( status == (gpointer)PACKET_DONE ){
            //printf("packet done\n");
        }
    }
}

/*static void printkey(gpointer key, gpointer value, gpointer data)
{
    value = NULL;
    data = NULL;
    printf(key);
}*/

static void packet_inpacket(struct ubpacket* p)
{
    /*void(*cb)(struct ubpacket*);
    gchar buf[2];
    buf[0] = p->data[0];
    buf[1] = 0;
    printf("registred:\n");
    g_hash_table_foreach(packet_callbacks,printkey,NULL);
    printf("end\n");
    cb = g_hash_table_lookup(packet_callbacks, buf);
    if( cb ){
        cb(p);
    }else{
        printf("There is no handler registerd for packet type %s\n",buf);
    }*/
    debug_packet("packet_inpacket",p);

    if( p->flags & UB_PACKET_MGT ){
        busmgt_inpacket(p);
    }else{
        //TODO: add packet forwarding
        printf("forwarding packet");
    }
}

void packet_init(void)
{
    //packet_callbacks = g_hash_table_new(g_str_hash, g_str_equal); 
    //packet_queues.packet_in = g_async_queue_new(); 
    packet_status = g_async_queue_new();
    packet_out = g_async_queue_new();
    //GThread * readerthread =
    g_thread_create(packet_readerThread,NULL,FALSE,NULL);
    g_thread_create(packet_writerThread,NULL,FALSE,NULL);
}

/*void packet_addCallback(gchar key, void(*cb)(struct ubpacket*))
{
    gchar buf[2];
    buf[0] = key;
    buf[1] = 0;

    g_hash_table_insert(packet_callbacks, g_strdup(buf), cb);

    printf("Added callback for message type %c to 0x%x\n",key,(unsigned int)cb);
}*/

void packet_outpacket(struct ubpacket* p)
{
    struct message * outmsg = g_new(struct message,1);
    p->src = 1;
    p->flags &= UB_PACKET_MGT | UB_PACKET_NOACK;

    printf("sending packet with dest=%u src=%u flags=%u len=%u\n",
                p->dest, p->src, p->flags,p->len);
    
    outmsg->len = p->len+UB_PACKET_HEADER;
    //g_assert(p->len + UB_PACKET_HEADER < );
    memcpy(outmsg->data, p, p->len + UB_PACKET_HEADER);
    g_async_queue_push(packet_out,outmsg);
}
