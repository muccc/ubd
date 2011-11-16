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

static GSocketAddress *sa;
static GSocket *dirserversocket;
static gboolean dirserver_read(GSocket *socket, GIOCondition condition,
                                        gpointer user_data);

static void dirserver_announce(const char *service_type,
                               const char *protocol,
                               const gboolean local_only);
static enum commandlist dirserver_parseCommand(const char *cmd);

enum commandlist {
NO_COMMAND, DISCOVER_DIRECTORY, UPDATE_SERVICE
};

struct command {
    char *command;
    enum commandlist id;
};

struct command commands[] = 
{
{"discover-directory", DISCOVER_DIRECTORY},
{"update-service", UPDATE_SERVICE}
};

#define COMMAND_COUNT (sizeof(commands)/sizeof(struct command))

void dirserver_init(void)
{
    syslog(LOG_DEBUG,"dirserver_init: starting directory server");
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
}

static const char *dirserver_getJsonString(struct json_object *json, const char *field, const char *dflt)
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

static gboolean dirserver_getJsonBool(struct json_object *json, const char *field, const gboolean dflt)
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

//Make sure data is sanatized
static void dirserver_newMCData(const char *data)
{
    struct json_object *json_tmp;

    struct json_object *json = json_tokener_parse(data);
    ub_assert(json != NULL);
    if( is_error(json) )
        return;
    
    json_tmp = json_object_object_get(json, "cmd");
    if( json_tmp == NULL || is_error(json_tmp) )
        return;
    if( json_object_get_type(json_tmp) != json_type_string )
        return;
    
    enum commandlist cmd = dirserver_parseCommand(
            json_object_get_string(json_tmp));
    
    switch( cmd ){
        case DISCOVER_DIRECTORY:
            dirserver_announce(dirserver_getJsonString(json, "service-type",NULL),
                               dirserver_getJsonString(json, "protocol",NULL),
                               dirserver_getJsonBool(json, "local-only",FALSE));
            break;
            default:
            break;
    };

    json_object_put(json);
}

static void dirserver_announce(const char *service_type,
                               const char *protocol,
                               const gboolean local_only)
{
    printf("announcing");    
    if( service_type ) printf(" service-type=%s", service_type);
    if( protocol ) printf(" protocol=%s", protocol);
    printf(" local_only=%s\n", local_only?"true":"false");
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
    GSocketAddress *src;
    user_data = NULL;
    gssize len;    
    if( condition == G_IO_IN ){
        len = g_socket_receive_from(socket,&src,(gchar*)buf,
                                sizeof(buf),NULL,NULL);
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


