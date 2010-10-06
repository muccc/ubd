#ifndef _CMDPARSER_H_
#define _CMDPARSER_H_
#include <glib.h>
#include <gio/gio.h>
#include "net_tcp.h"
void cmdparser_init(void);
gssize cmdparser_cmd(gchar* cmd, gsize n, gchar** result);
gboolean cmdparser_parse(struct nodebuffer *nb, gchar data);
#endif
