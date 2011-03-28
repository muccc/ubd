#include <config.h>
#include <stdio.h>
#include <glib.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
#include <gio/gio.h>
#include <syslog.h>

#include "ubpacket.h"
#include "serial.h"
#include "debug.h"
#include "net.h"
#include "packet.h"
#include "ubpacket.h"
#include "busmgt.h"
#include "address6.h"
#include "mgt.h"
#include "cmdparser.h"
#include "xmlconfig.h"
#include "groups.h"
#include "config.h"
#include "avahi.h"
#include "broadcast.h"
#include "daemon.h"

int main (int argc, char *argv[])
{
    //openlog("ubd",LOG_PID | LOG_PERROR ,LOG_DAEMON);
    openlog("ubd", LOG_PID, LOG_DAEMON);

    if (!g_thread_supported ()) g_thread_init (NULL);
    g_type_init();
    GMainLoop * mainloop = g_main_loop_new(NULL,FALSE);
    avahi_init(mainloop);

    nodes_init();
    groups_init();
    if( argc < 1 ){
        xml_init("/etc/ubdconfig.xml");
    }else{
        xml_init(argv[1]);
    }
    daemon_init();
    if( config.interface == NULL ){
        syslog(LOG_ERR, "Please specify an interface to use.\n");
        return -1;
    }

    if( config.base == NULL ){
        syslog(LOG_ERR, "Please specify a base address to use.\n");
        return -1;
    }

    if( config.multicastbase == NULL ){
        syslog(LOG_ERR, "Please specify a multicast base address to use.\n");
        return -1;
    }
    broadcast_init();
    if( net_init(config.interface, 
                        config.base, config.multicastbase) ){
        syslog(LOG_ERR, "Failed to set up network.\n"
                "Interface=%s\nBaseaddress=%s\n"
                "\nAborting.\n",
                config.interface, config.base);
        return -1;
    }

    xml_parsegroupsandnodes();
    mgt_init();

    if( serial_open(config.device) ){
        syslog(LOG_ERR, "Failed to open serial device %s\n"
                "Aborting.\n", config.device);
        return -1;
    }

    //activate bridge
    serial_switch();
    packet_init();     
    busmgt_init();
    cmdparser_init();

    //openlog("ubd", LOG_PID, LOG_DAEMON);
    daemon_close_stderror();
    
    g_main_loop_run(mainloop);
    return 0;
}

