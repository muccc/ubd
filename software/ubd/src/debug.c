#include <stdio.h>
#include <string.h>

#include "debug.h"
#include "packet.h"

void debug_hexdump(uint8_t * data, uint16_t len)
{
    uint16_t i;
    for(i=0; i<len; i++){
        if( data[i] < 0x10 ){
            printf(" 0x0%X", data[i]);
        }else if (data[i] < ' ' || data[i] > 0x7F){
            printf(" 0x%X", data[i]);
        }else{
            printf("%c", data[i]);
        }
    }
}

void debug_packet(gchar *reporter, struct ubpacket* p)
{
    gchar flags[200] = "";
    GTimeVal t;

    if( p->flags & 0x80 )
        strcat(flags, "PACKET NOT ACKED | ");
    if( p->flags & 0x40 )
        strcat(flags, "ACKSEQ | ");
    if( p->flags & UB_PACKET_MGT )
        strcat(flags, "MGT | ");
    if( p->flags & UB_PACKET_NOACK )
        strcat(flags, "NOACK | ");
    if( p->flags & UB_PACKET_DONE )
        strcat(flags, "DONE | ");
    if( p->flags & UB_PACKET_SEQ )
        strcat(flags, "SEQ | ");
    if( p->flags & UB_PACKET_ACK )
        strcat(flags, "ACK | ");
    if( p->flags & UB_PACKET_UNSOLICITED )
        strcat(flags, "UNSOLICITED | ");
   
    g_get_current_time(&t);
    printf("%ld.%04ld ",t.tv_sec,t.tv_usec);
    printf("%s: packet from %u to %u flags: (%x: %s) class: %u len: %u: ",
            reporter, p->src, p->dest, p->flags, flags, p->class,p->len);
    debug_hexdump(p->data, p->len);
    printf("\n");
}

