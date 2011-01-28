#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>

#include "debug.h"
#include "bus.h"
#include "cmdparser.h"
#include "nodes.h"

gboolean udp_read(GSocket *socket, GIOCondition condition,
                                        gpointer user_data)
{
    uint8_t buf[100];
    GSocketAddress * src;
    struct socketdata *sd = user_data;
    struct node *n = sd->n;
    guint classid = sd->classid;
    gssize len;    
    if( condition == G_IO_IN ){
        len = g_socket_receive_from(socket,&src,(gchar*)buf,
                                sizeof(buf),NULL,NULL);
        if( len > 0 ){
            printf("udp_read: Received:");
            debug_hexdump(buf,len);
            printf("\n");
            if( user_data != NULL ){
                bus_sendToID(n->id, buf, len, classid, FALSE);
            }else{
                //UDP data to the mgt port is ignored
            }
        }else{
            printf("udp_read: Error while receiving: len=%d\n",len);
        }
    }else{
        printf("udp_read: Received ");
        if( condition == G_IO_ERR ){
            printf("G_IO_ERR\n");
        }else if( condition == G_IO_HUP ){ 
            printf("G_IO_HUP\n");
        }else if( condition == G_IO_OUT ){ 
            printf("G_IO_OUT\n");
        }else if( condition == G_IO_PRI ){ 
            printf("G_IO_PRI\n");
        }else if( condition == G_IO_NVAL ){ 
            printf("G_IO_NVAL\ndropping source\n");
            return FALSE;
        }else{
            printf("unkown condition = %d\n",condition);
        }
    }
    return TRUE;
}

