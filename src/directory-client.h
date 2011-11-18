#ifndef _DIRECTORY_CLIENT_H_
#define _DIRECTORY_CLIENT_H_

void dirclient_init(void);
void dirclient_registerServices(struct node *n);
void dirclient_removeServices(struct node *n);
#endif
