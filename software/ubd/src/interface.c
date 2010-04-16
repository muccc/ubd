#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <glib.h>
#include <gio/gio.h>
#include "interface.h"
//this hold done requests for interfaces
GAsyncQueue * interface_donequeue;
gchar *interface_interface;

static gint interface_createInterface(GInetAddress *addr);
static gpointer interface_createThread(gpointer data);

void interface_init(gchar *interface)
{
    interface_donequeue = g_async_queue_new();
    interface_interface = interface;
}

void interface_pushAddress(GInetAddress *addr)
{
    g_thread_create(interface_createThread,addr,FALSE,NULL);
}

GInetAddress *interface_getConfiguredAddress(void)
{
    if( g_async_queue_length(interface_donequeue) > 0 )
        return g_async_queue_pop(interface_donequeue);
    return NULL;
}

static gpointer interface_createThread(gpointer addr)
{
    g_assert(interface_createInterface(addr) == 0);
    g_async_queue_push(interface_donequeue, addr);
    return NULL;
}


/*
 * creates an interface for the specified address.
 * return 0 on success.
 * This routine may block for a significant amount of time
 * as the os needs time to bring the interface up
*/

static gint interface_createInterface(GInetAddress *addr)
{
    char buf[1024];
    char *tmp = g_inet_address_to_string(addr);

    sprintf(buf,"ip addr del %s dev %s",
                            tmp, interface_interface);
    printf("shell: %s\n",buf);
    system(buf);
    //usleep(1000*1000*3); 
    sprintf(buf,"ip addr add %s dev %s",
                            tmp, interface_interface);

    printf("shell: %s\n",buf);
    int rc = system(buf);

    g_free(tmp);
    if( rc ){
        printf("%s\nerror: return value: %d\n",buf,rc);
        return -1;
    }
    usleep(1000*1000*3); 
    return 0;
}

//TODO: push into thread
void interface_removeAddress(GInetAddress *addr)
{
    char buf[1024];
    char *tmp = g_inet_address_to_string(addr);

    sprintf(buf,"ip addr del %s dev %s",
                            tmp, interface_interface);

    system(buf);
    g_free(tmp);
    return;
}
