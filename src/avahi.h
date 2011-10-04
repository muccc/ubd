#ifndef _AVAHI_H_
#define _AVAHI_H_
#include <config.h>
#include <stdio.h>
#include <glib.h>
#include <stdint.h>
#include <string.h>
#include <gio/gio.h>
#include "config.h"

#include <avahi-client/client.h>
#include <avahi-client/publish.h>
#include <avahi-common/error.h>
#include <avahi-common/timeval.h>
#include <avahi-glib/glib-watch.h>
#include <avahi-glib/glib-malloc.h>
#include "nodes.h"
//#include "groups.h"
struct multicastgroup;

void avahi_init(GMainLoop *mainloop);
void avahi_registerNode(struct node *n);
void avahi_removeNode(struct node *n);
void avahi_registerServices(struct node *n);
void avahi_removeServices(struct node *n);
void avahi_registerMulticastGroup(struct multicastgroup *g);
#endif
