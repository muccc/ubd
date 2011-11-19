#ifndef _DIRECTORY_CLIENT_H_
#define _DIRECTORY_CLIENT_H_
#include "groups.h"
#include "nodes.h"

void dirclient_init(void);
void dirclient_registerServices(struct node *n);
void dirclient_removeServices(struct node *n);
void dirclient_registerMulticastGroup(struct multicastgroup *g);
#endif
