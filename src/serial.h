#ifndef __SERIAL_H_
#define __SERIAL_H_
#include "message.h"

#define SERIAL_BUFFERLEN    100
#define SERIAL_ESCAPE   '\\'
#define SERIAL_START    '1'
#define SERIAL_END     '2'


void serial_readMessage(struct message *);
int serial_open(char * device);
void serial_switch(void);
void serial_writeMessage(struct message *);
#endif
