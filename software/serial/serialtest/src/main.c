#include <config.h>
#include <stdio.h>
#include <gnet.h>
#include <glib.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>

#include "ubpacket.h"
#include "serial.h"
#include "debug.h"
#include "net6.h"

struct queues{
    GAsyncQueue * packet_in;
    GAsyncQueue * status_in;
    GAsyncQueue * packet_out;
    GAsyncQueue * status_out;
};

struct queues q;

gpointer reader(gpointer data)
{
    struct queues * q = data;
    printf("reader thread started\n");
    while(1){
        printf("waiting for packet...");
        struct ubpacket * p = g_async_queue_pop(q->packet_in); 
        printf("read packet from %u to %u flags: %x seq=%u len %u: ", 
                p->src, p->dest, p->flags, p->seq, p->len);
        debug_hexdump(p->data, p->len);
        printf("\n");
    }
    printf("reader thread stoped\n");
}

void main_newmessage(struct message * msg)
{
    if( msg->data[0] == 'P' ){
        struct ubpacket * p = g_new(struct ubpacket,1);
        memcpy(p,msg->data+1,msg->len-1);
        g_async_queue_push(q.packet_in,p);
        g_free(msg);
    }else if( msg->data[0] == 'S' ){ 
        g_async_queue_push(q.status_in,msg);
    }
}

int main (int argc, char *argv[])
{
    if (!g_thread_supported ()) g_thread_init (NULL);
    printf ("This is " PACKAGE_STRING ".\n");

    net_init("::1");

    if( argc < 2 ){
        printf("Please specify a serial port to use.\n");
        return 0;
    }
    
    if( serial_open(argv[1], main_newmessage) == -1 ){
        printf("Failed to open serial device %s\nAborting.\n", argv[1]);
        return 0;
    }

    q.packet_in = g_async_queue_new(); 
    q.status_in = g_async_queue_new();

    GThread * readerthread = g_thread_create(reader,&q,FALSE,NULL);
    
    //g_io_channel_write_chars(serial, "\\0P\x01\xFF\x00\x00\x02Mr\\1",12,NULL,NULL);

    GMainLoop * mainloop = g_main_loop_new(NULL,TRUE);
    g_main_loop_run(mainloop);

    return 0;
}

