#include "classes.h"
#include <string.h>



//TODO: read from somewhere?
struct classdata classes[] =
{
{0,"unknown class","_unknown._tcp","_unknown._udp",2300},
{23,"moodlamp","_moodlamp._tcp","_moodlamp._udp",2323},
{42,"switch","_switch._tcp","_switch._udp",2342}
};
guint classcount = sizeof(classes)/sizeof(struct classdata);

gboolean classes_exists(guint class)
{ 
    guint i;
    for(i=0; i<classcount; i++){
        if( classes[i].class == class ){
            return TRUE;
        }
    }
    return FALSE; 
}

struct classdata *classes_getClass(guint class)
{
    guint i;
    for(i=0; i<classcount; i++){
        if( classes[i].class == class ){
            return &classes[i];
        }
    }
    return &classes[0];
}

struct classdata *classes_getClassByName(gchar *class)
{
    guint i;
    for(i=0; i<classcount; i++){
        if( strcmp(classes[i].name, class)==0 ){
            return &classes[i];
        }
    }
    return &classes[0];
}

gchar* classes_getClassName(guint class)
{
    return classes_getClass(class)->name;
}

gchar* classes_getTcpServiceName(guint class)
{
    return classes_getClass(class)->tcpservicename;
}

gchar* classes_getUdpServiceName(guint class)
{
    return classes_getClass(class)->udpservicename;
}

guint classes_getServicePort(guint class)
{
    return classes_getClass(class)->serviceport;
}

guint classes_getServicePortByName(gchar *class)
{
    return classes_getClassByName(class)->serviceport;
}

