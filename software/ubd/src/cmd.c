#include <glib.h>
#include <string.h>
#include <stdio.h>
#include "mgt.h"

gchar *cmd_list_nodes(void)
{
    int i;
    int count = nodes_getNodeCount();
    gchar *result = malloc(300*count);
    gchar *pos = result;
    pos += sprintf(pos, "List of local nodes(%d):\n", count);
    for(i=0;i<count;i++){
        struct node *n = nodes_getNode(i);
        if( n->active ){
            pos += sprintf(pos,"id=\"%s\" version=\"%s\" busadr=%d "
                "name=\"%s\" domain=\"%s\"\n",
                n->id, n->version, n->busadr, n->name, n->domain);
        }
    }
    return result;
}

gchar *cmd_list_groups(void)
{
/*    int i;
    int count = mgt_getGroupCount();
    gchar *result = malloc(300*count);
    gchar *pos = result;
    pos += sprintf(pos, "List of groups(%d):\n", count);
    for(i=0;i<MAX_GROUP;i++){
        struct node *n = mgt_getNode(i);
        if( n->type != TYPE_NONE ){
            pos += sprintf(pos,"id=\"%s\" version=\"%s\" busadr=%d "
            "name=\"%s\" domain=\"%s\"\n", n->id, n->version, n->busadr, n->name, n->domain);
        }
    }
    return result;
*/
    return NULL;
}
