#ifndef _MSG_H_
#define _MSG_H_

#include "frame.h"

struct message{
    uint16_t    len;
    uint8_t     data[FRAME_MAX];
};


#endif
