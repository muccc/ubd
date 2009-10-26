#ifndef _MSG_H_
#define _MSG_H_

#include "ubconfig.h"

struct message{
    uint16_t    len;
    uint8_t     data[UB_PACKETLEN];
};


#endif
