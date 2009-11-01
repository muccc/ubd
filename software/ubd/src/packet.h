#ifndef _PACKET_H_
#define _PACKET_H_
#include <stdint.h>
#include <glib.h>
#include "ubpacket.h"
#include "message.h"

#define PACKET_PACKET   'P'
#define PACKET_DONE     'S'
#define PACKET_ABORT    'A'

struct queues{
    GAsyncQueue * packet_in;
    GAsyncQueue * status_in;
    GAsyncQueue * packet_out;
    GAsyncQueue * status_out;
};
extern struct queues packet_queues;

void packet_init(void);
void packet_inmessage(struct message*);
void packet_addCallback(gchar key, void(*cb)(struct ubpacket*));
void packet_outpacket(struct ubpacket* p);
#endif
