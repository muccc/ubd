#include <glib.h>
#include <string.h>

#include "bus.h"
#include "packet.h"
#include "ubpacket.h"
#include "bus.h"
#include "mgt.h"

gint bus_sendToID(gchar *id, guchar *buf, gint len, gboolean reply)
{ 
    struct ubpacket packet;
    struct node* n = nodes_getNodeById(id);
    g_assert(n != NULL);

    packet.dest = n->busadr;
    packet.len = len;
    if( !reply )
        packet.flags = UB_PACKET_NOACK;
    memcpy(packet.data, buf, len);
    packet_outpacket(&packet);
    return 0;
}
