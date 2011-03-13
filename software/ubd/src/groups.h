#ifndef _GROUPS_H_
#define _GROUPS_H_
#include <glib.h>
#include <gio/gio.h>
#include "avahi.h"

//multicast addresses on the bus range from 0x80 to 0xFF-1
#define MAX_GROUPS (0xFF-0x80)
#define GROUPS_STARTADDRESS 0x80

//only one service is allowed per group!
struct multicastgroup{
    gchar           *name;
    gchar           *hostname;
    guint           class;
    GSocket         *socket;
    GSocketAddress  *sa;
    GSource         *source;
    //the multicast address on the bus
    gint            busadr;
    AvahiEntryGroup *avahientrygroup;
    char            *avahiservicename;
};

void groups_init(void);
void groups_addGroup(gchar *groupname, gchar *hostname, gchar *classname);
gint groups_getBusAddress(gchar *groupname);
gint groups_getGroupCount(void);
struct multicastgroup *groups_getGroup(gint group);
struct multicastgroup *groups_getGroupByBusAddress(gint busaddr);
struct multicastgroup *groups_getGroupByName(gchar *name);

#endif
