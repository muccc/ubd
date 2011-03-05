#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include "groups.h"

gchar *groups[MAX_GROUPS];
gint groupcount = 0;

void groups_init(void)
{
    gint i;
    for(i=0;i<MAX_GROUPS;i++){
        groups[i] = NULL;
    }

}

void groups_addGroup(gchar *groupname)
{
    if( groupcount < MAX_GROUPS ){
        groups[groupcount++] = g_strdup(groupname);
        printf("groups.c: added new group %s as %d\n",
                    groupname,groupcount-1);
    }else{
        printf("groups.c: warning: reached MAX_GROUPS\n");
    }
}

gint groups_getGroupNumber(gchar *groupname)
{
    gint i;
    for(i=0;i<MAX_GROUPS;i++){
        if( groups[i] && strcmp(groupname,groups[i])==0 ){
            return i;
        }
    }
    return -1;
}

gint groups_getGroupCount(void)
{
    return groupcount;
}

gchar* groups_getGroup(gint group)
{
    return groups[group];
}

