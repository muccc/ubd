#include <glib.h>
#include <gio/gio.h>

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <syslog.h>

#include "ubnetd.h"
#include "interface.h"

gchar *interface;
/*
 * creates an interface for the specified address.
 * return 0 on success.
 * This routine may block for a significant amount of time
 * as the os needs time to bring the interface up
*/

static gint ubnetd_createInterface(GInetAddress *addr,
                                      gchar *interface)
{
    char buf[1024]; //enough
    char *tmp = g_inet_address_to_string(addr);
    int rc;

    sprintf(buf,"ip addr del %s dev %s",
                            tmp, interface);
    syslog(LOG_DEBUG,"ubnetd: interface_createInterface: system(\"%s\")\n",buf);
    rc = system(buf);
    //seems like this works instantly. no need to wait
    //usleep(1000*1000*3); 
    
    sprintf(buf,"ip addr add %s dev %s",
                            tmp, interface);

    syslog(LOG_DEBUG,"ubnetd: interface_createInterface: system(\"%s\")\n",buf);
    rc = system(buf);

    g_free(tmp);
    if( rc ){
        syslog(LOG_WARNING,"ubnetd: interface_createInterface: Error: system() return value: %d\n",rc);
        return rc;
    }

    //we have to wait before crating sockets on it
    syslog(LOG_DEBUG,"ubnetd: interface_createInterface: New interface created. Now sleeping for 3s!\n");
    usleep(3*1000*1000); 
    syslog(LOG_DEBUG,"ubnetd: interface_createInterface: Done\n");
    return 0;
}

static gint ubnetd_removeAddress(GInetAddress *addr,
                                      gchar *interface)
{
    char buf[1024];
    char *tmp = g_inet_address_to_string(addr);
    int rc;
    sprintf(buf,"ip addr del %s dev %s",
                            tmp, interface);
    g_free(tmp);
    syslog(LOG_DEBUG,"ubnetd: interface_removeAddress: system(\"%s\")\n",buf);
    rc = system(buf);
    if( rc ){
        syslog(LOG_WARNING,"ubnetd: interface_removeAddress: Error: system() return value: %d\n",rc);
    }
    return rc;
}

static gsize receive(GSocket *s, gchar *buf, gsize len)
{
    gsize in = 0;
    while( in < len ){
        gint rc = g_socket_receive
                    (s,buf+in,len-in,NULL,NULL);
        if( rc < 1 )
            return 0;
        in+=rc;
    }
    return in;
}

static gpointer connection_handler(gpointer p)
{
    gchar buf[17];
    GSocket *s = (GSocket*)p;
    g_socket_set_blocking(s, TRUE);
    gint len = receive(s, buf, 17);
    syslog(LOG_DEBUG,"ubnetd: connection_handler: Received %d bytes\n", len);
    if( len != 17 || (buf[0] != 'A' && buf[0] != 'D') ){
        g_socket_close(s, NULL);
        return NULL;
    }

    GInetAddress *a = g_inet_address_new_from_bytes(
            (const guint8 *)(buf+1), G_SOCKET_FAMILY_IPV6);
    g_assert(a != NULL);

    gchar cmd = buf[0];

    gint rc;
    if( cmd == 'A' ){
        rc = ubnetd_createInterface(a, interface);
    }else{
        rc = ubnetd_removeAddress(a, interface);
    }
    
    if( rc == 0 ){
        g_socket_send(s, "A", 1, NULL, NULL);
    }else{
        g_socket_send(s, "N", 1, NULL, NULL);
    }
    g_object_unref(a);

    g_socket_receive(s,buf,1,NULL,NULL);
    g_socket_close(s, NULL);
    return NULL;
}

int main(int argc, char *argv[])
{
    openlog("ubnetd",LOG_PID | LOG_PERROR ,LOG_DAEMON);
    if (!g_thread_supported ()) g_thread_init (NULL);
    g_type_init();
    GInetAddress *lo = g_inet_address_new_loopback(
                            G_SOCKET_FAMILY_IPV6);
    g_assert(lo);

    GSocketAddress *sa = g_inet_socket_address_new(
                            lo,
                            42);
    g_assert(sa);

    if( argc < 2){
        printf("Please specify an interface ");
        printf("to use.\n");
        return -1;
    }
    openlog("ubnetd",LOG_PID ,LOG_DAEMON);
    interface_init(argv[1]);
    interface = argv[1];
    GError * e = NULL;
    GSocket *ss = g_socket_new(G_SOCKET_FAMILY_IPV6,
                            G_SOCKET_TYPE_STREAM,
                            G_SOCKET_PROTOCOL_TCP,
                            &e);
    if( e != NULL ){
        syslog(LOG_WARNING,
            "Error while creating socket: %s\n", e->message);
        g_error_free(e);
        return -1;
    }
   
    g_socket_bind(ss, sa, TRUE, &e);
    if( e != NULL ){
        syslog(LOG_WARNING,
            "Error while binding socket to port: %s\n", e->message);
        g_error_free(e);
        return -1;
    }
    
    g_socket_listen(ss, &e);
     if( e != NULL ){
        syslog(LOG_WARNING,
            "Cannot listen on socket: %s\n", e->message);
        g_error_free(e);
        return -1;
    }

    while(TRUE){
        syslog(LOG_DEBUG,"Waiting for connections\n");
        GSocket *s = g_socket_accept(ss, NULL, &e);        
        if( e != NULL ){
            syslog(LOG_WARNING,
                "ubnetd: Error while accepting: %s\n", e->message);
            g_error_free(e);
            continue;
        }
        syslog(LOG_DEBUG,"ubnetd: New connection\n");
        g_thread_create(connection_handler, s, FALSE, NULL);
    }
}

