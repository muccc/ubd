#include <config.h>
#include <stdio.h>
#include <glib.h>
#include <fcntl.h>
#include <termios.h>
#include <stdint.h>
#include <string.h>
#include <gio/gio.h>

#include "ubpacket.h"
#include "serial.h"
#include "debug.h"
#include "net6.h"
#include "packet.h"
#include "ubpacket.h"
#include "busmgt.h"
#include "address6.h"
#include "mgt.h"
#include "cmdparser.h"
#include "xmlconfig.h"
#include "groups.h"
#include "config.h"

int main (void)
{
    if (!g_thread_supported ()) g_thread_init (NULL);
    g_type_init();
    
    //printf("Please specify an interface to use.\n");
    //printf("Please specify a base address to use.\n");

    nodes_init();
    groups_init();
    xml_init("ubdconfig.xml");
    g_assert( !net_init(config.interface, 
                        config.base,
                        config.prefix) );
    mgt_init();

//    if( argc > 3 ){
        if( serial_open(config.device) == -1 ){
            printf("Failed to open serial device %s\n"
                "Aborting.\n", config.device);
            return 0;
        }
        //activate bridge
        serial_switch();
        packet_init();     
        busmgt_init();
//    }else{
//        printf("Please specify a serial port to use.\n");
//    }
    cmdparser_init();
    GMainLoop * mainloop = g_main_loop_new(NULL,TRUE);
    g_main_loop_run(mainloop);

    return 0;
}

