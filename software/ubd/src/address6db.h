#ifndef _ADDRESS6DB_H_
#define _ADDRESS6DB_H_

void address6db_init(GInetAddress *base);
GInetAddress* address6db_getFreeAddr(gchar *id);
#endif
