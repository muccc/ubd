#ifndef _CMDPARSER_H_
#define _CMDPARSER_H_
#include <glib.h>
#include <gio/gio.h>
void cmdparser_init(void);
gssize cmdparser_cmd(gchar* cmd, gsize n, gchar** result);
gboolean cmdparser_cmdtostream(gchar *cmd, gint len, GOutputStream *out);
#endif
