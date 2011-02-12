#include <glib.h>
#include <string.h>

#include "bus.h"
#include "packet.h"
#include "ubpacket.h"
#include "bus.h"
#include "mgt.h"

void bus_sendToClass(guint class, guchar *buf, gint len )
{
    struct ubpacket packet;
    guint nodecount = nodes_getNodeCount();
    guint i,j;

    packet.len = len;
    packet.flags = UB_PACKET_NOACK;
    memcpy(packet.data, buf, len);

    for(i=0; i<nodecount; i++){ 
        struct node *n = nodes_getNode(i);
        for(j=0; j<32; j++){
            if( n->classes[j] == class ){
                packet.dest = n->busadr;
                packet.class = n->classes[j];               
                packet_outpacket(&packet);
            }
        }
    }
}

gint bus_sendToID(gchar *id, guchar *buf, gint len, guint classid,
                  gboolean reply)
{ 
    struct ubpacket packet;
    struct node* n = nodes_getNodeById(id);
    g_assert(n != NULL);

    packet.dest = n->busadr;
    packet.class = n->classes[classid];
    packet.len = len;
    packet.flags = 0;
    if( !reply )
        packet.flags = UB_PACKET_NOACK;
    memcpy(packet.data, buf, len);
    packet_outpacket(&packet);
    return 0;
}

gint bus_streamToID(gchar *id, guchar *buf, gint len, guint classid,
                UBSTREAM_CALLBACK callback, gpointer data)

{
    struct ubpacket packet;
    struct node* n = nodes_getNodeById(id);
    g_assert(n != NULL);

    packet.dest = n->busadr;
    packet.class = n->classes[classid];
    packet.len = len;
    packet.flags = 0;
    memcpy(packet.data, buf, len);
    packet_streamPacket(n, &packet, callback, data);

    return 0;
}



