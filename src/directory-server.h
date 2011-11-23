#ifndef _DIRECTORY_SERVER_H_
#define _DIRECTORY_SERVER_H_

#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <glib.h>
#include <gio/gio.h>


struct dirservconnection {
    char buf[1500];
    GSocketConnection *connection;
    GOutputStream *out;
    GInputStream *in;
};

struct service {
    char *json;
    uint32_t ttl;
};

void dirserver_init(gchar* baseaddress);

#endif
