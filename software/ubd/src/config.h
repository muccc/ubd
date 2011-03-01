#ifndef __CONFIG_H_
#define __CONFIG_H_
#include <glib.h>

struct ubconfig {
    gchar *interface;
    gchar *base;
    gchar *device;
    gint rate;
    gint nodetimeout;
};

extern struct ubconfig config;
#endif
