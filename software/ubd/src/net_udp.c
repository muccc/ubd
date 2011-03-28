#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>
#include <syslog.h>

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
            syslog(LOG_DEBUG,"udp_read: Received:");
            debug_hexdump(buf,len);
            syslog(LOG_DEBUG,"\n");
            if( user_data != NULL ){
                bus_sendToID(n->id, buf, len, classid, FALSE);
            }else{
                //UDP data to the mgt port is ignored
            }
        }else{
            syslog(LOG_WARNING,"udp_read: Error while receiving: len=%d\n",len);
        }
    }else{
        syslog(LOG_DEBUG,"udp_read: Received ");
        if( condition == G_IO_ERR ){
            syslog(LOG_DEBUG,"G_IO_ERR\n");
        }else if( condition == G_IO_HUP ){ 
            syslog(LOG_DEBUG,"G_IO_HUP\n");
        }else if( condition == G_IO_OUT ){ 
            syslog(LOG_DEBUG,"G_IO_OUT\n");
        }else if( condition == G_IO_PRI ){ 
            syslog(LOG_DEBUG,"G_IO_PRI\n");
        }else if( condition == G_IO_NVAL ){ 
            syslog(LOG_DEBUG,"G_IO_NVAL\ndropping source\n");
            return FALSE;
        }else{
            syslog(LOG_DEBUG,"unkown condition = %d\n",condition);
        }
    }
    return TRUE;
}

