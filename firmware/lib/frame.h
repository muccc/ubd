#ifndef __FRAME_H_
#define __FRAME_H_
#include <stdint.h>

#define FRAME_MAX 50

struct frame {
    uint8_t len;
    uint8_t data[FRAME_MAX];
    uint16_t crc;
    uint8_t isnew;
};

#endif
