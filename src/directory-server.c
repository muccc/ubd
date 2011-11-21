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

static GSocketAddress *sa;
static GSocket *dirserversocket;
static gboolean dirserver_read(GSocket *socket, GIOCondition condition,
                                        gpointer user_data);

static void dirserver_announce(const char *service_type,
                               const char *protocol,
                               const gboolean local_only);
static void dirserver_announceUpdate(void);

static void dirserver_updateServiceCmd(struct json_object *json);
static void dirserver_deleteServiceCmd(struct json_object *json);
static void dirserver_deleteService(const char *id);

static enum commandlist dirserver_parseCommand(const char *cmd);
static gboolean dirserver_tcp_listener(GSocketService    *service,
                        GSocketConnection *connection,
                        GObject           *source_object,
                        gpointer           user_data);
static gboolean dirserver_tick(gpointer data);
enum commandlist {
NO_COMMAND, DISCOVER_DIRECTORY, UPDATE_SERVICE, DELETE_SERVICE
};

struct command {
    char *command;
    enum commandlist id;
};

struct command commands[] = 
{
{"discover-directory", DISCOVER_DIRECTORY},
{"update-service", UPDATE_SERVICE},
{"delete-service", DELETE_SERVICE}
};

#define COMMAND_COUNT (sizeof(commands)/sizeof(struct command))

static GHashTable *services;

void dirserver_init(gchar* baseaddress)
{
    syslog(LOG_DEBUG,"dirserver_init: starting directory server");
    services = g_hash_table_new_full(g_str_hash, g_str_equal, g_free, g_free);

    dirserversocket = multicast_createSocket("directoryserver", 2323, &sa);
    if( socket != NULL){
        syslog(LOG_DEBUG,"dirserver_init: socket open");
        GSource *source = g_socket_create_source(dirserversocket,
                            G_IO_IN, NULL);
        ub_assert(source != NULL);
        g_source_set_callback(source, (GSourceFunc)dirserver_read,
                                NULL, NULL);
        g_source_attach(source, g_main_context_default());

    }else{
        syslog(LOG_WARNING,
                "directory-server.c: warning: could not create socket");
    }
    GError * e = NULL;

    GInetAddress *net_base = g_inet_address_new_from_string(baseaddress);
    if( net_base == NULL ){
        syslog(LOG_ERR, "dirserver_init: Could not parse base address");
        return;
    }

    //set up http tcp listener
    GSocketAddress *httpsa = g_inet_socket_address_new(net_base,8080);
    syslog(LOG_DEBUG,"dirserver_init: Creating tcp socket on port 8080\n");
    GSocketService *gss = g_socket_service_new();
    
    //TODO: save a reference to the gss somewhere
    if( g_socket_listener_add_address(G_SOCKET_LISTENER(gss), httpsa,
        G_SOCKET_TYPE_STREAM, G_SOCKET_PROTOCOL_TCP, NULL, NULL, &e)
            == FALSE ){
        syslog(LOG_WARNING,
            "dirserver_init: error while creating socket listener: %s\n",e->message);
        g_error_free(e);
    }
    g_signal_connect(gss, "incoming", G_CALLBACK(dirserver_tcp_listener),NULL);
    g_socket_service_start(gss); 

    g_timeout_add_seconds(1,dirserver_tick,NULL);
}

static const char *dirserver_getJsonString(struct json_object *json,
                                    const char *field, const char *dflt)
{
    struct json_object *json_tmp;
    json_tmp = json_object_object_get(json, field);
    if( json_tmp != NULL &&
        !is_error(json_tmp) &&
        json_object_get_type(json_tmp) == json_type_string ){
        return json_object_get_string(json_tmp);
    }
    return dflt;
}

static gboolean dirserver_getJsonBool(struct json_object *json,
                                    const char *field, const gboolean dflt)
{
    struct json_object *json_tmp;
    json_tmp = json_object_object_get(json, field);
    if( json_tmp != NULL &&
        !is_error(json_tmp) &&
        json_object_get_type(json_tmp) == json_type_boolean ){
        return json_object_get_boolean(json_tmp)?TRUE:FALSE;
    }
    return dflt;
}

static int32_t dirserver_getJsonInt(struct json_object *json,
                                    const char *field, const int32_t dflt)
{
    struct json_object *json_tmp;
    json_tmp = json_object_object_get(json, field);
    if( json_tmp != NULL &&
        !is_error(json_tmp) &&
        json_object_get_type(json_tmp) == json_type_int ){
        return json_object_get_int(json_tmp);
    }
    return dflt;
}

//Make sure data is sanatized
static void dirserver_newMCData(const char *data)
{
    struct json_object *json_tmp;
    struct json_object *json = json_tokener_parse(data);

    ub_assert(json != NULL);
    if( is_error(json) ){
        syslog(LOG_DEBUG,"dirserver_newMCData: invalid json");
        return;
    }
    json_tmp = json_object_object_get(json, "cmd");
    if( json_tmp == NULL || is_error(json_tmp) ){
        syslog(LOG_DEBUG,"dirserver_newMCData: no cmd field in json");
        return;
    }
    if( json_object_get_type(json_tmp) != json_type_string ){
        syslog(LOG_DEBUG,"dirserver_newMCData: no cmd field is not a string");
        return;
    }

    enum commandlist cmd = dirserver_parseCommand(
            json_object_get_string(json_tmp));
    json_object_object_del(json, "cmd");

    switch( cmd ){
        case DISCOVER_DIRECTORY:
            dirserver_announce(dirserver_getJsonString(json, "service-type",NULL),
                               dirserver_getJsonString(json, "protocol",NULL),
                               dirserver_getJsonBool(json, "local-only",FALSE));
            break;
        case UPDATE_SERVICE:
            dirserver_updateServiceCmd(json);
            break;
        case DELETE_SERVICE:
            dirserver_deleteServiceCmd(json);
        case NO_COMMAND:
            break;
    };

    json_object_put(json);
}

static void dirserver_deleteService(const char *key)
{
    struct service *service = g_hash_table_lookup(services, key);
    if( service == NULL ){
        syslog(LOG_DEBUG,"dirserver_deleteService: id unknown");
        return;
    }
    syslog(LOG_DEBUG,"dirserver_deleteService: deleting %s", key);
    g_free(service->json);
    g_hash_table_remove(services, key);
}

static void dirserver_deleteServiceCmd(struct json_object *json)
{
    const char *id = dirserver_getJsonString(json,"id", NULL);
    const char *service_type = dirserver_getJsonString(json,"service-type", NULL);
    if( id == NULL ) syslog(LOG_DEBUG,"dirserver_deleteService: invalid id");
    if( service_type == NULL ) syslog(LOG_DEBUG,"dirserver_deleteService: invalid service-type");
    
    if( id == NULL || service_type == NULL)
        return;
    char *key = g_strdup_printf("%s%s", id, service_type);
    dirserver_deleteService(key);
    g_free(key);
}

static void dirserver_updateServiceCmd(struct json_object *json)
{
    const char *service_type = dirserver_getJsonString(json,"service-type", NULL);
    const char *url = dirserver_getJsonString(json,"url", NULL);
    const char *id = dirserver_getJsonString(json,"id", NULL);
    const char *name = dirserver_getJsonString(json,"name", NULL);
    int32_t port = dirserver_getJsonInt(json,"port", 0);

    if( service_type == NULL ) syslog(LOG_DEBUG,"dirserver_updateService: invalid service-type");
    if( url == NULL ) syslog(LOG_DEBUG,"dirserver_updateService: invalid url");
    if( id == NULL ) syslog(LOG_DEBUG,"dirserver_updateService: invalid id");
    if( name == NULL ) syslog(LOG_DEBUG,"dirserver_updateService: invalid name");
    if( port < 1 || port > 65635) syslog(LOG_DEBUG,"dirserver_updateService: invalid port");
    if( service_type == NULL || url == NULL || id == NULL
        || name == NULL || port < 1 || port > 65635 ){
        syslog(LOG_DEBUG,"dirserver_updateService: invalid fields");
        return;
    }

    char *key = g_strdup_printf("%s%s", id, service_type);
    syslog(LOG_DEBUG, "update for service %s", key);

    struct service *service = g_hash_table_lookup(services, key);
    if( service == NULL ){
        syslog(LOG_DEBUG,"dirserver_updateService: adding new service");
        service = g_new0(struct service,1);
        service->json = g_strdup(json_object_to_json_string(json));
        service->ttl = config.dirttl;
        g_hash_table_insert(services, g_strdup(key), service);
        dirserver_announceUpdate();
    }else{
        service->ttl = config.dirttl; 
        if(strcmp(service->json, json_object_to_json_string(json)) != 0 ){
            syslog(LOG_DEBUG,"dirserver_updateService: updating service");
            syslog(LOG_DEBUG,"old=%s new=%s",
                    service->json, json_object_to_json_string(json));
            g_free(service->json);
            service->json = g_strdup(json_object_to_json_string(json));
            dirserver_announceUpdate();
        }
    }
    g_free(key);
}

static void dirserver_announce(const char *service_type,
                               const char *protocol,
                               const gboolean local_only)
{
    char *response = g_strdup_printf(
        "{\"cmd\": \"directory\", \"url\": \"%s\" \"port\": %d }",
        config.base, config.dirserverport);
    g_socket_send_to(dirserversocket, sa, response, strlen(response), NULL, NULL);
    g_free(response);
}

static void dirserver_announceUpdate(void)
{
    syslog(LOG_DEBUG,"dirserver_announceUpdate()");
    char *response = g_strdup_printf(
        "{\"cmd\": \"updated-service\", \"url\": \"%s\" \"port\": %d }",
        config.base, config.dirserverport);
    g_socket_send_to(dirserversocket, sa, response, strlen(response), NULL, NULL);
    g_free(response);
}

static enum commandlist dirserver_parseCommand(const char *cmd)
{
    for(uint32_t i=0; i<COMMAND_COUNT; i++){
        if( strcmp(cmd, commands[i].command) == 0 )
            return commands[i].id;
    }
    return NO_COMMAND;
}


static gboolean dirserver_read(GSocket *socket, GIOCondition condition,
                                        gpointer user_data)
{
    uint8_t buf[1500];
    user_data = NULL;
    gssize len;    
    if( condition == G_IO_IN ){
        len = g_socket_receive_from(socket,NULL,(gchar*)buf,
                                sizeof(buf)-1,NULL,NULL);
        if( len > 0 ){
            syslog(LOG_DEBUG,"dirserver_read: Received:");
            debug_hexdump(buf,len);
            
            buf[len] = 0;
            //validate(buf, len);
            dirserver_newMCData((char*)buf);
        }else{
            syslog(LOG_WARNING,
                "dirserver_read: Error while receiving: len=%d",len);
        }
    }else{
        syslog(LOG_DEBUG,"dirserver_read: Received ");
        if( condition == G_IO_ERR ){
            syslog(LOG_DEBUG,"G_IO_ERR");
        }else if( condition == G_IO_HUP ){ 
            syslog(LOG_DEBUG,"G_IO_HUP");
        }else if( condition == G_IO_OUT ){ 
            syslog(LOG_DEBUG,"G_IO_OUT");
        }else if( condition == G_IO_PRI ){ 
            syslog(LOG_DEBUG,"G_IO_PRI");
        }else if( condition == G_IO_NVAL ){ 
            syslog(LOG_DEBUG,"G_IO_NVAL - dropping source");
            return FALSE;
        }else{
            syslog(LOG_DEBUG,"unkown condition = %d",condition);
        }
    }
    return TRUE;
}

static inline void writeString(GOutputStream *out, char *s)
{
    g_output_stream_write(out, s, strlen(s), NULL, NULL);
}

static void writeServices(GOutputStream *out)
{
    writeString(out,"{");
    gboolean first = TRUE;

    GHashTableIter iter;
    char *id;
    struct service *service;

    g_hash_table_iter_init (&iter, services);
    while( g_hash_table_iter_next (&iter, (void**)&id, (void**)&service) ){
        if( !first)
            writeString(out,",");
        first = FALSE;
        writeString(out,"\"service\":");
        writeString(out,service->json);
    }
    writeString(out,"}");
}

static void dirserver_finish(GOutputStream *out,
                            GAsyncResult *res, GSocketConnection *connection){
    res = NULL; out = NULL; connection=NULL;
    //XXX: No unref needed?
    //g_object_unref(connection);
}

#if 0
void dirserver_tcp_listener_read(GInputStream *in, GAsyncResult *res,
                            struct dirservconnection *dsconnection)
{
    GError * e = NULL;
    gssize len = g_input_stream_read_finish(in, res, &e);
    if( len > 0 ){
        syslog(LOG_DEBUG,"tcp_listener_read: Received %d data bytes\n", len);
        //data in dsconnection->buf
        //TODO: not sure if this is a clean way to close
        //the tcp session
        g_object_unref(dsconnection->connection);
        g_free(dsconnection);
        return;
        //keep the stream open
        g_input_stream_read_async(in, dsconnection->buf, sizeof(dsconnection->buf),
            G_PRIORITY_DEFAULT, NULL,
            (GAsyncReadyCallback) dirserver_tcp_listener_read, dsconnection); 
    }else if( len == 0){
        syslog(LOG_DEBUG,"tcp_listener_read: connection closed\n");
        g_object_unref(dsconnection->connection);
        g_free(dsconnection);
    }else{
        syslog(LOG_WARNING,"tcp_listener_read: received an error\n");
    }
}
#endif

static gboolean dirserver_tcp_listener(GSocketService    *service,
                        GSocketConnection *connection,
                        GObject           *source_object,
                        gpointer           user_data){
    service=NULL;
    source_object = NULL;
    user_data = NULL;
    //syslog(LOG_DEBUG,"new listener\n");

#if 0    
    struct dirservconnection *dsconnection = g_new0(struct dirservconnection,1);
    ub_assert(dsconnection != NULL);
    dsconnection->connection = connection; 
    dsconnection->out = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    dsconnection->in = g_io_stream_get_input_stream(G_IO_STREAM(connection));
#endif
    g_tcp_connection_set_graceful_disconnect(G_TCP_CONNECTION(connection), TRUE);
    GOutputStream *out = g_io_stream_get_output_stream(G_IO_STREAM(connection));
    char *msg = "HTTP/1.0 200 OK\r\n\
Server: ubd/0.1\r\n\
Connection: close\r\n\
Content-Type: application/json\r\n\r\n";
    g_output_stream_write(out, msg, strlen(msg),
                        NULL, NULL);
    writeServices(out);
#if 0
    g_input_stream_read_async(dsconnection->in, dsconnection->buf,
            sizeof(dsconnection->buf), G_PRIORITY_DEFAULT, NULL,
            (GAsyncReadyCallback)dirserver_tcp_listener_read, dsconnection);
    g_object_ref(connection);
#endif
    //g_object_ref(connection);
    g_output_stream_close_async(out,
                                0, NULL,
                                (GAsyncReadyCallback)dirserver_finish, connection);
    return FALSE;
}


static gboolean dirserver_tick(gpointer data)
{
    data = NULL;
    GHashTableIter iter;
    char *id;
    struct service *service;
    g_hash_table_iter_init (&iter, services);
    //syslog(LOG_DEBUG,"dirserver_tick: decrement");
    while( g_hash_table_iter_next (&iter, (void**)&id, (void**)&service) ){
        //syslog(LOG_DEBUG,"dirserver_tick: decrementing %s", id);
        if( service->ttl )
            service->ttl--;
    }

    //syslog(LOG_DEBUG,"dirserver_tick: delete");
    while( TRUE ){
        gboolean clean = TRUE;
        g_hash_table_iter_init (&iter, services);
        while( g_hash_table_iter_next (&iter, (void**)&id, (void**)&service) ){
            if( service->ttl == 0 ){
                dirserver_deleteService(id);
                clean = FALSE;
                break;
            }
        }
        if( clean == TRUE ){
            //syslog(LOG_DEBUG,"dirserver_tick: clean");
            break;
        }
    }
    return TRUE;
}

