#ifndef _GROUPS_H_
#define _GROUPS_H_
#include <glib.h>

#define MAX_GROUPS 255

void groups_init(void);
void groups_addGroup(gchar *groupname);
gint groups_getGroupNumber(gchar *groupname);
gint groups_getGroupCount(void);
gchar* groups_getGroup(gint group);

#endif
