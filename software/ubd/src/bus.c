#include <glib.h>
#include <string.h>

#include "bus.h"
#include "packet.h"
#include "ubpacket.h"
#include "bus.h"
#include "mgt.h"

gint bus_sendToID(gchar *id, guchar *buf, gint len, guint classid,
                  gboolean reply)
{ 
    struct ubpacket packet;
    struct node* n = nodes_getNodeById(id);
    g_assert(n != NULL);

    packet.dest = n->busadr;
    packet.classid = classid;
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
    packet.classid = classid;
    packet.len = len;
    packet.flags = 0;
    memcpy(packet.data, buf, len);
    packet_streamPacket(n, &packet, callback, data);

    return 0;
}

