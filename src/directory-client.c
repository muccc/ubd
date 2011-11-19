#include "directory-server.h"
#include <glib.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <gio/gio.h>
#include <syslog.h>
#include <json/json.h>

#include "groups.h"
#include "debug.h"
#include "bus.h"
#include "classes.h"
#include "net_multicast.h"
#include "config.h"
#include "nodes.h"

static gboolean dirclient_tick(gpointer data);

static GHashTable *services;

static GSocketAddress *sa;
static GSocket *dirclientsocket;

void dirclient_init(void)
{
    g_timeout_add_seconds(15,dirclient_tick,NULL);
    //services = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);
    services = g_hash_table_new_full(g_direct_hash, g_direct_equal, NULL, g_free);

    dirclientsocket = multicast_createSocket("directoryserver", 2323, &sa);
    if( socket != NULL){
        syslog(LOG_DEBUG,"dirclient_init: socket open");
    }else{
        syslog(LOG_WARNING,
                "directory-server.c: warning: could not create socket");
    }
}

void dirclient_addService(struct node *n, guint classid)
{
    guint class = n->classes[classid];
    struct socketdata *sd = &(n->tcpsockets[classid]);
    gchar *addr = g_inet_address_to_string(n->netadr);

    json_object *cmd = json_object_new_string("update-service");
    
    json_object *service_type = json_object_new_string(classes_getClassName(class));
    json_object *url = json_object_new_string(addr);
    json_object *id = json_object_new_string(n->id);
    json_object *name = json_object_new_string(n->name);
    json_object *port = json_object_new_int(classes_getServicePort(class));
    json_object *udpproto = json_object_new_boolean(1);
    json_object *tcpproto = json_object_new_boolean(1);
    json_object *multicast = json_object_new_boolean(0);

    json_object *json = json_object_new_object();
    json_object_object_add(json,"cmd", cmd);
    json_object_object_add(json,"name", name);
    json_object_object_add(json,"id", id);
    json_object_object_add(json,"url", url);
    json_object_object_add(json,"port", port);
    json_object_object_add(json,"service-type", service_type);
    json_object_object_add(json,"tcp", tcpproto);
    json_object_object_add(json,"udp", udpproto);
    json_object_object_add(json,"multicast", multicast);
    
    const char *jsons = json_object_to_json_string(json);
    syslog(LOG_DEBUG,"sending json: %s", jsons);
    g_socket_send_to(dirclientsocket, sa, jsons, strlen(jsons), NULL, NULL);
    g_hash_table_insert(services, sd, g_strdup(jsons));
    
    json_object_put(json);
    g_free(addr);
}

void dirclient_removeService(struct node *n, guint classid)
{
    guint class = n->classes[classid];
    struct socketdata *sd = &(n->tcpsockets[classid]);

    json_object *cmd = json_object_new_string("delete-service");
    
    json_object *service_type = json_object_new_string(classes_getClassName(class));
    json_object *id = json_object_new_string(n->id);

    json_object *json = json_object_new_object();
    json_object_object_add(json,"cmd", cmd);
    json_object_object_add(json,"id", id);
    json_object_object_add(json,"service-type", service_type);
    
    const char *jsons = json_object_to_json_string(json);
    syslog(LOG_DEBUG,"sending json: %s", jsons);
    g_socket_send_to(dirclientsocket, sa, jsons, strlen(jsons), NULL, NULL);
    json_object_put(json);

    g_hash_table_remove(services, sd);
}

void dirclient_registerServices(struct node *n)
{
    guint i;
    for(i=0; i<sizeof(n->classes); i++){
        if( n->classes[i] != 0 ){
            syslog(LOG_DEBUG,"adding service %d\n", n->classes[i]);
            dirclient_addService(n, i);
        }
    }
}

void dirclient_removeServices(struct node *n)
{
    guint i;
    for(i=0; i<sizeof(n->classes); i++){
        if( n->classes[i] != 0 ){
            syslog(LOG_DEBUG,"removing service %d\n", n->classes[i]);
            dirclient_removeService(n, i);
        }
    }
}

void dirclient_registerMulticastGroup(struct multicastgroup *g)
{
    printf("dirclient_registerMulticastGroup()\n");
    
    char *address = g_inet_address_to_string(
                    g_inet_socket_address_get_address(
                    (GInetSocketAddress*)g->sa));
    guint class = g->class;

    json_object *cmd = json_object_new_string("update-service");
    
    json_object *service_type = json_object_new_string(classes_getClassName(class));
    json_object *url = json_object_new_string(address);
    json_object *id = json_object_new_string(g->name);
    json_object *name = json_object_new_string(g->name);
    json_object *port = json_object_new_int(classes_getServicePort(class));
    json_object *udpproto = json_object_new_boolean(1);
    json_object *tcpproto = json_object_new_boolean(0);
    json_object *multicast = json_object_new_boolean(1);

    json_object *json = json_object_new_object();
    json_object_object_add(json,"cmd", cmd);
    json_object_object_add(json,"name", name);
    json_object_object_add(json,"id", id);
    json_object_object_add(json,"url", url);
    json_object_object_add(json,"port", port);
    json_object_object_add(json,"service-type", service_type);
    json_object_object_add(json,"tcp", tcpproto);
    json_object_object_add(json,"udp", udpproto);
    json_object_object_add(json,"multicast", multicast);
    
    const char *jsons = json_object_to_json_string(json);
    syslog(LOG_DEBUG,"sending json: %s", jsons);
    g_socket_send_to(dirclientsocket, sa, jsons, strlen(jsons), NULL, NULL);
    g_hash_table_insert(services, g, g_strdup(jsons));
    
    json_object_put(json);

    g_free(address);
}

static gboolean dirclient_tick(gpointer data)
{
    data = NULL;
    GHashTableIter iter;
    struct socketdata *id;
    char *service;

    g_hash_table_iter_init (&iter, services);
    //syslog(LOG_DEBUG,"dirserver_tick: decrement");
    while( g_hash_table_iter_next (&iter, (void**)&id, (void**)&service) ){
        //syslog(LOG_DEBUG,"dirserver_tick: decrementing %s", id);
        g_socket_send_to(dirclientsocket, sa, service, strlen(service), NULL, NULL);
    }


    return TRUE;
}

