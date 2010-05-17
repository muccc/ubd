#ifndef _CMDPARSER_H_
#define _CMDPARSER_H_
#include <glib.h>
void cmdparser_init(void);
gssize cmdparser_cmd(gchar* cmd, gsize n, gchar** result);
#endif
