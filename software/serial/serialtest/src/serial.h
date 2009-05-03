#ifndef __SERIAL_H_
#define __SERIAL_H_
#include "frame.h"

#define SERIAL_BUFFERLEN    100
#define SERIAL_ESCAPE   '\\'
#define SERIAL_START    '0'
#define SERIAL_END     '1'

struct message{
    uint16_t    len;
    uint8_t     data[FRAME_MAX];
};

uint16_t serial_in(uint8_t data);
gboolean serial_read(GIOChannel * serial, GIOCondition condition, gpointer data);
int serial_open (char * device, void (*cb)(struct message *));

#endif
