#ifndef __NET6_H__
#define __NET6_H__
#include <gnet.h>

gint net_addAddressForID(gchar * id);
gint net_removeAddressForID(gchar * id);
gint net_init(gchar* interface, gchar* baseaddress, gint prefix);
#endif
