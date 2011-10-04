#include <stdio.h>
#include <string.h>
#include <syslog.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <fcntl.h>
#include <errno.h>
#include <unistd.h>

#include "debug.h"
#include "packet.h"

void debug_hexdump(uint8_t * data, uint16_t len)
{
    uint16_t i,j;
    char *buf = malloc(len*5);
    if( buf == NULL ){
        syslog(LOG_ERR,"debug_hexdump: malloc failed");
        return;
    }

    j = 0;
    for(i=0; i<len; i++){
        if( data[i] < 0x10 ){
            j += sprintf(buf+j, " 0x0%X", data[i]);
        }else if (data[i] < ' ' || data[i] > 0x7F){
            j += sprintf(buf+j, " 0x%X", data[i]);
        }else{
            j += sprintf(buf+j, "%c", data[i]);
        }
    }
    syslog(LOG_DEBUG, "%s", buf);
    free(buf);
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
    syslog(LOG_DEBUG,"%ld.%04ld %s: packet from %u to %u flags: (%x: %s) class: %u len: %u: ",
            t.tv_sec, t.tv_usec,
            reporter, p->src, p->dest, p->flags, flags, p->class,p->len);
    debug_hexdump(p->data, p->len);
}


void ub_assertion_message_expr        (const char     *domain,
                                         const char     *file,
                                         int             line,
                                         const char     *func, 
                                         const char     *expr)
{
    syslog(LOG_ERR,"assertion failed. domain=%s file=%s line=%d func=%s expr=%s",
        domain, file, line, func, expr);
    exit(-1);
}
