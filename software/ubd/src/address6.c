#include <glib.h>
#include <gio/gio.h>
#include <stdio.h>

#include "address6.h"
#include "address6db.h"
#include "net6.h"
#include "interface.h"
#include "mgt.h"

void address6_init(GInetAddress *base)
{
    address6db_init(base);
}

/*
 * Tries to add a new address to the interface.
 * If there is already an address for this ID
 * no new address will be created.
 * Parameter: id
 * Returns: 0 on success, -1 on error
*/

void address6_createAddress(struct node *n)
{
    printf("adding address for %s\n",n->id);
    if( n->netadr == NULL ){
        n->netadr = address6db_getFreeAddr(n->id);
    }
    interface_createAddress(n);
}

void address6_removeAddress(struct node *n)
{
   interface_removeAddress(n);
}

