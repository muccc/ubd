#ifndef _ADDRESS6DB_H_
#define _ADDRESS6DB_H_

void address6db_init(GInetAddress *base, GInetAddress *multicastbase);
GInetAddress* address6db_getFreeAddr(gchar *id);
GInetAddress* address6db_getMulticastAddr(gchar *groupname);

#endif
