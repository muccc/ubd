#include "classes.h"

struct classdata{
    guint class;
    gchar name[100];
    gchar tcpservicename[100];
    gchar udpservicename[100];
    guint serviceport;
};

//TODO: read from xml
struct classdata classes[] =
{
{0,"unknown service","_unknown._tcp","_unknown._udp",0},
{23,"moodlamp","_moodlamp._tcp","_moodlamp._udp",2323},
{42,"switch","_switch._tcp","_switch._udp",2342}
};

gboolean classes_exists(guint class)
{ 
    guint classcount = sizeof(classes)/sizeof(struct classdata);
    guint i;
    for(i=0; i<classcount; i++){
        if( classes[i].class == class ){
            return TRUE;
        }
    }
    return FALSE; 
}

static struct classdata *classes_getClass(guint class)
{
    guint classcount = sizeof(classes)/sizeof(struct classdata);
    guint i;
    for(i=0; i<classcount; i++){
        if( classes[i].class == class ){
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

