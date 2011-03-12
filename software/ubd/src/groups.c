#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <gio/gio.h>
#include "groups.h"
#include "debug.h"
#include "bus.h"
#include "classes.h"
#include "net_multicast.h"

struct multicastgroup groups[MAX_GROUPS];
gint groupcount = 0;

gboolean groups_read(GSocket *socket, GIOCondition condition,
                                        gpointer user_data)
{
    uint8_t buf[100];
    GSocketAddress * src;
    struct multicastgroup *g = user_data;
    gssize len;    
    printf("foobar\n");
    if( condition == G_IO_IN ){
        len = g_socket_receive_from(socket,&src,(gchar*)buf,
                                sizeof(buf),NULL,NULL);
        if( len > 0 ){
            printf("group_read: Received:");
            debug_hexdump(buf,len);
            printf("\n");
            bus_sendToAddress(g->busadr, buf, len, g->class, FALSE);
        }else{
            printf("group_read: Error while receiving: len=%d\n",len);
        }
    }else{
        printf("group_read: Received ");
        if( condition == G_IO_ERR ){
            printf("G_IO_ERR\n");
        }else if( condition == G_IO_HUP ){ 
            printf("G_IO_HUP\n");
        }else if( condition == G_IO_OUT ){ 
            printf("G_IO_OUT\n");
        }else if( condition == G_IO_PRI ){ 
            printf("G_IO_PRI\n");
        }else if( condition == G_IO_NVAL ){ 
            printf("G_IO_NVAL\ndropping source\n");
            return FALSE;
        }else{
            printf("unkown condition = %d\n",condition);
        }
    }
    return TRUE;
}

void groups_init(void)
{
    gint i;
    for(i=0;i<MAX_GROUPS;i++){
        groups[i].name = NULL;
    }
}

void groups_addGroup(gchar *groupname, gchar *classname)
{
    if( groups_getGroupByName(groupname) != NULL ){
        printf("groups.c: warning: group already exists\n");
        return;
    }

    if( groupcount < MAX_GROUPS ){
        int g = groupcount;
        guint class = classes_getClassByName(classname)->class;
        guint udpport = classes_getClassByName(classname)->serviceport;
        GSocketAddress *sa;
        GSocket *socket = multicast_createSocket(groupname, udpport, &sa);
        if( socket != NULL){
            groups[g].name = g_strdup(groupname);
            groups[g].hostname[0] = 0;
	    strcat(groups[g].hostname,groupname);
	    strcat(groups[g].hostname,".local");
            groups[g].busadr = g + GROUPS_STARTADDRESS;
            groups[g].class = class;
            GSource *source = g_socket_create_source(socket,
                                G_IO_IN, NULL);
            g_assert(source != NULL);
            g_source_set_callback(source, (GSourceFunc)groups_read,
                                  &groups[g], NULL);
            g_source_attach(source, g_main_context_default());
            groups[g].socket = socket;
            groups[g].source = source;
            groups[g].sa = sa;
            groupcount++;
            printf("groups.c: added new group %s as %d\n", groupname,
                    groups[g].busadr);

            avahi_registerMulticastGroup(&groups[g]);
        }else{
            //TODO:log error
            printf("groups.c: warning: could not create socket\n");
        }
    }else{
        //TODO: log error
        printf("groups.c: warning: reached MAX_GROUPS\n");
    }
}

gint groups_getBusAddress(gchar *groupname)
{
    struct multicastgroup *g = groups_getGroupByName(groupname);
    if( g != NULL ){
        return g->busadr;
    }
    return -1;
}

gint groups_getGroupCount(void)
{
    return groupcount;
}

struct multicastgroup *groups_getGroup(gint group)
{
    return &groups[group];
}


struct multicastgroup *groups_getGroupByBusAddress(gint busadr)
{
    int i;
    for(i=0; i<groups_getGroupCount(); i++){
        if( groups_getGroup(i)->busadr == busadr )
            return groups_getGroup(i);
    }
    return NULL;
}

struct multicastgroup *groups_getGroupByName(gchar *name)
{
    int i;
    for(i=0; i<groups_getGroupCount(); i++){
        if( strcmp(groups_getGroup(i)->name, name)==0 )
            return groups_getGroup(i);
    }
    return NULL;
}

