#ifndef __SERIAL_H_
#define __SERIAL_H_
#include "message.h"

#define SERIAL_BUFFERLEN    100
#define SERIAL_ESCAPE   '\\'
#define SERIAL_START    '0'
#define SERIAL_END     '1'


uint16_t serial_in(uint8_t data);
gboolean serial_read(GIOChannel * serial, GIOCondition condition, gpointer data);
int serial_open(char * device, void (*cb)(struct message *));
void serial_writemessage(struct message *);
#endif
