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
    packet.dest = 0xFF;
    packet.class = class;
    packet.len = len;
    packet.flags = UB_PACKET_NOACK;
    memcpy(packet.data, buf, len);
    packet_outpacket(&packet);
}

gint bus_sendToID(gchar *id, guchar *buf, gint len, guint classid,
                  gboolean reply)
{ 
    struct ubpacket packet;
    struct node* n = nodes_getNodeById(id);
    ub_assert(n != NULL);

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
    ub_assert(n != NULL);

    packet.dest = n->busadr;
    packet.class = n->classes[classid];
    packet.len = len;
    packet.flags = 0;
    memcpy(packet.data, buf, len);
    packet_streamPacket(n, &packet, callback, data);

    return 0;
}

gint bus_sendToAddress(gint dest, guchar *buf, gint len, guint class,
                  gboolean reply)
{ 
    struct ubpacket packet;

    packet.dest = dest;
    packet.class = class;
    packet.len = len;
    packet.flags = 0;
    if( !reply )
        packet.flags = UB_PACKET_NOACK;
    memcpy(packet.data, buf, len);
    packet_outpacket(&packet);
    return 0;
}

